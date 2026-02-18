/**
 * @file ai_plugin_system.h
 * @brief AI插件系统 - 提供AI功能集成接口
 * 
 * 支持多种AI服务接入：
 * - OpenAI GPT/DALL-E
 * - Claude
 * - Stable Diffusion
 * - 本地模型
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <queue>
#include <condition_variable>

namespace v3d {
namespace ai {

enum class AIProvider {
    OpenAI,
    Claude,
    StableDiffusion,
    Local,
    Custom
};

enum class AIModelType {
    TextGeneration,
    ImageGeneration,
    ImageToImage,
    TextTo3D,
    ImageTo3D,
    Animation,
    VoiceSynthesis,
    VoiceRecognition
};

struct AIModelInfo {
    std::string id;
    std::string name;
    std::string description;
    AIProvider provider;
    AIModelType type;
    std::string version;
    int maxTokens;
    bool supportsStreaming;
    bool supportsImages;
    bool supportsAudio;
    std::vector<std::string> capabilities;
};

struct AIRequest {
    std::string id;
    std::string prompt;
    std::vector<std::string> images;
    std::vector<std::string> context;
    AIModelType type;
    std::string modelId;
    float temperature;
    int maxTokens;
    std::unordered_map<std::string, std::string> parameters;
    
    AIRequest() : type(AIModelType::TextGeneration), temperature(0.7f), maxTokens(2048) {}
};

struct AIResponse {
    std::string id;
    std::string requestId;
    bool success;
    std::string text;
    std::vector<uint8_t> imageData;
    int imageWidth;
    int imageHeight;
    std::string errorMessage;
    float processingTime;
    int tokensUsed;
    
    AIResponse() : success(false), imageWidth(0), imageHeight(0), processingTime(0), tokensUsed(0) {}
};

struct AIProviderConfig {
    AIProvider provider;
    std::string name;
    std::string apiKey;
    std::string apiEndpoint;
    std::string defaultModel;
    int timeout;
    int maxRetries;
    bool enabled;
    std::unordered_map<std::string, std::string> customHeaders;
    
    AIProviderConfig() : timeout(60), maxRetries(3), enabled(false) {}
};

class AIPluginInterface {
public:
    virtual ~AIPluginInterface() = default;
    
    virtual bool initialize(const AIProviderConfig& config) = 0;
    virtual void shutdown() = 0;
    
    virtual std::vector<AIModelInfo> getAvailableModels() const = 0;
    
    virtual void sendRequest(const AIRequest& request,
                            std::function<void(const AIResponse&)> callback) = 0;
    
    virtual void sendStreamingRequest(const AIRequest& request,
                                      std::function<void(const std::string&)> onChunk,
                                      std::function<void(const AIResponse&)> onComplete) = 0;
    
    virtual void cancelRequest(const std::string& requestId) = 0;
    
    virtual bool isReady() const = 0;
    virtual std::string getLastError() const = 0;
};

class OpenAIPlugin : public AIPluginInterface {
public:
    bool initialize(const AIProviderConfig& config) override;
    void shutdown() override;
    
    std::vector<AIModelInfo> getAvailableModels() const override;
    
    void sendRequest(const AIRequest& request,
                    std::function<void(const AIResponse&)> callback) override;
    
    void sendStreamingRequest(const AIRequest& request,
                              std::function<void(const std::string&)> onChunk,
                              std::function<void(const AIResponse&)> onComplete) override;
    
    void cancelRequest(const std::string& requestId) override;
    
    bool isReady() const override { return ready_; }
    std::string getLastError() const override { return lastError_; }

private:
    AIProviderConfig config_;
    bool ready_;
    std::string lastError_;
    std::unordered_map<std::string, bool> cancelledRequests_;
};

class ClaudePlugin : public AIPluginInterface {
public:
    bool initialize(const AIProviderConfig& config) override;
    void shutdown() override;
    std::vector<AIModelInfo> getAvailableModels() const override;
    void sendRequest(const AIRequest& request, std::function<void(const AIResponse&)> callback) override;
    void sendStreamingRequest(const AIRequest& request, std::function<void(const std::string&)> onChunk, std::function<void(const AIResponse&)> onComplete) override;
    void cancelRequest(const std::string& requestId) override;
    bool isReady() const override { return ready_; }
    std::string getLastError() const override { return lastError_; }

private:
    AIProviderConfig config_;
    bool ready_;
    std::string lastError_;
};

class StableDiffusionPlugin : public AIPluginInterface {
public:
    bool initialize(const AIProviderConfig& config) override;
    void shutdown() override;
    std::vector<AIModelInfo> getAvailableModels() const override;
    void sendRequest(const AIRequest& request, std::function<void(const AIResponse&)> callback) override;
    void sendStreamingRequest(const AIRequest& request, std::function<void(const std::string&)> onChunk, std::function<void(const AIResponse&)> onComplete) override;
    void cancelRequest(const std::string& requestId) override;
    bool isReady() const override { return ready_; }
    std::string getLastError() const override { return lastError_; }

private:
    AIProviderConfig config_;
    bool ready_;
    std::string lastError_;
};

class AIPluginManager {
public:
    static AIPluginManager& getInstance();
    
    void initialize();
    void shutdown();
    
    void registerProvider(AIProvider provider, std::unique_ptr<AIPluginInterface> plugin);
    void unregisterProvider(AIProvider provider);
    
    bool configureProvider(AIProvider provider, const AIProviderConfig& config);
    AIProviderConfig getProviderConfig(AIProvider provider) const;
    
    std::vector<AIProvider> getAvailableProviders() const;
    std::vector<AIModelInfo> getAvailableModels(AIProvider provider) const;
    std::vector<AIModelInfo> getModelsByType(AIModelType type) const;
    
    void sendRequest(AIProvider provider, const AIRequest& request,
                     std::function<void(const AIResponse&)> callback);
    
    void sendStreamingRequest(AIProvider provider, const AIRequest& request,
                              std::function<void(const std::string&)> onChunk,
                              std::function<void(const AIResponse&)> onComplete);
    
    void cancelRequest(const std::string& requestId);
    void cancelAllRequests();
    
    bool isProviderReady(AIProvider provider) const;
    std::string getLastError(AIProvider provider) const;
    
    void setDefaultProvider(AIProvider provider);
    AIProvider getDefaultProvider() const { return defaultProvider_; }
    
    void loadConfiguration(const std::string& filePath);
    void saveConfiguration(const std::string& filePath);
    
    void setApiKey(AIProvider provider, const std::string& apiKey);
    std::string getApiKey(AIProvider provider) const;
    
    struct UsageStats {
        int totalRequests;
        int successfulRequests;
        int failedRequests;
        int totalTokensUsed;
        float averageResponseTime;
    };
    
    UsageStats getUsageStats(AIProvider provider) const;
    void resetUsageStats(AIProvider provider);

private:
    AIPluginManager();
    ~AIPluginManager();
    
    std::unordered_map<AIProvider, std::unique_ptr<AIPluginInterface>> plugins_;
    std::unordered_map<AIProvider, AIProviderConfig> configs_;
    std::unordered_map<AIProvider, UsageStats> usageStats_;
    AIProvider defaultProvider_;
    std::mutex mutex_;
    bool initialized_;
};

class AIAssistant {
public:
    static AIAssistant& getInstance();
    
    void initialize();
    
    void generateModelFromText(const std::string& description,
                               std::function<void(const std::string& modelPath)> callback);
    
    void generateTextureFromText(const std::string& description,
                                 std::function<void(const std::vector<uint8_t>& imageData, int width, int height)> callback);
    
    void generateAnimation(const std::string& modelPath, const std::string& description,
                          std::function<void(const std::string& animPath)> callback);
    
    void enhanceModel(const std::string& modelPath,
                     std::function<void(const std::string& enhancedPath)> callback);
    
    void suggestImprovements(const std::string& modelPath,
                            std::function<void(const std::vector<std::string>& suggestions)> callback);
    
    void generateCode(const std::string& prompt,
                     std::function<void(const std::string& code)> callback);
    
    void explainCode(const std::string& code,
                    std::function<void(const std::string& explanation)> callback);
    
    void chat(const std::string& message,
              std::function<void(const std::string& response)> callback);
    
    void setContext(const std::vector<std::string>& context);
    void clearContext();
    
    void setSystemPrompt(const std::string& prompt);
    
    void enableAutoSuggestions(bool enable);
    bool isAutoSuggestionsEnabled() const { return autoSuggestionsEnabled_; }

private:
    AIAssistant();
    ~AIAssistant();
    
    std::vector<std::string> context_;
    std::string systemPrompt_;
    bool autoSuggestionsEnabled_;
};

}
}
