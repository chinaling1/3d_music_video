#include "test_reporter.h"
#include <iostream>
#include <algorithm>

namespace v3d {
namespace test {

TestReporter& TestReporter::getInstance() {
    static TestReporter instance;
    return instance;
}

TestReporter::TestReporter()
    : currentModule_(nullptr)
    , currentTest_(nullptr) {
}

TestReporter::~TestReporter() {
}

void TestReporter::startTestSuite(const std::string& suiteName) {
    suiteStartTime_ = std::chrono::high_resolution_clock::now();

    ModuleTestSummary summary;
    summary.moduleName = suiteName;
    summary.totalTests = 0;
    summary.passedTests = 0;
    summary.failedTests = 0;
    summary.skippedTests = 0;
    summary.totalExecutionTimeMs = 0.0;

    moduleSummaries_.push_back(summary);
    currentModule_ = &moduleSummaries_.back();

    std::cout << "\n========================================\n";
    std::cout << "Test Suite: " << suiteName << "\n";
    std::cout << "========================================\n";
}

void TestReporter::endTestSuite() {
    if (!currentModule_) return;

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(endTime - suiteStartTime_).count();
    currentModule_->totalExecutionTimeMs = duration;

    std::cout << "\n----------------------------------------\n";
    std::cout << "Suite Summary: " << currentModule_->moduleName << "\n";
    std::cout << "Total Tests: " << currentModule_->totalTests << "\n";
    std::cout << "Passed: " << currentModule_->passedTests << "\n";
    std::cout << "Failed: " << currentModule_->failedTests << "\n";
    std::cout << "Skipped: " << currentModule_->skippedTests << "\n";
    std::cout << "Execution Time: " << formatDuration(duration) << "\n";
    std::cout << "----------------------------------------\n";

    currentModule_ = nullptr;
}

void TestReporter::startTest(const std::string& testName) {
    testStartTime_ = std::chrono::high_resolution_clock::now();

    if (currentModule_) {
        currentModule_->totalTests++;
    }

    TestResult result;
    result.testName = testName;
    result.passed = false;
    result.errorMessage = "";
    result.executionTimeMs = 0.0;
    result.memoryUsageBytes = 0;

    if (currentModule_) {
        currentModule_->testResults.push_back(result);
        currentTest_ = &currentModule_->testResults.back();
    }

    std::cout << "  Running: " << testName << "... " << std::flush;
}

void TestReporter::endTest(bool passed, const std::string& errorMessage) {
    if (!currentTest_) return;

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(endTime - testStartTime_).count();

    currentTest_->passed = passed;
    currentTest_->errorMessage = errorMessage;
    currentTest_->executionTimeMs = duration;

    if (passed) {
        if (currentModule_) {
            currentModule_->passedTests++;
        }
        std::cout << "PASSED (" << formatDuration(duration) << ")\n";
    } else {
        if (currentModule_) {
            currentModule_->failedTests++;
        }
        std::cout << "FAILED (" << formatDuration(duration) << ")\n";
        if (!errorMessage.empty()) {
            std::cout << "    Error: " << errorMessage << "\n";
        }
    }

    currentTest_ = nullptr;
}

void TestReporter::recordTestResult(const TestResult& result) {
    if (currentModule_) {
        currentModule_->testResults.push_back(result);
        currentModule_->totalTests++;

        if (result.passed) {
            currentModule_->passedTests++;
        } else {
            currentModule_->failedTests++;
        }
    }
}

void TestReporter::generateHTMLReport(const std::string& outputPath) const {
    std::ofstream file(outputPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open HTML report file: " << outputPath << std::endl;
        return;
    }

    file << "<!DOCTYPE html>\n";
    file << "<html lang=\"en\">\n";
    file << "<head>\n";
    file << "  <meta charset=\"UTF-8\">\n";
    file << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    file << "  <title>3D Video Studio - Test Report</title>\n";
    file << "  <style>\n";
    file << "    body { font-family: Arial, sans-serif; margin: 20px; background-color: #f5f5f5; }\n";
    file << "    .container { max-width: 1200px; margin: 0 auto; background-color: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }\n";
    file << "    h1 { color: #333; border-bottom: 2px solid #007bff; padding-bottom: 10px; }\n";
    file << "    h2 { color: #555; margin-top: 30px; }\n";
    file << "    .summary { display: flex; gap: 20px; margin: 20px 0; }\n";
    file << "    .summary-card { flex: 1; padding: 15px; border-radius: 5px; text-align: center; }\n";
    file << "    .card-total { background-color: #e3f2fd; }\n";
    file << "    .card-passed { background-color: #e8f5e9; }\n";
    file << "    .card-failed { background-color: #ffebee; }\n";
    file << "    .card-time { background-color: #fff3e0; }\n";
    file << "    .card-number { font-size: 32px; font-weight: bold; }\n";
    file << "    .card-label { color: #666; margin-top: 5px; }\n";
    file << "    .module { margin: 20px 0; border: 1px solid #ddd; border-radius: 5px; overflow: hidden; }\n";
    file << "    .module-header { background-color: #007bff; color: white; padding: 15px; }\n";
    file << "    .module-stats { display: flex; gap: 15px; padding: 10px 15px; background-color: #f8f9fa; }\n";
    file << "    .test-table { width: 100%; border-collapse: collapse; }\n";
    file << "    .test-table th, .test-table td { padding: 10px; text-align: left; border-bottom: 1px solid #ddd; }\n";
    file << "    .test-table th { background-color: #f8f9fa; }\n";
    file << "    .test-passed { color: #28a745; }\n";
    file << "    .test-failed { color: #dc3545; }\n";
    file << "    .error-message { color: #dc3545; font-size: 12px; margin-top: 5px; }\n";
    file << "  </style>\n";
    file << "</head>\n";
    file << "<body>\n";
    file << "  <div class=\"container\">\n";
    file << "    <h1>3D Video Studio - Test Report</h1>\n";
    file << "    <p>Generated: " << getCurrentTimestamp() << "</p>\n";

    file << "    <div class=\"summary\">\n";
    file << "      <div class=\"summary-card card-total\">\n";
    file << "        <div class=\"card-number\">" << getTotalTests() << "</div>\n";
    file << "        <div class=\"card-label\">Total Tests</div>\n";
    file << "      </div>\n";
    file << "      <div class=\"summary-card card-passed\">\n";
    file << "        <div class=\"card-number\">" << getTotalPassed() << "</div>\n";
    file << "        <div class=\"card-label\">Passed</div>\n";
    file << "      </div>\n";
    file << "      <div class=\"summary-card card-failed\">\n";
    file << "        <div class=\"card-number\">" << getTotalFailed() << "</div>\n";
    file << "        <div class=\"card-label\">Failed</div>\n";
    file << "      </div>\n";
    file << "      <div class=\"summary-card card-time\">\n";
    file << "        <div class=\"card-number\">" << formatDuration(getTotalExecutionTime()) << "</div>\n";
    file << "        <div class=\"card-label\">Total Time</div>\n";
    file << "      </div>\n";
    file << "    </div>\n";

    for (const auto& module : moduleSummaries_) {
        file << "    <div class=\"module\">\n";
        file << "      <div class=\"module-header\">\n";
        file << "        <h2>" << module.moduleName << "</h2>\n";
        file << "      </div>\n";
        file << "      <div class=\"module-stats\">\n";
        file << "        <span>Total: " << module.totalTests << "</span>\n";
        file << "        <span>Passed: " << module.passedTests << "</span>\n";
        file << "        <span>Failed: " << module.failedTests << "</span>\n";
        file << "        <span>Time: " << formatDuration(module.totalExecutionTimeMs) << "</span>\n";
        file << "      </div>\n";
        file << "      <table class=\"test-table\">\n";
        file << "        <thead>\n";
        file << "          <tr>\n";
        file << "            <th>Test Name</th>\n";
        file << "            <th>Status</th>\n";
        file << "            <th>Time</th>\n";
        file << "            <th>Memory</th>\n";
        file << "          </tr>\n";
        file << "        </thead>\n";
        file << "        <tbody>\n";

        for (const auto& test : module.testResults) {
            file << "          <tr>\n";
            file << "            <td>" << test.testName << "</td>\n";
            file << "            <td class=\"" << (test.passed ? "test-passed" : "test-failed") << "\">"
                 << (test.passed ? "PASSED" : "FAILED") << "</td>\n";
            file << "            <td>" << formatDuration(test.executionTimeMs) << "</td>\n";
            file << "            <td>" << formatMemory(test.memoryUsageBytes) << "</td>\n";
            file << "          </tr>\n";
            if (!test.passed && !test.errorMessage.empty()) {
                file << "          <tr>\n";
                file << "            <td colspan=\"4\" class=\"error-message\">" << test.errorMessage << "</td>\n";
                file << "          </tr>\n";
            }
        }

        file << "        </tbody>\n";
        file << "      </table>\n";
        file << "    </div>\n";
    }

    file << "  </div>\n";
    file << "</body>\n";
    file << "</html>\n";

    file.close();
}

void TestReporter::generateJSONReport(const std::string& outputPath) const {
    std::ofstream file(outputPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open JSON report file: " << outputPath << std::endl;
        return;
    }

    file << "{\n";
    file << "  \"timestamp\": \"" << getCurrentTimestamp() << "\",\n";
    file << "  \"summary\": {\n";
    file << "    \"totalTests\": " << getTotalTests() << ",\n";
    file << "    \"passedTests\": " << getTotalPassed() << ",\n";
    file << "    \"failedTests\": " << getTotalFailed() << ",\n";
    file << "    \"totalExecutionTimeMs\": " << getTotalExecutionTime() << "\n";
    file << "  },\n";
    file << "  \"modules\": [\n";

    for (size_t i = 0; i < moduleSummaries_.size(); ++i) {
        const auto& module = moduleSummaries_[i];
        file << "    {\n";
        file << "      \"name\": \"" << module.moduleName << "\",\n";
        file << "      \"totalTests\": " << module.totalTests << ",\n";
        file << "      \"passedTests\": " << module.passedTests << ",\n";
        file << "      \"failedTests\": " << module.failedTests << ",\n";
        file << "      \"skippedTests\": " << module.skippedTests << ",\n";
        file << "      \"executionTimeMs\": " << module.totalExecutionTimeMs << ",\n";
        file << "      \"tests\": [\n";

        for (size_t j = 0; j < module.testResults.size(); ++j) {
            const auto& test = module.testResults[j];
            file << "        {\n";
            file << "          \"name\": \"" << test.testName << "\",\n";
            file << "          \"passed\": " << (test.passed ? "true" : "false") << ",\n";
            file << "          \"executionTimeMs\": " << test.executionTimeMs << ",\n";
            file << "          \"memoryUsageBytes\": " << test.memoryUsageBytes;
            if (!test.errorMessage.empty()) {
                file << ",\n";
                file << "          \"errorMessage\": \"" << test.errorMessage << "\"";
            }
            file << "\n        }";
            if (j < module.testResults.size() - 1) file << ",";
            file << "\n";
        }

        file << "      ]\n";
        file << "    }";
        if (i < moduleSummaries_.size() - 1) file << ",";
        file << "\n";
    }

    file << "  ]\n";
    file << "}\n";

    file.close();
}

void TestReporter::generateTextReport(const std::string& outputPath) const {
    std::ofstream file(outputPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open text report file: " << outputPath << std::endl;
        return;
    }

    file << "========================================\n";
    file << "3D Video Studio - Test Report\n";
    file << "========================================\n";
    file << "Generated: " << getCurrentTimestamp() << "\n\n";

    file << "Summary:\n";
    file << "  Total Tests: " << getTotalTests() << "\n";
    file << "  Passed: " << getTotalPassed() << "\n";
    file << "  Failed: " << getTotalFailed() << "\n";
    file << "  Total Time: " << formatDuration(getTotalExecutionTime()) << "\n\n";

    for (const auto& module : moduleSummaries_) {
        file << "----------------------------------------\n";
        file << "Module: " << module.moduleName << "\n";
        file << "----------------------------------------\n";
        file << "  Total: " << module.totalTests << "\n";
        file << "  Passed: " << module.passedTests << "\n";
        file << "  Failed: " << module.failedTests << "\n";
        file << "  Time: " << formatDuration(module.totalExecutionTimeMs) << "\n\n";

        for (const auto& test : module.testResults) {
            file << "  " << test.testName << ": ";
            file << (test.passed ? "PASSED" : "FAILED");
            file << " (" << formatDuration(test.executionTimeMs) << ")\n";
            if (!test.passed && !test.errorMessage.empty()) {
                file << "    Error: " << test.errorMessage << "\n";
            }
        }
        file << "\n";
    }

    file.close();
}

void TestReporter::setOutputDirectory(const std::string& path) {
    outputDirectory_ = path;
}

std::string TestReporter::getOutputDirectory() const {
    return outputDirectory_;
}

const std::vector<ModuleTestSummary>& TestReporter::getModuleSummaries() const {
    return moduleSummaries_;
}

double TestReporter::getTotalExecutionTime() const {
    double total = 0.0;
    for (const auto& module : moduleSummaries_) {
        total += module.totalExecutionTimeMs;
    }
    return total;
}

int TestReporter::getTotalTests() const {
    int total = 0;
    for (const auto& module : moduleSummaries_) {
        total += module.totalTests;
    }
    return total;
}

int TestReporter::getTotalPassed() const {
    int total = 0;
    for (const auto& module : moduleSummaries_) {
        total += module.passedTests;
    }
    return total;
}

int TestReporter::getTotalFailed() const {
    int total = 0;
    for (const auto& module : moduleSummaries_) {
        total += module.failedTests;
    }
    return total;
}

void TestReporter::clear() {
    moduleSummaries_.clear();
    currentModule_ = nullptr;
    currentTest_ = nullptr;
}

std::string TestReporter::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string TestReporter::formatDuration(double milliseconds) const {
    if (milliseconds < 1.0) {
        return std::to_string(static_cast<int>(milliseconds * 1000)) + " μs";
    } else if (milliseconds < 1000.0) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << milliseconds << " ms";
        return ss.str();
    } else {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << (milliseconds / 1000.0) << " s";
        return ss.str();
    }
}

std::string TestReporter::formatMemory(size_t bytes) const {
    const double KB = 1024.0;
    const double MB = KB * 1024.0;
    const double GB = MB * 1024.0;

    if (bytes < KB) {
        return std::to_string(bytes) + " B";
    } else if (bytes < MB) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << (bytes / KB) << " KB";
        return ss.str();
    } else if (bytes < GB) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << (bytes / MB) << " MB";
        return ss.str();
    } else {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << (bytes / GB) << " GB";
        return ss.str();
    }
}

}
}
