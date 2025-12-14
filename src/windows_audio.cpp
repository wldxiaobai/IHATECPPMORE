// File: windows_audio.cpp
#include "windows_audio.h"
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <vector>

#pragma comment(lib, "winmm.lib")

WindowsAudio::WindowsAudio() = default;

WindowsAudio::~WindowsAudio() {
    shutdown();
}

WindowsAudio& WindowsAudio::getInstance() {
    static WindowsAudio instance;
    return instance;
}

bool WindowsAudio::initialize() {
    if (m_initialized) {
        return true;
    }

    // Windows音频系统不需要显式初始化
    // PlaySound会自动初始化所需资源

    m_initialized = true;
    std::cout << "[Audio] Windows audio system initialized" << std::endl;
    return true;
}

void WindowsAudio::shutdown() {
    if (!m_initialized) {
        return;
    }

    // 停止所有正在播放的音效
    stopAll();

    m_initialized = false;
    std::cout << "[Audio] Windows audio system shutdown" << std::endl;
}

void WindowsAudio::playSound(const std::string& soundName, const std::string& filePath) {
    if (!m_initialized) {
        std::cerr << "[Audio] System not initialized, cannot play: " << soundName << std::endl;
        return;
    }

    // 使用Windows PlaySound API播放音效
    // SND_ASYNC: 异步播放（不阻塞）
    // SND_FILENAME: 从文件播放
    // SND_NODEFAULT: 如果文件不存在，不播放默认声音
    if (!PlaySound(filePath.c_str(), NULL, SND_ASYNC | SND_FILENAME | SND_NODEFAULT)) {
        std::cerr << "[Audio] Failed to play sound: " << soundName
            << " from: " << filePath << std::endl;
        return;
    }

    // 调试信息
#ifdef _DEBUG
    std::cout << "[Audio] Playing: " << soundName << std::endl;
#endif
}

void WindowsAudio::stopAll() {
    // 停止所有正在播放的音效
    PlaySound(NULL, NULL, 0);
}