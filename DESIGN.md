---
name: TRT Robot Guide
description: A Thai command guide for the TRT line-following robot, printed on the competition mat.
colors:
  vinyl: "#eef0ee"
  vinyl-2: "#e2e5e3"
  paper: "#f8f9f8"
  ink: "#1b1d21"
  ink-2: "#474c54"
  ink-3: "#5c626b"
  rule: "#c8ccca"
  navy: "#222b46"
  navy-ink: "#e9ecf4"
  can-yellow: "#e2aa17"
  can-yellow-tint: "#fff3cc"
  can-orange: "#e2591a"
  can-orange-tint: "#fde3d5"
  can-orange-ink: "#a8400d"
  silver: "#a9aeb5"
typography:
  display:
    fontFamily: '"Bai Jamjuree", "Leelawadee UI", "Tahoma", sans-serif'
    fontSize: "clamp(26px, 3.6vw, 54px)"
    fontWeight: 700
    lineHeight: 1.12
    letterSpacing: "-0.01em"
  headline:
    fontFamily: '"Bai Jamjuree", "Leelawadee UI", "Tahoma", sans-serif'
    fontSize: "clamp(30px, 3.4vw, 44px)"
    fontWeight: 700
    lineHeight: 1.25
    letterSpacing: "-0.01em"
  title:
    fontFamily: '"Bai Jamjuree", "Leelawadee UI", "Tahoma", sans-serif'
    fontSize: "24px"
    fontWeight: 700
    lineHeight: 1.3
  command:
    fontFamily: '"Cascadia Mono", "Consolas", "Menlo", monospace'
    fontSize: "26px"
    fontWeight: 700
    lineHeight: 1.25
    letterSpacing: "-0.01em"
  body:
    fontFamily: '"Sarabun", "Leelawadee UI", "Tahoma", sans-serif'
    fontSize: "18px"
    fontWeight: 400
    lineHeight: 1.75
  label:
    fontFamily: '"Bai Jamjuree", "Leelawadee UI", "Tahoma", sans-serif'
    fontSize: "13px"
    fontWeight: 600
    lineHeight: 1.3
    letterSpacing: "0.05em"
  code:
    fontFamily: '"Cascadia Mono", "Consolas", "Menlo", monospace'
    fontSize: "14px"
    fontWeight: 400
    lineHeight: 1.6
    fontFeature: '"tnum"'
rounded:
  tag: "3px"
  control: "6px"
  panel: "8px"
  pill: "999px"
  disc: "50%"
spacing:
  panel: "12px"
  gutter: "48px"
  entry: "64px"
  section: "96px"
components:
  button:
    backgroundColor: "{colors.paper}"
    textColor: "{colors.ink}"
    typography: "{typography.label}"
    rounded: "{rounded.control}"
    padding: "6px 14px"
    height: "40px"
  button-go:
    backgroundColor: "{colors.ink}"
    textColor: "{colors.vinyl}"
    rounded: "{rounded.control}"
    padding: "6px 14px"
    height: "40px"
  button-go-hover:
    backgroundColor: "{colors.navy}"
  sign-pad:
    backgroundColor: "{colors.navy}"
    textColor: "{colors.navy-ink}"
    typography: "{typography.display}"
    padding: "clamp(18px, 2.2vw, 30px) clamp(20px, 2.6vw, 36px)"
  ends-chip:
    backgroundColor: "{colors.paper}"
    textColor: "{colors.ink}"
    rounded: "{rounded.pill}"
    padding: "4px 14px"
  sim-panel:
    backgroundColor: "{colors.paper}"
    rounded: "{rounded.panel}"
  sensor-cell:
    backgroundColor: "#ffffff"
    textColor: "{colors.ink}"
    rounded: "5px"
    size: "46px"
  sensor-cell-black:
    backgroundColor: "{colors.ink}"
    textColor: "#ffffff"
  tag-warn:
    backgroundColor: "{colors.can-orange-tint}"
    textColor: "{colors.can-orange-ink}"
    rounded: "{rounded.tag}"
    padding: "4px 8px"
---

# Design System: TRT Robot Guide

## Overview

**Creative North Star: "Printed on the Mat"**

The guide reads as if it were printed on the competition mat itself. Cool vinyl white is the ground, graphite is the line ink, and one thick black line runs down the page as its spine. Every section hangs off that spine the way branches hang off the field's loop: a short stub ending in a T-cap. Tips sit at branch ends the way cans do, as coloured discs. The only saturated colours are the colours of the cans on the field.

It is a reading surface with working instruments in it. Long Thai prose sits in a plain column on the vinyl ground; the simulators, sensor readouts and Mission runner are bench panels set into that column, each holding a small field drawn at true scale with a faint 10 cm grid and a 10 cm scale bar. Density is moderate and calm: generous section gaps, tight instrument panels.

State is shown with marks, not with colour alone. A step that is done gets a drawn tick, the current step gets a drawn pennant, a failed step gets a drawn cross, and every sensor cell carries its name and value as text whether it is lit or not. The page refuses neon dark "tech docs" and the anonymous sidebar-docs template.

**Key Characteristics:**
- Cool vinyl ground, graphite ink, one navy sign-pad, can colours as the only accents.
- A 10px black spine with branch stubs and T-caps carries the section structure.
- Bai Jamjuree heavy for display, Sarabun for Thai reading, system mono for commands and code.
- Flat chrome with 1px rules; only physical objects on the mat cast a shadow.
- State marks are drawn shapes (tick, pennant, cross) paired with text, never colour alone.

## Colors

A mat palette: near-neutral cool greys, one navy, and three can colours used as signal.

### Primary
- **Graphite Line Ink** (ink): the line itself. Body text, the spine, branch stubs, T-caps, the black line on every simulated field, primary buttons, pressed segments, lit sensor cells, forward PWM bars, table header rules.

### Secondary
- **Sign-Pad Navy** (navy): the field's 20 x 20 pad. The masthead title plate, the robot body on the canvas, link text, the startup step counters, the done tick, success messages. Text on it is **Pad Ink** (navy-ink).

### Tertiary
- **Can Yellow** (can-yellow): tips and the reader's current position. Tip discs, the current-step highlight in the script line and Mission runner, text selection, link underlines. **Pale Can Yellow** (can-yellow-tint) washes the current line in the code view.
- **Can Orange** (can-orange): action and alarm. Focus rings, the active contents disc, the current-step pennant, sensors that see black on the robot drawing, reverse PWM bars, failed runner cells, warning tip discs. **Pale Can Orange** (can-orange-tint) washes an error line and the warning tag; **Deep Can Orange** (can-orange-ink) is orange text on light ground (timed "ends when" labels, warning tags).
- **Can Silver** (silver): reference, never action. The contents rail line and stubs, code-view line numbers.

### Neutral
- **Vinyl** (vinyl): page ground, simulator stage ground, ink-on-dark text.
- **Worn Vinyl** (vinyl-2): inline code ground, script strip, empty bar track, tags, done runner cells.
- **Print Paper** (paper): panels, buttons, chips, the code editor.
- **Graphite 2** (ink-2): secondary text, ledes, captions, rail links.
- **Graphite 3** (ink-3): tertiary text, labels, junction names on the field, footer.
- **Mat Rule** (rule): every 1px divider and panel border.

### Named Rules
**The Cans Only Rule.** The only saturated hues are the three can colours (yellow, orange, silver) plus navy for the pad. A new accent needs a physical counterpart on the field.

**The Mark Before Hue Rule.** Colour never carries state alone. Every coloured state also has a drawn mark or a text value: done gets a tick, current a pennant, failed a cross, sensors their C0-C4 label and reading.

## Typography

**Display Font:** Bai Jamjuree 600/700 (with Leelawadee UI, Tahoma)
**Body Font:** Sarabun 400/600 (with Leelawadee UI, Tahoma)
**Label/Mono Font:** system mono stack (Cascadia Mono, Consolas, Menlo), tabular numerals

**Character:** Bai Jamjuree is squared and engineered, the voice of field signage; Sarabun is a plain, highly legible Thai text face for long reading. Command names are set in mono because they are code the reader will type.

### Hierarchy
- **Display** (700, clamp 26-54px, 1.12): the masthead title on the navy sign-pad only.
- **Headline** (700, clamp 30-44px, 1.25): section headings that hang off the spine.
- **Title** (700, 24px, 1.3): command group headings above a 1px rule; tip headings at 19px.
- **Command** (mono 700, 26px; 22px under 560px): each command's name, e.g. `FL()`, with its alias at 0.8em in Graphite 3.
- **Body** (400, 18px, 1.75; 17px under 900px): reading text in a 68ch column; ledes at 20px, max 64ch.
- **Label** (600, 12-13px, 0.04-0.06em): table headers, "ends when" chip labels, tags, PWM labels.
- **Code** (mono 400, 13.5-14px, 1.5-1.65): code blocks, the script strip, the Mission editor, readout values.

### Named Rules
**The Embedded Weights Rule.** Only the embedded weights exist: Bai Jamjuree 600 and 700, Sarabun 400 and 600. Do not specify any other weight for these families.

**The Code Is Mono Rule.** Anything the reader would type into `Mission.ino` (command names, values, sensor patterns) is mono with tabular numerals; prose about it is Sarabun.

## Layout

A centred 1240px page with a two-column grid: a 240px sticky contents rail, a 48px gutter, and the main column. The black spine sits in the gutter, 48px left of the main column; section headings extend a stub and T-cap from it, command entries a thinner 24 x 4px stub. Reading text is capped at 68ch (lede 64ch, code details 76ch, tips 74ch); instrument panels and tables are capped at 860px. The playground breaks the cap and runs the full main width as a 1.55 : 1 split of field canvas and editor.

The masthead is a full-bleed (max 1500px) field mat canvas at a 2:1 ratio, with the navy sign-pad placed where the field's pad sits (left 25%, top 41%, width 50%) and registration corner marks at the mat's corners.

Vertical rhythm: 96px between sections, 64px between command entries, 72px above a command group heading, 18px around panels and tip notes; panel interiors pad at 10-12px. Page padding is 48px top, 120px bottom.

Responsive: at 1100px the playground stacks. At 900px the rail becomes a wrap of pill links above the content (sub-entries hidden), the spine thins to 4px with 10 x 4px stubs, the sign-pad drops out of the mat into normal flow below it, and page padding drops to 16px. At 600px tables collapse to stacked rows and code wraps. At 560px sensor cells shrink to 40px, the readout stacks, and registration marks are hidden.

## Elevation & Depth

Chrome is flat. Panels, buttons, chips and tables separate by 1px Mat Rule borders and by the ground shift between Vinyl, Worn Vinyl and Print Paper, never by shadow. Depth belongs only to physical objects sitting on the mat: cans and the robot cast a short, soft offset shadow, and the tip disc echoes a can lid with a small drop.

### Shadow Vocabulary
- **Can lid** (`box-shadow: inset 0 0 0 4px rgba(255,255,255,.55), inset 0 0 0 6px var(--can), 0 2px 4px rgba(0,0,0,.18)`): the tip disc only.
- **Rail halo** (`box-shadow: 0 0 0 3px var(--vinyl)`): cuts the active contents disc out of the silver rail line. A knockout, not elevation.
- **Canvas object shadow** (fill `rgba(0,0,0,.18)` for cans, `rgba(0,0,0,.16)` for the robot, offset about 0.4-0.9 field units down-right): drawn under objects on simulated fields.

### Named Rules
**The Objects Cast, Chrome Doesn't Rule.** A shadow means "this thing sits on the mat". Interface surfaces never get one.

## Shapes

Mostly squared, lightly softened. Inline code and tags round at 3px, buttons, segmented controls and code blocks at 6px, panels at 8px. Pills (999px) are reserved for the "ends when" chip, presets and the mobile rail links. Circles are reserved for things that are round on the field: can discs, step counters, the active rail disc, sensor dots on the robot. The spine is a solid 10px bar with 5px rounded ends; stubs and T-caps are square-ended. Registration marks are 22px L-shaped 2px corners.

## Components

### Buttons
Bench controls: outlined, solid-weighted, quiet until pressed.
- **Shape:** gently rounded (6px), 1.5px Graphite border, 40px min height, icon 16px inline SVG plus text.
- **Default:** Print Paper ground, Graphite text, Bai Jamjuree 600 15px.
- **Go (primary run action):** Graphite ground, Vinyl text; hover turns Sign-Pad Navy.
- **Hover / Active / Focus:** default hover lifts to white; active nudges down 1px; focus is a 3px Can Orange outline at 2px offset.
- **Segmented control:** buttons joined in one 1.5px bordered strip with 1px rule dividers; pressed segment inverts to Graphite.

### Chips
- **"Ends when" chip:** Print Paper pill with a 1px rule border, a 13px tracked label then the condition. Sensor conditions label in Navy, timed conditions in Deep Can Orange.
- **Tag:** 3px-rounded Worn Vinyl label beside tip headings; the warning variant uses Pale Can Orange with Deep Can Orange text.
- **Preset chips:** Vinyl pills with a rule border that darkens to Graphite on hover.

### Cards / Containers
- **Corner Style:** 8px.
- **Background:** Print Paper panel around a Vinyl stage.
- **Shadow Strategy:** none (see Elevation & Depth).
- **Border:** 1px Mat Rule outside and between bands (stage, controls, readout).
- **Internal Padding:** 10-12px per band, 16px for the explorer.

### Inputs / Fields
- **Mission editor:** borderless mono textarea on Print Paper, 13.5px/1.65, vertical resize.
- **Focus:** 2px inset Can Orange ring.
- **Run view:** the same text as a numbered line list; line numbers in Silver, current line washed Pale Can Yellow with a bold number and an orange pennant, error line washed Pale Can Orange.
- **Checkbox:** 18px with Can Orange accent.

### Navigation
- **Contents rail:** a 2px Silver line with 12px Silver stubs to each link (24px for sub-entries), opening with a START marker at the rail head like the field's start box. Links are Bai Jamjuree 600 15px in Graphite 2; sub-entries 14px in Graphite 3.
- **Active:** link turns Graphite and a 14px Can Orange disc sits on the rail, haloed in Vinyl.
- **Mobile (900px and below):** the rail line, stubs and disc disappear; links become Print Paper pills with a rule border; sub-entries are hidden.

### Spine and Branch Stubs (signature)
The page structure drawn as the field line. A 10px Graphite bar runs the full height of the main column. Each section heading carries a 16 x 10px stub out of the spine ending in a 10 x 28px T-cap; each command entry carries a 24 x 4px stub. At 900px and below the spine is 4px, stubs 10 x 4px, caps 4 x 16px.

### Simulated Field (signature)
Every simulator draws a Vinyl mat, a 10 cm grid at `rgba(27,29,33,.055)`, the black line in Graphite at true line width, junction names in Bai Jamjuree 600 Graphite 3, a 10 cm scale bar bottom-right, the robot's path as a soft navy trail, cans as can-coloured discs with a pale lid ring and a dashed ring at their start spot, and the robot in Navy with dark wheels and a pale scoop. A pushed can gets a drawn Graphite tick.

### Sensor Readout (signature)
Five cells C0-C4 always shown, 46px, 1.5px Graphite border. Unlit is white with Graphite text, lit (sees black) inverts to Graphite with white text; each cell carries its name and reading. Beside it the error value in 26px mono and two PWM bars (Graphite forward, Can Orange reverse) with mono values.

### Tip Disc
A 26px Can Yellow disc with a pale inset lid ring beside each tip and note; Can Orange for warnings and real past mistakes.

### Mission Runner Cells
Mono cells per command. Done: Worn Vinyl with a drawn Navy tick. Current: Can Yellow, Graphite border, bold, a drawn orange pennant. Failed: Can Orange with a drawn white cross. Pending: white with a rule border.

## Do's and Don'ts

### Do:
- **Do** hang every new section off the spine with the stub and T-cap (16 x 10px stub, 10 x 28px cap), and every command with the 24 x 4px stub.
- **Do** keep reading text at 68ch or less and instrument panels at 860px or less.
- **Do** draw every simulated field at true scale with the 10 cm grid and the 10 cm scale bar.
- **Do** pair every coloured state with a drawn mark (tick, pennant, cross) or a text value.
- **Do** show all five sensors at all times, lit or unlit, each with its C0-C4 label and reading.
- **Do** use Can Orange for focus, action and alarm, Can Yellow for tips and the reader's current position, and Silver for reference lines.
- **Do** use only the embedded weights: Bai Jamjuree 600/700, Sarabun 400/600.

### Don't:
- **Don't** introduce a saturated hue that has no counterpart on the competition field.
- **Don't** put a shadow on interface chrome; shadows are for cans and the robot only.
- **Don't** use a text glyph or emoji as a state mark; draw it with CSS or canvas.
- **Don't** switch to a dark "tech docs" theme or a generic sidebar-docs template.
- **Don't** put the measuring grid behind reading text; it belongs inside simulated fields.
