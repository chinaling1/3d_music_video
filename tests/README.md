# 3D Video Studio - Testing Guide

## Overview

This document provides comprehensive information about the testing infrastructure and execution procedures for the 3D Video Studio project.

## Test Structure

### Directory Layout

```
tests/
├── CMakeLists.txt              # Test build configuration
├── test_main.cpp               # Test entry point
├── utils/                      # Testing utilities
│   ├── test_utils.h/cpp        # Common test utilities
│   ├── test_data_generator.h/cpp # Test data generators
│   └── test_reporter.h/cpp    # Test reporting system
├── core/                       # Core module tests
│   ├── test_version_control.cpp
│   ├── test_memory_pool.cpp
│   ├── test_object_manager.cpp
│   ├── test_event_system.cpp
│   └── test_logger.cpp
├── concurrency/                # Concurrency module tests
│   ├── test_thread_pool.cpp
│   ├── test_task_scheduler.cpp
│   └── test_async_executor.cpp
├── animation/                  # Animation module tests
│   ├── test_skeleton.cpp
│   ├── test_pose.cpp
│   ├── test_curve.cpp
│   ├── test_ik_solver.cpp
│   └── test_animation_clip.cpp
├── modeling/                   # Modeling module tests
│   ├── test_mesh.cpp
│   ├── test_vertex.cpp
│   ├── test_face.cpp
│   ├── test_edge.cpp
│   ├── test_mesh_operations.cpp
│   ├── test_subdivision.cpp
│   └── test_material.cpp
├── audio/                      # Audio module tests
│   ├── test_audio_buffer.cpp
│   ├── test_audio_engine.cpp
│   ├── test_audio_effect.cpp
│   └── test_vocal_synthesizer.cpp
└── io/                         # I/O module tests
    ├── test_file_loader.cpp
    ├── test_file_saver.cpp
    ├── test_serializer.cpp
    ├── test_format_converter.cpp
    └── test_asset_manager.cpp
```

## Test Categories

### 1. Unit Tests
- Test individual classes and methods
- Fast execution (milliseconds)
- No external dependencies
- High coverage of core functionality

### 2. Integration Tests
- Test interactions between modules
- Medium execution time (seconds)
- May use external resources
- Verify system integration

### 3. Performance Tests
- Benchmark critical operations
- Measure execution time
- Identify bottlenecks
- Track performance regressions

### 4. Stress Tests
- Test system under high load
- Verify stability
- Check memory usage
- Identify resource leaks

## Running Tests

### Prerequisites

- CMake 3.20 or higher
- C++20 compatible compiler
- Google Test framework
- Python 3.6+ (for report generation)

### Build Tests

```bash
# Create build directory
mkdir build && cd build

# Configure with tests enabled
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON

# Build tests
cmake --build . --target v3d_tests
```

### Execute Tests

#### Linux/Mac
```bash
# Run all tests
./build/bin/v3d_tests

# Run with verbose output
./build/bin/v3d_tests --gtest_verbose

# Run specific test suite
./build/bin/v3d_tests --gtest_filter=CoreTest.*

# Run specific test
./build/bin/v3d_tests --gtest_filter=CoreTest.VersionControlTest.ExecuteCommand
```

#### Windows
```cmd
# Run all tests
build\bin\v3d_tests.exe

# Run with verbose output
build\bin\v3d_tests.exe --gtest_verbose

# Run specific test suite
build\bin\v3d_tests.exe --gtest_filter=CoreTest.*
```

### Using Test Scripts

#### Linux/Mac
```bash
chmod +x run_tests.sh
./run_tests.sh
```

#### Windows
```cmd
run_tests.bat
```

## Test Coverage

### Coverage Goals

- **Code Coverage**: ≥ 80%
- **Branch Coverage**: ≥ 70%
- **Critical Path Coverage**: 100%

### Generating Coverage Reports

```bash
# Build with coverage flags
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -DENABLE_COVERAGE=ON

# Run tests with coverage
./build/bin/v3d_tests

# Generate coverage report
make test_coverage
```

Coverage reports will be generated in `build/coverage_html/`.

## Test Reports

### Report Types

1. **XML Report**: Machine-readable test results
2. **JSON Report**: Structured test data
3. **HTML Report**: Interactive web-based report
4. **Text Report**: Plain text summary

### Report Locations

- XML: `test_reports/test_results.xml`
- JSON: `test_reports/summary.json`
- HTML: `test_reports/report.html`
- Text: `test_reports/summary.txt`

### Understanding Test Results

#### Success Indicators
- All tests passed
- No failures or errors
- Execution time within expected range
- Memory usage within limits

#### Failure Indicators
- Test assertion failed
- Unhandled exception
- Timeout
- Memory leak

## Test Utilities

### TestUtils

Common utility functions for testing:

```cpp
// Generate random data
std::string str = TestUtils::generateRandomString(10);
int num = TestUtils::generateRandomInt(0, 100);
float val = TestUtils::generateRandomFloat(0.0f, 1.0f);

// File operations
bool exists = TestUtils::fileExists("path/to/file");
TestUtils::createDirectory("test_temp");
TestUtils::deleteDirectory("test_temp");

// Timing
TestUtils::measureTime("Operation name", []() {
    // Code to measure
});

// Assertions
TestUtils::assertNear(expected, actual, tolerance);
```

### TestDataGenerator

Generate test data for various modules:

```cpp
// Mesh generation
auto cube = TestDataGenerator::createCubeMesh();
auto sphere = TestDataGenerator::createSphereMesh(32);
auto plane = TestDataGenerator::createPlaneMesh(10);

// Animation generation
auto skeleton = TestDataGenerator::createSimpleSkeleton();
auto humanoid = TestDataGenerator::createHumanoidSkeleton();

// Audio generation
auto silence = TestDataGenerator::createSilentBuffer(44100, 1.0f);
auto sine = TestDataGenerator::createSineWaveBuffer(440.0f, 44100, 1.0f);
auto noise = TestDataGenerator::createNoiseBuffer(44100, 1.0f);
```

### TestReporter

Generate and manage test reports:

```cpp
// Start test suite
TestReporter::getInstance().startTestSuite("ModuleName");

// Record test results
TestReporter::getInstance().startTest("TestName");
// ... run test ...
TestReporter::getInstance().endTest(passed, errorMessage);

// Generate reports
TestReporter::getInstance().generateHTMLReport("report.html");
TestReporter::getInstance().generateJSONReport("report.json");
TestReporter::getInstance().generateTextReport("report.txt");
```

## Writing New Tests

### Test Structure

```cpp
#include <gtest/gtest.h>
#include "module_under_test.h"

class ModuleTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code
    }

    void TearDown() override {
        // Cleanup code
    }

    // Test fixtures
};

TEST_F(ModuleTest, TestName) {
    // Arrange
    // Act
    // Assert
    EXPECT_EQ(expected, actual);
}
```

### Best Practices

1. **Test Naming**: Use descriptive names
   - Good: `CalculateBoundingBox_ReturnsCorrectBounds`
   - Bad: `Test1`

2. **AAA Pattern**: Arrange, Act, Assert
   ```cpp
   TEST_F(MeshTest, CalculateBoundingBox) {
       // Arrange
       auto mesh = createTestMesh();
       
       // Act
       mesh->calculateBoundingBox();
       
       // Assert
       EXPECT_EQ(mesh->getMin(), expectedMin);
   }
   ```

3. **Independent Tests**: Each test should be independent
4. **Fast Execution**: Keep tests fast
5. **Clear Assertions**: Use descriptive assertion messages

### Test Categories

```cpp
// Normal case
TEST_F(ModuleTest, NormalOperation) {
    EXPECT_EQ(result, expected);
}

// Edge case
TEST_F(ModuleTest, EmptyInput) {
    EXPECT_THROW(operation(), std::invalid_argument);
}

// Boundary case
TEST_F(ModuleTest, MaximumValue) {
    EXPECT_EQ(result, MAX_VALUE);
}

// Performance test
TEST_F(ModuleTest, Performance) {
    TestUtils::measureTime("Operation", []() {
        operation();
    });
}
```

## Continuous Integration

### GitHub Actions

```yaml
name: Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Configure
        run: cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
      - name: Build
        run: cmake --build build --target v3d_tests
      - name: Test
        run: ./build/bin/v3d_tests
      - name: Coverage
        run: make test_coverage
```

## Troubleshooting

### Common Issues

1. **Build Failures**
   - Check CMake version
   - Verify dependencies are installed
   - Check compiler compatibility

2. **Test Failures**
   - Review test output
   - Check test logs
   - Verify test environment

3. **Coverage Issues**
   - Ensure coverage flags are set
   - Check that tests are actually running
   - Verify coverage tool is installed

### Getting Help

- Check test logs in `test_output/`
- Review XML results in `test_reports/`
- Consult module documentation
- Contact development team

## Test Metrics

### Current Status

- **Total Tests**: 200+
- **Test Suites**: 25+
- **Code Coverage**: Target 80%
- **Execution Time**: ~5 minutes

### Module Coverage

| Module | Tests | Coverage | Status |
|--------|--------|----------|--------|
| Core | 50+ | 85% | ✅ |
| Concurrency | 40+ | 80% | ✅ |
| Animation | 30+ | 75% | 🔄 |
| Modeling | 35+ | 70% | 🔄 |
| Audio | 25+ | 65% | 🔄 |
| I/O | 20+ | 60% | 🔄 |

## Contributing

When adding new features:

1. Write tests first (TDD)
2. Ensure all tests pass
3. Maintain coverage ≥ 80%
4. Update documentation
5. Run full test suite before committing

## Resources

- [Google Test Documentation](https://google.github.io/googletest/)
- [CMake Documentation](https://cmake.org/documentation/)
- [C++ Testing Best Practices](https://github.com/cpp-best-practices/cpp_test_best_practices)
