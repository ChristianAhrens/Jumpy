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
    TimeStamp(const juce::String& tsString) { *this = fromString(tsString); };
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

    const bool isValid() const {
        return m_hours >= 0 && m_hours < 24 && m_minutes >= 0 && m_minutes < 60 && m_seconds >= 0 && m_seconds < 60 && m_frames >= 0 && m_frames <= 30;
    };

    const juce::String toString() const {
        juce::StringArray timeDigits = {
            juce::String(getHours()).paddedLeft('0', 2),
            juce::String(getMinutes()).paddedLeft('0', 2),
            juce::String(getSeconds()).paddedLeft('0', 2),
            juce::String(getFrames()).paddedLeft('0', 2)
        };
        return timeDigits.joinIntoString(":");
    };

    static TimeStamp fromString(const juce::String& tsString)
    {
        auto success = true;

        juce::StringArray time;
        time.addTokens(tsString, ":", "");
        if (time.size() != 4)
            success = false;

        auto ts = TimeStamp(
            time[0].getIntValue(),
            time[1].getIntValue(),
            time[2].getIntValue(),
            time[3].getIntValue());
        success = success && ts.isValid();

        if (!success)
        {
            juce::AlertWindow::showAsync(
                juce::MessageBoxOptions()
                    .withMessage("Invalid Timecode value " + tsString + ".")
                    .withButton("Ok")
                    .withIconType(juce::MessageBoxIconType::WarningIcon), 
                nullptr);
            return {};
        }
        else
            return ts;
    };

private:
    int m_hours = 0;
    int m_minutes = 0;
    int m_seconds = 0;
    int m_frames = 0;
};

class CustomTriggerButton  : public juce::Button
{
public:
    struct TriggerDetails
    {
        TriggerDetails() = default;
        TriggerDetails(const juce::String& name, const juce::Colour& colour, const TimeStamp& timestamp, const juce::OSCMessage& oscTrigger)
        {
            m_Name = name;
            m_Colour = colour;
            m_TS = timestamp;
            m_oscTrigger = oscTrigger;
        }

        juce::String        m_Name;
        juce::Colour        m_Colour;
        TimeStamp           m_TS;
        juce::OSCMessage    m_oscTrigger = { juce::OSCAddressPattern("/a/b"), 0 };
    };

public:
    //==============================================================================
    CustomTriggerButton(const juce::String& buttonName);
    ~CustomTriggerButton() override;

    void setTriggerDetails(const TriggerDetails& triggerDetails);
    const TriggerDetails& getTriggerDetails() const;

    //==============================================================================
    void clicked() override;

    //==============================================================================
    void mouseDown(const juce::MouseEvent& e) override;

    //==============================================================================
    void paint(juce::Graphics& g) override;
    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void resized() override;
    void lookAndFeelChanged() override;

    //==============================================================================
    std::function<void(const TimeStamp&)>   onTriggerClicked;

private:
    //==============================================================================
    void showTriggerSettings();
    
    //==============================================================================
    TriggerDetails  m_triggerDetails;

    std::unique_ptr<juce::Drawable> m_addDrawable;

    std::unique_ptr<juce::Label>            m_nameLabel;
    std::unique_ptr<juce::Label>            m_tsLabel;
    std::unique_ptr<juce::DrawableButton>   m_settingsButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomTriggerButton)
};

