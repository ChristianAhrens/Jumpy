![Showreel.001.png](Resources/Documentation/Showreel/Showreel.001.png "Jumper Headline Icons")

See [LATEST RELEASE](https://github.com/ChristianAhrens/Jumper/releases/latest) for available binary packages


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

### Jumper

![Showreel.002.png](Resources/Documentation/Showreel/Showreel.002.png "Jumper in action")
