/**
 * @file panel_ai_assistant.h
 * @brief AI助手面板 - 提供AI功能界面
 */

#pragma once

#include <Windows.h>
#include <string>
#include <memory>
#include <functional>
#include <CommCtrl.h>

namespace v3d {
namespace app {

class PanelAIAssistant {
public:
    PanelAIAssistant(HWND hParent, HINSTANCE hInstance);
    ~PanelAIAssistant();
    
    HWND getHandle() const { return hWnd_; }
    
    void addMessage(const std::wstring& message, bool isUser);
    void clearChat();
    void setStatus(const std::wstring& status);
    
    void setOnSendMessage(std::function<void(const std::wstring&)> callback);
    void setOnGenerateModel(std::function<void(const std::wstring&)> callback);
    void setOnGenerateTexture(std::function<void(const std::wstring&)> callback);

private:
    bool createWindow();
    void registerClass();
    static LRESULT CALLBACK panelProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    
    void onCreate();
    void onSize(int width, int height);
    void onCommand(WPARAM wParam);
    
    void onSend();
    void onGenerateModel();
    void onGenerateTexture();
    void onClearChat();
    
    HWND hWnd_;
    HWND hParent_;
    HINSTANCE hInstance_;
    
    HWND hChatEdit_;
    HWND hInputEdit_;
    HWND hSendButton_;
    HWND hModelButton_;
    HWND hTextureButton_;
    HWND hClearButton_;
    HWND hStatusLabel_;
    HWND hProviderCombo_;
    
    std::function<void(const std::wstring&)> onSendMessage_;
    std::function<void(const std::wstring&)> onGenerateModel_;
    std::function<void(const std::wstring&)> onGenerateTexture_;
    
    static const int ID_SEND_BUTTON = 7001;
    static const int ID_MODEL_BUTTON = 7002;
    static const int ID_TEXTURE_BUTTON = 7003;
    static const int ID_CLEAR_BUTTON = 7004;
    static const int ID_INPUT_EDIT = 7005;
    static const int ID_PROVIDER_COMBO = 7006;
    
    static const wchar_t* CLASS_NAME;
};

}
}
