/**
 * @file panel_timeline.h
 * @brief 时间轴面板 - 显示和编辑动画时间轴
 */

#pragma once

#include <Windows.h>
#include <string>
#include <vector>

namespace v3d {
namespace app {

struct TimelineTrack {
    std::wstring name;
    bool expanded;
    std::vector<int> keyframes;
};

class PanelTimeline {
public:
    PanelTimeline(HWND hParent, HINSTANCE hInstance);
    ~PanelTimeline();
    
    HWND getHandle() const { return hWnd_; }
    
    void setCurrentFrame(int frame);
    int getCurrentFrame() const { return currentFrame_; }
    
    void setTotalFrames(int frames);
    int getTotalFrames() const { return totalFrames_; }
    
    void play();
    void pause();
    void stop();
    bool isPlaying() const { return playing_; }
    
private:
    bool createWindow();
    void registerClass();
    static LRESULT CALLBACK panelProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    
    void onPaint();
    void onSize(int width, int height);
    void onLButtonDown(int x, int y);
    void onMouseMove(int x, int y);
    
    void drawTimeline(HDC hdc, int width, int height);
    void drawTracks(HDC hdc, int width, int height);
    void drawPlayhead(HDC hdc, int width);
    
    HWND hWnd_;
    HWND hParent_;
    HINSTANCE hInstance_;
    
    int currentFrame_;
    int totalFrames_;
    bool playing_;
    float fps_;
    
    std::vector<TimelineTrack> tracks_;
    int trackHeight_;
    int timelineHeight_;
    int headerWidth_;
    
    static const wchar_t* CLASS_NAME;
};

}
}
