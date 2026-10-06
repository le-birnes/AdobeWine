# Reports: crashes, problems and feedback

AdobeWine can send structured reports to the project's
[issue tracker](../../../issues?q=label%3Areport) so that problems on hardware we do not
have can be debugged. **Reports are opt-in and you see each one before it is sent.**

## What a report contains

- AdobeWine and Wine versions, the downloaded translation layers (DXVK, vkd3d-proton,
  nvidia-libs) and the names of the Adobe apps installed in your prefix;
- distribution, kernel, desktop and session type (Wayland/X11);
- CPU model, amount of RAM, GPU models, NVIDIA driver and Mesa versions, Vulkan devices;
- which app was run, its exit code and how long it ran, and whether it crashed;
- your own note, if you add one (`--note "what I was doing"`);
- the last ~120 lines of that run's Wine log (error lines; informational lines removed).

## What is removed before you see it

Your user name and host name; your home folder path and file names in it (shown as
`~/<path>`); Windows user folders (`C:\users\<user>`); e-mail and IP addresses;
anything that looks like an access token, bearer header, cookie, password or Adobe ID.
The report is a plain Markdown file in `~/.local/share/adobewine/reports/`, so you can
read and edit it before sending.

## Sending

```sh
adobewine report                             # newest log
adobewine report --note "exported a 4K H.264 file, it froze at 80 %"
adobewine report --log ~/.local/share/adobewine/logs/premiere-20261005-101500.log
```

- With the GitHub CLI logged in (`gh auth login`), the report is filed as an issue
  labelled `report`, under your GitHub account.
- Without it, your browser opens a pre-filled issue page; review it and click
  *Submit new issue*.

## Automatic reports after crashes

`~/.config/adobewine/config` sets what happens when an app crashes:

```sh
REPORTS=ask    # default: offer a report in the terminal, or a desktop notification
REPORTS=auto   # file crash reports automatically (needs `gh auth login`)
REPORTS=off    # never offer or send reports
```

There is no anonymous upload: reports go through your own GitHub account, so nothing is
sent by a hidden service and no write token is shipped inside AdobeWine.
