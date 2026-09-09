/* Copyright (c) 2026, Christian Ahrens
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

// Coverage for Jumpy::JumpyConfiguration::isValid() (the static, XmlElement-taking
// overload) -- the gate that decides whether a loaded/received config file is
// complete enough to apply. Deliberately does NOT construct a full
// JumpyConfiguration instance: its constructor does real file I/O and starts a
// background flush thread (see AppConfigurationBase::InitializeBase()), which is
// both unnecessary for testing the pure validation logic and adds real risk of an
// untested lifecycle interaction in a plain console-app test harness. The static
// isValid() overload only needs a live juce::JUCEApplication instance (for the
// application-name/root-tag check in JUCEAppBasics::AppConfigurationBase::isValid()),
// which is provided locally and torn down at the end of the test.

#include <JuceHeader.h>
#include <JumpyConfiguration.h>

namespace
{
    // JumpyConfiguration::isValid() calls (via AppConfigurationBase::isValid())
    // juce::JUCEApplication::getInstance()->getApplicationName(), which is null outside a
    // real JUCEApplication. Constructing this subclass is enough to satisfy that --
    // JUCEApplicationBase's constructor just registers the global instance pointer; the
    // message loop is never started and initialise()/shutdown() are never invoked.
    class DummyJumpyApplication : public juce::JUCEApplication
    {
    public:
        const juce::String getApplicationName() override { return "Jumpy"; }
        const juce::String getApplicationVersion() override { return "0.0.0"; }
        void initialise (const juce::String&) override {}
        void shutdown() override {}
    };

    // Mirrors the structure JumpyConfiguration builds up in the real app, and what
    // Resources/Default.config contains -- one of each section isValid() requires.
    std::unique_ptr<juce::XmlElement> makeValidConfig()
    {
        auto root = std::make_unique<juce::XmlElement> ("Jumpy");

        auto* devConfig = root->createNewChildElement (Jumpy::JumpyConfiguration::getTagName (Jumpy::JumpyConfiguration::TagID::DEVCONFIG));
        devConfig->setAttribute (Jumpy::JumpyConfiguration::getAttributeName (Jumpy::JumpyConfiguration::AttributeID::FRAMERATE), 1);
        devConfig->setAttribute (Jumpy::JumpyConfiguration::getAttributeName (Jumpy::JumpyConfiguration::AttributeID::OSCPORT), 53000);

        root->createNewChildElement (Jumpy::JumpyConfiguration::getTagName (Jumpy::JumpyConfiguration::TagID::CUSTOMTRIGGERS));

        return root;
    }
}

class JumpyConfigurationTest : public juce::UnitTest
{
public:
    JumpyConfigurationTest() : juce::UnitTest ("JumpyConfiguration", "Jumpy") {}

    void runTest() override
    {
        DummyJumpyApplication dummyApp;

        beginTest ("Valid configuration passes isValid()");
        {
            auto xml = makeValidConfig();
            expect (Jumpy::JumpyConfiguration::isValid (xml));
        }

        beginTest ("Missing DEVICECONFIG section fails isValid()");
        {
            auto root = std::make_unique<juce::XmlElement> ("Jumpy");
            root->createNewChildElement (Jumpy::JumpyConfiguration::getTagName (Jumpy::JumpyConfiguration::TagID::CUSTOMTRIGGERS));
            expect (! Jumpy::JumpyConfiguration::isValid (root));
        }

        beginTest ("Missing CUSTOMTRIGGERS section fails isValid()");
        {
            auto root = std::make_unique<juce::XmlElement> ("Jumpy");
            root->createNewChildElement (Jumpy::JumpyConfiguration::getTagName (Jumpy::JumpyConfiguration::TagID::DEVCONFIG));
            expect (! Jumpy::JumpyConfiguration::isValid (root));
        }

        beginTest ("Wrong root tag name fails isValid()");
        {
            auto root = std::make_unique<juce::XmlElement> ("NotJumpy");
            root->createNewChildElement (Jumpy::JumpyConfiguration::getTagName (Jumpy::JumpyConfiguration::TagID::DEVCONFIG));
            root->createNewChildElement (Jumpy::JumpyConfiguration::getTagName (Jumpy::JumpyConfiguration::TagID::CUSTOMTRIGGERS));
            expect (! Jumpy::JumpyConfiguration::isValid (root));
        }

        beginTest ("Null xml fails isValid()");
        {
            std::unique_ptr<juce::XmlElement> nullXml;
            expect (! Jumpy::JumpyConfiguration::isValid (nullXml));
        }
    }
};

static JumpyConfigurationTest jumpyConfigurationTest;
