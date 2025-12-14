// File: player_audio.cpp
#include "player_audio.h"
#include "windows_audio.h"
#include <iostream>

// 音效文件路径 - 相对于可执行文件的路径
const std::string PlayerAudio::JUMP_SOUND_PATH = "/audio/sound_jump.WAV";
const std::string PlayerAudio::SHOOT_SOUND_PATH = "/audio/sound_shoot.WAV";
const std::string PlayerAudio::LAND_SOUND_PATH = "/audio/fall.WAV";
// 静态成员初始化
float PlayerAudio::s_volume = 1.0f;
bool PlayerAudio::s_initialized = false;
int PlayerAudio::s_walkCooldown = 0;

bool PlayerAudio::initialize() {
    if (s_initialized) {
        return true;
    }

    // 初始化Windows音频系统
    if (!WindowsAudio::getInstance().initialize()) {
        std::cerr << "[Audio] Failed to initialize audio system" << std::endl;
        return false;
    }

    s_initialized = true;
    std::cout << "[Audio] Player audio initialized" << std::endl;
    return true;
}

void PlayerAudio::shutdown() {
    if (!s_initialized) {
        return;
    }

    WindowsAudio::getInstance().shutdown();
    s_initialized = false;
}

void PlayerAudio::playJump() {
    if (!s_initialized) return;
    WindowsAudio::getInstance().playSound("jump", JUMP_SOUND_PATH);
}

void PlayerAudio::playShoot() {
    if (!s_initialized) return;
    WindowsAudio::getInstance().playSound("shoot", SHOOT_SOUND_PATH);
}

void PlayerAudio::playLand() {
    if (!s_initialized) return;
    WindowsAudio::getInstance().playSound("land", LAND_SOUND_PATH);
}

void PlayerAudio::playHurt() {
    if (!s_initialized) return;
    WindowsAudio::getInstance().playSound("hurt", HURT_SOUND_PATH);
}

//void PlayerObject::OnCollisionExit(const ObjManager::ObjToken& other_token, const CF_Manifold& manifold) noexcept
//{
//    if (objs[other_token].GetColliderType() != ColliderType::SOLID) return;
//
//    // 离开碰撞时取消着地标记
//    s_grounded_map[this] = false;
//
//    // 启动 coyote 时间（离地后短时间仍可起跳）
//    s_coyote_time_left[this] = coyote_time_frames;
//}
//void PlayerAudio::playWalk() {
//    if (!s_initialized) return;
//
//    // 检查冷却时间
//    if (s_walkCooldown > 0) {
//        s_walkCooldown--;
//        return;
//    }
//
//    WindowsAudio::getInstance().playSound("walk", WALK_SOUND_PATH);
//    s_walkCooldown = WALK_COOLDOWN_FRAMES;
//}

void PlayerAudio::setVolume(float volume) {
    s_volume = volume;
    if (s_volume < 0.0f) s_volume = 0.0f;
    if (s_volume > 1.0f) s_volume = 1.0f;

    // 注意：Windows PlaySound API不支持音量调节
    // 这里只是记录音量值，实际播放时不会应用
}

float PlayerAudio::getVolume() {
    return s_volume;
}