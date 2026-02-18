/**
 * @file panel_modeling_tools.cpp
 * @brief 建模工具面板实现
 */

#include "panel_modeling_tools.h"
#include <windowsx.h>

namespace v3d {
namespace app {

const wchar_t* PanelModelingTools::CLASS_NAME = L"PanelModelingTools";

PanelModelingTools::PanelModelingTools(HWND hParent, HINSTANCE hInstance)
    : hWnd_(nullptr)
    , hParent_(hParent)
    , hInstance_(hInstance)
    , hPrimitiveGroup_(nullptr)
    , hEditGroup_(nullptr)
    , hTransformGroup_(nullptr)
    , hToolGroup_(nullptr)
    , hInfoLabel_(nullptr)
{
    registerClass();
    createWindow();
}

PanelModelingTools::~PanelModelingTools() {
    if (hWnd_) {
        DestroyWindow(hWnd_);
    }
}

void PanelModelingTools::registerClass() {
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

bool PanelModelingTools::createWindow() {
    hWnd_ = CreateWindowExW(
        0,
        CLASS_NAME,
        L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        0, 0, 200, 600,
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

void PanelModelingTools::onCreate() {
    int y = 10;
    
    hPrimitiveGroup_ = CreateWindowExW(0, L"BUTTON", L"基础几何体",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        5, y, 190, 120, hWnd_, nullptr, hInstance_, nullptr);
    y += 20;
    
    CreateWindowExW(0, L"BUTTON", L"立方体",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, y, 55, 25, hWnd_, (HMENU)ID_PRIMITIVE_CUBE, hInstance_, nullptr);
    CreateWindowExW(0, L"BUTTON", L"球体",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        70, y, 55, 25, hWnd_, (HMENU)ID_PRIMITIVE_SPHERE, hInstance_, nullptr);
    CreateWindowExW(0, L"BUTTON", L"圆柱",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        130, y, 55, 25, hWnd_, (HMENU)ID_PRIMITIVE_CYLINDER, hInstance_, nullptr);
    y += 30;
    
    CreateWindowExW(0, L"BUTTON", L"圆锥",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, y, 55, 25, hWnd_, (HMENU)ID_PRIMITIVE_CONE, hInstance_, nullptr);
    CreateWindowExW(0, L"BUTTON", L"圆环",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        70, y, 55, 25, hWnd_, (HMENU)ID_PRIMITIVE_TORUS, hInstance_, nullptr);
    CreateWindowExW(0, L"BUTTON", L"平面",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        130, y, 55, 25, hWnd_, (HMENU)ID_PRIMITIVE_PLANE, hInstance_, nullptr);
    y += 45;
    
    hEditGroup_ = CreateWindowExW(0, L"BUTTON", L"编辑操作",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        5, y, 190, 120, hWnd_, nullptr, hInstance_, nullptr);
    y += 20;
    
    CreateWindowExW(0, L"BUTTON", L"挤出",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, y, 55, 25, hWnd_, (HMENU)ID_EDIT_EXTRUDE, hInstance_, nullptr);
    CreateWindowExW(0, L"BUTTON", L"内插",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        70, y, 55, 25, hWnd_, (HMENU)ID_EDIT_INSET, hInstance_, nullptr);
    CreateWindowExW(0, L"BUTTON", L"倒角",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        130, y, 55, 25, hWnd_, (HMENU)ID_EDIT_BEVEL, hInstance_, nullptr);
    y += 30;
    
    CreateWindowExW(0, L"BUTTON", L"细分",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, y, 55, 25, hWnd_, (HMENU)ID_EDIT_SUBDIVIDE, hInstance_, nullptr);
    CreateWindowExW(0, L"BUTTON", L"合并",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        70, y, 55, 25, hWnd_, (HMENU)ID_EDIT_MERGE, hInstance_, nullptr);
    CreateWindowExW(0, L"BUTTON", L"删除",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        130, y, 55, 25, hWnd_, (HMENU)ID_EDIT_DELETE, hInstance_, nullptr);
    y += 45;
    
    hTransformGroup_ = CreateWindowExW(0, L"BUTTON", L"变换",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        5, y, 190, 55, hWnd_, nullptr, hInstance_, nullptr);
    y += 20;
    
    CreateWindowExW(0, L"BUTTON", L"移动",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, y, 55, 25, hWnd_, (HMENU)ID_TRANSFORM_TRANSLATE, hInstance_, nullptr);
    CreateWindowExW(0, L"BUTTON", L"旋转",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        70, y, 55, 25, hWnd_, (HMENU)ID_TRANSFORM_ROTATE, hInstance_, nullptr);
    CreateWindowExW(0, L"BUTTON", L"缩放",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        130, y, 55, 25, hWnd_, (HMENU)ID_TRANSFORM_SCALE, hInstance_, nullptr);
    y += 45;
    
    hToolGroup_ = CreateWindowExW(0, L"BUTTON", L"工具",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        5, y, 190, 55, hWnd_, nullptr, hInstance_, nullptr);
    y += 20;
    
    CreateWindowExW(0, L"BUTTON", L"选择",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, y, 55, 25, hWnd_, (HMENU)ID_TOOL_SELECT, hInstance_, nullptr);
    CreateWindowExW(0, L"BUTTON", L"笔刷",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        70, y, 55, 25, hWnd_, (HMENU)ID_TOOL_BRUSH, hInstance_, nullptr);
    CreateWindowExW(0, L"BUTTON", L"切割",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        130, y, 55, 25, hWnd_, (HMENU)ID_TOOL_KNIFE, hInstance_, nullptr);
    y += 45;
    
    hInfoLabel_ = CreateWindowExW(0, L"STATIC", L"选择: 无",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        10, y, 180, 60, hWnd_, nullptr, hInstance_, nullptr);
}

void PanelModelingTools::onSize(int width, int height) {
    UNREFERENCED_PARAMETER(width);
    UNREFERENCED_PARAMETER(height);
}

void PanelModelingTools::onCommand(WPARAM wParam) {
    int id = LOWORD(wParam);
    
    if (id >= ID_PRIMITIVE_CUBE && id <= ID_PRIMITIVE_PLANE) {
        onPrimitiveButton(id);
    } else if (id >= ID_EDIT_EXTRUDE && id <= ID_EDIT_DUPLICATE) {
        onEditButton(id);
    } else if (id >= ID_TRANSFORM_TRANSLATE && id <= ID_TRANSFORM_SCALE) {
        onTransformButton(id);
    }
}

void PanelModelingTools::onPrimitiveButton(int buttonId) {
    std::string type;
    switch (buttonId) {
        case ID_PRIMITIVE_CUBE: type = "cube"; break;
        case ID_PRIMITIVE_SPHERE: type = "sphere"; break;
        case ID_PRIMITIVE_CYLINDER: type = "cylinder"; break;
        case ID_PRIMITIVE_CONE: type = "cone"; break;
        case ID_PRIMITIVE_TORUS: type = "torus"; break;
        case ID_PRIMITIVE_PLANE: type = "plane"; break;
    }
    
    if (onPrimitiveCreated_ && !type.empty()) {
        onPrimitiveCreated_(type, "");
    }
}

void PanelModelingTools::onEditButton(int buttonId) {
    std::string tool;
    switch (buttonId) {
        case ID_EDIT_EXTRUDE: tool = "extrude"; break;
        case ID_EDIT_INSET: tool = "inset"; break;
        case ID_EDIT_BEVEL: tool = "bevel"; break;
        case ID_EDIT_SUBDIVIDE: tool = "subdivide"; break;
        case ID_EDIT_MERGE: tool = "merge"; break;
        case ID_EDIT_DELETE: tool = "delete"; break;
    }
    
    if (onToolSelected_ && !tool.empty()) {
        onToolSelected_(tool);
    }
}

void PanelModelingTools::onTransformButton(int buttonId) {
    std::string mode;
    switch (buttonId) {
        case ID_TRANSFORM_TRANSLATE: mode = "translate"; break;
        case ID_TRANSFORM_ROTATE: mode = "rotate"; break;
        case ID_TRANSFORM_SCALE: mode = "scale"; break;
    }
    
    if (onToolSelected_ && !mode.empty()) {
        onToolSelected_(mode);
    }
}

void PanelModelingTools::setOnToolSelected(std::function<void(const std::string&)> callback) {
    onToolSelected_ = callback;
}

void PanelModelingTools::setOnPrimitiveCreated(std::function<void(const std::string&, const std::string&)> callback) {
    onPrimitiveCreated_ = callback;
}

void PanelModelingTools::updateSelectionInfo(int vertexCount, int edgeCount, int faceCount) {
    wchar_t text[256];
    wsprintfW(text, L"顶点: %d\n边: %d\n面: %d", vertexCount, edgeCount, faceCount);
    SetWindowTextW(hInfoLabel_, text);
}

LRESULT CALLBACK PanelModelingTools::panelProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    PanelModelingTools* pThis = nullptr;
    
    if (message == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<PanelModelingTools*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<PanelModelingTools*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
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
