#include "globalplayer.h"

#include "player_object.h"
#include "blood.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <limits>
#include <optional>

namespace {
namespace fs = std::filesystem;

constexpr char kSaveDir[] = "save";
constexpr char kRespawnRecordFile[] = "respawn.record";

fs::path MakeSaveDirectoryPath() {
	return fs::current_path() / kSaveDir;
}

fs::path MakeRespawnRecordPath() {
	return MakeSaveDirectoryPath() / kRespawnRecordFile;
}

struct RespawnRecord {
	bool has_record = false;
	CF_V2 point = cf_v2(0.0f, 0.0f);
	std::string room_name;
};

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
	int amt = 16;
	float speed = 5.0f;
	auto time_seed = static_cast<int>(std::chrono::steady_clock::now().time_since_epoch().count());
	for (int i = 0; i < amt; i++) {
		float angle = pi * 2 * (i * 1.0f + 0.5f) / amt;
		auto tweak = cf_rnd_seed(time_seed + i);
		objs.Create<Blood>(pos, cf_rnd_range_float(&tweak, speed * 0.6f, speed * 1.2f) * v2math::get_dir(angle));
	}
	objs.Destroy(player_token);
}

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

bool GlobalPlayer::PersistRespawnRecord() const noexcept {
	return PersistRespawnRecordToDisk(has_record, respawn_room_name, respawn_point);
}