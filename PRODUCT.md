# Product

<!-- impeccable:product-schema 1 -->

## Platform

web

## Stack

Static single HTML file, opens offline by double-click (user choice 2026-11-18). No build step, no network dependency at the competition venue.

## Users

Teammates and new members of the TRT line-following robot team who do not yet know the code. Job: understand what each command in `TRT_arduino_nano.ino` / `Mission.ino` does, then write and tune their own `Mission()` route for a competition field. Thai readers; code identifiers stay English.

## Product Purpose

A Thai-language guide to the robot's command vocabulary (FL, FLR, FLL, FL_time, cross, cross_down, TL, TR, push, can_L/R/LR, GO/GB/GBL, B, STOP, Speed_motor, Set_kp...) that shows each command running on a simulated robot with its 5 line sensors, plus a playground where a typed Mission sequence runs on a simulated field. Success: a new member can read a `Mission()` row and predict where the robot stops, and can write a working row for a new junction.

## Positioning

Built from this robot's real code and field notes: real thresholds logic (`C[i] < S[i]` = black), real error mapping, real gotchas (STOP(x) = x+50 ms, timed moves drift with battery, speed_L goes to the right motor), real race rules (ระดับกลาง).

## Operating Context

Arduino Nano robot, 5 analog line sensors A0-A4 (left to right), 2 motors, button D2, buzzer. Competition: MRC-style field, fixed loop + 20x20 cm start box, branches with cans change on race day. Practice period for editing, then run 1, 30 min fix, run 2. Hardware is fixed; everything is solved in code.

## Capabilities and Constraints

- Simulation is illustrative, not physics-accurate; must say so.
- Field geometry source: `.wolf/field-map.md` (loop ~781 cm, start box).
- Must work offline, single file.

## Evidence on Hand

Source code `TRT_arduino_nano.ino`, `Mission.ino`; field notes `.wolf/field-map.md`; field photos `.wolf/field/`. No testimonials or benchmarks exist; do not invent any.

## Product Principles

1. Every explanation maps to the real code line it describes.
2. Show, then tell: the robot moves before the paragraph explains why.
3. Tips come from real mistakes on this robot, not generic advice.

## Accessibility & Inclusion

Thai text must render with a Thai-capable font; readable on a phone at the venue.
