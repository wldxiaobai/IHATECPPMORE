#include "globalplayer.h"

#include "player_object.h"
#include "blood.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <limits>
#include <optional>
#include <cute.h>

namespace {
namespace fs = std::filesystem;

constexpr char kSaveDir[] = "save";
constexpr char kRespawnRecordFile[] = "respawn.record";

// 预设好的存档目录路径
fs::path MakeSaveDirectoryPath() {
	return fs::current_path() / kSaveDir;
}

// 复活记录文件路径
fs::path MakeRespawnRecordPath() {
	return MakeSaveDirectoryPath() / kRespawnRecordFile;
}

struct RespawnRecord {
	bool has_record = false;
	CF_V2 point = cf_v2(0.0f, 0.0f);
	std::string room_name;
};

//open：LoadRespawnRecordFromDisk 中 std::ifstream ifs(path, std::ios::in); 
//		在文件存在的前提下构造了 ifstream，并通过 if (!ifs) 检查是否成功打开文件。
//read：紧接着的 ifs >> flag、std::getline(ifs, record.room_name) 以及 
//		ifs >> record.point.x >> record.point.y 分别从流中依次读取标志、房间名称和位置坐标。
//close：ifs 离开作用域（函数返回或退出）时会自动调用析构函数关闭文件，
//		所有路径都依赖析构完成关闭行为。
std::optional<RespawnRecord> LoadRespawnRecordFromDisk() noexcept {
	auto path = MakeRespawnRecordPath();
	if (!fs::exists(path)) {
		return std::nullopt;
	}

	std::ifstream ifs(path, std::ios::in);
	if (!ifs) {
		return std::nullopt;
	}

	RespawnRecord record;
	int flag = 0;
	ifs >> flag;
	if (!ifs) {
		return std::nullopt;
	}
	record.has_record = flag != 0;
	ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	if (!std::getline(ifs, record.room_name)) {
		record.room_name.clear();
	}

	if (!(ifs >> record.point.x >> record.point.y)) {
		return std::nullopt;
	}

	return record;
}

//open：std::ofstream ofs(MakeRespawnRecordPath(), std::ios::trunc); 
//		构造流并打开 respawn.record 文件，std::ios::trunc 清空旧内容。
//write：随后三行 ofs << … 将标志、房间名和坐标依次写入流。
//close：函数结束或异常跳出时，ofs 析构自动关闭文件（无需手动 close）。
bool PersistRespawnRecordToDisk(bool has_record, const std::string& room_name, CF_V2 point) noexcept {
	try {
		auto dir = MakeSaveDirectoryPath();
		if (!fs::exists(dir)) {
			fs::create_directories(dir);
		}

		std::ofstream ofs(MakeRespawnRecordPath(), std::ios::trunc);
		if (!ofs) {
			return false;
		}

		ofs << (has_record ? 1 : 0) << '\n';
		ofs << room_name << '\n';
		ofs << point.x << ' ' << point.y << '\n';
		return static_cast<bool>(ofs);
	} catch (...) {
		return false;
	}
}
}

// 复活玩家到当前复活点
void GlobalPlayer::Respawn() {
	if (respawn_room != RoomLoader::Instance().GetCurrentRoom()) {
		OUTPUT({ "GlobalPlayer::Respawn" }, "Warning: Respawning in a different room without loading it.");
	}
	if (!ObjManager::Instance().TryGetRegisteration(player_token)) {
		player_token = ObjManager::Instance().Create<PlayerObject>(respawn_point);
	}
	else {
		ObjManager::Instance()[player_token].SetPosition(respawn_point);
	}
}

// 处理玩家从出现点或复活点返回游戏的逻辑
void GlobalPlayer::Emerge() {
	if (!emerge_pos.need_emerge) Respawn();
	else if (!ObjManager::Instance().TryGetRegisteration(player_token)) {
		player_token = ObjManager::Instance().Create<PlayerObject>(emerge_pos.position);
	}
	else {
		ObjManager::Instance()[player_token].SetPosition(emerge_pos.position);
	}
	emerge_pos.need_emerge = false;
}

// 触发玩家受伤效果：产生血迹并销毁当前玩家实例
void GlobalPlayer::Hurt() {
	if (!objs.TryGetRegisteration(player_token)) return;
	CF_V2 pos = objs[player_token].GetPosition();
	int amt = 24;
	float speed = 5.0f;
	auto time_seed = static_cast<int>(std::chrono::steady_clock::now().time_since_epoch().count());
	for (int i = 0; i < amt; i++) {
		float angle = pi * 2 * (i * 1.0f + 0.5f) / amt;
		auto tweak = cf_rnd_seed(time_seed + i);
		objs.Create<Blood>(pos, cf_rnd_range_float(&tweak, speed * 0.6f, speed * 1.2f) * v2math::get_dir(angle));
	}
	objs.Destroy(player_token);
	cf_play_sound(cf_audio_load_wav("/audio/sound_die.WAV"), cf_sound_params_defaults());
}

// 从磁盘加载复活点记录，若存在则更新成员变量并返回 true，否则返回 false
bool GlobalPlayer::LoadSavedRespawn() noexcept {
	if (auto record = LoadRespawnRecordFromDisk()) {
		respawn_point = record->point;
		respawn_room_name = record->room_name;
		respawn_room = RoomLoader::Instance().GetRoomByName(respawn_room_name);
		if (!respawn_room) {
			respawn_room = RoomLoader::Instance().GetInitialRoom();
		}
		has_record = record->has_record;
		return true;
	}

	respawn_room_name.clear();
	respawn_room = RoomLoader::Instance().GetInitialRoom();
	has_record = false;
	return false;
}

// 将当前复活点记录保存到磁盘
bool GlobalPlayer::PersistRespawnRecord() const noexcept {
	return PersistRespawnRecordToDisk(has_record, respawn_room_name, respawn_point);
}