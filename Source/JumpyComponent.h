/* Copyright (c) 2025, Christian Ahrens
 *
 * This file is part of Jumpy <https://github.com/ChristianAhrens/Jumpy>
 *
 * This tool is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This tool is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this tool; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#pragma once

#include <JuceHeader.h>

#include <FixedFontTextEditor.h>

#include "JumpyConfiguration.h"
#include "CustomTriggerButton.h"

namespace Jumpy
{

/**
 * fwd. Decls.
 */
class AboutComponent;
class MidiInputCallbackToStdFuncWrapper;

/**
 * @class JumpyComponent
 * @brief Central UI and logic component for MTC generation, routing, and trigger management.
 *
 * JumpyComponent is the operational heart of the application.  It owns and coordinates:
 * - A fixed-font timecode editor that displays the current `hh:mm:ss:ff` position.
 * - A play/pause button that starts a 10 ms `juce::Timer` whose callback advances the
 *   timecode by the appropriate number of frames and re-transmits the MTC message.
 * - A one-shot trigger button that immediately sends the current timecode.
 * - A 4×3 grid of CustomTriggerButton instances, each storing a preset timecode target.
 * - An OSC receiver on a configurable UDP port (default 53000) accepting:
 *   - `/Jumpy/TS hh:mm:ss:ff` — sets and transmits the specified timecode.
 *   - Per-button address patterns — activates the matching CustomTriggerButton.
 * - A MIDI output device that transmits MIDI Full Frame (MTC) SysEx messages.
 * - A MIDI input device that activates buttons via assigned MIDI commands.
 *
 * ## MTC output format
 * Each call to `sendMessage(TimeStamp, int)` constructs a MIDI Full Frame Message:
 * ```
 * F0 7F 7F 01 01  <hr>  <mn>  <sc>  <fr>  F7
 * ```
 * where `<hr>` packs the frame-rate index into bits 5–6 and the hours value into
 * bits 0–4, per the MIDI MTC specification.
 *
 * ## Timer and timecode advancement
 * The `juce::Timer` fires every 10 ms.  Each callback computes the elapsed wall-clock
 * time since `m_startMillisecondsHiRes` (which incorporates any timecode offset already
 * set in the editor) and derives the new `hh:mm:ss:ff` position.  The MTC message is
 * only re-sent when the computed TimeStamp differs from the previous one.
 *
 * ## Threading
 * MIDI input callbacks arrive on a dedicated JUCE MIDI thread.  Incoming messages are
 * stored in the mutex-protected `m_midiInputMessageQueue` and dispatched to the JUCE
 * message loop via `juce::MessageManager::callAsync`, where `midiMessageReceived()`
 * processes them on the main thread.
 *
 * ## Configuration persistence
 * The component implements `JumpyConfiguration::Dumper` (serialises current state to XML)
 * and `JumpyConfiguration::Watcher` (applies state loaded from XML).  Persisted data
 * includes the frame-rate index, OSC port, MIDI device identifiers, and TriggerDetails
 * for every configured button slot.
 */
class JumpyComponent :
    public juce::Component,
    public juce::Timer,
    public juce::OSCReceiver::Listener<juce::OSCReceiver::MessageLoopCallback>,
    public JumpyConfiguration::Dumper,
    public JumpyConfiguration::Watcher
{
public:
    /**
     * @enum JumpyOptionsOption
     * @brief Stable menu-item identifiers used in the options popup menu.
     *
     * Values are passed to `handleOptionsMenuResult()` after the user selects an item.
     * MIDI device entries are allocated dynamically starting at `MidiIODevices`; the
     * actual upper bound is determined at runtime from the number of available devices
     * (see `getInputDeviceOptionIdRangeEnd()` / `getOutputDeviceOptionIdRangeEnd()`).
     */
    enum JumpyOptionsOption
    {
        ResetConfig = 1,                              ///<  Resets the entire configuration to built-in defaults.
        LookAndFeel_First,                            ///<  First LookAndFeel submenu entry (alias for FollowHost).
        LookAndFeel_FollowHost = LookAndFeel_First,   ///<  Automatically mirrors the OS dark/light preference.
        LookAndFeel_Dark,                             ///<  Forces the dark palette.
        LookAndFeel_Light,                            ///<  Forces the light palette.
        LookAndFeel_Last = LookAndFeel_Light,         ///<  Last LookAndFeel submenu entry.
        OscPort,                                      ///<  Opens the OSC port configuration dialog.
        FrameRate,                                    ///<  Opens the MTC frame-rate selection dialog.
        MidiIODevices,                                ///<  Base ID for dynamically generated MIDI input/output device entries.
    };

public:
    JumpyComponent();
    ~JumpyComponent() override;

    //==============================================================================
    /** @brief Fills the background with the current theme background colour. */
    void paint(juce::Graphics&) override;
    /** @brief Lays out all child components within the available bounds. */
    void resized() override;
    /** @brief Reloads all SVG button icons with the current theme foreground colour. */
    void lookAndFeelChanged() override;

    //==============================================================================
    /**
     * @brief Sets the active timecode, resets the timer reference, and transmits an MTC message.
     *
     * Updates `m_ts`, recalculates `m_startMillisecondsHiRes` so the running timer
     * continues from the new position, and immediately calls `sendMessage()`.
     *
     * @param ts  New timecode position to apply and transmit.
     */
    void setAndSendTimeCode(TimeStamp ts);

    /**
     * @brief (Re-)creates the OSC receiver and binds it to the configured UDP port.
     *
     * Tears down any existing `m_oscServer` before opening a new one on
     * `m_oscPortNumber`.  Shows a warning dialog if the port cannot be bound.
     */
    void connectToOscSocket();

    //==============================================================================
    /**
     * @brief Handles incoming OSC messages on the JUCE message thread.
     *
     * Accepts two address families:
     * - `/Jumpy/TS <string>` — parses the string as a timecode and calls `setAndSendTimeCode()`.
     * - Per-button patterns — activates the first matching CustomTriggerButton.
     *
     * @param message  The received OSC message.
     */
    void oscMessageReceived(const juce::OSCMessage& message) override;

    //==============================================================================
    /**
     * @brief Processes queued MIDI input messages on the JUCE message thread.
     *
     * Drains `m_midiInputMessageQueue` (populated by the MIDI thread) and compares
     * each message against the MIDI command assignment of every configured button.
     * A matching message calls `setAndSendTimeCode()` with the button's stored timecode.
     */
    void midiMessageReceived();

    //==========================================================================
    /** @brief Serialises current device settings and trigger configurations to the XML config. */
    void performConfigurationDump() override;
    /** @brief Applies device settings and trigger configurations loaded from the XML config. */
    void onConfigUpdated() override;

    //========================================================================*
    /** Invoked when the user changes the LookAndFeel preference from the options menu.
     *  Receives the new palette style ID and a flag indicating whether the OS
     *  preference should be followed automatically. */
    std::function<void(int, bool)> onPaletteStyleChange;

private:
    //==============================================================================
    /**
     * @brief Advances the running timecode and sends an MTC message when the frame changes.
     *
     * Fires every 10 ms while the play button is toggled on.  Computes elapsed
     * wall-clock time from `m_startMillisecondsHiRes` to derive the current
     * `hh:mm:ss:ff` position and calls `setAndSendTimeCode()` only when the frame
     * value has changed since the last tick.
     */
    void timerCallback() override;

    //==============================================================================
    /**
     * @brief Stores the current hi-res time minus the current timecode offset as the timer origin.
     *
     * After calling this, elapsed time from `m_startMillisecondsHiRes` maps directly
     * to the running timecode position starting from `m_ts`.
     */
    void setStartMilliseconds();

    //==============================================================================
    /** @brief Dispatches an options menu selection to the appropriate handler. @param selectedId Menu item ID. */
    void handleOptionsMenuResult(int selectedId);
    /** @brief Applies a LookAndFeel palette change from the submenu. @param selectedId One of the `LookAndFeel_*` enum values. */
    void handleOptionsLookAndFeelMenuResult(int selectedId);
    /** @brief Opens an alert dialog for the user to enter a new OSC port number. */
    void handleOptionsOscPortMenuResult();
    /** @brief Opens an alert dialog with a combo box for framerate selection. */
    void handleOptionsFramerateMenuResult();
    /** @brief Opens a selected MIDI input device by its dynamic menu index. @param selectedId ID within the input device range. */
    void handleOptionsInputDeviceSelectionMenuResult(int selectedId);
    /** @brief Opens a selected MIDI output device by its dynamic menu index. @param selectedId ID within the output device range. */
    void handleOptionsOutputDeviceSelectionMenuResult(int selectedId);

    /** @brief Returns the first menu ID reserved for MIDI input device entries. */
    int getInputDeviceOptionIdRangeStart();
    /** @brief Returns one past the last menu ID reserved for MIDI input device entries. */
    int getInputDeviceOptionIdRangeEnd();
    /** @brief Returns the first menu ID reserved for MIDI output device entries. */
    int getOutputDeviceOptionIdRangeStart();
    /** @brief Returns one past the last menu ID reserved for MIDI output device entries. */
    int getOutputDeviceOptionIdRangeEnd();

    //==============================================================================
    /** @brief Queries available MIDI devices and populates `m_optionsItems` with their names. */
    void updateAvailableDevices();
    /** @brief Opens the MIDI input device identified by `deviceIdentifier` and starts it. @param deviceIdentifier JUCE device identifier string. */
    void openMidiInputDevice(const juce::String& deviceIdentifier);
    /** @brief Opens the MIDI output device identified by `deviceIdentifier`. @param deviceIdentifier JUCE device identifier string. */
    void openMidiOutputDevice(const juce::String& deviceIdentifier);
    /** @brief Sends an MTC Full Frame SysEx for the current `m_ts` and `m_frameRate`. */
    void sendMessage();
    /**
     * @brief Sends an MTC Full Frame SysEx for the given timecode and frame-rate.
     *
     * Constructs the 8-byte payload `F0 7F 7F 01 01 <hr> <mn> <sc> <fr> F7` where
     * byte 4 encodes the frame-rate index in bits 5–6 and hours in bits 0–4.
     *
     * @param ts        Timecode position to transmit.
     * @param frameRate MTC frame-rate index (0 = 24 fps, 1 = 25 fps, 2 = 29.97 fps, 3 = 30 fps).
     */
    void sendMessage(TimeStamp ts, int frameRate);

    /**
     * @brief Reads the timecode editor text, parses it, and stores the result in `m_ts`.
     * @return `true` if parsing succeeded; `false` if the editor is unavailable.
     */
    bool parseTimecode();
    /** @brief Resets the timecode editor text to `"00:00:00:00"` and clears `m_ts`. */
    void resetTimecode();
    /**
     * @brief Converts a frame-rate string (e.g. `"29.97"`) to the MTC index stored in `m_frameRate`.
     * @param framerateStr  Numeric string without the `"fps"` suffix.
     * @return `true` if the value is one of 24, 25, 29.97, or 30.
     */
    bool FramerateFromString(const juce::String& framerateStr);
    /**
     * @brief Converts an MTC frame-rate index to its display string.
     * @param framerateIdent  Index: 0 = 24, 1 = 25, 2 = 29.97, 3 = 30.
     * @return Display string such as `"29.97"`, or an empty string for unknown values.
     */
    juce::String FramerateToString(int framerateIdent);

    /** @brief Returns the frame interval in milliseconds for the current frame rate. */
    int getCurrentFrameIntervalMs();
    /** @brief Returns the current frame rate as a floating-point Hz value. */
    double getCurrentFrameRateHz();

    /** @brief Returns the currently configured OSC listening port number. */
    int getOscPortNumber();
    /**
     * @brief Stores a new OSC port number and reconnects the OSC receiver.
     * @param portNumber  UDP port to listen on.
     */
    void setOscPortNumber(int portNumber);

    /** @brief Destroys and re-creates all CustomTriggerButton instances, resetting them to unconfigured. */
    void ResetCustomTriggers();

    //==============================================================================
    std::unique_ptr<juce::DrawableButton>               m_optionsButton;        ///<  Hamburger menu button; opens the options popup.
    std::map<int, std::pair<std::string, int>>          m_optionsItems;         ///<  Menu item labels and check-states keyed by `JumpyOptionsOption` ID.

    std::unique_ptr<juce::AlertWindow>                  m_messageBox;           ///<  Modal dialog reused for OSC port and framerate input.

    std::unique_ptr<juce::DrawableButton>               m_aboutButton;          ///<  Question-mark button; opens the about popup.
    std::unique_ptr<AboutComponent>                     m_aboutComponent;       ///<  Content shown inside the about popup menu item.

    std::unique_ptr<JUCEAppBasics::FixedFontTextEditor> m_timecodeEditor;       ///<  Fixed-font editor displaying and accepting `hh:mm:ss:ff` input.
    std::unique_ptr<juce::DrawableButton>               m_startRunningButton;   ///<  Toggle button that starts/stops the running timecode timer.
    std::unique_ptr<juce::DrawableButton>               m_triggerCurrentTCButton; ///<  One-shot button that immediately sends the current timecode as MTC.

    std::map<int, std::unique_ptr<CustomTriggerButton>> m_customTriggers;                           ///<  All 12 trigger buttons keyed by their grid index (0–11).
    juce::Grid                                          m_customTriggersGrid;                       ///<  JUCE grid used to lay out the trigger buttons.
    static constexpr int                                sc_customTriggersGrid_RowCount = 4;         ///<  Number of rows in the trigger grid.
    static constexpr int                                sc_customTriggersGrid_ColCount = 3;         ///<  Number of columns in the trigger grid.
    static constexpr double                             sc_customTriggersGrid_NodeGap = 2.0;        ///<  Gap in pixels between adjacent grid cells.

    juce::Array<juce::MidiDeviceInfo>                   m_currentMidiInputDevicesInfos;             ///<  Snapshot of available MIDI input devices at last menu open.
    std::unique_ptr<juce::MidiInput>                    m_midiInput;                                ///<  Currently open MIDI input device.
    std::unique_ptr<MidiInputCallbackToStdFuncWrapper>  m_midiCallbackWrapper;                      ///<  Adapter that routes JUCE MIDI callbacks to a std::function.
    std::vector<juce::MidiMessage>                      m_midiInputMessageQueue;                    ///<  Thread-safe queue of MIDI messages waiting to be processed on the main thread.
    std::mutex                                          m_midiInputMessageQueueMutex;               ///<  Guards `m_midiInputMessageQueue` against concurrent access from the MIDI thread.

    juce::Array<juce::MidiDeviceInfo>                   m_currentMidiOutputDevicesInfos;            ///<  Snapshot of available MIDI output devices at last menu open.
    std::unique_ptr<juce::MidiOutput>                   m_midiOutput;                               ///<  Currently open MIDI output device; MTC SysEx messages are sent here.

    std::unique_ptr<juce::OSCReceiver>                  m_oscServer;            ///<  OSC UDP receiver listening on `m_oscPortNumber`.

    std::unique_ptr<JumpyConfiguration>                 m_config;               ///<  Persistent XML configuration for device settings and trigger buttons.

    TimeStamp m_ts;             ///<  Current timecode position, kept in sync with the running timer and the editor field.
    /** MTC frame-rate index: 0 = 24 fps, 1 = 25 fps, 2 = 29.97 fps, 3 = 30 fps.
     *  Corresponds to the 2-bit rate-type field in the MIDI Full Frame SysEx (bits 5–6 of byte 4). */
    int m_frameRate = 1;
    double m_startMillisecondsHiRes = 0.0;  ///<  Hi-res wall-clock reference (ms) from which elapsed time is measured; adjusted to account for any initial timecode offset.

    int m_oscPortNumber = 53000;  ///<  UDP port number on which the OSC receiver listens.

    static constexpr int sc_millisInSec  = 1000;           ///<  Milliseconds per second.
    static constexpr int sc_millisInMin  = (1000 * 60);    ///<  Milliseconds per minute.
    static constexpr int sc_millisInHour = (1000 * 60 * 60); ///<  Milliseconds per hour.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JumpyComponent)
};

};
