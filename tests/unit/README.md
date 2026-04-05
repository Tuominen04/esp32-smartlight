# Unit Tests

Pure logic tests that run on the **linux** target — no hardware required.
They execute in CI on every push and can be run locally via Docker.

## What Is Tested

| File | What it covers |
| --- | --- |
| `test_common_defs.c` | Buffer size invariants, NVS key length limits, default value sanity |
| `test_credential_validation.c` | WiFi credential validation: happy path, NULL/empty rejection, buffer overflow boundaries |

## Running Locally (Docker)

From the **repository root**:

```bash
docker run --rm -v "${PWD}:/project" espressif/idf:v6.0 bash -c \
  "rm -rf /project/tests/unit/build /project/tests/unit/sdkconfig /project/tests/unit/sdkconfig.old && \
   cd /project/tests/unit && \
   . /opt/esp/idf/export.sh && \
   idf.py --preview set-target linux && \
   idf.py build && \
   ./build/unit_test_app.elf"
```

Expected output ends with:

``` txt
X Tests 0 Failures 0 Ignored
OK
```

Exit code `0` = all pass. Exit code `1` = at least one failure.

## Running in CI

The `Build and Run Unit Tests` job in `.github/workflows/test.yml` runs automatically
on every push to `main` or `develop` and on pull requests. It builds for the linux
target and executes the binary — no hardware needed.

## Adding a New Test

1. Create `tests/unit/test_main/test_<topic>.c`.
1. Write functions with the `test_` prefix using Unity macros:

    ```c
    #include "unity.h"
    #include "common_defs.h"

    void test_my_invariant(void)
    {
      TEST_ASSERT_GREATER_THAN(0, SOME_CONSTANT);
    }
    ```

1. Declare the function as `extern` and call `RUN_TEST` in `test_main.c`.
1. Add the `.c` file to the `SRCS` list in `test_main/CMakeLists.txt`.

## Constraints

- **No hardware APIs** — do not include `esp_wifi.h`, `nvs_flash.h`, FreeRTOS task
  APIs, or any component that requires physical peripherals.
- Only `unity` is listed in `REQUIRES`; add nothing else without a strong reason.
- Tests must be deterministic and order-independent.
