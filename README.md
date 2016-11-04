# arduino-menuscreen
An Arduino sketch to give a touch screen an interactive menu.

Don't know about Arduino? Start here... https://www.arduino.cc/en/Guide/Introduction

Hardware includes an Arduino Mega 2560, resistive touch screen shield, real-time clock, DHT (humidity/temperature sensor), and a lithium battery shield. Some more details about this are in the code comments.

This sketch combines timing with some variable comparisons (close to "protothreading") so that the linear code processing can update the info and listen for touch input "at the same time", while the device does not really look like it is beating itself to death.

Many parts of this code are copy/pasted from other sources.
What I did:
- Long list of "useful" and "timing" variables.
- Startup display, with some initial checks/feedback and user instruction.
- Layout of summary dials and details.
- Adjust a "screenmode" variable to simulate usage mode selection (the appropriate menu dial is highlighted, and the currently-selected dial's details are shown).

Sorry I don't have pics/vids of this thing in action. This is something I did awhile ago and decided to post here for the heck of filling in content on my profile. My Arduino rig is currently not in a state for me to try this again, for reasons.  :o|
