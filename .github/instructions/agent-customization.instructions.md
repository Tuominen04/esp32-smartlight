---
description: "Use when generating or updating code in this ESP32 project. Covers agent behavior, coding workflow, and quality expectations."
---

# Agent Customization for ESP32 Smart Light

## Workflow Rules

1. Use 2 spaces for indentation in all C/H files and scripts.
2. Document new functions with purpose, parameters, return values, and error codes.
3. Add or extend tests in `tests/unit` or `tests/system` for behavior changes.
4. Update docs in `docs/` or component `README.md` when adding features or configuration.
5. CI is hobby-focused: build and test checks only need to confirm the app is generally working.

## CI Intent (Hobby Project)

- Keep `.github/workflows/test.yml` focused on building unit and system test targets.
- Keep `.github/workflows/build.yml` focused on building the main firmware.
- Do not introduce release-gating or production-only compliance checks unless explicitly requested.

## Suggestions (strong preference)

- Prefer small self-contained functions and modules; avoid large monolithic functions.
- Keep each component in `components/*` with public header and private implementation.

## Scoped Rule Files

Detailed rules are split by concern—these are auto-attached via `applyTo`:
- `c-h-best-practices.instructions.md` — C/H style and structure
- `esp-idf-best-practices.instructions.md` — ESP-IDF API patterns
- `security.instructions.md` — secrets, PII, logging safety
- `documentation.instructions.md` — README and doc formatting
