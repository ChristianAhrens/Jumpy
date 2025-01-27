#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::Component
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    void updateAvailableDevices();
    void handleDeviceSelection();
    void sendMessage();
    
    bool parseTimecode();
    void resetTimecode();
    bool parseFramerate();
    void resetFramerate();
    
    //==============================================================================
    std::unique_ptr<juce::ComboBox>     m_devicesList;
    std::unique_ptr<juce::TextEditor>   m_timecodeEditor;
    std::unique_ptr<juce::TextEditor>   m_framerateEditor;
    std::unique_ptr<juce::TextButton>   m_triggerButton;
    
    juce::Array<juce::MidiDeviceInfo>   m_currentMidiDevicesInfos;
    std::unique_ptr<juce::MidiOutput>   m_midiOutput;
    
    int m_hours = 0;
    int m_minutes = 0;
    int m_seconds = 0;
    int m_frames = 0;
    int m_frameRate = 1; // 24fps=00, 25fps=01, 29,97fps=10, 30fps=11


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
