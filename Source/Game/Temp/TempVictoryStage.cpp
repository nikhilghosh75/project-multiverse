#include "TempVictoryStage.h"
#include "FontRenderer.h"
#include "Combat/CombatStage.h"
#include "Core/RunManager.h"
#include "Map/EncounterGenerator.h"
#include "Input.h"

TempVictoryStage::TempVictoryStage()
{

}

void TempVictoryStage::Update()
{
	FontRenderer::Get()->AddText("You have won", glm::vec2(-0.2, 0), 32);

	FontRenderer::Get()->AddText("Click anywhere to continue", glm::vec2(-0.1, 0.1));

	if (Input::GetMouseButtonDown(MouseButton::Left))
	{
		AdvanceToNextBattle();
	}
}

void TempVictoryStage::Render()
{

}

void TempVictoryStage::AdvanceToNextBattle()
{
	StageManager::PopStage();
	StageManager::PopStage();
	
	EncounterInfo info = EncounterGenerator::Generate(RunManager::GetEncounterNumber() + 1);
	StageManager::AddStage(new CombatStage(info));
}
