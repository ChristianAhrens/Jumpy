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


namespace Jumpy
{

class JumpyComponent;

/**
 * @class MainComponent
 * @brief Top-level JUCE component that hosts JumpyComponent and initialises application-wide services.
 *
 * MainComponent owns a single JumpyComponent that fills the entire available area,
 * respecting iOS/Android safe-area insets computed by `JUCEAppBasics::iOS_utils`.
 * It also creates the process-wide `juce::TooltipWindow` singleton required by JUCE
 * for tooltip rendering.
 *
 * On non-iOS and non-Android builds the constructor additionally starts
 * `JUCEAppBasics::WebUpdateDetector`, which checks GitHub for a newer release and
 * notifies the user if one is found.
 *
 * The `getOnPaletteStyleChangeCallback()` accessor tunnels the LookAndFeel-change
 * callback from JumpyComponent up to `MainWindow` so that palette switches selected
 * in the options menu propagate to the global `juce::LookAndFeel`.
 */
class MainComponent  : public juce::Component
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    /** @brief Fills the background with the current theme background colour. */
    void paint (juce::Graphics&) override;
    /** @brief Sizes JumpyComponent to fill the safe-area-adjusted local bounds. */
    void resized() override;

    //==============================================================================
    /**
     * @brief Returns a reference to the palette-style change callback.
     *
     * Forwards the callback from JumpyComponent if available; otherwise returns the
     * component's own (unused) fallback member.  The returned reference is stored by
     * `MainWindow` to receive palette-change notifications from the options menu.
     *
     * @return Reference to the `std::function<void(int, bool)>` callback.
     */
    std::function<void(int, bool)>& getOnPaletteStyleChangeCallback();

    //========================================================================*
    /** Fallback palette-style change callback used when JumpyComponent is unavailable. */
    std::function<void(int, bool)> onPaletteStyleChange;

private:
    //==============================================================================
    std::unique_ptr<JumpyComponent>    m_JumpyComponent;          ///<  The single operational UI component filling the window.
    std::unique_ptr<TooltipWindow>      m_toolTipWindowInstance;  ///<  Process-wide tooltip window; exactly one instance must exist.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

};
