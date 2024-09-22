#include "TempVictoryStage.h"
#include "FontRenderer.h"
#include "Combat/CombatStage.h"
#include "Core/RunManager.h"
#include "Map/EncounterGenerator.h"
#include "Input.h"

TempVictoryStage::TempVictoryStage()
{
	// TODO: Move to a Companion Select Screen
	if (RunManager::GetEncounterNumber() == 0)
	{
		CompanionCharacter* character = EncounterGenerator::GetNewCompanion();
		RunManager::GetPlayerState()->companions.push_back(character);
	}
}

void TempVictoryStage::Update()
{
	FontRenderer::Get()->AddText("You have won", glm::vec2(-0.2, 0), 32);

	FontRenderer::Get()->AddText("Click anywhere to continue", glm::vec2(-0.1, 0.1));

	// TODO: Move to a Companion Select Screen
	if (RunManager::GetEncounterNumber() == 0)
	{
		FontRenderer::Get()->AddText("You have recieved a new companion", glm::vec2(-0.4, -0.4));
	}

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
