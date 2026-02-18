#include "test_utils.h"
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <thread>

namespace v3d {
namespace test {

std::mt19937& TestUtils::getRandomGenerator() {
    static std::mt19937 generator(std::random_device{}());
    return generator;
}

std::string TestUtils::generateRandomString(size_t length) {
    const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string result;
    result.reserve(length);
    
    std::uniform_int_distribution<size_t> dist(0, chars.size() - 1);
    auto& gen = getRandomGenerator();
    
    for (size_t i = 0; i < length; ++i) {
        result += chars[dist(gen)];
    }
    
    return result;
}

int TestUtils::generateRandomInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(getRandomGenerator());
}

float TestUtils::generateRandomFloat(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(getRandomGenerator());
}

bool TestUtils::generateRandomBool() {
    std::uniform_int_distribution<int> dist(0, 1);
    return dist(getRandomGenerator()) == 1;
}

std::string TestUtils::getTestOutputPath() {
    return "test_output";
}

std::string TestUtils::getTestInputPath() {
    return "test_input";
}

std::string TestUtils::getTestTempPath() {
    return "test_temp";
}

bool TestUtils::createDirectory(const std::string& path) {
    try {
        return std::filesystem::create_directories(path);
    } catch (const std::exception&) {
        return false;
    }
}

bool TestUtils::deleteDirectory(const std::string& path) {
    try {
        return std::filesystem::remove_all(path) > 0;
    } catch (const std::exception&) {
        return false;
    }
}

bool TestUtils::fileExists(const std::string& path) {
    return std::filesystem::exists(path);
}

void TestUtils::sleepMs(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void TestUtils::measureTime(const std::string& label, std::function<void()> func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << label << ": " << duration << " microseconds" << std::endl;
}

void TestUtils::assertNear(float expected, float actual, float tolerance) {
    float diff = std::abs(expected - actual);
    if (diff > tolerance) {
        std::ostringstream oss;
        oss << std::setprecision(10) << "Expected: " << expected 
            << ", Actual: " << actual << ", Tolerance: " << tolerance;
        throw std::runtime_error(oss.str());
    }
}

void TestUtils::assertNear(double expected, double actual, double tolerance) {
    double diff = std::abs(expected - actual);
    if (diff > tolerance) {
        std::ostringstream oss;
        oss << std::setprecision(15) << "Expected: " << expected 
            << ", Actual: " << actual << ", Tolerance: " << tolerance;
        throw std::runtime_error(oss.str());
    }
}

}
}
