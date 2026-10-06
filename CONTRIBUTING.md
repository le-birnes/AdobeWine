# Contributing to AdobeWine

Thank you for helping. AdobeWine is tested on one machine so far; every report from other
hardware is valuable, positive or negative.

## Testing and reporting

- **Works for you?** Open a [compatibility report](../../issues/new?template=compatibility.yml)
  with your distribution, GPU and what you tried. "Photoshop 27.10 works on Fedora 42 with
  an RX 7800 XT" is a useful report.
- **Something broke?** Run `adobewine report --note "what you did"`, which collects the
  details with personal data removed (see [docs/REPORTING.md](docs/REPORTING.md)), or open a
  [bug report](../../issues/new?template=bug.yml).
- **Questions, ideas, other Adobe apps**: [Discussions](../../discussions).

Please do not post Adobe installers, Adobe binaries, licence files, access tokens or
anything copied from your Adobe account.

## Patches

1. Find the difference: run the failing scenario with Wine debug channels
   (`WINEDEBUG=+err,+seh,+loaddll` plus the relevant module, e.g. `+d2d`, `+winhttp`).
2. Write a small Windows test program for it (see `tests/`, built with
   `x86_64-w64-mingw32-gcc`). If you have Windows, run it there too: the Windows result is
   the reference.
3. Fix Wine so it gives the Windows result, without breaking Wine's own conformance tests
   for that module (`make -C <build>/dlls/<module>/tests test` when practical).
4. Add the patch as `patches/NNNN-module-short-description.patch` (`git diff` against Wine
   11.18 with all earlier patches applied), the test under `tests/`, and a line in
   `docs/PATCHES.md`.
5. Open a pull request saying what Adobe workflow it fixes and how you checked it.

`scripts/build-wine.sh` rebuilds from a clean tree with all patches. CI checks that
every patch still applies to Wine 11.18.

## Upstreaming

Most AdobeWine patches are general Wine fixes. Getting them into WineHQ
(<https://gitlab.winehq.org/wine/wine>) helps every Wine user and shrinks this patch set.
Upstream wants Wine-style tests in `dlls/*/tests`; converting our tests to that form is a
great first contribution.

## Ground rules

- No licence circumvention, cracks, or anything that weakens Adobe's sign-in. AdobeWine
  must stay usable only with a legitimate subscription.
- Be kind; see [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md).
- By contributing you agree your contribution is licensed under LGPL-2.1-or-later.
