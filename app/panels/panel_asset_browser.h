/**
 * @file panel_asset_browser.h
 * @brief 资源管理器面板 - 浏览和管理项目资源
 */

#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <CommCtrl.h>

namespace v3d {
namespace app {

struct AssetItem {
    std::wstring name;
    std::wstring type;
    std::wstring path;
    int iconIndex;
};

class PanelAssetBrowser {
public:
    PanelAssetBrowser(HWND hParent, HINSTANCE hInstance);
    ~PanelAssetBrowser();
    
    HWND getHandle() const { return hWnd_; }
    
    void refresh();
    void setProjectPath(const std::wstring& path);
    
private:
    bool createWindow();
    void registerClass();
    static LRESULT CALLBACK panelProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    
    void onPaint();
    void onSize(int width, int height);
    void createTreeView();
    void createListView();
    void populateTree();
    void populateList(const std::wstring& folder);
    
    HWND hWnd_;
    HWND hParent_;
    HINSTANCE hInstance_;
    HWND hTreeView_;
    HWND hListView_;
    HWND hSearchBox_;
    
    std::wstring projectPath_;
    std::vector<AssetItem> assets_;
    
    static const wchar_t* CLASS_NAME;
};

}
}
