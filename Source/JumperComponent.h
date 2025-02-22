/* Copyright (c) 2025, Christian Ahrens
 *
 * This file is part of MTCtrigger <https://github.com/ChristianAhrens/Jumper>
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

#include "JumperConfiguration.h"
#include "CustomTriggerButton.h"

namespace Jumper
{

class JumperComponent :   
    public juce::Component, 
    public juce::Timer, 
    public juce::OSCReceiver::Listener<juce::OSCReceiver::MessageLoopCallback>,
    public JumperConfiguration::Dumper,
    public JumperConfiguration::Watcher
{

public:
    JumperComponent();
    ~JumperComponent() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;
    void lookAndFeelChanged() override;

    //==============================================================================
    void setAndSendTimeCode(TimeStamp ts);

    //==============================================================================
    void oscMessageReceived(const OSCMessage& message) override;

    //==========================================================================
    void performConfigurationDump() override;
    void onConfigUpdated() override;

private:
    //==============================================================================
    void timerCallback() override;

    //==============================================================================
    void setStartMilliseconds();

    //==============================================================================
    void updateAvailableDevices();
    void handleDeviceSelection();
    void openMidiDevice(const juce::String& deviceIdentifier);
    void sendMessage(TimeStamp ts, int frameRate);
    void sendMessage();

    bool parseTimecode();
    void resetTimecode();
    bool parseFramerate();
    void resetFramerate();

    int getCurrentFrameIntervalMs();
    int getCurrentFrameRateHz();

    //==============================================================================
    std::unique_ptr<juce::ComboBox>                     m_devicesList;
    std::unique_ptr<juce::Label>                        m_oscInfoLabel;

    std::unique_ptr<JUCEAppBasics::FixedFontTextEditor> m_timecodeEditor;
    std::unique_ptr<JUCEAppBasics::FixedFontTextEditor> m_framerateEditor;
    std::unique_ptr<juce::DrawableButton>               m_startRunningButton;
    std::unique_ptr<juce::TextButton>                   m_triggerTCButton;

    std::map<int, std::unique_ptr<CustomTriggerButton>> m_customTriggers;
    juce::Grid                                          m_customTriggersGrid;
    static constexpr int                                sc_customTriggersGrid_RowCount = 4;
    static constexpr int                                sc_customTriggersGrid_ColCount = 3;
    static constexpr double                             sc_customTriggersGrid_NodeGap = 2.0;

    juce::Array<juce::MidiDeviceInfo>                   m_currentMidiDevicesInfos;
    std::unique_ptr<juce::MidiOutput>                   m_midiOutput;

    std::unique_ptr<juce::OSCReceiver>                  m_oscServer;

    std::unique_ptr<JumperConfiguration>                m_config;

    TimeStamp m_ts;
    int m_frameRate = 1; // 24fps=00, 25fps=01, 29,97fps=10, 30fps=11
    double m_startMillisecondsHiRes = 0.0;

    static constexpr int sc_millisInSec = 1000;
    static constexpr int sc_millisInMin = (1000 * 60);
    static constexpr int sc_millisInHour = (1000 * 60 * 60);

    static constexpr int sc_oscPortNumber = 53000;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JumperComponent)
};

};

