# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

# OpenWolf

@.wolf/OPENWOLF.md

This project uses OpenWolf for context management. Read and follow .wolf/OPENWOLF.md every session. Check .wolf/cerebrum.md before generating code. Check .wolf/anatomy.md before reading files.

# User Rules (MUST follow)

- **Never deploy code without the user's explicit permission.** This includes editing project source files (.ino, .h, .cpp), uploading to the Arduino board, and installing libraries. Ask first, every time.
- **Always show the solution code first.** Put the proposed change in the reply (before/after or a diff) and wait for approval before applying it.
- Reply in Thai (code identifiers stay in English). Code comments in Thai.

# Project

Line-following robot (Arduino Nano, ATmega328, AVR core 1.8.6) for a competition field: follow black lines on white, turn at junctions, push cans at the ends of branch lines. No tests, no linter. The only check is compiling.

## Commands

```sh
# Compile (the real error check). Run from the sketch folder
"C:/Users/acer/AppData/Local/Programs/Arduino IDE/resources/app/lib/backend/resources/arduino-cli.exe" compile --fqbn arduino:avr:nano --warnings all .

# Upload: only with the user's permission. Find the port first with `board list`
"C:/Users/acer/AppData/Local/Programs/Arduino IDE/resources/app/lib/backend/resources/arduino-cli.exe" upload -p COMx --fqbn arduino:avr:nano .
```

The compiler runs with `-fpermissive`, so some real errors (such as a missing return value) show up only as warnings.

## Architecture

- **Two .ino files, one program.** Arduino joins `TRT_arduino_nano.ino` (main file: pins, globals, setup, all motion/sensor/PID helpers) and `Mission.ino` (`Mission()` route plus combo helpers) into one translation unit. Globals and functions are shared across both files without headers.
- **IntelliSense errors are fake.** VS Code reports false "identifier X is undefined" errors in `Mission.ino`, so `C_Cpp.errorSquiggles` is set to disabled in `.vscode/settings.json`. Trust only `arduino-cli compile`.
- **Everything runs once in `setup()`.** `loop()` is empty. Startup order: beep, `SW()` waits for the button on D2, `Mission()`, stop, end song.
- **Calibration mode.** If D2 is held at boot, the robot reads white, then black (pressing the button between steps), and stores the midpoint threshold in EEPROM bytes 0–4 as `/10`, then halts. Normal boot loads `S[i] = EEPROM.read(i) * 10`.
- **Sensors.** `C[0..4]` = `analogRead(A0..A4)`, read from left to right. `C[i] < S[i]` means the sensor sees black. `cheack_error()` maps sensor patterns to `error` values (±1, ±5, ±7, and ±6/±9 when the line is lost). Turning sharply depends on the robot remembering the last error.
- **Motors (TB6612-style).** PWMA/AIN* = right motor, PWMB/BIN* = left motor. `drive_motor(AIN1, AIN2, BIN1, BIN2)`: `(1,0,1,0)` = forward, `(1,0,0,1)` = spin left. `motor_control()` spins in place when `error == ±9`.
- **PID** runs on global state (`Kp`, `Kd`, `previous_error` and others). After each move command, `PID_speed_reset()` clears it and resets the base speed to `SPEED`, the value set by `Speed_motor()`.

## Command vocabulary (used in `Mission()`)

| Command | Ends when |
|---|---|
| `FL()` | C[0] **and** C[4] both see black (full cross) |
| `FLR()` / `FLL()` | C[4] / C[0] sees black (branch on right / left) |
| `FLR2()` / `FLL2()` | 2nd time the right / left branch is seen |
| `FL_time(ms)` | after the time runs out (follows the line meanwhile) |
| `cross()` | 200 ms of line following, to pass over a junction |
| `cross_down()` | `time_cross` ms at `speed_cross`, ignoring the outer sensors, then stop. Use before a turn |
| `TL()` / `TR()` | spin until C[0]/C[4] sees black, then C[2] sees black |
| `GO/GB/R/L/M_*(ms)` | after the time runs out, with no sensors (uses `speed_L`/`speed_R`) |
| `B*` variants | same as the matching `F*` command but driving backward |

Typical can cycle: `FL(); cross_down(); TL(); pushkuy(); GB(575); TR(); ...`

## Gotchas

- `STOP(x)` really takes x+50 ms because `B()` already has `delay(50)` inside.
- Timed moves (`GO`, `GB`, `R`, `L`, `M_*`) write `speed_L` to PWMA, which is the right motor. This is swapped compared with the PID code. It has no effect only while `speed_L == speed_R`.
- Time-based moves drift with battery level. Prefer commands that end on a sensor event. No extra hardware (encoders etc.) can be added.
- Check for an unsaved VS Code buffer (`getDiagnostics` linesInFile vs disk) before editing a `.ino` on disk. Editing over a stale buffer corrupted the file once.
