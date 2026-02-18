/**
 * @file main_window.cpp
 * @brief 主窗口类实现
 */

#include "main_window.h"
#include "../panels/panel_3d_view.h"
#include "../panels/panel_timeline.h"
#include "../panels/panel_properties.h"
#include "../panels/panel_asset_browser.h"
#include "../../src/core/logger.h"
#include <CommCtrl.h>
#include <windowsx.h>

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace v3d {
namespace app {

static const wchar_t* CLASS_NAME = L"3DVideoStudioMainWindow";

MainWindow::MainWindow()
    : hWnd_(nullptr)
    , hInstance_(nullptr)
    , width_(0)
    , height_(0)
    , hMenuBar_(nullptr)
    , hToolBar_(nullptr)
    , hStatusBar_(nullptr)
{
}

MainWindow::~MainWindow() {
    panel3DView_.reset();
    panelTimeline_.reset();
    panelProperties_.reset();
    panelAssetBrowser_.reset();
    
    if (hToolBar_) {
        DestroyWindow(hToolBar_);
    }
    if (hStatusBar_) {
        DestroyWindow(hStatusBar_);
    }
    if (hWnd_) {
        DestroyWindow(hWnd_);
    }
}

bool MainWindow::create(HINSTANCE hInstance, const std::wstring& title, int width, int height) {
    hInstance_ = hInstance;
    title_ = title;
    width_ = width;
    height_ = height;

    INITCOMMONCONTROLSEX icc = {0};
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_WIN95_CLASSES | ICC_COOL_CLASSES | ICC_BAR_CLASSES;
    InitCommonControlsEx(&icc);

    if (!registerClass()) {
        return false;
    }

    DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
    DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;

    RECT rc = {0, 0, width, height};
    AdjustWindowRectEx(&rc, dwStyle, TRUE, dwExStyle);

    hWnd_ = CreateWindowExW(
        dwExStyle,
        CLASS_NAME,
        title.c_str(),
        dwStyle,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        nullptr,
        nullptr,
        hInstance,
        this
    );

    if (!hWnd_) {
        return false;
    }

    createMenuBar();
    createToolBar();
    createStatusBar();
    createPanels();

    return true;
}

void MainWindow::show(int nCmdShow) {
    ShowWindow(hWnd_, nCmdShow);
    UpdateWindow(hWnd_);
}

bool MainWindow::registerClass() {
    WNDCLASSEXW wcex = {0};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc = windowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = sizeof(MainWindow*);
    wcex.hInstance = hInstance_;
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = CLASS_NAME;
    wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

    return RegisterClassExW(&wcex) != 0;
}

void MainWindow::createMenuBar() {
    HMENU hMenu = CreateMenu();
    
    HMENU hFileMenu = CreatePopupMenu();
    AppendMenuW(hFileMenu, MF_STRING, ID_MENU_FILE_NEW, L"新建(&N)\tCtrl+N");
    AppendMenuW(hFileMenu, MF_STRING, ID_MENU_FILE_OPEN, L"打开(&O)...\tCtrl+O");
    AppendMenuW(hFileMenu, MF_STRING, ID_MENU_FILE_SAVE, L"保存(&S)\tCtrl+S");
    AppendMenuW(hFileMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hFileMenu, MF_STRING, ID_MENU_FILE_EXIT, L"退出(&X)\tAlt+F4");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"文件(&F)");
    
    HMENU hEditMenu = CreatePopupMenu();
    AppendMenuW(hEditMenu, MF_STRING, ID_MENU_EDIT_UNDO, L"撤销(&U)\tCtrl+Z");
    AppendMenuW(hEditMenu, MF_STRING, ID_MENU_EDIT_REDO, L"重做(&R)\tCtrl+Y");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hEditMenu, L"编辑(&E)");
    
    HMENU hViewMenu = CreatePopupMenu();
    AppendMenuW(hViewMenu, MF_STRING, ID_MENU_VIEW_3D, L"3D视图(&3)");
    AppendMenuW(hViewMenu, MF_STRING, ID_MENU_VIEW_TIMELINE, L"时间轴(&T)");
    AppendMenuW(hViewMenu, MF_STRING, ID_MENU_VIEW_PROPERTIES, L"属性(&P)");
    AppendMenuW(hViewMenu, MF_STRING, ID_MENU_VIEW_ASSETS, L"资源管理器(&A)");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hViewMenu, L"视图(&V)");
    
    HMENU hHelpMenu = CreatePopupMenu();
    AppendMenuW(hHelpMenu, MF_STRING, ID_MENU_HELP_ABOUT, L"关于(&A)...");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hHelpMenu, L"帮助(&H)");
    
    SetMenu(hWnd_, hMenu);
    hMenuBar_ = (HWND)hMenu;
}

void MainWindow::createToolBar() {
    const int buttonCount = 8;
    
    hToolBar_ = CreateWindowExW(
        0, TOOLBARCLASSNAMEW, nullptr,
        WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS,
        0, 0, 0, 0,
        hWnd_, (HMENU)1, hInstance_, nullptr
    );
    
    SendMessageW(hToolBar_, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
    SendMessageW(hToolBar_, TB_SETBITMAPSIZE, 0, MAKELPARAM(16, 16));
    SendMessageW(hToolBar_, TB_SETBUTTONSIZE, 0, MAKELPARAM(24, 24));
    
    TBBUTTON tbButtons[buttonCount] = {0};
    
    tbButtons[0].iBitmap = STD_FILENEW;
    tbButtons[0].idCommand = ID_TOOLBAR_NEW;
    tbButtons[0].fsState = TBSTATE_ENABLED;
    tbButtons[0].fsStyle = BTNS_BUTTON;
    
    tbButtons[1].iBitmap = STD_FILEOPEN;
    tbButtons[1].idCommand = ID_TOOLBAR_OPEN;
    tbButtons[1].fsState = TBSTATE_ENABLED;
    tbButtons[1].fsStyle = BTNS_BUTTON;
    
    tbButtons[2].iBitmap = STD_FILESAVE;
    tbButtons[2].idCommand = ID_TOOLBAR_SAVE;
    tbButtons[2].fsState = TBSTATE_ENABLED;
    tbButtons[2].fsStyle = BTNS_BUTTON;
    
    tbButtons[3].fsStyle = BTNS_SEP;
    
    tbButtons[4].iBitmap = STD_UNDO;
    tbButtons[4].idCommand = ID_TOOLBAR_UNDO;
    tbButtons[4].fsState = TBSTATE_ENABLED;
    tbButtons[4].fsStyle = BTNS_BUTTON;
    
    tbButtons[5].iBitmap = STD_REDOW;
    tbButtons[5].idCommand = ID_TOOLBAR_REDO;
    tbButtons[5].fsState = TBSTATE_ENABLED;
    tbButtons[5].fsStyle = BTNS_BUTTON;
    
    tbButtons[6].fsStyle = BTNS_SEP;
    
    tbButtons[7].iBitmap = STD_COPY;
    tbButtons[7].idCommand = ID_TOOLBAR_PLAY;
    tbButtons[7].fsState = TBSTATE_ENABLED;
    tbButtons[7].fsStyle = BTNS_BUTTON;
    
    SendMessageW(hToolBar_, TB_ADDBUTTONS, buttonCount, (LPARAM)&tbButtons);
    SendMessageW(hToolBar_, TB_AUTOSIZE, 0, 0);
}

void MainWindow::createStatusBar() {
    hStatusBar_ = CreateWindowExW(
        0, STATUSCLASSNAMEW, nullptr,
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0,
        hWnd_, (HMENU)2, hInstance_, nullptr
    );
    
    const int parts = 4;
    int widths[parts] = {200, 400, 600, -1};
    SendMessageW(hStatusBar_, SB_SETPARTS, parts, (LPARAM)widths);
    SendMessageW(hStatusBar_, SB_SETTEXTW, 0, (LPARAM)L"就绪");
    SendMessageW(hStatusBar_, SB_SETTEXTW, 1, (LPARAM)L"帧: 0");
    SendMessageW(hStatusBar_, SB_SETTEXTW, 2, (LPARAM)L"时间: 00:00:00");
    SendMessageW(hStatusBar_, SB_SETTEXTW, 3, (LPARAM)L"3D Video Studio v1.0");
}

void MainWindow::createPanels() {
    panel3DView_ = std::make_unique<Panel3DView>(hWnd_, hInstance_);
    panelTimeline_ = std::make_unique<PanelTimeline>(hWnd_, hInstance_);
    panelProperties_ = std::make_unique<PanelProperties>(hWnd_, hInstance_);
    panelAssetBrowser_ = std::make_unique<PanelAssetBrowser>(hWnd_, hInstance_);
}

void MainWindow::layoutChildren() {
    RECT rcClient;
    GetClientRect(hWnd_, &rcClient);
    
    RECT rcToolBar = {0};
    if (hToolBar_) {
        GetWindowRect(hToolBar_, &rcToolBar);
    }
    
    RECT rcStatusBar = {0};
    if (hStatusBar_) {
        GetWindowRect(hStatusBar_, &rcStatusBar);
    }
    
    int toolbarHeight = rcToolBar.bottom - rcToolBar.top;
    int statusbarHeight = rcStatusBar.bottom - rcStatusBar.top;
    
    int clientWidth = rcClient.right - rcClient.left;
    int clientHeight = rcClient.bottom - rcClient.top - toolbarHeight - statusbarHeight;
    
    const int leftPanelWidth = 250;
    const int rightPanelWidth = 300;
    const int bottomPanelHeight = 200;
    
    int centerWidth = clientWidth - leftPanelWidth - rightPanelWidth;
    int topHeight = clientHeight - bottomPanelHeight;
    
    if (panelAssetBrowser_) {
        SetWindowPos(panelAssetBrowser_->getHandle(), nullptr,
            0, toolbarHeight,
            leftPanelWidth, clientHeight,
            SWP_NOZORDER | SWP_SHOWWINDOW);
    }
    
    if (panel3DView_) {
        SetWindowPos(panel3DView_->getHandle(), nullptr,
            leftPanelWidth, toolbarHeight,
            centerWidth, topHeight,
            SWP_NOZORDER | SWP_SHOWWINDOW);
    }
    
    if (panelProperties_) {
        SetWindowPos(panelProperties_->getHandle(), nullptr,
            leftPanelWidth + centerWidth, toolbarHeight,
            rightPanelWidth, clientHeight,
            SWP_NOZORDER | SWP_SHOWWINDOW);
    }
    
    if (panelTimeline_) {
        SetWindowPos(panelTimeline_->getHandle(), nullptr,
            leftPanelWidth, toolbarHeight + topHeight,
            centerWidth, bottomPanelHeight,
            SWP_NOZORDER | SWP_SHOWWINDOW);
    }
}

void MainWindow::handleCommand(WPARAM wParam) {
    int id = LOWORD(wParam);
    
    switch (id) {
        case ID_MENU_FILE_NEW:
        case ID_TOOLBAR_NEW:
            handleMenuCommand(ID_MENU_FILE_NEW);
            break;
        case ID_MENU_FILE_OPEN:
        case ID_TOOLBAR_OPEN:
            handleMenuCommand(ID_MENU_FILE_OPEN);
            break;
        case ID_MENU_FILE_SAVE:
        case ID_TOOLBAR_SAVE:
            handleMenuCommand(ID_MENU_FILE_SAVE);
            break;
        case ID_MENU_FILE_EXIT:
            PostMessage(hWnd_, WM_CLOSE, 0, 0);
            break;
        case ID_MENU_EDIT_UNDO:
        case ID_TOOLBAR_UNDO:
            handleMenuCommand(ID_MENU_EDIT_UNDO);
            break;
        case ID_MENU_EDIT_REDO:
        case ID_TOOLBAR_REDO:
            handleMenuCommand(ID_MENU_EDIT_REDO);
            break;
        case ID_TOOLBAR_PLAY:
            handleMenuCommand(ID_TOOLBAR_PLAY);
            break;
        default:
            break;
    }
}

void MainWindow::handleMenuCommand(int menuId) {
    auto logger = v3d::core::LoggerManager::getInstance().getOrCreateLogger("App");
    
    switch (menuId) {
        case ID_MENU_FILE_NEW:
            LOG_INFO(logger, "New project");
            MessageBoxW(hWnd_, L"创建新项目", L"新建", MB_OK | MB_ICONINFORMATION);
            break;
        case ID_MENU_FILE_OPEN:
            LOG_INFO(logger, "Open project");
            MessageBoxW(hWnd_, L"打开项目", L"打开", MB_OK | MB_ICONINFORMATION);
            break;
        case ID_MENU_FILE_SAVE:
            LOG_INFO(logger, "Save project");
            MessageBoxW(hWnd_, L"保存项目", L"保存", MB_OK | MB_ICONINFORMATION);
            break;
        case ID_MENU_EDIT_UNDO:
            LOG_INFO(logger, "Undo");
            break;
        case ID_MENU_EDIT_REDO:
            LOG_INFO(logger, "Redo");
            break;
        case ID_TOOLBAR_PLAY:
            LOG_INFO(logger, "Play");
            break;
        case ID_MENU_HELP_ABOUT:
            MessageBoxW(hWnd_,
                L"3D Video Studio v1.0\n\n"
                L"一个功能强大的3D视频制作工作室软件\n\n"
                L"功能:\n"
                L"- 3D建模和动画\n"
                L"- 音频处理\n"
                L"- 视频编辑\n"
                L"- 格式转换\n\n"
                L"© 2026 3D Video Studio Team",
                L"关于 3D Video Studio",
                MB_OK | MB_ICONINFORMATION);
            break;
        default:
            break;
    }
}

void MainWindow::handleSize(int width, int height) {
    width_ = width;
    height_ = height;
    
    SendMessageW(hToolBar_, TB_AUTOSIZE, 0, 0);
    SendMessageW(hStatusBar_, WM_SIZE, 0, 0);
    
    layoutChildren();
}

LRESULT CALLBACK MainWindow::windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    MainWindow* pThis = nullptr;
    
    if (message == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<MainWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }
    
    switch (message) {
        case WM_CREATE:
            return 0;
            
        case WM_SIZE:
            if (pThis) {
                pThis->handleSize(LOWORD(lParam), HIWORD(lParam));
            }
            return 0;
            
        case WM_COMMAND:
            if (pThis) {
                pThis->handleCommand(wParam);
            }
            return 0;
            
        case WM_NOTIFY:
            return 0;
            
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                BeginPaint(hWnd, &ps);
                EndPaint(hWnd, &ps);
            }
            return 0;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
            
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

}
}
