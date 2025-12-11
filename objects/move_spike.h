#pragma once
#include "base_object.h"
#include "act_seq.h"

class MoveSpike : public BaseObject {
public:
	MoveSpike() noexcept : BaseObject() {}
	~MoveSpike() noexcept {}
	void Start() override;
	void Update() override;
private:
	ActSeq m_act_seq;
};