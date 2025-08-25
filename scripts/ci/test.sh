#!/usr/bin/env bash
set -euxo pipefail
ctest --test-dir build --output-on-failure --parallel
