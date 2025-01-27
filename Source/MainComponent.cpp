#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    m_devicesList = std::make_unique<juce::ComboBox>();
    m_devicesList->setTextWhenNothingSelected("Select MIDI output");
    m_devicesList->onChange = [=](){ handleDeviceSelection(); };
    addAndMakeVisible(m_devicesList.get());
    
    m_timecodeEditor = std::make_unique<juce::TextEditor>("tc");
    m_timecodeEditor->setInputFilter(new juce::TextEditor::LengthAndCharacterRestriction(11, "0123456789:"), true);
    m_timecodeEditor->onReturnKey = [=](){ if (!parseTimecode()) resetTimecode(); };
    m_timecodeEditor->onFocusLost = [=](){ if (!parseTimecode()) resetTimecode(); };
    resetTimecode();
    addAndMakeVisible(m_timecodeEditor.get());
    
    m_framerateEditor = std::make_unique<juce::TextEditor>("fr");
    m_framerateEditor->setInputFilter(new juce::TextEditor::LengthAndCharacterRestriction(5, "0123456789.,"), true);
    m_framerateEditor->onReturnKey = [=](){ if (!parseFramerate()) resetFramerate(); };
    m_framerateEditor->onFocusLost = [=](){ if (!parseFramerate()) resetFramerate(); };
    resetFramerate();
    addAndMakeVisible(m_framerateEditor.get());
    
    m_triggerButton = std::make_unique<juce::TextButton>("SendMessage", "Send message");
    m_triggerButton->onClick = [=]() { sendMessage(); };
    addAndMakeVisible(m_triggerButton.get());
    
    setSize (200, 100);
    
    updateAvailableDevices();
}

MainComponent::~MainComponent()
{
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds();
    
    m_devicesList->setBounds(bounds.removeFromTop(30).reduced(1));
    
    bounds.removeFromTop(6);
    
    auto valueBounds = bounds.removeFromTop(24);
    m_framerateEditor->setBounds(valueBounds.removeFromLeft(60).reduced(1));
    m_timecodeEditor->setBounds(valueBounds.reduced(1));
    m_triggerButton->setBounds(bounds.removeFromTop(30).reduced(1));
}

//==============================================================================
void MainComponent::updateAvailableDevices()
{
    if (!m_devicesList)
        return;
    
    m_currentMidiDevicesInfos = juce::MidiOutput::getAvailableDevices();
    m_devicesList->clear();
    for (int i = 0; i < m_currentMidiDevicesInfos.size(); i++)
        m_devicesList->addItem(m_currentMidiDevicesInfos[i].name, i + 1);
}

void MainComponent::handleDeviceSelection()
{
    auto midiOutputIdentifier = m_currentMidiDevicesInfos[m_devicesList->getSelectedId() - 1].identifier;
    m_midiOutput = juce::MidiOutput::openDevice(midiOutputIdentifier);
    jassert(m_midiOutput);
}

void MainComponent::sendMessage()
{
    if (!m_midiOutput)
        return;
    
    unsigned char bytes[] = { 0x7F, 0x7F, 0x01, 0x01, 0x20, 0x00, 0x00, 0x00 };
    bytes[4] = (m_hours & 0x1F) + ((m_frameRate << 5) & 0xE0);
    bytes[5] = m_minutes;
    bytes[6] = m_seconds;
    bytes[7] = m_frames;
    auto midiMessage = juce::MidiMessage::createSysExMessage(bytes, 8);
    
    m_midiOutput->sendMessageNow(midiMessage);
}

bool MainComponent::parseTimecode()
{
    if (!m_timecodeEditor)
        return false;
    
    auto stringToTokenize = m_timecodeEditor->getText();
    juce::StringArray time;
    time.addTokens(stringToTokenize, ":", "");
    if (time.size() != 4)
    {
        resetTimecode();
        return false;
    }
    m_hours = time[0].getIntValue();
    m_minutes = time[1].getIntValue();
    m_seconds = time[2].getIntValue();
    m_frames = time[3].getIntValue();
    
    return true;
}

void MainComponent::resetTimecode()
{
    if (m_timecodeEditor)
        m_timecodeEditor->setText("00:00:00:00");
    
    m_hours = 0;
    m_minutes = 0;
    m_seconds = 0;
    m_frames = 0;
}

bool MainComponent::parseFramerate()
{
    if (!m_framerateEditor)
        return false;
    
    auto frameRate = m_framerateEditor->getText().getDoubleValue();
    if (frameRate == 24)
        m_frameRate = 0;
    else if (frameRate == 25)
        m_frameRate = 1;
    else if (frameRate == 29.97)
        m_frameRate = 2;
    else if (frameRate == 30)
        m_frameRate = 3;
    else
        return false;
    
    return true;
}

void MainComponent::resetFramerate()
{
    if (m_framerateEditor)
        m_framerateEditor->setText("25");
    
    m_frameRate = 1;
}
