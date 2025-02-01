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

#pragma once

#include <JuceHeader.h>

class TimeStamp
{
public:
    TimeStamp() {};
    TimeStamp(int hours, int minutes, int seconds, int frames) { m_hours = hours; m_minutes = minutes; m_seconds = seconds; m_frames = frames; };
    ~TimeStamp() {};

    bool operator==(const TimeStamp& other) const { return m_hours == other.getHours() && m_minutes == other.getMinutes() && m_seconds == other.getSeconds() && m_frames == other.getFrames(); };
    bool operator!=(const TimeStamp& other) const { return !(*this == other); };

    void setHours(int hours) { m_hours = hours; };
    void setMinutes(int minutes) { m_minutes = minutes; };
    void setSeconds(int seconds) { m_seconds = seconds; };
    void setFrames(int frames) { m_frames = frames; };

    int getHours() const { return m_hours; };
    int getMinutes() const { return m_minutes; };
    int getSeconds() const { return m_seconds; };
    int getFrames() const { return m_frames; };

    void clear() { m_hours = 0; m_minutes = 0; m_seconds = 0; m_frames = 0; };

private:
    int m_hours = 0;
    int m_minutes = 0;
    int m_seconds = 0;
    int m_frames = 0;
};

class CustomTriggerButton  : public juce::Button
{
public:
    //==============================================================================
    CustomTriggerButton(const juce::String& buttonName);
    ~CustomTriggerButton() override;

    void clicked() override;

    //==============================================================================
    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void resized() override;

    //==============================================================================
    std::function<void(const TimeStamp&)>   onTriggerClicked;

private:
    //==============================================================================
    TimeStamp m_triggerTS;

    bool m_configured = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomTriggerButton)
};

