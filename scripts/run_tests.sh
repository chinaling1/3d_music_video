#!/bin/bash

set -e

echo "========================================"
echo "3D Video Studio - Test Execution Script"
echo "========================================"

BUILD_DIR="build"
TEST_OUTPUT_DIR="test_output"
TEST_REPORT_DIR="test_reports"

echo ""
echo "Step 1: Creating build directory..."
if [ ! -d "$BUILD_DIR" ]; then
    mkdir -p "$BUILD_DIR"
fi

echo ""
echo "Step 2: Configuring CMake..."
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON

echo ""
echo "Step 3: Building tests..."
cmake --build . --target v3d_tests -j$(nproc)

echo ""
echo "Step 4: Creating output directories..."
cd ..
mkdir -p "$TEST_OUTPUT_DIR"
mkdir -p "$TEST_REPORT_DIR"

echo ""
echo "Step 5: Running tests..."
cd "$BUILD_DIR"
./bin/v3d_tests --gtest_output=xml:"../$TEST_REPORT_DIR/test_results.xml"

echo ""
echo "Step 6: Generating test reports..."
cd ..

python3 << 'EOF'
import xml.etree.ElementTree as ET
import json
from datetime import datetime

tree = ET.parse('test_reports/test_results.xml')
root = tree.getroot()

total_tests = int(root.attrib['tests'])
failures = int(root.attrib['failures'])
disabled = int(root.attrib['disabled'])
errors = int(root.attrib['errors'])
time = float(root.attrib['time'])

summary = {
    'timestamp': datetime.now().isoformat(),
    'total_tests': total_tests,
    'passed': total_tests - failures - errors,
    'failures': failures,
    'disabled': disabled,
    'errors': errors,
    'time': time,
    'success_rate': (total_tests - failures - errors) / total_tests * 100 if total_tests > 0 else 0
}

with open('test_reports/summary.json', 'w') as f:
    json.dump(summary, f, indent=2)

print(f"\n========================================")
print(f"Test Summary")
print(f"========================================")
print(f"Total Tests: {summary['total_tests']}")
print(f"Passed: {summary['passed']}")
print(f"Failed: {summary['failures']}")
print(f"Disabled: {summary['disabled']}")
print(f"Errors: {summary['errors']}")
print(f"Execution Time: {summary['time']:.2f}s")
print(f"Success Rate: {summary['success_rate']:.2f}%")
print(f"========================================")

if summary['failures'] > 0 or summary['errors'] > 0:
    print(f"\n⚠️  Some tests failed. Check test_reports/test_results.xml for details.")
    exit(1)
else:
    print(f"\n✅ All tests passed!")
    exit(0)
EOF

echo ""
echo "Test execution completed!"
