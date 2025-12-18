#include "room_loader.h"
#include "UI_draw.h"
#include "input.h"

#include "player_object.h"
#include "backgroud.h"
#include "block_object.h"
#include "move_spike.h"
#include "up_move_spike.h"
#include "spike.h"
#include "down_spike.h"
#include "checkpoint.h"
#include "globalplayer.h"
#include "right_move_block.h"
#include "vertical_moving_spike.h"
#include "left_move_block.h"
#include "diagonal_move_spike.h"
#include "diagonal_move_spike_left.h"

class EmptyRoom : public BaseRoom {
public:
	EmptyRoom() noexcept {}
	~EmptyRoom() noexcept override {}

	void RoomLoad() override {
		OUTPUT({ "EmptyRoom" }, "RoomLoad called.");

		auto& objs = ObjManager::Instance();
		auto& g_player = GlobalPlayer::Instance();

		float hw = DrawUI::half_w;
		float hh = DrawUI::half_h;

		//复活点1
		objs.Create<Checkpoint>(cf_v2(-hw + 36.0f,-hh + 36.0f));

		// 创建背景对象
		auto background_token = objs.Create<Backgroud>();

	
		if (!g_player.HasRespawnRecord())g_player.SetRespawnPoint(cf_v2(-hw + 36 * 1.5f, -hh + 36 * 2));
		g_player.Emerge();


		////获取玩家位置
		//auto& player = GlobalPlayer::Instance().Player();
		//CF_V2 pos = objs[player].GetPosition();
		//float pos_x = objs[player].GetPosition().x;
		//float pos_y = objs[player].GetPosition().y;

		//方块系统
		
		//静止系统!!!加静止方块时注意要考虑true和false;
		
		//第一行的方块
		for(float x = -hw; x < hw; x += 36.0f)
		{
			objs.Create<BlockObject>(cf_v2(x,hh - 36.0f),false);
		}

		//第一列下方的方块
		for (float y = -hh; y < hh - 5 * 36; y += 72) {
			objs.Create<BlockObject>(cf_v2(-hw, y), false);
		}

		//第二列的方块
		objs.Create<BlockObject>(cf_v2(-hw + 36.0f, -hh),true);
		
		//第三列下方的方块
		for (float y = -hh; y < hh - 6 * 36; y += 72) {
			objs.Create<BlockObject>(cf_v2(-hw + 72, y), false);
		}
		
		//最后一列的方块
		{
			objs.Create<BlockObject>(cf_v2(hw - 36.0f, 2 * 36.0f),false);
		}
		//移动方块

		//第二行的横向移动方块
		objs.Create<RightMoveBlock>(cf_v2(-hw + 6*36.0f,hh - 6 * 36.0f));
		
		//第二行的横向移动方块
		objs.Create<LeftMoveBlock>(cf_v2(hw - 6 * 36.0f, hh - 6 * 36.0f));

		//中间行的横向移动方块
		objs.Create<RightMoveBlock>(cf_v2(-hw + 6 * 36.0f, hh - 12 * 36.0f));

		//中间行的横向移动方块
		objs.Create<LeftMoveBlock>(cf_v2(hw - 6 * 36.0f, hh - 12 * 36.0f));
		
		//刺方块系统 （刺是以中心线为中心轴，为对齐需往下写两格）
		
		//行刺系统
		//上方第二行刺
		for (float x = -hw + 4.5 * 36.0f; x < hw - 2 * 36.0f; x += 36.0f)
		{
			objs.Create<DownSpike>(CF_V2(x, hh - 36.0f));
		}

		//中上行倒刺
		for (float x = -hw + 4.5 * 36.0f; x < hw - 2 * 36.0f; x += 36.0f)
		{
			objs.Create<DownSpike>(CF_V2(x, 5.5 * 36.0f));
		}

		//最后一行倒刺
		for (float x = -hw + 4.5 * 36.0f; x < hw; x += 36.0f)
		{
			objs.Create<Spike>(CF_V2(x, -hh));
		}

		//中间下面两行倒刺
		for (float x = -hw + 5.5 * 36.0f; x < hw; x += 36.0f)
		{
			objs.Create<Spike>(CF_V2(x, -2*36.0f));
		}
		
		// 列刺系统
		//	左边屏幕外的刺
		 for (float y = -hh; y < hh ; y += 36.0f)
		{
			objs.Create<Spike>(CF_V2(-hw  - 0.5 * 36.0f, y));
		}

		//左方第四列刺 
		for (float y = -hh; y < hh - 3 * 36.0f; y += 36.0f)
		{
			objs.Create<Spike>(CF_V2(-hw + 4 * 36.0f - 0.5 * 36.0f,y));
		}

		// Vertical movement configuration
   /* float move_distance = 200.0f;*/ // Total movement distance
	//float move_speed = 0.8f;      // Time for one vertical movement (seconds)
	//float wait_time = 0.5f;       // Wait time at endpoints

		//第二列的竖向移动方块 上面的
		objs.Create<VerticalMovingSpike>(CF_V2(-hw + 1.5 * 36.0f),1.0f,0.1f,220.0f);

		//第二列的竖向移动方块 下面的
		objs.Create<VerticalMovingSpike>(CF_V2(-hw + 1.5 * 36.0f, -hh + 3 * 36.0f) , 0.8f,0.2f,220.0f);

		//中间中方移动的刺 左边的
		objs.Create<VerticalMovingSpike>(CF_V2( - 3 * 36.0f,36.0f), 1.0f, 0.1f, 120.0f);

		//中间中方移动的刺 右边的
		objs.Create<VerticalMovingSpike>(CF_V2( 3 * 36.0f,36.0f), 1.0f, 0.1f, 120.0f);

		//斜着移动的左动刺
		objs.Create<DiogonalLefMoveSpike>(CF_V2(-2 * 36.0f, 7*36.0f),1.0f,0.1f);

		//斜着移动的右动刺
		objs.Create<DiogonalRigMoveSpike>(CF_V2(36.0f,7 * 36.0f),0.8f,0.1f);
		
	}

	void RoomUpdate() override {
		if (Input::IsKeyInState(CF_KEY_P, KeyState::Down)) {
			GlobalPlayer::Instance().SetEmergePosition(CF_V2(-DrawUI::half_w + 36 * 2, -DrawUI::half_h + 36 * 2));
			RoomLoader::Instance().Load("FirstRoom");
		}
	}
	void RoomUnload() override {
		OUTPUT({ "TestRoom" }, "RoomUnload called.");
	}
};

REGISTER_ROOM("EmptyRoom", EmptyRoom);
