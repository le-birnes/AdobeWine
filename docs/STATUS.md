# Status

Results from the reference machine (see [COMPATIBILITY.md](COMPATIBILITY.md)), October
2026, AdobeWine 0.1.0. Each item was done with real mouse and keyboard input or with the
app's own scripting, and checked objectively: file properties with ffprobe or PIL, audio
measurements, pixel comparisons against the same export made on Windows, or a screenshot.

**Your results will differ on other hardware.** Please send a report with
`adobewine report` or a [compatibility report](../../../issues/new?template=compatibility.yml).

## Known issues

- Creative Cloud: the first visit to the Apps tab can stay on its loading placeholder;
  switch to Home and back. (`adobewine cc` already applies a workaround.)
- Premiere Pro: the UI hung once after Home > New Project (1 of 4 runs); not reproduced since.
- Illustrator (fixed on main, next release): SVG/PSD exports used Times New Roman instead of Myriad Pro because
  Adobe's shared font folder was missing; the launcher now fills it.
- Photoshop reports Windows 10 to itself (it crashes at start on the Windows 11 build
  number); everything else reports Windows 11.
- Only NVIDIA has been tested. On AMD and Intel, CUDA-based features (Premiere's Mercury
  GPU engine, NVENC export) are not available.

## Photoshop 2026
| # | Function | How | Pass criterion | Status |
|---|---|---|---|---|
| 1 | New document | Ctrl+N | canvas with chosen size/mode | PASS 2026-10-04: File > New, 800x600 RGB 8-bit white, canvas and Properties show 800x600 |
| 2 | Open image | Ctrl+O / launch with file | renders, correct size | PASS: `adobewine photoshop subject.jpg` renders; Ctrl+O dialog |
| 3 | Crop | C, drag, Enter | canvas size changes | PASS: canvas cropped to 994x424 (PSD header) |
| 4 | Image Size | Alt+Ctrl+I | new pixel size | PASS: width 1280 -> ps_blur1280.png 1280x540 |
| 5 | Free Transform | Ctrl+T | layer scaled/rotated | PASS: Layer 1 scaled to 622x265 (PSD layer record) |
| 6 | Layer mask | Layer > Layer Mask | mask thumbnail, hidden pixels | PASS: Layer 1 has user mask channel (-2) in the PSD |
| 7 | Adjustment layer | Layer > New Adjustment Layer > Curves | layer added, tones change | PASS: Curves 1 layer with `curv` data in the PSD |
| 8 | Type | T, click, type | text layer with the text | PASS: type layer "Acceptance 2026" (`TySh`) |
| 9 | AI remove/fill | Remove Background / Generative Fill | content generated/removed | PASS 2026-10-03: Remove Background mask, Generative Fill 3 variations, Neural Filters |
| 10 | Export | File > Export As (PNG/JPEG), Save As PSD | file on disk, right size/format | PASS: PNG 1280x540; JPG (quality 6 of 7) 994x424, PSNR 46.2 dB vs PSD composite; PSD 8BPS 994x424 RGB/8 with 4 layers. Windows reference comparison pending (018) |

## Premiere Pro 2026
| # | Function | How | Pass criterion | Status |
|---|---|---|---|---|
| 1 | New project | Ctrl+Alt+N / New Project | project panel | PASS (3 of 4 runs): Home > New Project > Create opens the edit workspace (acc_pr, acc_pr2, acc_pr3). First run: UI thread hung, panels white (IsHungAppWindow 1); not reproduced from Home or cold start since; capture with tools/wbt.sh if seen again |
| 2 | Import media | Ctrl+I | clip with thumbnail | PASS: test_4k30.mp4 in Project panel, thumbnail, 30:00 |
| 3 | Sequence from clip | New Sequence From Clip | matches clip settings | PASS: drag to empty timeline made sequence test_4k30, 3840x2160 30 fps (export settings Match Source) |
| 4 | Insert clip | drag to timeline | clip on V1/A1 | PASS: V1 + A1 |
| 5 | Cut/trim | Ctrl+K at playhead | two clips | PASS: playhead 00:00:10:00, Ctrl+K, two clips on V1 and A1 |
| 6 | Transition | Ctrl+D | transition at edit | PASS: cross dissolve; export frame at 10 s is a 50% mix (mean saturation 144 vs 251 before, 0 after) |
| 7 | Audio gain | G | waveform changes | PASS: -6 dB on clip 2; export mean -24.1 dB (0-9 s) vs -30.1 dB (11-30 s) = 6.0 dB |
| 8 | Lumetri | Lumetri Color | program monitor changes | PASS: Creative > Saturation 0 on clip 2; monitor and export grey (saturation 0) |
| 9 | Title | Type tool | text in monitor/timeline | PASS: "ACC TITLE" graphic on V2 15-20 s; export title region luma 255 at 15.5/18 s, absent at 25 s |
| 10 | Export | Ctrl+M, Export | file matches timeline (res, fps, duration, codec) | PASS: Match Source - Adaptive High Bitrate, NVENC: H.264 3840x2160, 30/1, 900 frames, 30.000 s, 70.7 Mb/s, AAC stereo 317 kb/s; 30 s timeline exported in ~15 s. Export -3 dB vs source = mono clip panned into stereo sequence  |

## Audition 2026
| # | Function | How | Pass criterion | Status |
|---|---|---|---|---|
| 1 | Record | Shift+Space / record button | file with audio, right rate | PASS 2026-10-02: 12.4 s take, 48 kHz stereo float, peak identical to the played source |
| 2 | Multitrack session | File > New > Multitrack | session with tracks | PASS: acc_mt, 48 kHz 32-bit, 6 tracks; beeps.wav dragged to T1 @0 s and T2 @10 s (snap) |
| 3 | Normalize | Effects > Amplitude > Normalize | peak at target | PASS: -1 dB, channels equal: peak L/R -12.495 -> -1.000 dBFS; saved 32-bit float (bit depth via Sample Type > Change, WAV Settings lists only the matching type - by design) |
| 4 | Noise Reduction | Effects > Noise Reduction | noise floor lower | PASS: DeNoise 80%: median noise floor -28/-27/-27 dB (0.1-1/1-5/5-20 kHz) |
| 5 | Mixdown export | Export > Multitrack Mixdown | duration = session | PASS: Entire Session -> 20.000 s float WAV; each half = beeps.wav x 0.7071 (-3 dB pan law), residual 2.6e-8 |
| 6 | Parametric EQ | Effects > Filter and EQ | spectrum changes | PASS: band 3 dragged to 775 Hz -12.2 dB Q 2; measured -11.9 dB at 775 Hz, -9/-9 at 600/1000, ~0 at 100/8000 Hz. Curve drawn as a ribbon with gaps: Windows screenshot requested |
| 7 | Hard Limiter | Effects > Amplitude > Hard Limiter | peaks at threshold | PASS: max amplitude -6 dB: peaks -6.000 dBFS both channels, samples above -6 dB 19% -> 0 |
| 8 | Import | Ctrl+I | file in Files panel | PASS: File > Import > File and Ctrl+I (editor focused): beeps.wav 0:10.000 48 kHz stereo |
| 9 | Match Loudness | Effects > Match Loudness | LUFS at target | PASS: -23 LUFS target; ffmpeg ebur128 I = -23.0 LUFS |
| 10 | Auto Heal | Ctrl+U | artifact removed | PASS: 1.19 s selection healed; changed samples only 276.21-277.42 s |

## Media Encoder 2026
| # | Function | How | Pass criterion | Status |
|---|---|---|---|---|
| 1 | Add source | Ctrl+I / launch with file | item queued | PASS: + button, file dialog, test_4k30.mp4 queued twice (Ready) |
| 2 | Output path | click output name | path changes | PASS: queue link and Export Settings > Output Name -> (test file) |
| 3 | Format | Format dropdown | codec matches | PASS: HEVC default -> H264; outputs are h264 |
| 4 | Preset | Preset dropdown | settings follow preset | PASS: High Quality 1080p HD -> 1920x1080 High 20.18 Mb/s (Windows 20.17); Match Source - High bitrate -> 3840x2160 Main 10.25 Mb/s (Windows 10.22) |
| 5 | Video bitrate | Export Settings | bitrate near target | PASS: VBR target 10 Mb/s -> 10.25 Mb/s |
| 6 | Audio settings | AAC 320 kbps | audio matches | PASS: Channels Stereo (mono source allows max 256), 320 -> AAC LC stereo 317 kb/s. Summary line kept saying Mono (cosmetic) |
| 7 | Start queue | Enter / green button | Done | PASS: 2 of 2 Done in 26 s (CUDA renderer) |
| 8-10 | Verify duration, resolution, frame rate | ffprobe | equal to source/preset | PASS: 30.000 s, 900 frames, 30/1; frames vs the Windows reference export: 1080p 83-85 dB PSNR, Match Source 3/15 s identical, 27 s 40 dB |

## Creative Cloud desktop
| # | Function | How | Pass criterion | Status |
|---|---|---|---|---|
| 1 | Launch | KDE menu / bin/adobe cc | Home shown | PASS: Home (Featured, Firefly quick actions, Suggested) |
| 2 | Signed in | profile icon | avatar, account | PASS: name, account, plan benefits, Firefly credits shown |
| 3 | Install/update | Apps tab | progress to Open | PASS: Bridge 16.0.8 Install -> progress -> Open (Camera Raw 18.7 and KANC installed with it). Apps tab first visit can stay on "Loading Apps tab..."; Home -> Apps loads it |
| 4 | Open installed app | Open button | app starts | PASS: Bridge opened from CC. All four apps then installed through CC (Photoshop 27.10, Premiere 26.5.2, Audition 26.5, Media Encoder 26.5.2); each starts from CC licensed, Premiere opens the acceptance project with all edits |
| 5 | Search | search bar | results | PASS: "premiere", "bridge" -> shortcuts and suggestions; app page opens |
| 6 | Uninstall | ... menu | removed | PASS: ... > Uninstall > Remove preferences: Bridge folder removed in 15 s, button back to Install |
| 7 | Fonts | Fonts | font activates | PASS after 0040: 365 added fonts listed; installing Bilo Thin writes livetype/r/44244 and other processes now enumerate "Bilo Thin" (before 0040 cached families got garbage names, so no CC font was visible outside CoreSync); 0043: also visible to processes already running, as on Windows (wine/tests/font/fontlive.c) |
| 8 | Cloud sync | cloud icon | status shown | PASS: storage 972.4 MB of 100 GB, "Libraries not currently syncing" |
| 9 | Preferences | menu > Preferences | settings open | PASS: General (account, storage, launch-at-login), Services (Adobe Fonts on, download folder) |
| 10 | Files / offline | Files tab | listing | PASS: cloud documents with thumbnails (Express projects, PDFs), Shared/Projects/Libraries sections |

## Illustrator 2026 (30.8.2, installed by Creative Cloud)
Scripted with an ExtendScript scenario run inside Illustrator under Wine; outputs compared with the outputs of the same scenario on Windows.
| # | Function | How | Pass criterion | Status |
|---|---|---|---|---|
| 1 | Launch / Home | KDE menu or `adobewine illustrator` | window, Home with recents | PASS 2026-10-05: "Welcome to Illustrator" Home with Generate vectors, Featured, Recent; first-run "What's new" carousel as on Windows (0052) |
| 2 | New document | Ctrl+N / New file | New Document dialog | PASS: dialog with Recent/Print/Web... presets; Create opens Untitled-1 (A4, CMYK); canvas fills the view at any window size (0053) |
| 3 | Shapes, pen, text | script steps 2-3 | objects created | PASS 2026-10-05: rectangle, ellipse, star, 2-segment pen path, point and area text (Myriad Pro) |
| 4 | Gradient, effects | script step 4 | applied | PASS: linear gradient, Drop Shadow, Gaussian Blur; their rasters in the SVG are pixel-identical to Windows |
| 5 | Align, Pathfinder | script step 5 | centres equal, 1 path | PASS: centres 317/317/317, Unite -> 1 path, same bounds as Windows |
| 6 | Place + Image Trace | script step 6 | traced group | PASS: 8439 items (Windows 8439), 36.6 s (Windows 32.6 s, RTX 3090 desktop) |
| 7 | Save AI / PDF | script step 7 | files open, match Windows | PASS: .ai 5.97 MB (Win 5.97), PDF renders like Windows (40 px of 480,000 differ >8 levels), Myriad Pro embedded |
| 8 | Export PNG / JPEG | script step 7 | match Windows | PASS: PNG 0.12 % pixels differ (max 29 levels), JPEG 1.4 % (max 22) |
| 9 | Export SVG | script step 7 | match Windows | PASS with a note: same 8438 paths; the embedded SVG font uses Times New Roman metrics instead of Myriad Pro (SVG fonts only; browsers ignore them) |
| 10 | GPU canvas | `ai_gpu.jsx` zoom 800 % pan 10 s | smooth | PASS: 39.6 redraws/s (Windows 31.4/s); panels and status bar draw correctly (0048, 0049) |
| 11 | Quit | `adobewine off` / close | clean exit | PASS: graceful close in ~2 s, no processes left |
