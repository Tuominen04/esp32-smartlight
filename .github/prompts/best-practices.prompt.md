---
description: "Audit files against project best practices (style, security, error handling) and suggest or apply fixes."
agent: "agent"
---

# Check Best Practices Prompt

## Purpose
Use this prompt to instruct the agent to audit a specified file (or set of files) in this repository against project best practices and offer actionable improvements.

## When to use
- During code reviews
- After adding new code in `components/*`, `main/*`, or `tests/*`
- Prior to commits/PRs for quality, security, and performance

## Core behavior
1. Identify the file(s) linked in the request.
2. Evaluate code style (tabs vs spaces, function naming, include guards, etc.) according to `agent-customization.instructions.md`, `c-h-best-practices.instructions.md`, `esp-idf-best-practices.instructions.md`, and `security.instructions.md`.
3. Evaluate correctness and best-practice coverage (error handling, secure logging, Null checks, resource cleanup, no hardcoded secrets).
4. Recommend concrete additions/edits for readability, size reduction, performance, test coverage, and security hardening.
5. Perform in-place edits when asked: update file, create helper file, or add docs/test files.

## Output format
- Short summary headline
- Findings (pass/fail + bullet list)
- Suggested quick fixes
- Patch or edited file section when changes are made

## Example requests
- "Check `components/wifi/wifi_manager.c` for best-practices compliance and suggest improvements."
- "Audit the last changed files and apply formatting/fix according to policy, then add needed unit tests."
- "Make this function safer and more efficient, and document the changes in comments."

## Ambiguity clarifications (ask user)
- Which file(s) should be analyzed? A path or working tree selection is required.
- Should the agent auto-apply changes (modify/create files) or only suggest them?
- Should speed/size/security priority be high/medium/low?
