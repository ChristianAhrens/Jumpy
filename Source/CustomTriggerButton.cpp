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
}

CustomTriggerButton::~CustomTriggerButton()
{
}

void CustomTriggerButton::clicked()
{
    if (m_configured)
    {
        if (onTriggerClicked)
            onTriggerClicked(m_triggerTS);
    }
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

}

