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

/**
 * @class JumpyConfiguration
 * @brief XML-backed persistent configuration for the Jumpy application.
 *
 * Extends `JUCEAppBasics::AppConfigurationBase` to add Jumpy-specific schema
 * validation and version-conflict handling.  The on-disk file is a versioned XML
 * document with the following top-level structure:
 * ```xml
 * <ROOT version="1.0.0">
 *   <DEVICECONFIG FRAMERATE="1" OSCPORT="53000">
 *     <MIDIINPUT>device-identifier</MIDIINPUT>
 *     <MIDIOUTPUT>device-identifier</MIDIOUTPUT>
 *   </DEVICECONFIG>
 *   <CUSTOMTRIGGERS>
 *     <TRIGGERDETAILS IDENT="0">name;colourHex;hh:mm:ss:ff;/osc/address;midiHex</TRIGGERDETAILS>
 *     ...
 *   </CUSTOMTRIGGERS>
 * </ROOT>
 * ```
 * A built-in default configuration is embedded as a binary resource (`Default_config`)
 * and is loaded when no valid file exists on disk or when the user selects
 * *Reset configuration* from the options menu.
 *
 * If the on-disk version differs from `JUMPY_CONFIG_VERSION`, the user is offered the
 * choice to reset to defaults or quit (see `HandleConfigVersionConflict()`).
 */
class JumpyConfiguration : public JUCEAppBasics::AppConfigurationBase
{

public:
    /**
     * @enum TagID
     * @brief Symbolic identifiers for XML element tag names used in the config file.
     */
    enum TagID
    {
        DEVCONFIG,       ///<  Root device-settings section (`<DEVICECONFIG>`).
        MIDIINPUT,       ///<  Text element containing the MIDI input device identifier (`<MIDIINPUT>`).
        MIDIOUTPUT,      ///<  Text element containing the MIDI output device identifier (`<MIDIOUTPUT>`).
        CUSTOMTRIGGERS,  ///<  Container element for all configured trigger buttons (`<CUSTOMTRIGGERS>`).
        TRIGGERDETAILS   ///<  Per-button data element, one per configured slot (`<TRIGGERDETAILS>`).
    };

    /**
     * @brief Returns the XML tag name string for a given TagID.
     * @param ID  Tag identifier to look up.
     * @return Corresponding tag name string, e.g. `"DEVICECONFIG"`.
     */
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

    /**
     * @enum AttributeID
     * @brief Symbolic identifiers for XML attribute names used in the config file.
     */
    enum AttributeID
    {
        IDENT,      ///<  Integer index identifying a trigger button slot (`IDENT`).
        FRAMERATE,  ///<  MTC frame-rate index stored on `<DEVICECONFIG>` (`FRAMERATE`).
        OSCPORT     ///<  OSC UDP port number stored on `<DEVICECONFIG>` (`OSCPORT`).
    };

    /**
     * @brief Returns the XML attribute name string for a given AttributeID.
     * @param ID  Attribute identifier to look up.
     * @return Corresponding attribute name string, e.g. `"FRAMERATE"`.
     */
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
    /**
     * @brief Constructs the configuration object and initialises it from the given file.
     * @param file  Path to the on-disk XML configuration file.
     */
    explicit JumpyConfiguration(const File &file);
    ~JumpyConfiguration() override;

    /**
     * @brief Returns true if the in-memory XML state passes Jumpy-specific validation.
     *
     * Checks that both `<DEVICECONFIG>` and `<CUSTOMTRIGGERS>` sections are present,
     * in addition to the base-class version check.
     *
     * @return `true` if the configuration is structurally valid.
     */
    bool isValid() override;

    /**
     * @brief Validates an arbitrary XML element against the Jumpy config schema.
     *
     * Used during `ResetToDefault()` to verify the embedded binary resource before
     * applying it, without modifying `m_xml`.
     *
     * @param xmlConfig  XML element to validate.
     * @return `true` if the element passes both base-class and Jumpy-specific checks.
     */
    static bool isValid(const std::unique_ptr<juce::XmlElement>& xmlConfig);

    /**
     * @brief Replaces the current configuration with the built-in default and flushes to disk.
     *
     * Parses `BinaryData::Default_config`, validates it, and applies it atomically with
     * flush/update suppressed during the operation to avoid partial-update side effects.
     * Falls back to `triggerConfigurationDump()` if the embedded resource is invalid.
     *
     * @return `true` if the reset succeeded; `false` if the embedded resource was invalid.
     */
    bool ResetToDefault();

protected:
    /**
     * @brief Handles a version mismatch between the on-disk config and `JUMPY_CONFIG_VERSION`.
     *
     * Presents a modal dialog offering the user a choice between resetting to defaults or
     * quitting the application.  Returns `false` immediately so the base class knows the
     * conflict is not silently accepted; the user's choice is handled asynchronously.
     *
     * @param configVersionFound  Version string read from the on-disk file.
     * @return `false` (conflict not auto-resolved; user must choose).
     */
    bool HandleConfigVersionConflict(const Version& configVersionFound) override;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JumpyConfiguration)
};

} // namespace Jumpy
