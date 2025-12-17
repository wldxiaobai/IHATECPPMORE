#pragma once
#include<cute.h>
#include<algorithm>
#include<chrono>
#include<string>

namespace DrawUI {
	extern float half_w;
	extern float half_h;
	void GameOverDraw();
	void EscDraw(std::chrono::steady_clock::time_point esc_down_start, std::chrono::seconds esc_hold_threshold);
	void TestDraw();
}