#!/usr/bin/env bash

set -euo pipefail

CMD=${1:-}
BUILD_DIR=${2:-build}

if [[ -z "$CMD" ]]; then
  echo "Usage: ./builder <clean|build> [build_dir]"
  exit 1
fi

case "$CMD" in
  clean)
    echo "[builder] Cleaning build directory: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
    ;;

  build)
    echo "[builder] Building in: $BUILD_DIR"
    cmake -S . -B "$BUILD_DIR"
    cmake --build "$BUILD_DIR"
    ;;

  *)
    echo "[builder] Unknown command: $CMD"
    echo "Usage: ./builder <clean|build> [build_dir]"
    exit 1
    ;;
esac