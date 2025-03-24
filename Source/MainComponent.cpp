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

#include "MainComponent.h"

#include "JumperComponent.h"

#include <CustomLookAndFeel.h>
#include <WebUpdateDetector.h>
#include <iOS_utils.h>


namespace Jumper
{

MainComponent::MainComponent()
{
    m_jumperComponent = std::make_unique<JumperComponent>();
    addAndMakeVisible(m_jumperComponent.get());
    
    setSize (300, 600);

#if defined JUCE_IOS
    // iOS is updated via AppStore
#define IGNORE_UPDATES
#elif defined JUCE_ANDROID
    // Android as well
#define IGNORE_UPDATES
#endif

#if defined IGNORE_UPDATES
#else
    auto updater = JUCEAppBasics::WebUpdateDetector::getInstance();
    updater->SetReferenceVersion(ProjectInfo::versionString);
    updater->SetDownloadUpdateWebAddress("https://github.com/christianahrens/Jumper/releases/latest");
    updater->CheckForNewVersion(true, "https://raw.githubusercontent.com/ChristianAhrens/Jumper/refs/heads/main/");
#endif
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
    auto safety = JUCEAppBasics::iOS_utils::getDeviceSafetyMargins();
    auto safeBounds = getLocalBounds();
#if JUCE_IOS
    safeBounds.reduce(3, 3);
#endif
    safeBounds.removeFromTop(safety._top);
    safeBounds.removeFromBottom(safety._bottom);
    safeBounds.removeFromLeft(safety._left);
    safeBounds.removeFromRight(safety._right);
    
    m_jumperComponent->setBounds(safeBounds);
}

std::function<void(int, bool)>& MainComponent::getOnPaletteStyleChangeCallback()
{
    if (m_jumperComponent)
        return m_jumperComponent->onPaletteStyleChange;
    else
        return onPaletteStyleChange;
}


}

