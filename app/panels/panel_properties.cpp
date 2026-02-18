/**
 * @file panel_properties.cpp
 * @brief 属性面板实现
 */

#include "panel_properties.h"
#include <windowsx.h>

namespace v3d {
namespace app {

const wchar_t* PanelProperties::CLASS_NAME = L"PanelProperties";

PanelProperties::PanelProperties(HWND hParent, HINSTANCE hInstance)
    : hWnd_(nullptr)
    , hParent_(hParent)
    , hInstance_(hInstance)
    , hPropertyList_(nullptr)
    , selectedObject_(L"无")
{
    properties_ = {
        {L"名称", L"立方体_001", L"string", true},
        {L"位置", L"(0, 0, 0)", L"vector3", true},
        {L"旋转", L"(0, 0, 0)", L"vector3", true},
        {L"缩放", L"(1, 1, 1)", L"vector3", true},
        {L"可见", L"是", L"bool", true},
        {L"材质", L"默认", L"material", true},
        {L"顶点数", L"24", L"int", false},
        {L"面数", L"12", L"int", false}
    };
    
    registerClass();
    createWindow();
}

PanelProperties::~PanelProperties() {
    if (hPropertyList_) {
        DestroyWindow(hPropertyList_);
    }
    if (hWnd_) {
        DestroyWindow(hWnd_);
    }
}

void PanelProperties::registerClass() {
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

bool PanelProperties::createWindow() {
    hWnd_ = CreateWindowExW(
        0,
        CLASS_NAME,
        L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        0, 0, 100, 100,
        hParent_,
        nullptr,
        hInstance_,
        this
    );
    
    if (hWnd_) {
        createPropertyList();
        return true;
    }
    return false;
}

void PanelProperties::createPropertyList() {
    hPropertyList_ = CreateWindowExW(
        0,
        WC_LISTVIEWW,
        L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | WS_BORDER,
        0, 30, 280, 400,
        hWnd_,
        (HMENU)1,
        hInstance_,
        nullptr
    );
    
    LVCOLUMNW lvc = {0};
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    
    lvc.iSubItem = 0;
    lvc.pszText = (LPWSTR)L"Property";
    lvc.cx = 100;
    lvc.fmt = LVCFMT_LEFT;
    ListView_InsertColumn(hPropertyList_, 0, &lvc);
    
    lvc.iSubItem = 1;
    lvc.pszText = (LPWSTR)L"Value";
    lvc.cx = 160;
    ListView_InsertColumn(hPropertyList_, 1, &lvc);
    
    updatePropertyList();
}

void PanelProperties::updatePropertyList() {
    if (!hPropertyList_) return;
    
    ListView_DeleteAllItems(hPropertyList_);
    
    for (size_t i = 0; i < properties_.size(); i++) {
        LVITEMW lvi = {0};
        lvi.mask = LVIF_TEXT;
        lvi.iItem = (int)i;
        lvi.iSubItem = 0;
        lvi.pszText = (LPWSTR)properties_[i].name.c_str();
        ListView_InsertItem(hPropertyList_, &lvi);
        
        lvi.iSubItem = 1;
        lvi.pszText = (LPWSTR)properties_[i].value.c_str();
        ListView_SetItem(hPropertyList_, &lvi);
    }
}

void PanelProperties::setSelectedObject(const std::wstring& name) {
    selectedObject_ = name;
    InvalidateRect(hWnd_, nullptr, TRUE);
}

void PanelProperties::updateProperties(const std::vector<PropertyItem>& properties) {
    properties_ = properties;
    updatePropertyList();
}

void PanelProperties::onPaint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd_, &ps);
    
    RECT rc;
    GetClientRect(hWnd_, &rc);
    
    HBRUSH hBrush = CreateSolidBrush(RGB(240, 240, 240));
    FillRect(hdc, &rc, hBrush);
    DeleteObject(hBrush);
    
    SetTextColor(hdc, RGB(0, 0, 0));
    SetBkMode(hdc, TRANSPARENT);
    
    HFONT hFont = CreateFontW(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    
    std::wstring title = L"属性: " + selectedObject_;
    TextOutW(hdc, 10, 8, title.c_str(), title.length());
    
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
    
    EndPaint(hWnd_, &ps);
}

void PanelProperties::onSize(int width, int height) {
    if (hPropertyList_) {
        SetWindowPos(hPropertyList_, nullptr, 0, 30, width, height - 30, SWP_NOZORDER);
    }
}

LRESULT CALLBACK PanelProperties::panelProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    PanelProperties* pThis = nullptr;
    
    if (message == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<PanelProperties*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<PanelProperties*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }
    
    switch (message) {
        case WM_PAINT:
            if (pThis) pThis->onPaint();
            return 0;
            
        case WM_SIZE:
            if (pThis) pThis->onSize(LOWORD(lParam), HIWORD(lParam));
            return 0;
            
        case WM_NOTIFY:
            return 0;
            
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

}
}
