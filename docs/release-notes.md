# Release & Packaging — Working Notes

> **Status: not yet finalized.** This captures the current understanding of how pedals get built, packaged, and distributed. It is NOT a settled procedure — confirm and formalize after the next real release, then promote the stable parts into `.claude/rules/release.md`.

## Two audiences, two concerns
Releasing a pedal involves two separate things — keep them distinct:
1. **Build & package** (you) — compile a release binary, assemble the release folder, tag.
2. **End-user install docs** (them) — instructions that ship *with* the binary, as a release artifact in the pedal's `releases/` folder (not a Claude rule).

## Flashing / distribution methods (situational)
- **dfu-util (command line)** — primary method for your own build-and-flash.
- **Daisy Web Programmer** — used (a) when the **bootloader itself** needs updating (different operation than flashing an app), and (b) by **end users** who download only the binary and prefer a no-install, browser-based flash.
- End-user install docs should therefore cover the Web Programmer path, even though your own workflow is dfu-util.

## Build for release
- Release builds historically used `OPT = -O2` (vs `-Ofast` for development). **Confirm this is still intended** before formalizing.
- QSPI projects need the bootloader installed first (`make program-boot`), then `make program-dfu`.

## Packaging (from prior practice — verify)
- Per-pedal `releases/` folder holds the versioned release (e.g. `ambien-flux-hothouse/releases/ambien-flux-v1.0/`).
- A `binary/` directory is preserved via `.gitignore` safety net (`!binary/`, `!binary/*.bin`) so release binaries are version-controlled while build artifacts are ignored.
- Release artifacts seen in the Ambien Flux v1.0 release: LICENSE, CONTROLS, CHANGELOG, the `.bin`.
- **Attribution:** bundle license/attribution for any third-party code (e.g. Neptune delayline, MIT — required in Ambien Flux releases).

## Release sequence (from dev notes — verify before relying on it)
`git mv` (rename/restructure as needed) → update content → commit → build fresh binary → tag.
Always build a fresh binary after the final code changes.

## Open questions to resolve before writing the real rule
- Is `-O2` still the release optimization level?
- Canonical home for binaries: per-pedal `releases/` only, or also a central `binary/`?
- Standard set of bundled artifacts (LICENSE / CONTROLS / CHANGELOG / README) — finalize the template.
- End-user install doc template (Web Programmer steps + controls reference).
