# Security policy

## Reporting a vulnerability

If you find a security problem in AdobeWine's patches or scripts (for example a patch that
weakens a Windows security boundary, or a script that could leak credentials), please
**do not open a public issue**. Use GitHub's private vulnerability reporting
(*Security* tab > *Report a vulnerability*). We will answer within a week.

Problems in Wine itself go to WineHQ; problems in Adobe software go to Adobe.

## What AdobeWine does with your data

- It stores nothing outside `~/.local/share/adobewine`, `~/.config/adobewine` and the
  desktop menu entries you ask for.
- It downloads only Wine, DXVK, vkd3d-proton, nvidia-libs, OpenCL headers, wine-gecko
  and wine-mono, from their official locations, with pinned versions and checksums where
  the upstream provides fixed release files.
- It sends nothing anywhere unless you send a report (see
  [docs/REPORTING.md](docs/REPORTING.md)), and reports are scrubbed of personal data first.
- Your Adobe sign-in happens inside Adobe's own software; AdobeWine never reads or stores
  Adobe credentials or tokens.
