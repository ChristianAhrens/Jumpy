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

#include "MainComponent.h"

#include "JumpyComponent.h"

#include <CustomLookAndFeel.h>
#include <WebUpdateDetector.h>
#include <iOS_utils.h>


namespace Jumpy
{

MainComponent::MainComponent()
{
    // a single instance of tooltip window is required and used by JUCE everywhere a tooltip is required.
    m_toolTipWindowInstance = std::make_unique<TooltipWindow>();

    m_JumpyComponent = std::make_unique<JumpyComponent>();
    addAndMakeVisible(m_JumpyComponent.get());
    
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
    updater->SetDownloadUpdateWebAddress("https://github.com/christianahrens/Jumpy/releases/latest");
    updater->CheckForNewVersion(true, "https://raw.githubusercontent.com/ChristianAhrens/Jumpy/refs/heads/main/");
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
    
    m_JumpyComponent->setBounds(safeBounds);
}

std::function<void(int, bool)>& MainComponent::getOnPaletteStyleChangeCallback()
{
    if (m_JumpyComponent)
        return m_JumpyComponent->onPaletteStyleChange;
    else
        return onPaletteStyleChange;
}


}

