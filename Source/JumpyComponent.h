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

class JumpyComponent :   
    public juce::Component, 
    public juce::Timer, 
    public juce::OSCReceiver::Listener<juce::OSCReceiver::MessageLoopCallback>,
    public JumpyConfiguration::Dumper,
    public JumpyConfiguration::Watcher
{
public:
    enum JumpyOptionsOption
    {
        ResetConfig = 1,
        LookAndFeel_First,
        LookAndFeel_FollowHost = LookAndFeel_First,
        LookAndFeel_Dark,
        LookAndFeel_Light,
        LookAndFeel_Last = LookAndFeel_Light,
        OscPort,
        FrameRate,
        OutputDevice,
    };

public:
    JumpyComponent();
    ~JumpyComponent() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;
    void lookAndFeelChanged() override;

    //==============================================================================
    void setAndSendTimeCode(TimeStamp ts);

    void connectToOscSocket();

    //==============================================================================
    void oscMessageReceived(const OSCMessage& message) override;

    //==========================================================================
    void performConfigurationDump() override;
    void onConfigUpdated() override;

    //========================================================================*
    std::function<void(int, bool)> onPaletteStyleChange;

private:
    //==============================================================================
    void timerCallback() override;

    //==============================================================================
    void setStartMilliseconds();

    //==============================================================================
    void handleOptionsMenuResult(int selectedId);
    void handleOptionsLookAndFeelMenuResult(int selectedId);
    void handleOptionsOscPortMenuResult();
    void handleOptionsFramerateMenuResult();
    void handleOptionsOutputDeviceSelectionMenuResult(int selectedId);

    //==============================================================================
    void updateAvailableDevices();
    void openMidiDevice(const juce::String& deviceIdentifier);
    void sendMessage(TimeStamp ts, int frameRate);
    void sendMessage();

    bool parseTimecode();
    void resetTimecode();
    bool FramerateFromString(const juce::String& framerateStr);
    juce::String FramerateToString(int framerateIdent);

    int getCurrentFrameIntervalMs();
    double getCurrentFrameRateHz();

    int getOscPortNumber();
    void setOscPortNumber(int portNumber);

    void ResetCustomTriggers();

    //==============================================================================
    std::unique_ptr<juce::DrawableButton>               m_optionsButton;
    std::map<int, std::pair<std::string, int>>          m_optionsItems;

    std::unique_ptr<juce::AlertWindow>                  m_messageBox;

    std::unique_ptr<juce::DrawableButton>               m_aboutButton;
    std::unique_ptr<AboutComponent>                     m_aboutComponent;

    std::unique_ptr<JUCEAppBasics::FixedFontTextEditor> m_timecodeEditor;
    std::unique_ptr<juce::DrawableButton>               m_startRunningButton;
    std::unique_ptr<juce::DrawableButton>               m_triggerCurrentTCButton;

    std::map<int, std::unique_ptr<CustomTriggerButton>> m_customTriggers;
    juce::Grid                                          m_customTriggersGrid;
    static constexpr int                                sc_customTriggersGrid_RowCount = 4;
    static constexpr int                                sc_customTriggersGrid_ColCount = 3;
    static constexpr double                             sc_customTriggersGrid_NodeGap = 2.0;

    juce::Array<juce::MidiDeviceInfo>                   m_currentMidiInputDevicesInfos;
    juce::Array<juce::MidiDeviceInfo>                   m_currentMidiOutputDevicesInfos;
    std::unique_ptr<juce::MidiOutput>                   m_midiOutput;

    std::unique_ptr<juce::OSCReceiver>                  m_oscServer;

    std::unique_ptr<JumpyConfiguration>                m_config;

    TimeStamp m_ts;
    int m_frameRate = 1; // 24fps=00, 25fps=01, 29,97fps=10, 30fps=11
    double m_startMillisecondsHiRes = 0.0;

    int m_oscPortNumber = 53000;

    static constexpr int sc_millisInSec = 1000;
    static constexpr int sc_millisInMin = (1000 * 60);
    static constexpr int sc_millisInHour = (1000 * 60 * 60);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JumpyComponent)
};

};

