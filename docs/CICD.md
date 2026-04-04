# CI/CD Pipeline

Overview of the automated build and test workflows used in this project.

## Workflows

Two GitHub Actions workflows run on every push and pull request to `main` and
`develop`:

| File | Purpose |
|------|---------|
| `.github/workflows/build.yml` | Build the production firmware |
| `.github/workflows/test.yml` | Build unit and system test firmware |

Both workflows can also be triggered manually from the **Actions** tab using
`workflow_dispatch`.

## Build Workflow (`build.yml`)

```
push / pull_request
       │
       ▼
 build-firmware          ← idf.py build (esp32c6, ESP-IDF v6.0)
       │
       ▼
 build-summary           ← prints pass/fail to job summary
```

Artifacts saved for 14 days: `build/*.bin`, `build/*.elf`, `build/*.map`.

## Test Workflow (`test.yml`)

```
push / pull_request
       │
       ├──► build-unit-tests   ← cd tests/unit  && idf.py build
       │
       └──► build-system-tests ← cd tests/system && idf.py build
                │
                ▼
         test-summary          ← prints pass/fail to job summary
```

Artifacts saved for 7 days: `tests/unit/build/` and `tests/system/build/`.

## ESP-IDF Version

All jobs run inside the `espressif/idf:v6.0` Docker container, matching the
`>=6.0.0` requirement declared in `main/idf_component.yml`.

## Scope

This is a hobby project. CI is used as a health check — the only goal is to
confirm the firmware and test targets compile without errors. Hardware-in-the-
loop execution and release-gating checks are out of scope.

## Further Reading

- Unit and system test details: [`docs/TESTING.md`](TESTING.md)
- Full test infrastructure: [`tests/README.md`](../tests/README.md)
- GitHub Actions documentation: <https://docs.github.com/en/actions>
