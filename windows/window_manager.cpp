#include "include/window_manager/window_manager_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

#include <shobjidl_core.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <dwmapi.h>
#include <codecvt>
#include <map>
#include <memory>
#include <sstream>

#pragma comment(lib, "dwmapi.lib")

namespace {

class WindowManager {
 public:
  WindowManager();

  virtual ~WindowManager();

  HWND native_window;

  bool WindowManager::IsMaximized();
  void WindowManager::Maximize();
  HWND GetMainWindow();
  bool WindowManager::IsFullScreen();
  void WindowManager::SetFullScreen(const flutter::EncodableMap& args);

 private:
  bool is_window_fullscreen_ = false;
  std::string title_bar_style_before_fullscreen_;
  RECT frame_before_fullscreen_;
  bool maximized_before_fullscreen_;
  LONG style_before_fullscreen_;
  LONG ex_style_before_fullscreen_;
};

WindowManager::WindowManager() {}

WindowManager::~WindowManager() {}

HWND WindowManager::GetMainWindow() {
  return native_window;
}

bool WindowManager::IsFullScreen() {
  return is_window_fullscreen_;
}

void WindowManager::SetFullScreen(const flutter::EncodableMap& args) {
  bool is_fullscreen =
      std::get<bool>(args.at(flutter::EncodableValue("isFullScreen")));
  HWND main_window = GetMainWindow();
  // Inspired by how Chromium does this
  // https://src.chromium.org/viewvc/chrome/trunk/src/ui/views/win/fullscreen_handler.cc?revision=247204&view=markup
  // Save current window state if not already fullscreen.
  if (!is_window_fullscreen_) {
    // Save current window information.
    maximized_before_fullscreen_ = !!::IsZoomed(main_window);
    style_before_fullscreen_ = GetWindowLong(main_window, GWL_STYLE);
    ex_style_before_fullscreen_ = GetWindowLong(main_window, GWL_EXSTYLE);
    if (maximized_before_fullscreen_) {
      SendMessage(main_window, WM_SYSCOMMAND, SC_RESTORE, 0);
    }
    ::GetWindowRect(main_window, &frame_before_fullscreen_);
  }

  if (is_fullscreen) {
    // Set new window style and size.
    ::SetWindowLong(main_window, GWL_STYLE,
                    style_before_fullscreen_ & ~(WS_CAPTION | WS_THICKFRAME));
    ::SetWindowLong(
        main_window, GWL_EXSTYLE,
        ex_style_before_fullscreen_ & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE |
                                        WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));

    MONITORINFO monitor_info;
    monitor_info.cbSize = sizeof(monitor_info);
    ::GetMonitorInfo(::MonitorFromWindow(main_window, MONITOR_DEFAULTTONEAREST),
                     &monitor_info);
    ::SetWindowPos(main_window, NULL, monitor_info.rcMonitor.left,
                   monitor_info.rcMonitor.top,
                   monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
                   monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
                   SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    ::SendMessage(main_window, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
  } else {
    ::SetWindowLong(main_window, GWL_STYLE, style_before_fullscreen_);
    ::SetWindowLong(main_window, GWL_EXSTYLE, ex_style_before_fullscreen_);
    SendMessage(main_window, WM_SYSCOMMAND, SC_RESTORE, 0);
    if (maximized_before_fullscreen_) {
      Maximize();
    } else {
      ::SetWindowPos(
          main_window, NULL, frame_before_fullscreen_.left,
          frame_before_fullscreen_.top,
          frame_before_fullscreen_.right - frame_before_fullscreen_.left,
          frame_before_fullscreen_.bottom - frame_before_fullscreen_.top,
          SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    }
  }
  is_window_fullscreen_ = is_fullscreen;
}

bool WindowManager::IsMaximized() {
  HWND main_window = GetMainWindow();
  WINDOWPLACEMENT window_placement;
  GetWindowPlacement(main_window, &window_placement);

  return window_placement.showCmd == SW_MAXIMIZE;
}

void WindowManager::Maximize() {
  HWND hwnd = GetMainWindow();
  WINDOWPLACEMENT window_placement;
  GetWindowPlacement(hwnd, &window_placement);
  if (window_placement.showCmd != SW_MAXIMIZE) {
    PostMessage(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
  }
}

}  // namespace
