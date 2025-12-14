// File: player_audio.h
#pragma once
#include <string>

class PlayerAudio {
public:
    // 初始化音频系统
    static bool initialize();
    
    // 关闭音频系统
    static void shutdown();
    
    // 玩家音效
    static void playJump();
    static void playShoot();
    static void playLand();
    static void playHurt();
    static void playWalk();
    
    // 音量控制（简单实现）
    static void setVolume(float volume); // 0.0 - 1.0
    static float getVolume();
    
private:
    PlayerAudio() = delete;
    ~PlayerAudio() = delete;
    
    // 音效文件路径
    static const std::string JUMP_SOUND_PATH;
    static const std::string SHOOT_SOUND_PATH;
    static const std::string LAND_SOUND_PATH;
    static const std::string HURT_SOUND_PATH;
    static const std::string WALK_SOUND_PATH;
    
    // 音量
    static float s_volume;
    
    // 是否已初始化
    static bool s_initialized;
    
    // 移动音效冷却
    static int s_walkCooldown;
    static const int WALK_COOLDOWN_FRAMES = 20;
};