# alarm-clock
A normal clock with alarm features.

## Repo Contents

- *src* contains the source code and libraries. It contains:
  - *Alarm_Clock.X* -> An **MPLABX IDE** 5.3 Project Folder.
  - *lib* -> Contains self-written C libraries (.h files) from which some functions are used within the main program.
    - *7-seg.h* -> functions for 7-segment display.
    - *eeprom.h* -> functions to utilize PIC EEPROM.

- *schematics* contains a Proteus **8.9** Project file containing schematics for the project and screenshot of it.

## Project Description

### Components Used
- A 5V Power source
- PIC16F877A MCU
- 20MHz Crystal Oscillator
- Two 7-segment Displays
- A buzzer
- Four push buttons

### Working Process Description
The alarm clock has two modes:
- Clock mode
- Alarm mode

All user operations are performed using the push buttons.
There are four of them:
- MODE
- SET
- UP
- DOWN

#### Clock Mode
This is the default mode at startup which displays the current time.
The default time at any power-on is 00:00.
The time is counted using TIMER1 (check comments in the code for more description on that).
Two buttons have functions in this mode:
1. MODE: This swittches the device to Alarm mode.
2. SET: **Press and hold** to set the time. (This process is explained below)

#### Alarm mode
The device has 4 *alarms*, each independent of the other.
NOTE: Alarm times and states are stored in memory, so the clock going OFF doing clear them.
The first time you switch into this mode after startup, it'll be on *alarm 1*.
Each alarm also has a *snooze* function.
Buttons and their functions in this mode:
1. MODE: Switches back to Clock mode.
2. SET:
    - Press once to switch alarm *state* (**beeps once**).
    - Press and Hold to Set alarm time.
3. UP: Next alarm (1 -> 2 -> 3 -> 4 -> 1 ...)
4. DOWN: Previous alarm (4 -> 3 -> 2 -> 1 -> 4...)

#### Setting Time
In either mode, time-setting is initiated by holding down the *SET* button.
While holding the *SET* button, the display blinks 'SET' for a few seconds.
If the button is released during this period, the operation is cancelled.
Otherwise, the clock enters the time-setting 'mode' and **beeps once** immediately.
By default, the cursor is on the *hour*.
There are Five possible operations in this 'mode'. The button operations are as follows:
1. MODE: Switches the 'cursor' between *hour* and *minute*.
2. SET:
    - Press once to cancel the operation.
    - Press and Hold to Set the time. **Once it beeps, you can release**.
3. UP: Increase number (rolls over e.g 59 -> 00)
4. DOWN: Previous alarm (rolls over e.g 00 -> 59)
Hold down *UP* or *DOWN* to increase or decrease (respectively) **continually**.

#### Alarms: Ringing and Snoozing
NOTE: Alarms can only ring while in *Clock Mode*.
An alarm can only ring if it's *state* is HIGH.
An alarm rings for 30 seconds (15 *double-beeps*), while displaying the alarm/snooze time.
WHile ringing, these are the applicable buttons and their functions:
1. MODE: Stops the ringing and goes back to normal *Clock mode* but doesn't set alarm state to LOW.
2. SET: Stops the ringing and goes back to normal *Clock mode*. Sets alarm state to LOW.
3. UP: Snooze by 5 minutes.

If the ringing is not stopped manually, the alarm is automatically snoozed by 5 mins.
*Auto-snooze* can only work maximum of 3 times i.e the max *auto-snooze* time you can have is *alarm time* + 15 minutes. Though, manual snoozes can go forever.

