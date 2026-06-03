/* Copyright (c) 2025, Christian Ahrens
 *
 * This file is part of Jumpy <https://github.com/ChristianAhrens/Jumpy>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <JuceHeader.h>

#include "MainComponent.h"

#include <CustomLookAndFeel.h>

/**
 * @class MainApplication
 * @brief JUCE application entry point for Jumpy.
 *
 * Creates and owns the single MainWindow on startup; tears it down on shutdown.
 * Multiple instances are permitted (`moreThanOneInstanceAllowed()` returns `true`),
 * matching the use case of running several Jumpy windows to drive independent MIDI
 * outputs simultaneously.
 */
class MainApplication : public juce::JUCEApplication
{
public:
    //==============================================================================
    MainApplication() {}

    const String getApplicationName() override { return ProjectInfo::projectName; }
    const String getApplicationVersion() override { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override { return true; }

    //==============================================================================
    void initialise(const String& commandLine) override
    {
        mainWindow.reset(std::make_unique<MainWindow>(getApplicationName(), commandLine).release());
    }

    void shutdown() override
    {
        mainWindow.reset();
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        // This is called when the app is being asked to quit: you can ignore this
        // request and let the app carry on running, or call quit() to allow the app to close.
        quit();
    }

    void anotherInstanceStarted(const juce::String& commandLine) override
    {
        ignoreUnused(commandLine);
    }

    //==============================================================================
    /**
     * @class MainWindow
     * @brief Desktop document window that hosts MainComponent and mirrors the OS dark-mode setting.
     *
     * On construction the window creates a `Jumpy::MainComponent`, wires up the
     * palette-style change callback so that user-selected themes propagate to the global
     * `juce::LookAndFeel`, and registers itself as a `juce::DarkModeSettingListener`
     * so the OS dark/light preference is followed automatically whenever
     * `m_followLocalStyle` is `true`.
     *
     * Platform behaviour:
     * - **iOS / Android**: runs full-screen with the screen saver disabled.
     * - **Linux**: enters kiosk mode.
     * - **macOS / Windows**: resizable window, centred at its preferred size.
     */
    class MainWindow : public juce::DocumentWindow, juce::DarkModeSettingListener
    {
    public:
        /**
         * @brief Constructs the main window and initialises all child components.
         * @param name         Application name used as the window title.
         * @param commandLine  Command-line arguments passed by the OS (currently unused).
         */
        MainWindow(const juce::String& name, const juce::String& commandLine) : juce::DocumentWindow(name,
            juce::Desktop::getInstance().getDefaultLookAndFeel()
            .findColour(juce::ResizableWindow::backgroundColourId),
            juce::DocumentWindow::allButtons)
        {
            ignoreUnused(commandLine);

            setUsingNativeTitleBar(true);
            auto mainComponent = std::make_unique<Jumpy::MainComponent>();
            mainComponent->getOnPaletteStyleChangeCallback() = [=](int paletteStyle, bool followLocalStyle) {
                m_followLocalStyle = followLocalStyle;
                if (followLocalStyle)
                    darkModeSettingChanged();
                else
                    applyPaletteStyle(static_cast<JUCEAppBasics::CustomLookAndFeel::PaletteStyle>(paletteStyle));
            };
            setContentOwned(mainComponent.release(), true);

#if JUCE_IOS || JUCE_ANDROID
            setFullScreen(true);
            juce::Desktop::getInstance().setScreenSaverEnabled(false);
#elif JUCE_LINUX
            juce::Desktop::getInstance().setKioskModeComponent(getTopLevelComponent());
#else
            setResizable(true, true);
            centreWithSize(getWidth(), getHeight());
#endif

            setVisible(true);

            juce::Desktop::getInstance().addDarkModeSettingListener(this);
            darkModeSettingChanged(); // initially trigger correct colourscheme

            //applyPaletteStyle(JUCEAppBasics::CustomLookAndFeel::PaletteStyle::PS_Dark);
        }

        /** @brief Delegates to `juce::JUCEApplication::systemRequestedQuit()`. */
        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }

        /**
         * @brief Applies the dark or light palette when the OS dark-mode setting changes.
         *
         * Only acts when `m_followLocalStyle` is `true`; user-selected explicit themes
         * set `m_followLocalStyle` to `false` and suppress this callback.
         */
        void darkModeSettingChanged() override
        {
            if (!m_followLocalStyle)
                return;

            if (juce::Desktop::getInstance().isDarkModeActive())
            {
                // go dark
                applyPaletteStyle(JUCEAppBasics::CustomLookAndFeel::PS_Dark);
            }
            else
            {
                // go light
                applyPaletteStyle(JUCEAppBasics::CustomLookAndFeel::PS_Light);
            }

            lookAndFeelChanged();
        }

        /**
         * @brief Creates a new CustomLookAndFeel for the given palette and sets it as the global default.
         * @param paletteStyle  The palette to apply (e.g. `PS_Dark` or `PS_Light`).
         */
        void applyPaletteStyle(JUCEAppBasics::CustomLookAndFeel::PaletteStyle paletteStyle)
        {
            m_lookAndFeel = std::make_unique<JUCEAppBasics::CustomLookAndFeel>(paletteStyle);
            juce::Desktop::getInstance().setDefaultLookAndFeel(m_lookAndFeel.get());
        }

    private:
        std::unique_ptr<juce::LookAndFeel>  m_lookAndFeel;         ///<  Currently active global LookAndFeel instance.
        bool m_followLocalStyle = true;  ///<  When true the window tracks the OS dark/light preference; set to false by explicit palette selection.

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;  ///<  The single application window.
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(MainApplication)
