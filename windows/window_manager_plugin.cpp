#include "include/window_manager/window_manager_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <codecvt>
#include <map>
#include <memory>
#include <sstream>

#include "window_manager.cpp"

namespace {

class WindowManagerPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows* registrar);

  WindowManagerPlugin(flutter::PluginRegistrarWindows* registrar);

 private:
  std::unique_ptr<WindowManager> window_manager_;
  flutter::PluginRegistrarWindows* registrar_;

  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
};

// static
void WindowManagerPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows* registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "window_manager",
          &flutter::StandardMethodCodec::GetInstance());
  auto plugin = std::make_unique<WindowManagerPlugin>(registrar);
  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto& call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });
  registrar->AddPlugin(std::move(plugin));
}

WindowManagerPlugin::WindowManagerPlugin(
    flutter::PluginRegistrarWindows* registrar)
    : registrar_(registrar),
      window_manager_(std::make_unique<WindowManager>()) {}

void WindowManagerPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  std::string method_name = method_call.method_name();

  if (method_name.compare("ensureInitialized") == 0) {
    window_manager_->native_window =
        ::GetAncestor(registrar_->GetView()->GetNativeWindow(), GA_ROOT);
    result->Success(flutter::EncodableValue(true));
  } else if (method_name.compare("isFullScreen") == 0) {
    bool value = window_manager_->IsFullScreen();
    result->Success(flutter::EncodableValue(value));
  } else if (method_name.compare("setFullScreen") == 0) {
    const flutter::EncodableMap& args =
        std::get<flutter::EncodableMap>(*method_call.arguments());
    window_manager_->SetFullScreen(args);
    result->Success(flutter::EncodableValue(true));
  } else {
    result->NotImplemented();
  }
}

}  // namespace

void WindowManagerPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  WindowManagerPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
