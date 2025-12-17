#pragma once
#include "room_loader.h"
#include "obj_manager.h"
#include "player_object.h"
#include "blood.h"

class GlobalPlayer {
public:
	GlobalPlayer() noexcept
		: respawn_room(RoomLoader::Instance().GetInitialRoom()) // 明确初始化 current_room
	{}

	~GlobalPlayer() noexcept {}

	// 创建单例实例的接口
	static GlobalPlayer& Instance() noexcept {
		static GlobalPlayer instance;
		return instance;
	}

	void SetRespawnPoint(CF_V2 pos) noexcept {
		respawn_point = pos;
		respawn_room = RoomLoader::Instance().GetCurrentRoom();
		has_record = true;
	}

	bool HasRespawnRecord() const noexcept { return has_record; }

	const BaseRoom* GetRespawnRoom() const noexcept {
		return respawn_room;
	}

	void Respawn() {
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

	void SetEmergePosition(CF_V2 pos) noexcept {
		emerge_pos.need_emerge = true;
		emerge_pos.position = pos;
	}

	void Emerge() {
		if (!emerge_pos.need_emerge) Respawn();
		else if (!ObjManager::Instance().TryGetRegisteration(player_token)) {
			player_token = ObjManager::Instance().Create<PlayerObject>(emerge_pos.position);
		}
		else {
			ObjManager::Instance()[player_token].SetPosition(emerge_pos.position);
		}
		emerge_pos.need_emerge = false;
	}

	ObjManager::ObjToken& Player() { return player_token; }

	void Hurt() {
		if (!objs.TryGetRegisteration(player_token)) return;
		CF_V2 pos = objs[player_token].GetPosition();
		int amt = 8;
		float speed = 3.0f;
		for (int i = 0; i < amt; i++) {
			objs.Create<Blood>(pos, speed * v2math::get_dir(pi * 2 * i / amt));
		}
		objs.Destroy(player_token);
	}

private:
	CF_V2 respawn_point = cf_v2(0.0f, 0.0f);
	const BaseRoom* respawn_room;
	bool has_record = false;

	ObjManager::ObjToken player_token = ObjManager::ObjToken::Invalid();
	struct EmergePos {
		bool need_emerge = false;
		CF_V2 position = cf_v2(0.0f, 0.0f);
	} emerge_pos;
};