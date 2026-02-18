#pragma once

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>

namespace v3d {
namespace test {

struct TestResult {
    std::string testName;
    bool passed;
    std::string errorMessage;
    double executionTimeMs;
    size_t memoryUsageBytes;
};

struct ModuleTestSummary {
    std::string moduleName;
    int totalTests;
    int passedTests;
    int failedTests;
    int skippedTests;
    double totalExecutionTimeMs;
    std::vector<TestResult> testResults;
};

class TestReporter {
public:
    static TestReporter& getInstance();

    void startTestSuite(const std::string& suiteName);
    void endTestSuite();

    void startTest(const std::string& testName);
    void endTest(bool passed, const std::string& errorMessage = "");

    void recordTestResult(const TestResult& result);

    void generateHTMLReport(const std::string& outputPath) const;
    void generateJSONReport(const std::string& outputPath) const;
    void generateTextReport(const std::string& outputPath) const;

    void setOutputDirectory(const std::string& path);
    std::string getOutputDirectory() const;

    const std::vector<ModuleTestSummary>& getModuleSummaries() const;
    double getTotalExecutionTime() const;
    int getTotalTests() const;
    int getTotalPassed() const;
    int getTotalFailed() const;

    void clear();

private:
    TestReporter();
    ~TestReporter();

    std::string getCurrentTimestamp() const;
    std::string formatDuration(double milliseconds) const;
    std::string formatMemory(size_t bytes) const;

    std::string outputDirectory_;
    std::vector<ModuleTestSummary> moduleSummaries_;
    ModuleTestSummary* currentModule_;
    TestResult* currentTest_;
    std::chrono::high_resolution_clock::time_point testStartTime_;
    std::chrono::high_resolution_clock::time_point suiteStartTime_;
};

}
}
