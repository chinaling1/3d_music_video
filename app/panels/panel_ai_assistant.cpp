/**
 * @file panel_ai_assistant.cpp
 * @brief AI助手面板实现
 */

#include "panel_ai_assistant.h"
#include <windowsx.h>

namespace v3d {
namespace app {

const wchar_t* PanelAIAssistant::CLASS_NAME = L"PanelAIAssistant";

PanelAIAssistant::PanelAIAssistant(HWND hParent, HINSTANCE hInstance)
    : hWnd_(nullptr)
    , hParent_(hParent)
    , hInstance_(hInstance)
    , hChatEdit_(nullptr)
    , hInputEdit_(nullptr)
    , hSendButton_(nullptr)
    , hModelButton_(nullptr)
    , hTextureButton_(nullptr)
    , hClearButton_(nullptr)
    , hStatusLabel_(nullptr)
    , hProviderCombo_(nullptr)
{
    registerClass();
    createWindow();
}

PanelAIAssistant::~PanelAIAssistant() {
    if (hWnd_) {
        DestroyWindow(hWnd_);
    }
}

void PanelAIAssistant::registerClass() {
    WNDCLASSEXW wcex = {0};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = panelProc;
    wcex.hInstance = hInstance_;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    wcex.lpszClassName = CLASS_NAME;
    RegisterClassExW(&wcex);
}

bool PanelAIAssistant::createWindow() {
    hWnd_ = CreateWindowExW(
        0,
        CLASS_NAME,
        L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        0, 0, 300, 400,
        hParent_,
        nullptr,
        hInstance_,
        this
    );
    
    if (hWnd_) {
        onCreate();
        return true;
    }
    return false;
}

void PanelAIAssistant::onCreate() {
    CreateWindowExW(0, L"STATIC", L"AI Provider:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        10, 10, 80, 20, hWnd_, nullptr, hInstance_, nullptr);
    
    hProviderCombo_ = CreateWindowExW(0, L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        100, 8, 180, 200, hWnd_, (HMENU)ID_PROVIDER_COMBO, hInstance_, nullptr);
    
    ComboBox_AddString(hProviderCombo_, L"OpenAI");
    ComboBox_AddString(hProviderCombo_, L"Claude");
    ComboBox_AddString(hProviderCombo_, L"Stable Diffusion");
    ComboBox_AddString(hProviderCombo_, L"Local");
    ComboBox_SetCurSel(hProviderCombo_, 0);
    
    hChatEdit_ = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | WS_VSCROLL,
        10, 35, 280, 250, hWnd_, nullptr, hInstance_, nullptr);
    
    hInputEdit_ = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL,
        10, 295, 200, 60, hWnd_, (HMENU)ID_INPUT_EDIT, hInstance_, nullptr);
    
    hSendButton_ = CreateWindowExW(0, L"BUTTON", L"发送",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        220, 295, 70, 25, hWnd_, (HMENU)ID_SEND_BUTTON, hInstance_, nullptr);
    
    hModelButton_ = CreateWindowExW(0, L"BUTTON", L"生成模型",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, 360, 90, 25, hWnd_, (HMENU)ID_MODEL_BUTTON, hInstance_, nullptr);
    
    hTextureButton_ = CreateWindowExW(0, L"BUTTON", L"生成纹理",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        110, 360, 90, 25, hWnd_, (HMENU)ID_TEXTURE_BUTTON, hInstance_, nullptr);
    
    hClearButton_ = CreateWindowExW(0, L"BUTTON", L"清空",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        210, 360, 70, 25, hWnd_, (HMENU)ID_CLEAR_BUTTON, hInstance_, nullptr);
    
    hStatusLabel_ = CreateWindowExW(0, L"STATIC", L"就绪",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        10, 390, 280, 20, hWnd_, nullptr, hInstance_, nullptr);
    
    addMessage(L"欢迎使用AI助手！我可以帮助您：\n- 生成3D模型\n- 创建纹理\n- 回答问题\n- 编写代码", false);
}

void PanelAIAssistant::onSize(int width, int height) {
    UNREFERENCED_PARAMETER(width);
    UNREFERENCED_PARAMETER(height);
}

void PanelAIAssistant::onCommand(WPARAM wParam) {
    int id = LOWORD(wParam);
    
    switch (id) {
        case ID_SEND_BUTTON:
            onSend();
            break;
        case ID_MODEL_BUTTON:
            onGenerateModel();
            break;
        case ID_TEXTURE_BUTTON:
            onGenerateTexture();
            break;
        case ID_CLEAR_BUTTON:
            onClearChat();
            break;
    }
}

void PanelAIAssistant::onSend() {
    int len = GetWindowTextLengthW(hInputEdit_);
    if (len > 0) {
        std::wstring text(len + 1, 0);
        GetWindowTextW(hInputEdit_, &text[0], len + 1);
        text.resize(len);
        
        if (!text.empty()) {
            addMessage(text, true);
            SetWindowTextW(hInputEdit_, L"");
            
            if (onSendMessage_) {
                onSendMessage_(text);
            }
        }
    }
}

void PanelAIAssistant::onGenerateModel() {
    int len = GetWindowTextLengthW(hInputEdit_);
    std::wstring text;
    if (len > 0) {
        text.resize(len + 1);
        GetWindowTextW(hInputEdit_, &text[0], len + 1);
        text.resize(len);
    }
    
    if (text.empty()) {
        text = L"一个简单的立方体";
    }
    
    addMessage(L"正在生成模型: " + text, false);
    setStatus(L"生成中...");
    
    if (onGenerateModel_) {
        onGenerateModel_(text);
    }
}

void PanelAIAssistant::onGenerateTexture() {
    int len = GetWindowTextLengthW(hInputEdit_);
    std::wstring text;
    if (len > 0) {
        text.resize(len + 1);
        GetWindowTextW(hInputEdit_, &text[0], len + 1);
        text.resize(len);
    }
    
    if (text.empty()) {
        text = L"木纹纹理";
    }
    
    addMessage(L"正在生成纹理: " + text, false);
    setStatus(L"生成中...");
    
    if (onGenerateTexture_) {
        onGenerateTexture_(text);
    }
}

void PanelAIAssistant::onClearChat() {
    SetWindowTextW(hChatEdit_, L"");
    addMessage(L"聊天已清空", false);
}

void PanelAIAssistant::addMessage(const std::wstring& message, bool isUser) {
    int len = GetWindowTextLengthW(hChatEdit_);
    
    std::wstring prefix = isUser ? L"\r\n[用户]: " : L"\r\n[AI]: ";
    std::wstring newText = prefix + message + L"\r\n";
    
    SendMessageW(hChatEdit_, EM_SETSEL, len, len);
    SendMessageW(hChatEdit_, EM_REPLACESEL, TRUE, (LPARAM)newText.c_str());
    SendMessageW(hChatEdit_, EM_SCROLLCARET, 0, 0);
}

void PanelAIAssistant::clearChat() {
    SetWindowTextW(hChatEdit_, L"");
}

void PanelAIAssistant::setStatus(const std::wstring& status) {
    SetWindowTextW(hStatusLabel_, status.c_str());
}

void PanelAIAssistant::setOnSendMessage(std::function<void(const std::wstring&)> callback) {
    onSendMessage_ = callback;
}

void PanelAIAssistant::setOnGenerateModel(std::function<void(const std::wstring&)> callback) {
    onGenerateModel_ = callback;
}

void PanelAIAssistant::setOnGenerateTexture(std::function<void(const std::wstring&)> callback) {
    onGenerateTexture_ = callback;
}

LRESULT CALLBACK PanelAIAssistant::panelProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    PanelAIAssistant* pThis = nullptr;
    
    if (message == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<PanelAIAssistant*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<PanelAIAssistant*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }
    
    switch (message) {
        case WM_CREATE:
            return 0;
            
        case WM_SIZE:
            if (pThis) pThis->onSize(LOWORD(lParam), HIWORD(lParam));
            return 0;
            
        case WM_COMMAND:
            if (pThis) pThis->onCommand(wParam);
            return 0;
            
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                BeginPaint(hWnd, &ps);
                EndPaint(hWnd, &ps);
            }
            return 0;
            
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

}
}
