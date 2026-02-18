#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <functional>
#include <random>

namespace v3d {
namespace test {

class TestUtils {
public:
    static std::string generateRandomString(size_t length);
    static int generateRandomInt(int min, int max);
    static float generateRandomFloat(float min, float max);
    static bool generateRandomBool();

    static std::string getTestOutputPath();
    static std::string getTestInputPath();
    static std::string getTestTempPath();

    static bool createDirectory(const std::string& path);
    static bool deleteDirectory(const std::string& path);
    static bool fileExists(const std::string& path);

    static void sleepMs(int milliseconds);

    template<typename T>
    static std::vector<T> generateRandomVector(size_t size, T min, T max);

    static void measureTime(const std::string& label, std::function<void()> func);
    static void assertNear(float expected, float actual, float tolerance = 0.0001f);
    static void assertNear(double expected, double actual, double tolerance = 0.000001);

private:
    static std::mt19937& getRandomGenerator();
};

template<typename T>
std::vector<T> TestUtils::generateRandomVector(size_t size, T min, T max) {
    std::vector<T> result;
    result.reserve(size);
    
    std::uniform_real_distribution<T> dist(min, max);
    auto& gen = getRandomGenerator();
    
    for (size_t i = 0; i < size; ++i) {
        result.push_back(dist(gen));
    }
    
    return result;
}

}
}
