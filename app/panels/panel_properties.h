/**
 * @file panel_properties.h
 * @brief 属性面板 - 显示和编辑选中对象的属性
 */

#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <CommCtrl.h>

namespace v3d {
namespace app {

struct PropertyItem {
    std::wstring name;
    std::wstring value;
    std::wstring type;
    bool editable;
};

class PanelProperties {
public:
    PanelProperties(HWND hParent, HINSTANCE hInstance);
    ~PanelProperties();
    
    HWND getHandle() const { return hWnd_; }
    
    void setSelectedObject(const std::wstring& name);
    void updateProperties(const std::vector<PropertyItem>& properties);
    
private:
    bool createWindow();
    void registerClass();
    static LRESULT CALLBACK panelProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    
    void onPaint();
    void onSize(int width, int height);
    void createPropertyList();
    void updatePropertyList();
    
    HWND hWnd_;
    HWND hParent_;
    HINSTANCE hInstance_;
    HWND hPropertyList_;
    
    std::wstring selectedObject_;
    std::vector<PropertyItem> properties_;
    
    static const wchar_t* CLASS_NAME;
};

}
}
