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

#include "CustomTriggerButton.h"


CustomTriggerButton::CustomTriggerButton(const juce::String& buttonName)
    : juce::Button(buttonName)
{
    setEnabled(false);

    m_nameLabel = std::make_unique<juce::Label>("Name", buttonName);
    m_nameLabel->setJustificationType(juce::Justification::centred);
    m_nameLabel->setInterceptsMouseClicks(false, false);
    m_tsLabel = std::make_unique<juce::Label>("TS");
    m_tsLabel->setJustificationType(juce::Justification::centred);
    m_tsLabel->setInterceptsMouseClicks(false, false);
    m_settingsButton = std::make_unique<juce::DrawableButton>("SettingsButton", juce::DrawableButton::ButtonStyle::ImageFitted);
    m_settingsButton->setColour(juce::DrawableButton::ColourIds::backgroundColourId, juce::Colours::transparentBlack);
    m_settingsButton->setColour(juce::DrawableButton::ColourIds::backgroundOnColourId, juce::Colours::transparentBlack);
    m_settingsButton->onClick = [=]() { showTriggerSettings(); };
}

CustomTriggerButton::~CustomTriggerButton()
{
}

void CustomTriggerButton::setTriggerDetails(const TriggerDetails& triggerDetails)
{
    if (!isEnabled())
        setEnabled(true);

    m_triggerDetails = triggerDetails;

    setColour(juce::TextButton::ColourIds::buttonColourId, m_triggerDetails.m_Colour);
    setColour(juce::TextButton::ColourIds::buttonOnColourId, m_triggerDetails.m_Colour.brighter());

    m_nameLabel->setText(m_triggerDetails.m_Name, juce::dontSendNotification);
    m_nameLabel->setFont(m_nameLabel->getFont().withPointHeight(30));
    addAndMakeVisible(m_nameLabel.get());

    m_tsLabel->setText(m_triggerDetails.m_TS.toString(), juce::dontSendNotification);
    addAndMakeVisible(m_tsLabel.get());

    addAndMakeVisible(m_settingsButton.get());

    resized();
}

void CustomTriggerButton::clicked()
{
    if (isEnabled())
    {
        if (onTriggerClicked)
            onTriggerClicked(m_triggerDetails.m_TS);
    }

    juce::Button::clicked();
}

void CustomTriggerButton::mouseDown(const juce::MouseEvent& e)
{
    if (!isEnabled())
    {
        m_triggerDetails = TriggerDetails(getName(), juce::Colours::cornflowerblue, TimeStamp());
        showTriggerSettings();
    }

    juce::Button::mouseDown(e);
}

void CustomTriggerButton::paint(juce::Graphics& g)
{
    juce::Button::paint(g);

    if (!isEnabled() && m_addDrawable)
        m_addDrawable->drawWithin(g, getLocalBounds().reduced(15).toFloat(), juce::RectanglePlacement::centred, 0.6f);
}

void CustomTriggerButton::paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto& lf = getLookAndFeel();
    lf.drawButtonBackground(g, *this,
        findColour(getToggleState() ? juce::TextButton::buttonOnColourId : juce::TextButton::buttonColourId),
        shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
}

void CustomTriggerButton::resized()
{
    if (m_nameLabel && m_tsLabel && m_settingsButton)
    {
        auto bounds = getLocalBounds().reduced(5);

        m_tsLabel->setBounds(bounds.removeFromBottom(15));
        m_nameLabel->setBounds(bounds);

        m_settingsButton->setBounds(getLocalBounds().removeFromTop(25).removeFromRight(25));
    }
}

void CustomTriggerButton::lookAndFeelChanged()
{
    m_addDrawable = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(BinaryData::add24px_svg).get());
    m_addDrawable->replaceColour(juce::Colours::black, getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));

    auto settingsDrawable = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(BinaryData::settings24px_svg).get());
    settingsDrawable->replaceColour(juce::Colours::black, getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));
    if (m_settingsButton) m_settingsButton->setImages(settingsDrawable.get());
}

void CustomTriggerButton::showTriggerSettings()
{
    class TriggerSettingsComponent : public juce::Component
    {
    public:
        TriggerSettingsComponent(const TriggerDetails& td)
        {
            m_nameEdit = std::make_unique<juce::TextEditor>();
            m_nameEdit->setText(td.m_Name);
            addAndMakeVisible(m_nameEdit.get());

            m_tcEdit = std::make_unique<juce::TextEditor>();
            m_tcEdit->setText(td.m_TS.toString());
            addAndMakeVisible(m_tcEdit.get());

            m_colourSelector = std::make_unique<juce::ColourSelector>();
            m_colourSelector->setName("Colour");
            m_colourSelector->setCurrentColour(td.m_Colour);
            m_colourSelector->setColour(juce::ColourSelector::backgroundColourId, Colours::transparentBlack);
            addAndMakeVisible(m_colourSelector.get());

            m_closeButton = std::make_unique<juce::TextButton>("CloseButton");
            m_closeButton->setButtonText("Close");
            m_closeButton->onClick = [=]() { findParentComponentOfClass<CallOutBox>()->exitModalState(0); };
            addAndMakeVisible(m_closeButton.get());

            setSize(300, 500);
        };
        ~TriggerSettingsComponent() override
        {
            if (onFinished)
                onFinished(TriggerDetails(m_nameEdit->getText(), m_colourSelector->getCurrentColour(), TimeStamp(m_tcEdit->getText())));
        }

        void resized() override
        {
            auto bounds = getLocalBounds();

            m_nameEdit->setBounds(bounds.removeFromTop(30).reduced(5));
            m_tcEdit->setBounds(bounds.removeFromTop(30).reduced(5));
            m_closeButton->setBounds(bounds.removeFromBottom(30).removeFromRight(bounds.getWidth() / 2).reduced(1));
            bounds.removeFromBottom(20);
            m_colourSelector->setBounds(bounds.removeFromBottom(400));
        }

        std::function<void(const TriggerDetails&)>  onFinished;

    private:
        std::unique_ptr<juce::TextEditor>       m_nameEdit;
        std::unique_ptr<juce::TextEditor>       m_tcEdit;
        std::unique_ptr<juce::ColourSelector>   m_colourSelector;
        std::unique_ptr<juce::TextButton>       m_closeButton;
    };


    auto tsc = std::make_unique<TriggerSettingsComponent>(m_triggerDetails);
    tsc->onFinished = [=](const TriggerDetails& td) { setTriggerDetails(td); repaint(); };

    juce::CallOutBox::launchAsynchronously(std::move(tsc), ((isEnabled() && m_settingsButton) ? m_settingsButton->getScreenBounds() : getScreenBounds()), nullptr);
}