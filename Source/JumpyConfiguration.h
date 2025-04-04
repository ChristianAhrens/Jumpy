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

#include <AppConfigurationBase.h>

#define JUMPY_CONFIG_VERSION "1.0.0"

namespace Jumpy
{

class JumpyConfiguration : public JUCEAppBasics::AppConfigurationBase
{

public:
    enum TagID
    {
        DEVCONFIG,
        MIDIINPUT,
        MIDIOUTPUT,
        CUSTOMTRIGGERS,
        TRIGGERDETAILS
    };
    static juce::String getTagName(TagID ID)
    {
        switch(ID)
        {
        case DEVCONFIG:
            return "DEVICECONFIG";
        case MIDIINPUT:
            return "MIDIINPUT";
        case MIDIOUTPUT:
            return "MIDIOUTPUT";
        case CUSTOMTRIGGERS:
            return "CUSTOMTRIGGERS";
        case TRIGGERDETAILS:
            return "TRIGGERDETAILS";
        default:
            return "INVALID";
        }
    };

    enum AttributeID
    {
        IDENT,
        FRAMERATE,
        OSCPORT
    };
    static juce::String getAttributeName(AttributeID ID)
    {
        switch (ID)
        {
        case IDENT:
            return "IDENT";
        case FRAMERATE:
            return "FRAMERATE";
        case OSCPORT:
            return "OSCPORT";
        default:
            return "-";
        }
    };

public:
    explicit JumpyConfiguration(const File &file);
    ~JumpyConfiguration() override;

    bool isValid() override;
    static bool isValid(const std::unique_ptr<juce::XmlElement>& xmlConfig);

    bool ResetToDefault();

protected:
    bool HandleConfigVersionConflict(const Version& configVersionFound) override;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JumpyConfiguration)
};

} // namespace Jumpy
