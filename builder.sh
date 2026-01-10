#!/usr/bin/env bash

set -euo pipefail

CMD=${1:-}
BUILD_DIR=${2:-build}
INVARIANT_PROFILE=${3:-default}
DEBUG_FLAG=${4:-}

if [[ -z "$CMD" ]]; then
  echo "Usage: ./builder <clean|build> [build_dir] [profile] [-d]"
  exit 1
fi

# Determine build type
BUILD_TYPE="Release"
if [[ "$DEBUG_FLAG" == "-d" ]]; then
  BUILD_TYPE="Debug"
fi

case "$CMD" in
  clean)
    echo "[builder] Cleaning build directory: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
    ;;

  build)
    echo "[builder] Building in: $BUILD_DIR"
    echo "[builder] Build type: $BUILD_TYPE"
    conan install . --output-folder=$BUILD_DIR \
                    --build=missing \
                    --profile:host=$INVARIANT_PROFILE \
                    --profile:build=$INVARIANT_PROFILE
    cmake -S . -B "$BUILD_DIR" \
          -DCMAKE_TOOLCHAIN_FILE=$BUILD_DIR/conan_toolchain.cmake \
          -DCMAKE_BUILD_TYPE=$BUILD_TYPE
    cmake --build "$BUILD_DIR"
    ;;

  *)
    echo "[builder] Unknown command: $CMD"
    echo "Usage: ./builder <clean|build> [build_dir] [profile] [-d]"
    exit 1
    ;;
esac