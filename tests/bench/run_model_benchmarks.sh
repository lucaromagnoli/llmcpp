#!/usr/bin/env bash
set -euo pipefail

# Resolve repo root (prefer git); fallback to two levels up from this script
if repo_root=$(git rev-parse --show-toplevel 2>/dev/null); then
  :
else
  script_dir="$(cd "$(dirname "$0")" && pwd)"
  repo_root="$(cd "$script_dir/../.." && pwd)"
fi

cd "$repo_root"

# Load API key from .env if present
if [[ ! -f .env && -z "${OPENAI_API_KEY:-}" ]]; then
  echo "ERROR: OPENAI_API_KEY not set and .env missing." >&2
  exit 1
fi
# shellcheck disable=SC1091
set -a && source .env 2>/dev/null || true && set +a

build_dir=cmake-build-bench
cmake -S . -B "$build_dir" -G Ninja -DCMAKE_BUILD_TYPE=Release -DLLMCPP_BUILD_TESTS=ON >/dev/null
cmake --build "$build_dir" -j >/dev/null

echo "model,ms,status,input_tokens,output_tokens,total_tokens"
LLMCPP_RUN_BENCHMARKS=1 "$build_dir"/tests/llmcpp_tests "[openai][integration][benchmark]" --reporter compact 2>/dev/null |
  awk -F, '/^gpt|^o[13]-|^o4-mini/ { print $0 }'


