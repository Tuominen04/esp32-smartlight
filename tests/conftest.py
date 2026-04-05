"""
Pytest configuration and shared fixtures for ESP32 Smart Light tests.

This module provides common fixtures and configuration for both unit and system tests.
It uses pytest-embedded for ESP-IDF test execution.
"""

import os
import pytest
from pathlib import Path


# Root directory of the project
PROJECT_ROOT = Path(__file__).parent.parent


def pytest_addoption(parser):
  """Add custom command-line options for pytest."""
  parser.addoption(
    "--port",
    action="store",
    default=None,
    help="Serial port for DUT connection (e.g., /dev/ttyUSB0 or COM3)"
  )
  parser.addoption(
    "--target",
    action="store",
    default="esp32c6",
    help="ESP-IDF target chip (default: esp32c6)"
  )


@pytest.fixture(scope="session")
def project_root():
  """Return the project root directory."""
  return PROJECT_ROOT


@pytest.fixture(scope="session")
def serial_port(request):
  """Get serial port from command line or environment."""
  port = request.config.getoption("--port")
  if not port:
    port = os.environ.get("ESPPORT")
  return port


@pytest.fixture(scope="session")
def target_chip(request):
  """Get target chip from command line."""
  return request.config.getoption("--target")


@pytest.fixture(scope="session")
def unit_test_build_dir(project_root):
  """Return the unit test build directory."""
  return project_root / "tests" / "unit" / "build"


@pytest.fixture(scope="session")
def system_test_build_dir(project_root):
  """Return the system test build directory."""
  return project_root / "tests" / "system" / "build"


@pytest.fixture(scope="module")
def test_results_dir(project_root):
  """Create and return test results directory."""
  results_dir = project_root / "tests" / "test-results"
  results_dir.mkdir(parents=True, exist_ok=True)
  return results_dir


# Configuration for pytest-embedded
# This is used when pytest-embedded is available
def pytest_configure(config):
  """Configure pytest with custom settings."""
  # Register custom markers
  config.addinivalue_line(
    "markers", "unit: mark test as a unit test"
  )
  config.addinivalue_line(
    "markers", "system: mark test as a system test"
  )
  config.addinivalue_line(
    "markers", "slow: mark test as slow running"
  )
