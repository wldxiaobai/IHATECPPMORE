// File: windows_audio.h
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

// Windows专用音频系统 - 使用Windows原生API
class WindowsAudio {
public:
    // 获取单例实例
    static WindowsAudio& getInstance();

    // 初始化音频系统
    bool initialize();

    // 关闭音频系统
    void shutdown();

    // 播放音效（异步播放）
    void playSound(const std::string& soundName, const std::string& filePath);

    // 停止所有音效
    void stopAll();

    // 检查是否已初始化
    bool isInitialized() const { return m_initialized; }

private:
    WindowsAudio();
    ~WindowsAudio();

    // 禁止拷贝和移动
    WindowsAudio(const WindowsAudio&) = delete;
    WindowsAudio& operator=(const WindowsAudio&) = delete;
    WindowsAudio(WindowsAudio&&) = delete;
    WindowsAudio& operator=(WindowsAudio&&) = delete;

    bool m_initialized = false;
};