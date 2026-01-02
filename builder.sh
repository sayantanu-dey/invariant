#!/usr/bin/env bash

set -euo pipefail

CMD=${1:-}
BUILD_DIR=${2:-build}
INVARIANT_PROFILE=${3:-default}

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
    conan install . --output-folder=$BUILD_DIR \
                    --build=missing \
                    --profile:host=$INVARIANT_PROFILE \
                    --profile:build=$INVARIANT_PROFILE \
    cmake -S . -B "$BUILD_DIR" -DCMAKE_TOOLCHAIN_FILE=$BUILD_DIR/conan_toolchain.cmake
    cmake --build "$BUILD_DIR"
    ;;

  *)
    echo "[builder] Unknown command: $CMD"
    echo "Usage: ./builder <clean|build> [build_dir]"
    exit 1
    ;;
esac