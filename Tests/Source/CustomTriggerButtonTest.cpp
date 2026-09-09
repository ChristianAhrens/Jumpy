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

// Coverage for Jumpy::TimeStamp and Jumpy::CustomTriggerButton::TriggerDetails --
// the header-only, pure data-manipulation types behind the trigger-button grid's
// persisted state (serialisation round-trip through the semicolon-delimited
// string stored in <TRIGGERDETAILS> elements, plus derived-state helpers).
//
// Deliberately does NOT exercise TimeStamp::fromString()/TriggerDetails::fromString()
// with malformed input: on failure TimeStamp::fromString() calls
// juce::AlertWindow::showAsync(), which needs a running message loop/display and has
// no place in a headless console-app test binary. isValid()/isEmpty() are checked
// directly on constructed values instead, which covers the same logic without going
// through that path.
//
// CustomTriggerButton itself (the juce::Button subclass) is not covered here -- it is
// GUI-heavy (painting, mouse handling, popups) and not compiled into this test target.

#include <JuceHeader.h>
#include <CustomTriggerButton.h>

namespace
{
    // TriggerDetails' default constructor calls (via its m_oscTrigger initialiser)
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
}

class TimeStampTest : public juce::UnitTest
{
public:
    TimeStampTest() : juce::UnitTest ("TimeStamp", "Jumpy") {}

    void runTest() override
    {
        beginTest ("Default-constructed TimeStamp is zero and valid");
        {
            auto ts = Jumpy::TimeStamp();
            expectEquals (ts.getHours(), 0);
            expectEquals (ts.getMinutes(), 0);
            expectEquals (ts.getSeconds(), 0);
            expectEquals (ts.getFrames(), 0);
            expect (ts.isValid());
        }

        beginTest ("toString() zero-pads each field");
        {
            auto ts = Jumpy::TimeStamp (1, 2, 3, 4);
            expectEquals (ts.toString(), juce::String ("01:02:03:04"));
        }

        beginTest ("fromString() round-trips toString() output");
        {
            auto ts = Jumpy::TimeStamp (23, 59, 58, 29);
            auto roundTripped = Jumpy::TimeStamp::fromString (ts.toString());
            expect (roundTripped == ts);
        }

        beginTest ("isValid() accepts in-range boundary values");
        {
            expect (Jumpy::TimeStamp (0, 0, 0, 0).isValid());
            expect (Jumpy::TimeStamp (23, 59, 59, 30).isValid());
        }

        beginTest ("isValid() rejects out-of-range values");
        {
            expect (! Jumpy::TimeStamp (24, 0, 0, 0).isValid());
            expect (! Jumpy::TimeStamp (0, 60, 0, 0).isValid());
            expect (! Jumpy::TimeStamp (0, 0, 60, 0).isValid());
            expect (! Jumpy::TimeStamp (0, 0, 0, 31).isValid());
            expect (! Jumpy::TimeStamp (-1, 0, 0, 0).isValid());
        }

        beginTest ("operator== / operator!= compare by field values");
        {
            auto a = Jumpy::TimeStamp (1, 2, 3, 4);
            auto b = Jumpy::TimeStamp (1, 2, 3, 4);
            auto c = Jumpy::TimeStamp (1, 2, 3, 5);
            expect (a == b);
            expect (a != c);
        }

        beginTest ("clear() resets all fields to zero");
        {
            auto ts = Jumpy::TimeStamp (1, 2, 3, 4);
            ts.clear();
            expect (ts == Jumpy::TimeStamp());
        }
    }
};

static TimeStampTest timeStampTest;

class TriggerDetailsTest : public juce::UnitTest
{
public:
    TriggerDetailsTest() : juce::UnitTest ("CustomTriggerButton::TriggerDetails", "Jumpy") {}

    void runTest() override
    {
        DummyJumpyApplication dummyApp;

        beginTest ("Default-constructed TriggerDetails is NOT empty");
        {
            // TriggerDetails' default constructor deliberately pre-fills a suggested colour
            // (cornflowerblue) and a placeholder OSC address ("/" + app name + "/n") -- it is
            // meant to seed the settings popup with sensible defaults (see
            // CustomTriggerButton::mouseDown()), not to represent an unconfigured/blank slot.
            // isEmpty() checks every field against its true zero/default value, which this
            // constructor does not produce, so it must return false here.
            auto td = Jumpy::CustomTriggerButton::TriggerDetails();
            expect (! td.isEmpty());
        }

        beginTest ("All-default-field TriggerDetails is empty");
        {
            // The only way to get an isEmpty() OSCMessage is an address pattern of exactly "/"
            // -- OSCAddressPattern's constructor trims trailing slashes into an empty string,
            // but rejects a genuinely empty/non-"/"-prefixed address outright.
            auto td = Jumpy::CustomTriggerButton::TriggerDetails (
                "",
                juce::Colour(),
                Jumpy::TimeStamp(),
                juce::OSCMessage (juce::OSCAddressPattern ("/")),
                JUCEAppBasics::MidiCommandRangeAssignment());
            expect (td.isEmpty());
        }

        beginTest ("Explicitly constructed TriggerDetails is not empty");
        {
            auto td = Jumpy::CustomTriggerButton::TriggerDetails (
                "Cue 1",
                juce::Colours::red,
                Jumpy::TimeStamp (1, 2, 3, 4),
                juce::OSCMessage (juce::OSCAddressPattern ("/jumpy/cue1")),
                JUCEAppBasics::MidiCommandRangeAssignment());
            expect (! td.isEmpty());
        }

        beginTest ("toString()/fromString() round-trips name, colour, timecode and OSC address");
        {
            auto original = Jumpy::CustomTriggerButton::TriggerDetails (
                "Cue 1",
                juce::Colours::red,
                Jumpy::TimeStamp (1, 2, 3, 4),
                juce::OSCMessage (juce::OSCAddressPattern ("/jumpy/cue1")),
                JUCEAppBasics::MidiCommandRangeAssignment());

            auto roundTripped = Jumpy::CustomTriggerButton::TriggerDetails::fromString (original.toString());

            expectEquals (roundTripped.m_Name, original.m_Name);
            expect (roundTripped.m_Colour == original.m_Colour);
            expect (roundTripped.m_TS == original.m_TS);
            expectEquals (roundTripped.m_oscTrigger.getAddressPattern().toString(), original.m_oscTrigger.getAddressPattern().toString());
        }

        beginTest ("Deserialising constructor is equivalent to fromString()");
        {
            auto original = Jumpy::CustomTriggerButton::TriggerDetails (
                "Cue 2",
                juce::Colours::green,
                Jumpy::TimeStamp (5, 6, 7, 8),
                juce::OSCMessage (juce::OSCAddressPattern ("/jumpy/cue2")),
                JUCEAppBasics::MidiCommandRangeAssignment());
            auto serialised = original.toString();

            auto viaConstructor = Jumpy::CustomTriggerButton::TriggerDetails (serialised);
            auto viaFromString = Jumpy::CustomTriggerButton::TriggerDetails::fromString (serialised);

            expectEquals (viaConstructor.m_Name, viaFromString.m_Name);
            expect (viaConstructor.m_Colour == viaFromString.m_Colour);
            expect (viaConstructor.m_TS == viaFromString.m_TS);
        }
    }
};

static TriggerDetailsTest triggerDetailsTest;
