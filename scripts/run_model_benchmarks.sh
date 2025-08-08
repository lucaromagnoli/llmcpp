#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

if [[ ! -f .env && -z "${OPENAI_API_KEY:-}" ]]; then
  echo "ERROR: OPENAI_API_KEY not set and .env missing." >&2
  exit 1
fi

# shellcheck disable=SC1091
set -a && source .env 2>/dev/null || true && set +a

build_dir=cmake-build-bench
cmake -S . -B "$build_dir" -G Ninja -DCMAKE_BUILD_TYPE=Release -DLLMCPP_BUILD_TESTS=ON >/dev/null
cmake --build "$build_dir" -j >/dev/null

echo "model,ms,status"
LLMCPP_RUN_BENCHMARKS=1 "$build_dir"/tests/llmcpp_tests "[openai][integration][benchmark]" --reporter compact 2>/dev/null |
  awk -F, '/^gpt|^o[13]-|^o4-mini/ { print $0 }'


