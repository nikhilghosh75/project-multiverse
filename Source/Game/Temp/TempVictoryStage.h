#pragma once
#include "Core/Stage.h"

class TempVictoryStage : public Stage
{
public:
	TempVictoryStage();

	void Update();

	void Render();

private:
	void AdvanceToNextBattle();
};