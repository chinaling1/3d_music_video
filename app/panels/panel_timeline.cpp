/**
 * @file panel_timeline.cpp
 * @brief 时间轴面板实现
 */

#include "panel_timeline.h"
#include <windowsx.h>

namespace v3d {
namespace app {

const wchar_t* PanelTimeline::CLASS_NAME = L"PanelTimeline";

PanelTimeline::PanelTimeline(HWND hParent, HINSTANCE hInstance)
    : hWnd_(nullptr)
    , hParent_(hParent)
    , hInstance_(hInstance)
    , currentFrame_(0)
    , totalFrames_(100)
    , playing_(false)
    , fps_(30.0f)
    , trackHeight_(25)
    , timelineHeight_(40)
    , headerWidth_(150)
{
    tracks_.push_back({L"动画", true, {0, 30, 60, 90}});
    tracks_.push_back({L"位置", true, {0, 50}});
    tracks_.push_back({L"旋转", true, {0, 25, 75}});
    tracks_.push_back({L"缩放", true, {0}});
    
    registerClass();
    createWindow();
}

PanelTimeline::~PanelTimeline() {
    if (hWnd_) {
        DestroyWindow(hWnd_);
    }
}

void PanelTimeline::registerClass() {
    WNDCLASSEXW wcex = {0};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = panelProc;
    wcex.hInstance = hInstance_;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(DKGRAY_BRUSH);
    wcex.lpszClassName = CLASS_NAME;
    RegisterClassExW(&wcex);
}

bool PanelTimeline::createWindow() {
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
    
    return hWnd_ != nullptr;
}

void PanelTimeline::setCurrentFrame(int frame) {
    currentFrame_ = frame;
    if (currentFrame_ < 0) currentFrame_ = 0;
    if (currentFrame_ >= totalFrames_) currentFrame_ = totalFrames_ - 1;
    InvalidateRect(hWnd_, nullptr, FALSE);
}

void PanelTimeline::setTotalFrames(int frames) {
    totalFrames_ = frames;
    InvalidateRect(hWnd_, nullptr, FALSE);
}

void PanelTimeline::play() {
    playing_ = true;
}

void PanelTimeline::pause() {
    playing_ = false;
}

void PanelTimeline::stop() {
    playing_ = false;
    currentFrame_ = 0;
    InvalidateRect(hWnd_, nullptr, FALSE);
}

void PanelTimeline::onPaint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd_, &ps);
    
    RECT rc;
    GetClientRect(hWnd_, &rc);
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;
    
    drawTimeline(hdc, width, height);
    drawTracks(hdc, width, height);
    drawPlayhead(hdc, width);
    
    EndPaint(hWnd_, &ps);
}

void PanelTimeline::drawTimeline(HDC hdc, int width, int height) {
    RECT rcHeader = {0, 0, headerWidth_, timelineHeight_};
    FillRect(hdc, &rcHeader, (HBRUSH)GetStockObject(GRAY_BRUSH));
    
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(100, 100, 100));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    
    SetTextColor(hdc, RGB(200, 200, 200));
    SetBkMode(hdc, TRANSPARENT);
    
    int timelineWidth = width - headerWidth_;
    int frameWidth = timelineWidth / totalFrames_;
    
    for (int i = 0; i <= totalFrames_; i += 10) {
        int x = headerWidth_ + i * frameWidth;
        
        MoveToEx(hdc, x, 0, nullptr);
        LineTo(hdc, x, timelineHeight_);
        
        wchar_t buf[16];
        wsprintfW(buf, L"%d", i);
        TextOutW(hdc, x + 2, 5, buf, wcslen(buf));
    }
    
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

void PanelTimeline::drawTracks(HDC hdc, int width, int height) {
    int y = timelineHeight_;
    
    for (size_t i = 0; i < tracks_.size(); i++) {
        RECT rcHeader = {0, y, headerWidth_, y + trackHeight_};
        COLORREF bgColor = (i % 2 == 0) ? RGB(60, 60, 60) : RGB(50, 50, 50);
        HBRUSH hBrush = CreateSolidBrush(bgColor);
        FillRect(hdc, &rcHeader, hBrush);
        DeleteObject(hBrush);
        
        SetTextColor(hdc, RGB(200, 200, 200));
        SetBkMode(hdc, TRANSPARENT);
        TextOutW(hdc, 10, y + 5, tracks_[i].name.c_str(), tracks_[i].name.length());
        
        RECT rcTrack = {headerWidth_, y, width, y + trackHeight_};
        bgColor = (i % 2 == 0) ? RGB(40, 40, 40) : RGB(35, 35, 35);
        hBrush = CreateSolidBrush(bgColor);
        FillRect(hdc, &rcTrack, hBrush);
        DeleteObject(hBrush);
        
        int timelineWidth = width - headerWidth_;
        int frameWidth = timelineWidth / totalFrames_;
        
        HPEN hKeyPen = CreatePen(PS_SOLID, 1, RGB(255, 200, 0));
        HPEN hOldPen = (HPEN)SelectObject(hdc, hKeyPen);
        
        for (int kf : tracks_[i].keyframes) {
            int x = headerWidth_ + kf * frameWidth;
            MoveToEx(hdc, x, y + 5, nullptr);
            LineTo(hdc, x, y + trackHeight_ - 5);
        }
        
        SelectObject(hdc, hOldPen);
        DeleteObject(hKeyPen);
        
        y += trackHeight_;
    }
}

void PanelTimeline::drawPlayhead(HDC hdc, int width) {
    int timelineWidth = width - headerWidth_;
    int frameWidth = timelineWidth / totalFrames_;
    int x = headerWidth_ + currentFrame_ * frameWidth;
    
    HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 100, 100));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    
    MoveToEx(hdc, x, 0, nullptr);
    LineTo(hdc, x, 200);
    
    POINT triangle[3] = {
        {x - 5, 0},
        {x + 5, 0},
        {x, 10}
    };
    HBRUSH hBrush = CreateSolidBrush(RGB(255, 100, 100));
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
    Polygon(hdc, triangle, 3);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hBrush);
    
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

void PanelTimeline::onSize(int width, int height) {
    UNREFERENCED_PARAMETER(width);
    UNREFERENCED_PARAMETER(height);
    InvalidateRect(hWnd_, nullptr, FALSE);
}

void PanelTimeline::onLButtonDown(int x, int y) {
    if (x > headerWidth_ && y < timelineHeight_) {
        int timelineWidth = 800;
        RECT rc;
        GetClientRect(hWnd_, &rc);
        timelineWidth = rc.right - rc.left - headerWidth_;
        
        int frameWidth = timelineWidth / totalFrames_;
        int frame = (x - headerWidth_) / frameWidth;
        setCurrentFrame(frame);
    }
}

void PanelTimeline::onMouseMove(int x, int y) {
    UNREFERENCED_PARAMETER(x);
    UNREFERENCED_PARAMETER(y);
}

LRESULT CALLBACK PanelTimeline::panelProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    PanelTimeline* pThis = nullptr;
    
    if (message == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<PanelTimeline*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<PanelTimeline*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }
    
    switch (message) {
        case WM_PAINT:
            if (pThis) pThis->onPaint();
            return 0;
            
        case WM_SIZE:
            if (pThis) pThis->onSize(LOWORD(lParam), HIWORD(lParam));
            return 0;
            
        case WM_LBUTTONDOWN:
            if (pThis) pThis->onLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
            
        case WM_MOUSEMOVE:
            if (wParam & MK_LBUTTON) {
                if (pThis) pThis->onLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            }
            return 0;
            
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

}
}
