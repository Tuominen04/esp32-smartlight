# Testing

The ESP32 Smart Light project has three test suites, each targeting a different
level of the stack.

| Suite | Hardware needed | What it covers |
| ---- | ---- | ---- |
| Unit | No (linux target) | Constants, buffer limits, credential validation logic |
| Integration | Yes (ESP32-C6) | Cross-component data flows between `device_info`, `wifi_manager`, and `nvs_manager` |
| On-target | Yes (ESP32-C6) | Individual component behaviour on real hardware: GPIO, NVS, WiFi, device info |

Unit tests run automatically in CI on every push. Integration and on-target tests
require a physical board and are run manually.

For build commands, Docker usage, and how to add new tests see the
[tests/README.md](../tests/README.md) and each suite's own README.

## CI Pipeline

Unit tests are built and executed in GitHub Actions (`test.yml`) using the linux
target inside the `espressif/idf:v6.0` Docker image — no hardware needed. The job
fails if any test assertion fails.

See [docs/CICD.md](CICD.md) for the full pipeline overview.

## Further Reading

- [tests/README.md](../tests/README.md) — suite overview and quick start
- [tests/unit/README.md](../tests/unit/README.md) — unit test details and CI usage
- [tests/integration/README.md](../tests/integration/README.md) — integration test details
- [tests/on_target/README.md](../tests/on_target/README.md) — on-target test details
- [ESP-IDF unit testing guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/unit-tests.html)
- [Unity test framework](https://github.com/ThrowTheSwitch/Unity)
