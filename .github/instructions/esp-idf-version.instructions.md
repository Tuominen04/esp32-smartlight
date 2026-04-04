---
description: "Use when making changes that involve ESP-IDF framework dependencies, component manifests, or CI/CD workflows. Ensures consistency with the required ESP-IDF version."
applyTo: "main/idf_component.yml, .github/workflows/*.yml, components/**/idf_component.yml"
---

# ESP-IDF Version Requirements

## Required Version

This project **requires ESP-IDF version 6.0.0 or later**.

The application has been migrated to ESP-IDF 6.x and uses features available in this version.

## Where Version is Specified

1. **Component Manifest** (`main/idf_component.yml`):
   ```yaml
   dependencies:
     idf:
       version: ">=6.0.0"
   ```

2. **CI/CD Workflows** (`.github/workflows/test.yml`):
   ```yaml
   container:
     image: espressif/idf:v6.0
   ```

## Rules for Changes

1. **DO NOT** downgrade the ESP-IDF version requirement below 6.0.0
2. When updating CI workflows, ensure the Docker image version matches the requirement in `main/idf_component.yml`
3. All three CI jobs (unit tests, system tests, production build) must use the same ESP-IDF version
4. If upgrading to a newer ESP-IDF version:
   - Update `main/idf_component.yml` first
   - Update all container images in `.github/workflows/test.yml`
   - Test locally before committing
   - Document any breaking changes or required migrations

## Local Development

Developers must have ESP-IDF v6.0.0 or later installed locally:

```bash
# Verify your ESP-IDF version
idf.py --version

# Expected: ESP-IDF v6.0.0 or higher
```

## Migration Notes

- The app was migrated from ESP-IDF 5.x to 6.x
- Component naming changes from 5.x to 6.x have been applied
- Managed components use ESP-IDF 6.x compatible versions
