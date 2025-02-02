/* Copyright (c) 2025, Christian Ahrens
 *
 * This file is part of MTCtrigger <https://github.com/ChristianAhrens/MTCtrigger>
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

#include "JumperComponent.h"


JumperComponent::JumperComponent()
    : juce::Component()
{
    m_devicesList = std::make_unique<juce::ComboBox>();
    m_devicesList->setTextWhenNothingSelected("Select MIDI output");
    m_devicesList->onChange = [=]() { handleDeviceSelection(); };
    addAndMakeVisible(m_devicesList.get());

    m_timecodeEditor = std::make_unique<JUCEAppBasics::FixedFontTextEditor>("tc", 0U, true);
    m_timecodeEditor->setInputFilter(new juce::TextEditor::LengthAndCharacterRestriction(11, "0123456789:"), true);
    m_timecodeEditor->onReturnKey = [=]() { if (!parseTimecode()) resetTimecode(); };
    m_timecodeEditor->onFocusLost = [=]() { if (!parseTimecode()) resetTimecode(); };
    resetTimecode();
    addAndMakeVisible(m_timecodeEditor.get());

    m_framerateEditor = std::make_unique<JUCEAppBasics::FixedFontTextEditor>("fr", 0U, true);
    m_framerateEditor->setInputFilter(new juce::TextEditor::LengthAndCharacterRestriction(5, "0123456789.,"), true);
    m_framerateEditor->onReturnKey = [=]() { if (!parseFramerate()) resetFramerate(); };
    m_framerateEditor->onFocusLost = [=]() { if (!parseFramerate()) resetFramerate(); };
    resetFramerate();
    addAndMakeVisible(m_framerateEditor.get());

    m_startRunningButton = std::make_unique<juce::DrawableButton>("RunTC", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground);
    m_startRunningButton->setClickingTogglesState(true);
    m_startRunningButton->onClick = [=]() {
        if (m_startRunningButton && m_startRunningButton->getToggleState())
        {
            setStartMilliseconds();
            startTimer(10);
        }
        else
        {
            stopTimer();
        }
    };
    addAndMakeVisible(m_startRunningButton.get());

    m_triggerTCButton = std::make_unique<juce::TextButton>("Trigger current TC", "SendMessage");
    m_triggerTCButton->onClick = [=]() { sendMessage(); };
    addAndMakeVisible(m_triggerTCButton.get());

    m_customTriggersGrid.rowGap.pixels = sc_customTriggersGrid_NodeGap;
    m_customTriggersGrid.columnGap.pixels = sc_customTriggersGrid_NodeGap;
    for (int i = 0; i < sc_customTriggersGrid_ColCount; i++)
        m_customTriggersGrid.templateColumns.add(juce::Grid::TrackInfo(juce::Grid::Fr(1)));
    for (int i = 0; i < sc_customTriggersGrid_RowCount; i++)
        m_customTriggersGrid.templateRows.add(juce::Grid::TrackInfo(juce::Grid::Fr(1)));
    for (int i = 0; i < sc_customTriggersGrid_RowCount * sc_customTriggersGrid_ColCount; i++)
    {
        m_customTriggers[i] = std::make_unique<CustomTriggerButton>(juce::String("CT") + juce::String(i));
        m_customTriggers[i]->onTriggerClicked = [=](const TimeStamp& ts) { setAndSendTimeCode(ts); };
        addAndMakeVisible(m_customTriggers[i].get());
        m_customTriggersGrid.items.add(m_customTriggers[i].get());
    }

    updateAvailableDevices();
}

JumperComponent::~JumperComponent()
{
}

void JumperComponent::paint(juce::Graphics &g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void JumperComponent::resized()
{
    auto bounds = getLocalBounds();

    m_devicesList->setBounds(bounds.removeFromTop(30).reduced(1));

    bounds.removeFromTop(6);

    auto valueBounds = bounds.removeFromTop(30);
    m_framerateEditor->setBounds(valueBounds.removeFromLeft(50).reduced(1));
    m_startRunningButton->setBounds(valueBounds.removeFromRight(valueBounds.getHeight()).reduced(1));
    m_timecodeEditor->setBounds(valueBounds.reduced(1));

    bounds.removeFromTop(2);

    m_triggerTCButton->setBounds(bounds.removeFromTop(30).reduced(1));

    bounds.removeFromTop(6);

    m_customTriggersGrid.performLayout(bounds.reduced(1));
}

void JumperComponent::timerCallback()
{
    auto currentMillisecondsHiRes = juce::Time::getMillisecondCounterHiRes();
    auto elapsedMillisecondsHiRes = currentMillisecondsHiRes - m_startMillisecondsHiRes;

    auto hours = int(elapsedMillisecondsHiRes / sc_millisInHour);
    auto overrunMillis = (hours * sc_millisInHour);
    auto minutes = int(int(elapsedMillisecondsHiRes - overrunMillis) / sc_millisInMin);
    overrunMillis += (minutes * sc_millisInMin);
    auto seconds = int(int(elapsedMillisecondsHiRes - overrunMillis) / sc_millisInSec);
    overrunMillis += (seconds * sc_millisInSec);
    auto frames = int(int(elapsedMillisecondsHiRes - overrunMillis) / getCurrentFrameIntervalMs());

    auto newTS = TimeStamp(hours, minutes, seconds, frames);

    if (newTS != m_ts)
        setAndSendTimeCode(newTS);
}

void JumperComponent::setStartMilliseconds()
{
    m_startMillisecondsHiRes = juce::Time::getMillisecondCounterHiRes(); // get the counter start reference value
    m_startMillisecondsHiRes -= (m_ts.getHours() * sc_millisInHour) + (m_ts.getMinutes() * sc_millisInMin) + (m_ts.getSeconds() * sc_millisInSec) + (m_ts.getFrames() * getCurrentFrameIntervalMs()); // add any offset from currently set TC value
}

void JumperComponent::lookAndFeelChanged()
{
    auto startRunningButtonDrawable = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(BinaryData::play_arrow24px_svg).get());
    startRunningButtonDrawable->replaceColour(juce::Colours::black, getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));
    m_startRunningButton->setImages(startRunningButtonDrawable.get());
}

void JumperComponent::setAndSendTimeCode(TimeStamp ts)
{
    m_ts = ts;

    setStartMilliseconds();

    sendMessage();
}


//==============================================================================
void JumperComponent::updateAvailableDevices()
{
    if (!m_devicesList)
        return;

    m_currentMidiDevicesInfos = juce::MidiOutput::getAvailableDevices();
    m_devicesList->clear();
    for (int i = 0; i < m_currentMidiDevicesInfos.size(); i++)
        m_devicesList->addItem(m_currentMidiDevicesInfos[i].name, i + 1);
}

void JumperComponent::handleDeviceSelection()
{
    auto midiOutputIdentifier = m_currentMidiDevicesInfos[m_devicesList->getSelectedId() - 1].identifier;
    m_midiOutput = juce::MidiOutput::openDevice(midiOutputIdentifier);
    jassert(m_midiOutput);
}

void JumperComponent::sendMessage()
{
    sendMessage(m_ts, m_frameRate);
}

void JumperComponent::sendMessage(TimeStamp ts, int frameRate)
{

    if (m_timecodeEditor) m_timecodeEditor->setText(ts.toString());
    DBG(juce::String(__FUNCTION__) << " " << ts.toString());

    if (!m_midiOutput)
        return;

    unsigned char bytes[] = { 0x7F, 0x7F, 0x01, 0x01, 0x20, 0x00, 0x00, 0x00 };
    bytes[4] = (ts.getHours() & 0x1F) + ((frameRate << 5) & 0xE0);
    bytes[5] = static_cast<unsigned char>(ts.getMinutes());
    bytes[6] = static_cast<unsigned char>(ts.getSeconds());
    bytes[7] = static_cast<unsigned char>(ts.getFrames());
    auto midiMessage = juce::MidiMessage::createSysExMessage(bytes, 8);

    m_midiOutput->sendMessageNow(midiMessage);
}

bool JumperComponent::parseTimecode()
{
    if (!m_timecodeEditor)
        return false;

    m_ts = TimeStamp::fromString(m_timecodeEditor->getText());

    return true;
}

void JumperComponent::resetTimecode()
{
    if (m_timecodeEditor)
        m_timecodeEditor->setText("00:00:00:00");

    m_ts.clear();
}

bool JumperComponent::parseFramerate()
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

void JumperComponent::resetFramerate()
{
    if (m_framerateEditor)
        m_framerateEditor->setText("25");

    m_frameRate = 1;
}

int JumperComponent::getCurrentFrameRateHz()
{
    switch (m_frameRate)
    {
    case 0:
        return 24;
    case 1:
        return 25;
    case 2:
        return 30;
    case 3:
    default:
        return 30;
    }
}

int JumperComponent::getCurrentFrameIntervalMs()
{
    return 1000 / getCurrentFrameRateHz();
}

