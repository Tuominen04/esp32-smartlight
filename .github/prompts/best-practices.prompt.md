---
description: "Audit files against project best practices (style, security, error handling) and suggest or apply fixes."
name: "Best Practices Audit"
argument-hint: "Path(s) to audit and whether to auto-apply fixes"
agent: "agent"
---

# Best Practices Prompt

## Purpose
Use this prompt to instruct the agent to audit a specified file (or set of files) in this repository against project best practices and offer actionable improvements.

## When to use
- During code reviews
- After adding new code in `components/*`, `main/*`, or `tests/*`
- Prior to commits/PRs for quality, security, and performance

## Core behavior
1. Identify the file(s) linked in the request.
2. Evaluate code style (tabs vs spaces, function naming, include guards, etc.) according to `agent-customization.instructions.md`, `c-h-best-practices.instructions.md`, `esp-idf-best-practices.instructions.md`, and `security.instructions.md`.
3. Evaluate correctness and best-practice coverage (error handling, secure logging, null checks, resource cleanup, and no hardcoded secrets).
4. Recommend concrete additions/edits for readability, maintainability, performance, test coverage, and security hardening.
5. If the user links file(s) without requesting analysis-only mode, apply safe in-place fixes immediately and summarize exactly what changed.
6. When behavior changes, add or update tests under `tests/unit` or `tests/system`, and mention doc updates needed in `docs/`.

## Output format
- Short summary headline
- Findings (pass/fail + bullet list)
- Suggested quick fixes
- Applied changes summary with file list
- Remaining recommendations (only if not auto-applied)

## Example requests
- "Check `components/wifi/wifi_manager.c` for best-practices compliance and suggest improvements."
- "Audit the last changed files and apply formatting/fix according to policy, then add needed unit tests."
- "Make this function safer and more efficient, and document the changes in comments."

## Ambiguity clarifications (ask user)
- Which file(s) should be analyzed? A path or working tree selection is required.
- Should the agent use analysis-only mode? Default is auto-apply safe fixes when file paths are provided.
- Should speed/size/security priority be high/medium/low?
