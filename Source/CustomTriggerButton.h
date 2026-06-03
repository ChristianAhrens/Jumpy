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
#include <MidiCommandRangeAssignment.h>


namespace Jumpy
{

/**
 * @class TimeStamp
 * @brief SMPTE-style timecode position expressed as hours, minutes, seconds, and frames.
 *
 * TimeStamp stores a single timecode coordinate in the form `hh:mm:ss:ff` — the same
 * addressing scheme used by MIDI Time Code (MTC).  The frame component is frame-rate
 * agnostic; the caller is responsible for interpreting it relative to the active rate
 * (see `JumpyComponent::m_frameRate`).
 *
 * Valid ranges:
 * - hours  : 0–23
 * - minutes: 0–59
 * - seconds: 0–59
 * - frames : 0–30  (accommodates all four MTC rates: 24, 25, 29.97, and 30 fps)
 *
 * The primary serialisation round-trip is through the `hh:mm:ss:ff` string format,
 * which is what is both displayed in and accepted by the timecode editor field.
 * An invalid input string causes an asynchronous warning dialog and returns a
 * default-constructed (all-zero) TimeStamp.
 */
class TimeStamp
{
public:
    /** @brief Default constructor — initialises all fields to zero. */
    TimeStamp() {};

    /**
     * @brief Constructs a TimeStamp from explicit field values.
     * @param hours   Hours component (0–23).
     * @param minutes Minutes component (0–59).
     * @param seconds Seconds component (0–59).
     * @param frames  Frames component (0–30).
     */
    TimeStamp(int hours, int minutes, int seconds, int frames) { m_hours = hours; m_minutes = minutes; m_seconds = seconds; m_frames = frames; };

    /**
     * @brief Constructs a TimeStamp by parsing a `hh:mm:ss:ff` string.
     * @param tsString  Colon-separated timecode string.
     */
    TimeStamp(const juce::String& tsString) { *this = fromString(tsString); };

    ~TimeStamp() {};

    /** @brief Returns true if both TimeStamps represent identical positions. */
    bool operator==(const TimeStamp& other) const { return m_hours == other.getHours() && m_minutes == other.getMinutes() && m_seconds == other.getSeconds() && m_frames == other.getFrames(); };
    /** @brief Returns true if the two TimeStamps differ in any field. */
    bool operator!=(const TimeStamp& other) const { return !(*this == other); };

    /** @brief Sets the hours component. @param hours Value in the range 0–23. */
    void setHours(int hours) { m_hours = hours; };
    /** @brief Sets the minutes component. @param minutes Value in the range 0–59. */
    void setMinutes(int minutes) { m_minutes = minutes; };
    /** @brief Sets the seconds component. @param seconds Value in the range 0–59. */
    void setSeconds(int seconds) { m_seconds = seconds; };
    /** @brief Sets the frames component. @param frames Value in the range 0–30. */
    void setFrames(int frames) { m_frames = frames; };

    /** @brief Returns the hours component. */
    int getHours() const { return m_hours; };
    /** @brief Returns the minutes component. */
    int getMinutes() const { return m_minutes; };
    /** @brief Returns the seconds component. */
    int getSeconds() const { return m_seconds; };
    /** @brief Returns the frames component. */
    int getFrames() const { return m_frames; };

    /** @brief Resets all fields to zero, equivalent to timecode 00:00:00:00. */
    void clear() { m_hours = 0; m_minutes = 0; m_seconds = 0; m_frames = 0; };

    /**
     * @brief Returns true when all field values are within their legal ranges.
     * @return `true` if hours < 24, minutes < 60, seconds < 60, and frames <= 30.
     */
    const bool isValid() const {
        return m_hours >= 0 && m_hours < 24 && m_minutes >= 0 && m_minutes < 60 && m_seconds >= 0 && m_seconds < 60 && m_frames >= 0 && m_frames <= 30;
    };

    /**
     * @brief Serialises the timecode to a zero-padded `hh:mm:ss:ff` string.
     * @return Four colon-delimited two-digit fields, e.g. `"01:23:45:12"`.
     */
    const juce::String toString() const {
        juce::StringArray timeDigits = {
            juce::String(getHours()).paddedLeft('0', 2),
            juce::String(getMinutes()).paddedLeft('0', 2),
            juce::String(getSeconds()).paddedLeft('0', 2),
            juce::String(getFrames()).paddedLeft('0', 2)
        };
        return timeDigits.joinIntoString(":");
    };

    /**
     * @brief Parses a `hh:mm:ss:ff` string and returns the corresponding TimeStamp.
     *
     * Shows an asynchronous warning dialog and returns a default TimeStamp if the
     * string does not contain exactly four colon-separated tokens or if the resulting
     * values fail `isValid()`.
     *
     * @param tsString  Colon-separated timecode string, e.g. `"01:23:45:12"`.
     * @return Parsed TimeStamp, or a zero TimeStamp on failure.
     */
    static TimeStamp fromString(const juce::String& tsString)
    {
        auto success = true;

        juce::StringArray time;
        time.addTokens(tsString, ":", "");
        if (time.size() != 4)
            success = false;

        auto ts = TimeStamp(
            time[0].getIntValue(),
            time[1].getIntValue(),
            time[2].getIntValue(),
            time[3].getIntValue());
        success = success && ts.isValid();

        if (!success)
        {
            juce::AlertWindow::showAsync(
                juce::MessageBoxOptions()
                    .withMessage("Invalid Timecode value " + tsString + ".")
                    .withButton("Ok")
                    .withIconType(juce::MessageBoxIconType::WarningIcon),
                nullptr);
            return {};
        }
        else
            return ts;
    };

private:
    int m_hours = 0;    ///<  Hours component of the timecode position.
    int m_minutes = 0;  ///<  Minutes component.
    int m_seconds = 0;  ///<  Seconds component.
    int m_frames = 0;   ///<  Frames component; valid range depends on the active frame rate.
};

/**
 * @class CustomTriggerButton
 * @brief Coloured trigger button that jumps to a preconfigured MTC timecode on activation.
 *
 * Each CustomTriggerButton occupies one cell in the 4×3 trigger grid shown on the
 * main UI.  A button starts in a **disabled / unconfigured** state (shown as a faint
 * add-icon placeholder); any mouse-down on an unconfigured button opens a settings
 * popup where the user assigns a name, background colour, target timecode, an OSC
 * address pattern, and an optional MIDI command.  Once configured, the button becomes
 * enabled and a click fires `onTriggerClicked` with the stored TimeStamp.
 *
 * External trigger paths (OSC and MIDI) are handled upstream by JumpyComponent:
 * matching messages call `JumpyComponent::setAndSendTimeCode()` directly, bypassing
 * the button's click handler.
 *
 * Button state is persisted through JumpyConfiguration as TriggerDetails serialised
 * inside `<TRIGGERDETAILS>` XML elements under `<CUSTOMTRIGGERS>`.
 */
class CustomTriggerButton  : public juce::Button
{
public:
    /**
     * @struct TriggerDetails
     * @brief All persistent properties associated with a single trigger button slot.
     *
     * A TriggerDetails bundle carries everything needed to configure one button:
     * a display name, a background colour, the target timecode jumped to on activation,
     * and two optional external trigger sources (an OSC address pattern and a MIDI
     * command assignment).
     *
     * Serialisation uses a semicolon-delimited string:
     * ```
     * name;colourHex;hh:mm:ss:ff;/osc/address;midiHex
     * ```
     * This string is stored verbatim as text content inside a `<TRIGGERDETAILS>` XML
     * element in the configuration file.
     */
    struct TriggerDetails
    {
        /** @brief Default constructor — initialises all fields to their neutral defaults. */
        TriggerDetails()
        {
            m_Name = "";
            m_Colour = juce::Colours::cornflowerblue;
            m_TS = TimeStamp();
            m_oscTrigger = juce::OSCMessage(juce::String("/") + juce::JUCEApplication::getInstance()->getApplicationName() + "/n");
            m_midiTrigger = JUCEAppBasics::MidiCommandRangeAssignment();
        }

        /**
         * @brief Constructs TriggerDetails from explicit field values.
         * @param name        Display name shown on the button face.
         * @param colour      Background colour of the button.
         * @param timestamp   Target timecode to be sent when the button is activated.
         * @param oscTrigger  OSC message whose address pattern triggers this button externally.
         * @param midiTrigger MIDI command assignment that triggers this button externally.
         */
        TriggerDetails(const juce::String& name, const juce::Colour& colour, const TimeStamp& timestamp, const juce::OSCMessage& oscTrigger, const JUCEAppBasics::MidiCommandRangeAssignment& midiTrigger)
        {
            m_Name = name;
            m_Colour = colour;
            m_TS = timestamp;
            m_oscTrigger = oscTrigger;
            m_midiTrigger = midiTrigger;
        }

        /**
         * @brief Deserialising constructor — parses a semicolon-delimited parameter string.
         * @param paramStr  String produced by `toString()`.
         */
        TriggerDetails(const juce::String& paramStr)
        {
            *this = fromString(paramStr);
        }

        juce::String                                m_Name;         ///<  User-visible button label.
        juce::Colour                                m_Colour;       ///<  Button background colour.
        TimeStamp                                   m_TS;           ///<  Target timecode sent on activation.
        juce::OSCMessage                            m_oscTrigger = { juce::OSCAddressPattern("/a/b"), 0 };  ///<  OSC address pattern accepted as an external trigger.
        JUCEAppBasics::MidiCommandRangeAssignment   m_midiTrigger;  ///<  MIDI command assignment accepted as an external trigger.

        /**
         * @brief Serialises all fields to a semicolon-delimited string.
         * @return String of the form `name;colourHex;hh:mm:ss:ff;/osc/address;midiHex`.
         */
        juce::String toString() const
        {
            return m_Name + ";" + m_Colour.toString() + ";" + m_TS.toString() + ";" + m_oscTrigger.getAddressPattern().toString() + ";" + m_midiTrigger.serializeToHexString();
        }

        /**
         * @brief Parses a semicolon-delimited string produced by `toString()`.
         * @param paramStr  String containing exactly five semicolon-separated tokens.
         * @return Populated TriggerDetails.
         */
        static TriggerDetails fromString(const juce::String& paramStr)
        {
            auto sa = juce::StringArray();
            auto cnt = sa.addTokens(paramStr, ";", "");
            jassert(5 == cnt);

            TriggerDetails td;
            td.m_Name = sa[0];
            td.m_Colour = juce::Colour::fromString(sa[1]);
            td.m_TS = TimeStamp::fromString(sa[2]);
            td.m_oscTrigger.setAddressPattern(sa[3]);
            if (sa[4].isNotEmpty())
                td.m_midiTrigger.deserializeFromHexString(sa[4]);

            return td;
        }

        /**
         * @brief Returns true when all fields hold their default (unset) values.
         * @return `true` if name is empty, colour is default, timecode is zero,
         *         OSC address is empty, and MIDI assignment is default.
         */
        bool isEmpty() const
        {
            return m_Name.isEmpty() && m_Colour == juce::Colour() && m_TS == TimeStamp() && m_oscTrigger.getAddressPattern().toString().isEmpty() && m_midiTrigger == JUCEAppBasics::MidiCommandRangeAssignment();
        }
    };

public:
    //==============================================================================
    /**
     * @brief Constructs an unconfigured trigger button with the given name.
     * @param buttonName  Internal JUCE button name (e.g. `"CT0"`); also used as the
     *                    default label until the user configures the button.
     */
    CustomTriggerButton(const juce::String& buttonName);
    ~CustomTriggerButton() override;

    /**
     * @brief Applies a full TriggerDetails configuration to this button.
     *
     * Enables the button, updates its colour, label, and timecode display, and fires
     * `onDetailsChanged` to notify the configuration system of the change.
     *
     * @param triggerDetails  New trigger configuration to apply.
     */
    void setTriggerDetails(const TriggerDetails& triggerDetails);

    /**
     * @brief Returns the currently active TriggerDetails for this button.
     * @return Const reference to the internal TriggerDetails.
     */
    const TriggerDetails& getTriggerDetails() const;

    /**
     * @brief Informs the button which MIDI input device is currently open.
     *
     * The identifier is forwarded to the settings popup's MIDI learner so that
     * live MIDI input is routed correctly during assignment.
     *
     * @param midiInputDeviceIdentifier  JUCE device identifier string from `juce::MidiInput`.
     */
    void setMidiInputDeviceIdentifier(const juce::String& midiInputDeviceIdentifier);

    //==============================================================================
    /** @brief Fires `onTriggerClicked` with the stored timecode when the button is enabled. */
    void clicked() override;

    //==============================================================================
    /**
     * @brief Opens the trigger settings popup when the button is unconfigured.
     *
     * If the button is already enabled, normal button behaviour applies.  When
     * disabled (unconfigured), any mouse-down pre-populates a default TriggerDetails
     * and opens the settings `CallOutBox`.
     *
     * @param e  Mouse event forwarded from JUCE.
     */
    void mouseDown(const juce::MouseEvent& e) override;

    //==============================================================================
    /** @brief Draws the add-icon placeholder when the button is unconfigured. */
    void paint(juce::Graphics& g) override;
    /** @brief Delegates background painting to the active LookAndFeel. */
    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    /** @brief Lays out the name label, timecode label, and settings gear button. */
    void resized() override;
    /** @brief Reloads SVG icon drawables with the current theme colour. */
    void lookAndFeelChanged() override;

    //==============================================================================
    /** Invoked on the message thread when the button is clicked and enabled.
     *  Receives the stored TimeStamp so the caller can send the MTC jump. */
    std::function<void(const TimeStamp&)>       onTriggerClicked;

    /** Invoked whenever TriggerDetails change (via settings popup or `setTriggerDetails()`),
     *  allowing the caller to persist the updated configuration. */
    std::function<void(const TriggerDetails&)>  onDetailsChanged;

private:
    //==============================================================================
    /** @brief Opens a `juce::CallOutBox` containing a TriggerSettingsComponent. */
    void showTriggerSettings();

    //==============================================================================
    TriggerDetails  m_triggerDetails;   ///<  Current configuration for this button slot.

    juce::String    m_midiInputDeviceIdentifier;  ///<  Identifier of the currently open MIDI input device; passed to the settings popup.

    std::unique_ptr<juce::Drawable> m_addDrawable;  ///<  SVG icon shown when the button is unconfigured.

    std::unique_ptr<juce::Label>            m_nameLabel;      ///<  Label displaying `m_triggerDetails.m_Name`.
    std::unique_ptr<juce::Label>            m_tsLabel;        ///<  Label displaying the target timecode string.
    std::unique_ptr<juce::DrawableButton>   m_settingsButton; ///<  Gear icon button that opens the settings popup.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomTriggerButton)
};

};
