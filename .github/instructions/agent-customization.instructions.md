# Agent Customization Instructions for ESP32 Smart Light

## Purpose
This instruction file defines project-specific best practices for AI-assisted code generation and updates, ensuring consistent style, safety, and documentation across the repository.

## Rules (mandatory)
1. Tab width: Use 2 spaces for indentation in all C/CPP files and scripts when creating or updating code.
2. Logging: Use ESP logging macros in runtime code (`ESP_LOGI`, `ESP_LOGW`, `ESP_LOGE`, `ESP_LOGD`) with correct tags (e.g., `WIFI_MANAGER`, `OTA`, etc.) and meaningful messages.
3. Documentation: Add or update inline comments, function headers, and module README docs for any new code paths or methods. Include:
   - purpose of function
   - parameter descriptions
   - return values / error codes
4. Security: Do not expose secrets or PII in code/data. Validate that no passwords, keys, tokens, device IDs, user information, or other sensitive data are hard-coded in source files.
5. PIM/private data check: Confirm with a short comment/commit note that code changes were reviewed for PIM leakage and sanitized in logs and storage paths.
6. Error handling: For IDF API calls check and propagate error codes (`esp_err_t`) with `ESP_LOGE` and return appropriate control flow.
7. Style consistency: continue existing naming conventions: snake_case for functions/variables, UPPER_SNAKE_CASE for macros/constants.

## Suggestions (strong preference)
- Prefer small self-contained functions and modules; avoid large monolithic functions.
- Keep each component in `components/*` with public header and private implementation.
- Add/extend tests in `tests/unit` or `tests/system` for behavior changes.
- Keep README updates in `docs/` or component `README.md` when adding features or configuration options.

## Enforcement checkpoints for PRs
- Verify formatting with `idf.py build` or existing repository linters.
- Confirm tab width is 2 spaces in changed lines.
- Confirm no secrets in diff (scan for `password`, `pass`, `token`, `key`, `secret` except well-defined placeholders).
- Confirm new methods are documented and logged.

## Example request prompts
- "Add a secure WiFi reconnect routine in `components/wifi`, with 2-space tabs, ESP logs, and inline API docs."
- "Refactor `ota` module to handle timeout errors and include unit tests; ensure no credentials are leaked."
- "Generate helper for component method doc comments and confirm security filters are applied."

## Small-Ruleset Reference Files
- `c-h-best-practices.instructions.md`
- `esp-idf-best-practices.instructions.md`
