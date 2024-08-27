#include "StartScreenStage.h"
#include "Combat/CombatStage.h"
#include "Core/RunManager.h"
#include "Temp/TempStage.h"
#include "Input.h"
#include "FontRenderer.h"
#include "Combat/EncounterInfo.h"
#include <iostream>

void StartScreenStage::Update()
{
	// TODO: Support Localization
	const char* text = "CLICK ANYWHERE TO START";

	FontRenderer::Get()->AddText(text, glm::vec2(-0.3f, 0.5f), 24);

	if (Input::GetMouseButtonDown(MouseButton::Left))
	{
		// TODO: Change to Main Menu when we have one
		RunManager::StartRun();
		StageManager::PopStage();
		
		// TODO: Move this to the map generator
		EncounterInfo info;

		EnemyInfo debufferInfo;
		debufferInfo.enemyName = "Debuffer";
		debufferInfo.texture = new Texture("Data/Sprites/Enemies/Debuffer.png");
		debufferInfo.startingHealth = 40;
		debufferInfo.enemyAttack = new MeleeAttack("Head Butt", 7, 2, 2);
		info.enemies.push_back(debufferInfo);
		
		CombatStage* combatStage = new CombatStage(info);
		StageManager::AddStage(combatStage);
		
		
		/*
		TempStage* tempStage = new TempStage();
		StageManager::AddStage(tempStage);
		*/
	}
}

void StartScreenStage::Render()
{
}
