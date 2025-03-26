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

#include "JumperComponent.h"

#include <CustomLookAndFeel.h>


namespace Jumper
{


class AboutComponent : public juce::Component
{
public:
    AboutComponent(const char* imageData, int imageDataSize)
        : juce::Component()
    {
        m_appIcon = std::make_unique<juce::DrawableButton>("App Icon", juce::DrawableButton::ButtonStyle::ImageFitted);
        m_appIcon->setColour(juce::DrawableButton::ColourIds::backgroundColourId, juce::Colours::transparentBlack);
        m_appIcon->setColour(juce::DrawableButton::ColourIds::backgroundOnColourId, juce::Colours::transparentBlack);
        m_appIcon->setImages(juce::Drawable::createFromImageData(imageData, imageDataSize).get());
        addAndMakeVisible(m_appIcon.get());

        m_appInfoLabel = std::make_unique<juce::Label>("Version", juce::JUCEApplication::getInstance()->getApplicationName() + " " + juce::JUCEApplication::getInstance()->getApplicationVersion());
        m_appInfoLabel->setJustificationType(juce::Justification::centredBottom);
        m_appInfoLabel->setFont(juce::Font(juce::FontOptions(16.0, juce::Font::plain)));
        addAndMakeVisible(m_appInfoLabel.get());

        m_appRepoLink = std::make_unique<juce::HyperlinkButton>(juce::JUCEApplication::getInstance()->getApplicationName() + juce::String(" on GitHub"), URL("https://www.github.com/ChristianAhrens/Jumper"));
        m_appRepoLink->setFont(juce::Font(juce::FontOptions(16.0, juce::Font::plain)), false /* do not resize */);
        m_appRepoLink->setJustificationType(juce::Justification::centredTop);
        addAndMakeVisible(m_appRepoLink.get());
        
#if JUCE_IOS
        m_iOSMIDISetupLink = std::make_unique<juce::HyperlinkButton>("MIDI connection to macOS", URL("https://github.com/ChristianAhrens/Jumper/blob/main/README.md#midi-network-session-setup---ios-to-macos"));
        https://github.com/ChristianAhrens/Mema/blob/main/README.md#mobilerecordingusecase
        m_iOSMIDISetupLink->setFont(juce::Font(juce::FontOptions(16.0, juce::Font::plain)), false /* do not resize */);
        m_iOSMIDISetupLink->setJustificationType(juce::Justification::centredTop);
        addAndMakeVisible(m_iOSMIDISetupLink.get());
#endif
    }

    ~AboutComponent() override
    {
    }

    //========================================================================*
    void paint(juce::Graphics& g) override
    {
        g.fillAll(getLookAndFeel().findColour(juce::DrawableButton::backgroundColourId));

        juce::Component::paint(g);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        auto margin = bounds.getHeight() / 8;
        bounds.reduce(margin, margin);
        auto iconBounds = bounds.removeFromTop(bounds.getHeight() / 2);
        auto infoBounds = bounds.removeFromTop(bounds.getHeight() / 2);
#if JUCE_IOS
        auto repoLinkBounds = bounds.removeFromTop(18);
        auto& howToMIDIiOS = bounds;
#else
        auto& repoLinkBounds = bounds;
#endif

        m_appIcon->setBounds(iconBounds);
        m_appInfoLabel->setBounds(infoBounds);
        m_appRepoLink->setBounds(repoLinkBounds);
#if JUCE_IOS
        m_iOSMIDISetupLink->setBounds(howToMIDIiOS);
#endif
    }

private:
    std::unique_ptr<juce::DrawableButton>   m_appIcon;
    std::unique_ptr<juce::Label>            m_appInfoLabel;
    std::unique_ptr<juce::HyperlinkButton>  m_appRepoLink;
#if JUCE_IOS
    std::unique_ptr<juce::HyperlinkButton>  m_iOSMIDISetupLink;
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AboutComponent)
};

class CustomAboutItem : public juce::PopupMenu::CustomComponent
{
public:
    CustomAboutItem(juce::Component* componentToHold, juce::Rectangle<int> minIdealSize)
    {
        m_component = componentToHold;
        addAndMakeVisible(m_component);

        m_minIdealSize = minIdealSize;
    }
    virtual ~CustomAboutItem() = default;

    void getIdealSize(int& idealWidth, int& idealHeight) override
    {
        auto resultingIdealSize = juce::Rectangle<int>(idealWidth, idealHeight);
        auto mc = juce::Desktop::getInstance().getComponent(0);
        if (mc)
        {
            auto fBounds = mc->getBounds().toFloat();
            auto h = fBounds.getHeight();
            auto w = fBounds.getWidth();
            if (h > 0.0f && w > 0.0f)
            {
                if (h > w)
                {
                    w = 0.75f * w;
                    h = w;
                }
                else
                {
                    h = 0.75f * h;
                    w = h;
                }

                resultingIdealSize = juce::Rectangle<float>(w, h).toNearestInt();
            }
        }

        if (resultingIdealSize.getWidth() < m_minIdealSize.getWidth() && resultingIdealSize.getHeight() < m_minIdealSize.getHeight())
        {
            idealWidth = m_minIdealSize.getWidth();
            idealHeight = m_minIdealSize.getHeight();
        }
        else
        {
            idealWidth = resultingIdealSize.getWidth();
            idealHeight = resultingIdealSize.getHeight();
        }
    }

    void resized() override
    {
        if (m_component)
            m_component->setBounds(getLocalBounds());
    }

private:
    juce::Component* m_component = nullptr;
    juce::Rectangle<int>    m_minIdealSize;
};


JumperComponent::JumperComponent()
    : juce::Component()
{
    // create the configuration object (is being initialized from disk automatically)
    m_config = std::make_unique<JumperConfiguration>(JUCEAppBasics::AppConfigurationBase::getDefaultConfigFilePath());
    m_config->addDumper(this);

    // check if config creation was able to read a valid config from disk...
    if (!m_config->isValid())
    {
        m_config->ResetToDefault();
    }

    // add this main component to watchers
    m_config->addWatcher(this, true); // this initial update cannot yet reach all parts of the app, esp. settings page that relies on fully initialized pagecomponentmanager, therefor a manual watcher update is triggered below

    // reset trigger does not need default
    m_optionsItems[JumperOptionsOption::ResetConfig] = std::make_pair("Reset config", 0);
    // default lookandfeel is follow local, therefor none selected
    m_optionsItems[JumperOptionsOption::LookAndFeel_FollowHost] = std::make_pair("Automatic", 1);
    m_optionsItems[JumperOptionsOption::LookAndFeel_Dark] = std::make_pair("Dark", 0);
    m_optionsItems[JumperOptionsOption::LookAndFeel_Light] = std::make_pair("Light", 0);
    // default output visu is normal meterbridge
    m_optionsItems[JumperOptionsOption::OscPort] = std::make_pair("OSC Port", 1);
    m_optionsItems[JumperOptionsOption::FrameRate] = std::make_pair("Framerate", 1);

    m_optionsButton = std::make_unique<juce::DrawableButton>("Options", juce::DrawableButton::ButtonStyle::ImageFitted);
    m_optionsButton->setTooltip(juce::JUCEApplication::getInstance()->getApplicationName() + " Options");
    m_optionsButton->onClick = [this] {
        juce::PopupMenu lookAndFeelSubMenu;
        for (int i = JumperOptionsOption::LookAndFeel_First; i <= JumperOptionsOption::LookAndFeel_Last; i++)
            lookAndFeelSubMenu.addItem(i, m_optionsItems[i].first, true, m_optionsItems[i].second == 1);

        juce::PopupMenu midiOutputSubMenu;
        m_currentMidiOutputDevicesInfos = juce::MidiOutput::getAvailableDevices();
        for (int i = JumperOptionsOption::OutputDevice; i < JumperOptionsOption::OutputDevice + m_currentMidiOutputDevicesInfos.size(); i++)
            midiOutputSubMenu.addItem(i, m_optionsItems[i].first, true, m_optionsItems[i].second == 1);

        juce::PopupMenu optionsMenu;
        optionsMenu.addSubMenu("LookAndFeel", lookAndFeelSubMenu);
        optionsMenu.addSubMenu("MIDI output device", midiOutputSubMenu);
        optionsMenu.addSeparator();
        optionsMenu.addItem(JumperOptionsOption::OscPort, "OSC port: " + juce::String(m_oscPortNumber));
        optionsMenu.addItem(JumperOptionsOption::FrameRate, "Framerate: " + FramerateToString(m_frameRate) + "fps");
        optionsMenu.addSeparator();
        optionsMenu.addItem(JumperOptionsOption::ResetConfig, "Reset configuration");
        optionsMenu.showMenuAsync(juce::PopupMenu::Options().withStandardItemHeight(32), [=](int selectedId) { handleOptionsMenuResult(selectedId); });
    };
    m_optionsButton->setAlwaysOnTop(true);
    m_optionsButton->setColour(juce::DrawableButton::ColourIds::backgroundColourId, juce::Colours::transparentBlack);
    m_optionsButton->setColour(juce::DrawableButton::ColourIds::backgroundOnColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(m_optionsButton.get());

    m_aboutComponent = std::make_unique<AboutComponent>(BinaryData::JumperRect_png, BinaryData::JumperRect_pngSize);
    m_aboutButton = std::make_unique<juce::DrawableButton>("About", juce::DrawableButton::ButtonStyle::ImageFitted);
    m_aboutButton->setTooltip(juce::String("About ") + juce::JUCEApplication::getInstance()->getApplicationName());
    m_aboutButton->onClick = [this] {
        juce::PopupMenu aboutMenu;
        aboutMenu.addCustomItem(1, std::make_unique<CustomAboutItem>(m_aboutComponent.get(), juce::Rectangle<int>(250, 250)), nullptr, juce::String("Info about") + juce::JUCEApplication::getInstance()->getApplicationName());
        aboutMenu.showMenuAsync(juce::PopupMenu::Options());
    };
    m_aboutButton->setAlwaysOnTop(true);
    m_aboutButton->setColour(juce::DrawableButton::ColourIds::backgroundColourId, juce::Colours::transparentBlack);
    m_aboutButton->setColour(juce::DrawableButton::ColourIds::backgroundOnColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(m_aboutButton.get());

    m_timecodeEditor = std::make_unique<JUCEAppBasics::FixedFontTextEditor>("tc", 0U, true);
    m_timecodeEditor->setInputFilter(new juce::TextEditor::LengthAndCharacterRestriction(11, "0123456789:"), true);
    m_timecodeEditor->onReturnKey = [=]() { if (!parseTimecode()) resetTimecode(); };
    m_timecodeEditor->onFocusLost = [=]() { if (!parseTimecode()) resetTimecode(); };
    resetTimecode();
    addAndMakeVisible(m_timecodeEditor.get());

    m_startRunningButton = std::make_unique<juce::DrawableButton>("RunTC", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground);
    m_startRunningButton->setTooltip("Start running timecode.");
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

    m_triggerCurrentTCButton = std::make_unique<juce::DrawableButton>("Trigger current TC", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground);
    m_triggerCurrentTCButton->setTooltip("Trigger current timecode once.");
    m_triggerCurrentTCButton->onClick = [=]() { sendMessage(); };
    addAndMakeVisible(m_triggerCurrentTCButton.get());

    m_customTriggersGrid.rowGap.pixels = sc_customTriggersGrid_NodeGap;
    m_customTriggersGrid.columnGap.pixels = sc_customTriggersGrid_NodeGap;
    for (int i = 0; i < sc_customTriggersGrid_ColCount; i++)
        m_customTriggersGrid.templateColumns.add(juce::Grid::TrackInfo(juce::Grid::Fr(1)));
    for (int i = 0; i < sc_customTriggersGrid_RowCount; i++)
        m_customTriggersGrid.templateRows.add(juce::Grid::TrackInfo(juce::Grid::Fr(1)));
    ResetCustomTriggers();

    updateAvailableDevices();

    connectToOscSocket();

    // do the initial update for the whole application with config contents
    m_config->triggerWatcherUpdate();
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

    if (bounds.isEmpty() || !m_optionsButton || !m_aboutButton || !m_startRunningButton || !m_timecodeEditor || !m_triggerCurrentTCButton)
        return;

    auto headerBounds = bounds.removeFromTop(40);
    m_optionsButton->setBounds(headerBounds.removeFromLeft(headerBounds.getHeight()).reduced(1));
    m_aboutButton->setBounds(headerBounds.removeFromRight(headerBounds.getHeight()).reduced(2));

    bounds.removeFromTop(1);

    auto valueBounds = bounds.removeFromTop(44);
    m_triggerCurrentTCButton->setBounds(valueBounds.removeFromRight(valueBounds.getHeight()).reduced(1));
    m_startRunningButton->setBounds(valueBounds.removeFromRight(valueBounds.getHeight()).reduced(1));
    m_timecodeEditor->setBounds(valueBounds.reduced(1));

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
    auto optionsButtonDrawable = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(BinaryData::menu_24dp_svg).get());
    optionsButtonDrawable->replaceColour(juce::Colours::black, getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));
    m_optionsButton->setImages(optionsButtonDrawable.get());

    auto aboutButtonDrawable = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(BinaryData::question_mark_24dp_svg).get());
    aboutButtonDrawable->replaceColour(juce::Colours::black, getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));
    m_aboutButton->setImages(aboutButtonDrawable.get());

    auto startRunningButtonDrawable = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(BinaryData::play_arrow24px_svg).get());
    startRunningButtonDrawable->replaceColour(juce::Colours::black, getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));
    m_startRunningButton->setImages(startRunningButtonDrawable.get());

    auto triggerCurrentTCButtonDrawable = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(BinaryData::Jumper_svg).get());
    triggerCurrentTCButtonDrawable->replaceColour(juce::Colours::black, getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));
    m_triggerCurrentTCButton->setImages(triggerCurrentTCButtonDrawable.get());
}

void JumperComponent::setAndSendTimeCode(TimeStamp ts)
{
    m_ts = ts;

    setStartMilliseconds();

    sendMessage();
}

void JumperComponent::connectToOscSocket()
{
    if (m_oscServer)
    {
        m_oscServer->disconnect();
        m_oscServer->removeListener(this);
        m_oscServer.reset();
    }

    m_oscServer = std::make_unique<juce::OSCReceiver>();
    m_oscServer->addListener(this);
    if (!m_oscServer->connect(getOscPortNumber()))
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "Error", juce::String("OSCReceiver: connecting to port ") + juce::String(getOscPortNumber()) + " failed.");
}

void JumperComponent::oscMessageReceived(const OSCMessage& message)
{
    DBG(juce::String(__FUNCTION__) << " " << message.getAddressPattern().toString());

    if (message.getAddressPattern().matches(juce::OSCAddress(juce::String("/") + juce::JUCEApplication::getInstance()->getApplicationName() + "/TS")) 
        && 1 == message.size()
        && juce::OSCTypes::string == message.begin()->getType())
    {
        auto ts = TimeStamp(message.begin()->getString());
        if (ts.isValid())
            setAndSendTimeCode(ts);
    }
    else
    {
        for (auto const& ct : m_customTriggers)
        {
            auto& customTriggerDetails = ct.second->getTriggerDetails();
            auto customTriggerOSCAddress = juce::OSCAddress(customTriggerDetails.m_oscTrigger.getAddressPattern().toString());
            if (message.getAddressPattern().matches(customTriggerOSCAddress) && customTriggerDetails.m_TS.isValid())
                setAndSendTimeCode(ct.second->getTriggerDetails().m_TS);
        }
    }
}

void JumperComponent::performConfigurationDump()
{
    if (m_config)
    {
        auto stateXml = m_config->getConfigState();

        if (stateXml)
        {
            auto deviceConfigXml = std::make_unique<juce::XmlElement>(JumperConfiguration::getTagName(JumperConfiguration::TagID::DEVCONFIG));
            deviceConfigXml->setAttribute(JumperConfiguration::getAttributeName(JumperConfiguration::AttributeID::FRAMERATE), juce::String(m_frameRate));
            deviceConfigXml->setAttribute(JumperConfiguration::getAttributeName(JumperConfiguration::AttributeID::OSCPORT), juce::String(m_oscPortNumber));
            auto midiOutputXml = std::make_unique<juce::XmlElement>(JumperConfiguration::getTagName(JumperConfiguration::TagID::MIDIOUTPUT));
            if (m_midiOutput) midiOutputXml->addTextElement(m_midiOutput->getIdentifier());
            deviceConfigXml->addChildElement(midiOutputXml.release());
            m_config->setConfigState(std::move(deviceConfigXml), JumperConfiguration::getTagName(JumperConfiguration::TagID::DEVCONFIG));

            auto customTriggersXml = std::make_unique<juce::XmlElement>(JumperConfiguration::getTagName(JumperConfiguration::TagID::CUSTOMTRIGGERS));
            for (auto const& customTrigger : m_customTriggers)
            {
                if (nullptr == customTrigger.second || !customTrigger.second->isEnabled() || customTrigger.second->getTriggerDetails().isEmpty())
                    continue;
                auto triggerDetailsXml = std::make_unique<juce::XmlElement>(JumperConfiguration::getTagName(JumperConfiguration::TagID::TRIGGERDETAILS));
                triggerDetailsXml->setAttribute(JumperConfiguration::getAttributeName(JumperConfiguration::AttributeID::IDENT), juce::String(customTrigger.first));
                triggerDetailsXml->addTextElement(customTrigger.second->getTriggerDetails().toString());
                customTriggersXml->addChildElement(triggerDetailsXml.release());
            }
            m_config->setConfigState(std::move(customTriggersXml), JumperConfiguration::getTagName(JumperConfiguration::TagID::CUSTOMTRIGGERS));
        }
    }
}

void JumperComponent::onConfigUpdated()
{
    auto deviceConfigXml = m_config->getConfigState(JumperConfiguration::getTagName(JumperConfiguration::TagID::DEVCONFIG));
    if (deviceConfigXml)
    {
        m_frameRate = deviceConfigXml->getIntAttribute(JumperConfiguration::getAttributeName(JumperConfiguration::AttributeID::FRAMERATE));

        m_oscPortNumber = deviceConfigXml->getIntAttribute(JumperConfiguration::getAttributeName(JumperConfiguration::AttributeID::OSCPORT));
        
        auto midiDeviceIdentifier = deviceConfigXml->getChildElementAllSubText(JumperConfiguration::getTagName(JumperConfiguration::TagID::MIDIOUTPUT), "");
        if (midiDeviceIdentifier.isNotEmpty())
        {
            for (auto i = 0; i <= m_optionsItems.size() - JumperOptionsOption::OutputDevice; i++)
                m_optionsItems[JumperOptionsOption::OutputDevice + i].second = m_optionsItems[JumperOptionsOption::OutputDevice + i].first == midiDeviceIdentifier;
            openMidiDevice(midiDeviceIdentifier);
        }
        else
            m_midiOutput.reset();
    }

    auto customTriggersXml = m_config->getConfigState(JumperConfiguration::getTagName(JumperConfiguration::TagID::CUSTOMTRIGGERS));
    if (customTriggersXml)
    {
        if (nullptr == customTriggersXml->getChildByName(JumperConfiguration::getTagName(JumperConfiguration::TagID::TRIGGERDETAILS)))
            ResetCustomTriggers();
        for (auto* elm : customTriggersXml->getChildWithTagNameIterator(JumperConfiguration::getTagName(JumperConfiguration::TagID::TRIGGERDETAILS)))
        {
            if (nullptr != elm)
            {
                auto key = elm->getIntAttribute(JumperConfiguration::getAttributeName(JumperConfiguration::AttributeID::IDENT));
                if (0 < m_customTriggers.count(key) && nullptr != m_customTriggers.at(key) && !m_customTriggers.at(key)->getTriggerDetails().isEmpty())
                    m_customTriggers[key]->setTriggerDetails(CustomTriggerButton::TriggerDetails::fromString(elm->getAllSubText()));
            }
        }
    }
    else
        ResetCustomTriggers();
}


//==============================================================================

void JumperComponent::handleOptionsMenuResult(int selectedId)
{
    if (0 == selectedId)
        return; // nothing selected, dismiss
    else if (JumperOptionsOption::ResetConfig == selectedId)
    {
        if (m_config)
            m_config->ResetToDefault();
        resized();
        lookAndFeelChanged();
    }
    else if (JumperOptionsOption::LookAndFeel_First <= selectedId && JumperOptionsOption::LookAndFeel_Last >= selectedId)
        handleOptionsLookAndFeelMenuResult(selectedId);
    else if (JumperOptionsOption::OscPort == selectedId)
        handleOptionsOscPortMenuResult();
    else if (JumperOptionsOption::FrameRate == selectedId)
        handleOptionsFramerateMenuResult();
    else if (JumperOptionsOption::OutputDevice <= selectedId)
        handleOptionsOutputDeviceSelectionMenuResult(selectedId);
    else
        jassertfalse; // unhandled menu entry!?
}

void JumperComponent::handleOptionsLookAndFeelMenuResult(int selectedId)
{
    // helper internal function to avoid code clones
    std::function<void(int, int, int)> setSettingsItemsCheckState = [=](int a, int b, int c) {
        m_optionsItems[JumperOptionsOption::LookAndFeel_FollowHost].second = a;
        m_optionsItems[JumperOptionsOption::LookAndFeel_Dark].second = b;
        m_optionsItems[JumperOptionsOption::LookAndFeel_Light].second = c;
        };

    switch (selectedId)
    {
    case JumperOptionsOption::LookAndFeel_FollowHost:
        setSettingsItemsCheckState(1, 0, 0);
        if (onPaletteStyleChange)
            onPaletteStyleChange(-1, true);
        break;
    case JumperOptionsOption::LookAndFeel_Dark:
        setSettingsItemsCheckState(0, 1, 0);
        if (onPaletteStyleChange)
            onPaletteStyleChange(JUCEAppBasics::CustomLookAndFeel::PS_Dark, false);
        break;
    case JumperOptionsOption::LookAndFeel_Light:
        setSettingsItemsCheckState(0, 0, 1);
        if (onPaletteStyleChange)
            onPaletteStyleChange(JUCEAppBasics::CustomLookAndFeel::PS_Light, false);
        break;
    default:
        jassertfalse; // unknown id fed in unintentionally ?!
        break;
    }
}

void JumperComponent::handleOptionsOscPortMenuResult()
{
    m_messageBox = std::make_unique<juce::AlertWindow>("OSC port", "OSC port to open\nand listen on for incoming data:", juce::MessageBoxIconType::NoIcon);
    m_messageBox->addTextEditor("OSC port", juce::String(getOscPortNumber()));
    m_messageBox->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
    m_messageBox->addButton("Ok", 1, juce::KeyPress(juce::KeyPress::returnKey));
    m_messageBox->enterModalState(true, juce::ModalCallbackFunction::create([=](int returnValue) {
        if (returnValue == 1)
        {
            setOscPortNumber(m_messageBox->getTextEditorContents("OSC port").getIntValue());

            if (m_config)
                m_config->triggerConfigurationDump();
        }

        m_messageBox.reset();
    }));
}

void JumperComponent::handleOptionsFramerateMenuResult()
{
    m_messageBox = std::make_unique<juce::AlertWindow>("Framerate", "Framerate to use\nfor timecode generation:", juce::MessageBoxIconType::NoIcon);
    m_messageBox->addComboBox("Framerate", { "24fps", "25fps", "29.97fps", "30fps" });
    m_messageBox->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
    m_messageBox->addButton("Ok", 1, juce::KeyPress(juce::KeyPress::returnKey));
    m_messageBox->enterModalState(true, juce::ModalCallbackFunction::create([=](int returnValue) {
        if (returnValue == 1)
        {
            if (auto cb = m_messageBox->getComboBoxComponent("Framerate"))
            {
                if (!FramerateFromString(cb->getItemText(cb->getSelectedItemIndex()).removeCharacters("fps")))
                    juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "Error", "Invalid framerate specified");
                else if (m_config)
                    m_config->triggerConfigurationDump();
            }
        }

        m_messageBox.reset();
        }));
}

void JumperComponent::handleOptionsOutputDeviceSelectionMenuResult(int selectedId)
{
    int selectedDeviceIdx = selectedId - JumperOptionsOption::OutputDevice;
    if (JumperOptionsOption::OutputDevice + selectedDeviceIdx <= m_optionsItems.size() && selectedDeviceIdx <= m_currentMidiOutputDevicesInfos.size())
    {
        for (auto i = 0; i <= m_optionsItems.size() - JumperOptionsOption::OutputDevice; i++)
            m_optionsItems[JumperOptionsOption::OutputDevice + i].second = i == selectedDeviceIdx;
        auto midiOutputIdentifier = m_currentMidiOutputDevicesInfos[selectedDeviceIdx].identifier;
        openMidiDevice(midiOutputIdentifier);

        if (m_config)
            m_config->triggerConfigurationDump();
    }
    else
    {
        jassertfalse;
    }
}

void JumperComponent::updateAvailableDevices()
{
    m_currentMidiOutputDevicesInfos = juce::MidiOutput::getAvailableDevices();
    for (int i = 0; i < m_currentMidiOutputDevicesInfos.size(); i++)
    {
        m_optionsItems[JumperOptionsOption::OutputDevice + i].first = m_currentMidiOutputDevicesInfos[i].name.toStdString();
        m_optionsItems[JumperOptionsOption::OutputDevice + i].second = int(false);
    }
}

void JumperComponent::openMidiDevice(const juce::String& deviceIdentifier)
{
    m_midiOutput = juce::MidiOutput::openDevice(deviceIdentifier);
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

bool JumperComponent::FramerateFromString(const juce::String& framerateStr)
{
    auto frameRate = framerateStr.getDoubleValue();
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

juce::String JumperComponent::FramerateToString(int framerateIdent)
{
    switch (framerateIdent)
    {
    case 0:
        return "24";
    case 1:
        return "25";
    case 2:
        return "29.97";
    case 3:
        return "30";
    default:
        return "";
    }
}

double JumperComponent::getCurrentFrameRateHz()
{
    switch (m_frameRate)
    {
    case 0:
        return 24;
    case 1:
        return 25;
    case 2:
        return 29.97;
    case 3:
    default:
        return 30;
    }
}

int JumperComponent::getCurrentFrameIntervalMs()
{
    return int(1000.0 / getCurrentFrameRateHz());
}

int JumperComponent::getOscPortNumber()
{
    return m_oscPortNumber;
}

void JumperComponent::setOscPortNumber(int portNumber)
{
    m_oscPortNumber = portNumber;

    connectToOscSocket();
}

void JumperComponent::ResetCustomTriggers()
{
    m_customTriggersGrid.items.clear();

    for (int i = 0; i < sc_customTriggersGrid_RowCount * sc_customTriggersGrid_ColCount; i++)
    {
        m_customTriggers[i] = std::make_unique<CustomTriggerButton>(juce::String("CT") + juce::String(i));
        m_customTriggers[i]->onTriggerClicked = [=](const TimeStamp& ts) { setAndSendTimeCode(ts); };
        m_customTriggers[i]->onDetailsChanged = [=](const CustomTriggerButton::TriggerDetails&) { if (m_config) m_config->triggerConfigurationDump(false); };
        addAndMakeVisible(m_customTriggers[i].get());
        m_customTriggersGrid.items.add(m_customTriggers[i].get());
    }
}


};
