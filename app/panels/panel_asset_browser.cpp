/**
 * @file panel_asset_browser.cpp
 * @brief 资源管理器面板实现
 */

#include "panel_asset_browser.h"
#include <windowsx.h>
#include <Shlobj.h>

namespace v3d {
namespace app {

const wchar_t* PanelAssetBrowser::CLASS_NAME = L"PanelAssetBrowser";

PanelAssetBrowser::PanelAssetBrowser(HWND hParent, HINSTANCE hInstance)
    : hWnd_(nullptr)
    , hParent_(hParent)
    , hInstance_(hInstance)
    , hTreeView_(nullptr)
    , hListView_(nullptr)
    , hSearchBox_(nullptr)
    , projectPath_(L"项目")
{
    assets_ = {
        {L"角色.fbx", L"模型", L"/Models/Character.fbx", 0},
        {L"动画.anim", L"动画", L"/Animations/Animation.anim", 1},
        {L"纹理.png", L"纹理", L"/Textures/Texture.png", 2},
        {L"音效.wav", L"音频", L"/Audio/Sound.wav", 3},
        {L"场景.scene", L"场景", L"/Scenes/Scene.scene", 4}
    };
    
    registerClass();
    createWindow();
}

PanelAssetBrowser::~PanelAssetBrowser() {
    if (hTreeView_) DestroyWindow(hTreeView_);
    if (hListView_) DestroyWindow(hListView_);
    if (hSearchBox_) DestroyWindow(hSearchBox_);
    if (hWnd_) DestroyWindow(hWnd_);
}

void PanelAssetBrowser::registerClass() {
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

bool PanelAssetBrowser::createWindow() {
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
        createTreeView();
        createListView();
        populateTree();
        populateList(L"");
        return true;
    }
    return false;
}

void PanelAssetBrowser::createTreeView() {
    hTreeView_ = CreateWindowExW(
        0,
        WC_TREEVIEWW,
        L"",
        WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT | WS_BORDER,
        0, 30, 240, 200,
        hWnd_,
        (HMENU)1,
        hInstance_,
        nullptr
    );
}

void PanelAssetBrowser::createListView() {
    hListView_ = CreateWindowExW(
        0,
        WC_LISTVIEWW,
        L"",
        WS_CHILD | WS_VISIBLE | LVS_LIST | LVS_SINGLESEL | WS_BORDER,
        0, 240, 240, 300,
        hWnd_,
        (HMENU)2,
        hInstance_,
        nullptr
    );
    
    ListView_SetTextColor(hListView_, RGB(0, 0, 0));
    ListView_SetBkColor(hListView_, RGB(255, 255, 255));
    ListView_SetTextBkColor(hListView_, RGB(255, 255, 255));
}

void PanelAssetBrowser::populateTree() {
    if (!hTreeView_) return;
    
    HTREEITEM hRoot = nullptr;
    
    TVINSERTSTRUCTW tvis = {0};
    tvis.hParent = TVI_ROOT;
    tvis.hInsertAfter = TVI_LAST;
    tvis.item.mask = TVIF_TEXT;
    tvis.item.pszText = (LPWSTR)L"项目";
    hRoot = TreeView_InsertItem(hTreeView_, &tvis);
    
    tvis.hParent = hRoot;
    tvis.item.pszText = (LPWSTR)L"模型";
    TreeView_InsertItem(hTreeView_, &tvis);
    
    tvis.item.pszText = (LPWSTR)L"动画";
    TreeView_InsertItem(hTreeView_, &tvis);
    
    tvis.item.pszText = (LPWSTR)L"纹理";
    TreeView_InsertItem(hTreeView_, &tvis);
    
    tvis.item.pszText = (LPWSTR)L"音频";
    TreeView_InsertItem(hTreeView_, &tvis);
    
    tvis.item.pszText = (LPWSTR)L"场景";
    TreeView_InsertItem(hTreeView_, &tvis);
    
    TreeView_Expand(hTreeView_, hRoot, TVE_EXPAND);
}

void PanelAssetBrowser::populateList(const std::wstring& folder) {
    UNREFERENCED_PARAMETER(folder);
    
    if (!hListView_) return;
    
    ListView_DeleteAllItems(hListView_);
    
    for (size_t i = 0; i < assets_.size(); i++) {
        LVITEMW lvi = {0};
        lvi.mask = LVIF_TEXT;
        lvi.iItem = (int)i;
        lvi.pszText = (LPWSTR)assets_[i].name.c_str();
        ListView_InsertItem(hListView_, &lvi);
    }
}

void PanelAssetBrowser::refresh() {
    populateList(L"");
}

void PanelAssetBrowser::setProjectPath(const std::wstring& path) {
    projectPath_ = path;
    refresh();
}

void PanelAssetBrowser::onPaint() {
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
    
    TextOutW(hdc, 10, 8, L"资源管理器", 5);
    
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
    
    EndPaint(hWnd_, &ps);
}

void PanelAssetBrowser::onSize(int width, int height) {
    int treeHeight = height / 2 - 30;
    int listHeight = height / 2 - 10;
    
    if (hTreeView_) {
        SetWindowPos(hTreeView_, nullptr, 0, 30, width, treeHeight, SWP_NOZORDER);
    }
    
    if (hListView_) {
        SetWindowPos(hListView_, nullptr, 0, 30 + treeHeight + 10, width, listHeight, SWP_NOZORDER);
    }
}

LRESULT CALLBACK PanelAssetBrowser::panelProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    PanelAssetBrowser* pThis = nullptr;
    
    if (message == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<PanelAssetBrowser*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<PanelAssetBrowser*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
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
