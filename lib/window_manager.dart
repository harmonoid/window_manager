import 'dart:async';
import 'package:flutter/services.dart';

class WindowManager {
  static const WindowManager instance = WindowManager._();

  const WindowManager._();

  Future<bool> isFullScreen() async {
    return await _channel.invokeMethod(
      'isFullScreen',
    );
  }

  Future<void> setFullScreen(bool isFullScreen) async {
    await _channel.invokeMethod(
      'setFullScreen',
      {
        'isFullScreen': isFullScreen,
      },
    );
  }

  static MethodChannel _channel = const MethodChannel('window_manager');
}
