/**
 * @file main.cpp
 * @brief 3D Video Studio 应用程序入口
 * 
 * 这是3D视频工作室的主程序入口，初始化应用程序并启动主窗口。
 * 
 * @author 3D Video Studio Team
 * @version 1.0
 * @date 2026-02-17
 */

#include "ui/main_window.h"
#include "../src/core/logger.h"
#include <Windows.h>
#include <memory>

using namespace v3d::core;
using namespace v3d::app;

/**
 * @brief 应用程序主入口
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    auto logger = LoggerManager::getInstance().getOrCreateLogger("App");
    logger->addAppender(std::make_shared<ConsoleAppender>());
    logger->setLevel(LogLevel::Debug);
    
    LOG_INFO(logger, "3D Video Studio starting...");

    MainWindow mainWindow;
    if (!mainWindow.create(hInstance, L"3D Video Studio", 1600, 900)) {
        LOG_ERROR(logger, "Failed to create main window");
        return -1;
    }

    LOG_INFO(logger, "Main window created successfully");
    
    mainWindow.show(nCmdShow);

    MSG msg = {0};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    LOG_INFO(logger, "3D Video Studio shutting down");
    
    return (int)msg.wParam;
}
