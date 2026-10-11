# Roadmap

Where AdobeWine is going, how far along it is, and where you can help. Comments, ideas and
disagreement are welcome in the [roadmap discussion](../../../discussions/9), or open an issue
for anything specific.

## The goal: 100% usable

An app counts as **100% usable** when every function it has on Windows either works under
AdobeWine or behaves exactly as it does on Windows (same output within tolerance, same
error). Functions that need hardware a machine doesn't have (CUDA on AMD, for example) count
as n.a., not as failures.

How we check: a Windows reference machine lists every function of each app (menus, filters,
effects, export formats...) and saves a reference output for each one. The same steps then run
under AdobeWine, and the outputs are compared objectively (pixels, audio, file properties), not
by eye. Each app gets a checklist, and its score is PASS / (functions - n.a.).

Beyond the per-app checklists, five gates apply to all apps:

1. **Install, update, sign-in** through Creative Cloud, including Adobe's monthly updates.
2. **Speed:** exports and renders at most 10% slower than Windows on comparable hardware.
   The reference machine is a different computer, so times are corrected with a native
   hardware calibration (the same ffmpeg encodes and CPU benchmark on both machines).
3. **Stability:** 20 cold launches without a hang, and a scripted 4-hour session per app
   without a crash or memory growth beyond Windows.
4. **Hardware:** NVIDIA (the reference), plus at least one AMD and one Intel machine.
5. **No open serious bug:** no known crash, data loss, hang or wrong output.

Apps in scope: Creative Cloud desktop, Photoshop (with Camera Raw), Illustrator, Premiere
Pro, Media Encoder, Audition, After Effects, InDesign, Bridge.

## Where we are (October 2026)

| App | Core functions | Full checklist (tested / known) | Notes |
|---|---|---|---|
| Creative Cloud | 10/10 | 10 / 248 | close button doesn't quit; hang after a self-update |
| Photoshop | 10/10 | **85 / 661** | all 75 filters and adjustments run so far match Windows (60 pixel-identical, the random ones at the same noise level). Web Home panel blank |
| Illustrator | 11/11 | 118 / 1730 | 107 of 139 Windows reference outputs match, the rest not compared yet; memory use on 16 GB machines (#2) |
| Premiere Pro | 10/10 | 10 / 368 | Import/Export modes draw black; HEVC export passes the speed gate (5-10% faster than Windows after correction) |
| Media Encoder | 10/10 | 10 / 547 | |
| Audition | 10/10 | 10 / 498 | |
| After Effects | partial | being inventoried | renders on main (Render Queue crash fixed, patch 0061); a real 300-layer comp matches the Windows render. Scripting, keyframes, panels work |
| InDesign | not tested yet | | installs |
| Bridge | opens | 0 / 149 | |

The full checklists come from the Windows inventories (menus, filters, effects, presets...): 3213
known functions so far, 136 tested (4%). That is the real starting line, and most of what is
untested has simply not been run yet, not found broken.

"Core functions" are the 10-11 everyday functions per app in [STATUS.md](STATUS.md). They
say an app is usable, not that all of it is: the full checklists answer that.

Tested hardware so far: NVIDIA RTX 4070 Laptop (reference), and community reports on
RTX 4060, RTX 4090 and Intel UHD 620 (see [COMPATIBILITY.md](COMPATIBILITY.md)). **No AMD
report yet.**

## Milestones

| | Gate | State |
|---|---|---|
| M1 | 0.2.0 released, Wine's own tests green again for our patches | done (the last two regressions, 0035 and 0039, fixed on main) |
| M2 | Full checklists for all 9 apps, so each has a real score | 7 of 9 (After Effects and InDesign to come) |
| M3 | Every app at 90% or more; no open serious bug | |
| M4 | Every app at 99% or more; speed and stability gates pass on NVIDIA | |
| M5 | 100%: all apps, all gates, including one AMD and one Intel machine | |

Dates after M2 depend on what the checklists show.

## What's being worked on

**Now**
- **Premiere's Import/Export modes** (still black): finding out what draws them on Windows.
- Running the inventoried functions under Wine and comparing with the Windows outputs, app by
  app (Photoshop's filters are done; Illustrator, Media Encoder and Premiere references are next).
- A real production job (an After Effects comp with a few hundred layers and animated
  captions, plus its Premiere cut) as an end-to-end test.

**Next, by cause (each one blocks functions in several apps)**
1. **Direct2D:** stroke styles, layers (PushLayer/PopLayer), Flush, command lists. Affects
   Audition's UI and After Effects panels.
2. **DirectComposition** beyond the minimum we have: Premiere's Import/Export modes.
3. **Web panels** (CEF and MSHTML): Photoshop Home, Premiere Learn, the licensing window.
4. **Creative Cloud life cycle:** the hang after it updates itself, the close button.
5. **Memory:** Illustrator on 16 GB machines.
6. ~~**HiDPI** scaling by default~~ done: `adobewine setup` follows the desktop's scale.

**Always**
- Re-run the checklists after each Adobe update; a drop counts as a serious bug.
- Move to each new Wine release when the tests are green on it (11.19 already applies).
- Send the generic patches upstream to WineHQ, so there is less to maintain here.

## How you can help

- **AMD and Intel GPUs:** the biggest gap. Run `adobewine report` and send a
  [compatibility report](../../../issues/new?template=compatibility.yml), even if "it starts
  and crashes" is all there is to say.
- **Other distributions and Wayland sessions:** same, a report tells us more than you think.
- **Your real workflow:** if you use an app daily, tell us what you do in it and what breaks.
  Real use finds things a checklist misses.
- **Patches:** any of the "Next" items above, or anything you hit. Every patch here starts
  with a small test program that shows the bug; see [HOW-IT-WORKS.md](HOW-IT-WORKS.md) and
  [CONTRIBUTING.md](../CONTRIBUTING.md). Wine developers: the patches in
  [PATCHES.md](PATCHES.md) with origin "AdobeWine" are meant to go upstream, and review there
  is very welcome.
- **This roadmap:** if the priorities look wrong to you, say so in the
  [roadmap discussion](../../../discussions/9). If you want to pick up an item, say so there
  first, so two people don't end up on the same bug.
