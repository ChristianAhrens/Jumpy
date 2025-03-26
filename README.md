![Showreel.001.png](Resources/Documentation/Showreel/Showreel.001.png "Jumper Headline Icons")

See [LATEST RELEASE](https://github.com/ChristianAhrens/Jumper/releases/latest) for available binary packages


<a name="toc" />

## Table of contents

* [Introduction](#introduction)
* [Jumper - app functionality](#jumper---app-functionality)
  * [Main Jumper UI](#main-jumper-ui)
  * [Jumper options menu](#jumper-options-menu)
  * [Custom trigger configuration](#custom-trigger-configuration)
* [MIDI network session setup - iOS to macOS](#midi-network-session-setup---ios-to-macos)


<a name="introduction" />

## Overview

Jumper is a small test utility to send MIDI TimeCode messages to a selected MIDI output device.
Features:
- Manual timecode and framerate selection
- Triggering MTC (SysEx) message with current timecode value
- Play/pause for continuous timecode value sending (generated through async system timer - no high accuracy!)
- Triggering MTC (SysEx) message via OSC ("/Jumper/TS hh:mm:ss:ff")
- 3x4 custom timecode trigger buttons, incl. custom name and color on UI
- OSC trigger button control via custom string

Its sourcecode and prebuilt binaries are made publicly available to enable interested users to experiment, extend and create own adaptations.

Use what is provided here at your own risk!

<a name="Jumper" />

## Jumper - app functionality

### Main Jumper UI

![Showreel.002.png](Resources/Documentation/Showreel/Showreel.002.png "Jumper main")

### Jumper options menu

![Showreel.003.png](Resources/Documentation/Showreel/Showreel.003.png "Jumper options")

### Custom trigger configuration

![Showreel.004.png](Resources/Documentation/Showreel/Showreel.004.png "Jumper custom triggers")


<a name="iOS to macOS MIDI Network Session" />

## MIDI network session setup - iOS to macOS

![Showreel.005.png](Resources/Documentation/Showreel/Showreel.005.png "iOS to macOS MIDI")