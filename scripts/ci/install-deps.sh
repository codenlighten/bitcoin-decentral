#!/usr/bin/env bash
set -euxo pipefail

# Retries to reduce transient apt flakiness
retry() { for i in {1..3}; do "$@" && break || sleep 5; done; }

retry sudo apt-get update -y
retry sudo apt-get install -y --no-install-recommends \
  build-essential cmake ninja-build pkg-config \
  curl ca-certificates git python3 python3-pip \
  libssl-dev libevent-dev libzmq3-dev libleveldb-dev \
  libsqlite3-dev libminiupnpc-dev libnatpmp-dev \
  libsecp256k1-dev libcrc32c-dev libboost-all-dev

# BerkeleyDB deliberately omitted (wallet disabled by default)
