/**
 * @file panel_modeling_tools.h
 * @brief 建模工具面板 - 提供建模工具界面
 */

#pragma once

#include <Windows.h>
#include <string>
#include <memory>
#include <functional>
#include <CommCtrl.h>

namespace v3d {
namespace app {

class PanelModelingTools {
public:
    PanelModelingTools(HWND hParent, HINSTANCE hInstance);
    ~PanelModelingTools();
    
    HWND getHandle() const { return hWnd_; }
    
    void setOnToolSelected(std::function<void(const std::string&)> callback);
    void setOnPrimitiveCreated(std::function<void(const std::string&, const std::string&)> callback);
    
    void updateSelectionInfo(int vertexCount, int edgeCount, int faceCount);
    
private:
    bool createWindow();
    void registerClass();
    static LRESULT CALLBACK panelProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    
    void onCreate();
    void onSize(int width, int height);
    void onCommand(WPARAM wParam);
    
    void createPrimitiveSection();
    void createEditSection();
    void createTransformSection();
    void createToolSection();
    
    void onPrimitiveButton(int buttonId);
    void onEditButton(int buttonId);
    void onTransformButton(int buttonId);
    
    HWND hWnd_;
    HWND hParent_;
    HINSTANCE hInstance_;
    
    HWND hPrimitiveGroup_;
    HWND hEditGroup_;
    HWND hTransformGroup_;
    HWND hToolGroup_;
    HWND hInfoLabel_;
    
    std::function<void(const std::string&)> onToolSelected_;
    std::function<void(const std::string&, const std::string&)> onPrimitiveCreated_;
    
    static const int ID_PRIMITIVE_CUBE = 6001;
    static const int ID_PRIMITIVE_SPHERE = 6002;
    static const int ID_PRIMITIVE_CYLINDER = 6003;
    static const int ID_PRIMITIVE_CONE = 6004;
    static const int ID_PRIMITIVE_TORUS = 6005;
    static const int ID_PRIMITIVE_PLANE = 6006;
    
    static const int ID_EDIT_EXTRUDE = 6101;
    static const int ID_EDIT_INSET = 6102;
    static const int ID_EDIT_BEVEL = 6103;
    static const int ID_EDIT_SUBDIVIDE = 6104;
    static const int ID_EDIT_MERGE = 6105;
    static const int ID_EDIT_DELETE = 6106;
    static const int ID_EDIT_DUPLICATE = 6107;
    
    static const int ID_TRANSFORM_TRANSLATE = 6201;
    static const int ID_TRANSFORM_ROTATE = 6202;
    static const int ID_TRANSFORM_SCALE = 6203;
    
    static const int ID_TOOL_SELECT = 6301;
    static const int ID_TOOL_BRUSH = 6302;
    static const int ID_TOOL_SMOOTH = 6303;
    static const int ID_TOOL_KNIFE = 6304;
    
    static const wchar_t* CLASS_NAME;
};

}
}
