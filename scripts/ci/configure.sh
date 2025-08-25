#!/usr/bin/env bash
set -euxo pipefail

cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_WALLET=OFF \
  -DENABLE_INTEROP=OFF \
  -DBUILD_TESTING=ON
