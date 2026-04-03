---
description: "Use when reviewing code for security, secrets, PII, or credential handling. Covers logging safety, NVS storage, and hardcoded-secret detection."
applyTo: "**/*.c, **/*.h"
---

# Security Rules for ESP32 Smart Light

1. Do not hard-code passwords, keys, tokens, device IDs, or other secrets in source files.
2. Never log credentials or PII via `ESP_LOGI/W/E/D`; sanitize before output.
3. Scan diffs for `password`, `pass`, `token`, `key`, `secret` (except well-defined placeholders).
4. Store sensitive configuration in NVS with appropriate access controls, not in `sdkconfig`.
5. OTA URLs must use HTTPS; reject plain HTTP endpoints.
6. Confirm with a short commit note that changes were reviewed for PII leakage in logs and storage paths.
