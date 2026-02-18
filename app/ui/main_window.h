/**
 * @file main_window.h
 * @brief 主窗口类 - 应用程序的主界面框架
 * 
 * 提供应用程序的主窗口框架，包含菜单栏、工具栏、
 * 面板布局和状态栏。
 * 
 * @author 3D Video Studio Team
 * @version 1.0
 * @date 2026-02-17
 */

#pragma once

#include <Windows.h>
#include <string>
#include <memory>
#include <functional>

namespace v3d {
namespace app {

class Panel3DView;
class PanelTimeline;
class PanelProperties;
class PanelAssetBrowser;

/**
 * @class MainWindow
 * @brief 主窗口类 - 应用程序的主界面框架
 * 
 * 管理应用程序的主窗口，包括：
 * - 菜单栏和工具栏
 * - 面板布局管理
 * - 状态栏
 * - 事件处理
 */
class MainWindow {
public:
    MainWindow();
    ~MainWindow();

    /**
     * @brief 创建主窗口
     * @param hInstance 应用程序实例句柄
     * @param title 窗口标题
     * @param width 窗口宽度
     * @param height 窗口高度
     * @return 成功返回true
     */
    bool create(HINSTANCE hInstance, const std::wstring& title, int width, int height);
    
    /**
     * @brief 显示窗口
     * @param nCmdShow 显示方式
     */
    void show(int nCmdShow);
    
    /**
     * @brief 获取窗口句柄
     * @return 窗口句柄
     */
    HWND getHandle() const { return hWnd_; }
    
    /**
     * @brief 获取实例句柄
     * @return 实例句柄
     */
    HINSTANCE getInstance() const { return hInstance_; }

private:
    /**
     * @brief 注册窗口类
     */
    bool registerClass();
    
    /**
     * @brief 创建菜单栏
     */
    void createMenuBar();
    
    /**
     * @brief 创建工具栏
     */
    void createToolBar();
    
    /**
     * @brief 创建状态栏
     */
    void createStatusBar();
    
    /**
     * @brief 创建面板布局
     */
    void createPanels();
    
    /**
     * @brief 布局所有子窗口
     */
    void layoutChildren();
    
    /**
     * @brief 窗口过程
     */
    static LRESULT CALLBACK windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    
    /**
     * @brief 处理命令
     */
    void handleCommand(WPARAM wParam);
    
    /**
     * @brief 处理菜单命令
     */
    void handleMenuCommand(int menuId);
    
    /**
     * @brief 处理大小改变
     */
    void handleSize(int width, int height);

    HWND hWnd_;                          ///< 主窗口句柄
    HINSTANCE hInstance_;                 ///< 应用程序实例
    std::wstring title_;                  ///< 窗口标题
    int width_;                           ///< 窗口宽度
    int height_;                          ///< 窗口高度
    
    HWND hMenuBar_;                       ///< 菜单栏句柄
    HWND hToolBar_;                       ///< 工具栏句柄
    HWND hStatusBar_;                     ///< 状态栏句柄
    
    std::unique_ptr<Panel3DView> panel3DView_;           ///< 3D视图面板
    std::unique_ptr<PanelTimeline> panelTimeline_;       ///< 时间轴面板
    std::unique_ptr<PanelProperties> panelProperties_;   ///< 属性面板
    std::unique_ptr<PanelAssetBrowser> panelAssetBrowser_; ///< 资源管理器面板
    
    static const int ID_MENU_FILE_NEW = 1001;
    static const int ID_MENU_FILE_OPEN = 1002;
    static const int ID_MENU_FILE_SAVE = 1003;
    static const int ID_MENU_FILE_EXIT = 1004;
    static const int ID_MENU_EDIT_UNDO = 2001;
    static const int ID_MENU_EDIT_REDO = 2002;
    static const int ID_MENU_VIEW_3D = 3001;
    static const int ID_MENU_VIEW_TIMELINE = 3002;
    static const int ID_MENU_VIEW_PROPERTIES = 3003;
    static const int ID_MENU_VIEW_ASSETS = 3004;
    static const int ID_MENU_HELP_ABOUT = 4001;
    
    static const int ID_TOOLBAR_NEW = 5001;
    static const int ID_TOOLBAR_OPEN = 5002;
    static const int ID_TOOLBAR_SAVE = 5003;
    static const int ID_TOOLBAR_UNDO = 5004;
    static const int ID_TOOLBAR_REDO = 5005;
    static const int ID_TOOLBAR_PLAY = 5006;
    static const int ID_TOOLBAR_PAUSE = 5007;
    static const int ID_TOOLBAR_STOP = 5008;
};

}
}
