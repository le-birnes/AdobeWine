# Troubleshooting

First steps for any problem:

1. Every run writes a log to `~/.local/share/adobewine/logs/<app>-<date>.log`; the path is
   printed when you start an app from a terminal.
2. `adobewine report` builds a report (system, versions, log tail) with personal data
   removed, shows it to you, and can send it to the project. See [REPORTING.md](REPORTING.md).
3. More detail in the log: start the app with `WINEDEBUG=+err,+seh,+loaddll adobewine photoshop`.

## Installation and sign-in

| Symptom | What to do |
|---|---|
| `no AdobeWine runtime` | Install the `adobewine-bin`/`adobewine` package, or run `scripts/build-wine.sh` |
| The Creative Cloud installer stays on "Loading..." | `adobewine setup` installs wine-gecko; run it again and check its output |
| The sign-in window is blank or white | Close it, `adobewine fix-signin`, then `adobewine cc` |
| Creative Cloud's Apps tab stays on its placeholder the first time | Click Home, then Apps again |
| An app says it is not licensed | Sign out and in again in Creative Cloud (avatar menu); the apps take the licence from it |

## Display

| Symptom | What to do |
|---|---|
| Black or empty windows, nothing renders | Check `vulkaninfo --summary` lists your GPU; on hybrid laptops try `ADOBEWINE_NVIDIA_OFFLOAD=1` |
| Photoshop menu bar missing, UI frozen after start | Segoe UI is missing: `adobewine setup --windows-fonts <your Windows Fonts folder>` |
| Text in some dialogs is blank | Same as above: add your Windows fonts |
| A panel draws only after resizing | Please report it with `adobewine report`; resizing the window is the workaround |
| Wrong scaling on HiDPI screens | `adobewine winecfg` > Graphics > Screen resolution (DPI); 96 = 100 %, 144 = 150 % |

## Media

| Symptom | What to do |
|---|---|
| MP4/MKV files do not import or play | Install GStreamer plugins: `gst-plugins-good`, `gst-plugins-bad`, `gst-libav` (names vary by distribution) |
| Premiere/Media Encoder: no CUDA renderer, no hardware export | NVIDIA only: check `/proc/driver/nvidia/version` exists and run `adobewine setup` again (it installs nvidia-libs) |
| No sound | Wine uses PulseAudio/PipeWire; check the output device in `adobewine winecfg` > Audio |

## Closing and crashes

| Symptom | What to do |
|---|---|
| "recovered from a crash" after a normal close | Close apps with their own Quit or `adobewine off`, not by killing Wine |
| Audition asks about a recovered session at every start | Choose **Delete** once |
| Everything hangs | `adobewine off --force` stops all Wine processes (unsaved work is lost) |

## Starting over

```sh
adobewine off --force
mv ~/.local/share/adobewine/prefix ~/.local/share/adobewine/prefix.old
adobewine setup --windows-fonts <your Windows Fonts folder>
```
Then install Creative Cloud again. Delete `prefix.old` once the new prefix works.
