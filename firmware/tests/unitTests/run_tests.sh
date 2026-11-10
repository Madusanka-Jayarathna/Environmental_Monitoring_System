#!/usr/bin/env bash
# Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
# Use of this source code is governed by the MIT license.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

echo "=========================================================="
echo "  Building and Running Host Unit Tests + Code Coverage   "
echo "=========================================================="

mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
make -j

echo ""
./run_unit_tests
echo ""

# Merge LLVM profiling data
if [ -f "default.profraw" ]; then
    echo "=========================================================="
    echo "                CODE COVERAGE SUMMARY                    "
    echo "=========================================================="
    xcrun llvm-profdata merge -sparse default.profraw -o coverage.profdata
    xcrun llvm-cov report ./run_unit_tests -instr-profile=coverage.profdata -sources "${SCRIPT_DIR}/../../components"
    
    # Generate HTML coverage report if requested with --html
    if [ "$1" == "--html" ]; then
        REPORT_DIR="${BUILD_DIR}/coverage_html"
        xcrun llvm-cov show ./run_unit_tests -instr-profile=coverage.profdata -sources "${SCRIPT_DIR}/../../components" -format=html -output-dir="${REPORT_DIR}"
        echo ""
        echo "[+] HTML coverage report generated at: ${REPORT_DIR}/index.html"
        open "${REPORT_DIR}/index.html" || true
    fi
fi
