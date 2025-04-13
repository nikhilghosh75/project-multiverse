#include "StartScreenStage.h"
#include "Combat/CombatStage.h"
#include "Core/RunManager.h"
#include "Temp/TempStage.h"
#include "Input.h"
#include "FontRenderer.h"
#include "Combat/EncounterInfo.h"
#include <iostream>

const float START_SCREEN_ORDER = 100.f;

void StartScreenStage::Update()
{
	// TODO: Support Localization
	const char* text = "CLICK ANYWHERE TO START";

	FontRenderer::Get()->AddText(text, glm::vec2(-0.3f, 0.5f), START_SCREEN_ORDER, 72);

	if (Input::GetMouseButtonDown(MouseButton::Left))
	{
		// TODO: Change to Main Menu when we have one
		StageManager::PopStage();
		RunManager::StartRun();
		
		/*
		TempStage* tempStage = new TempStage();
		StageManager::AddStage(tempStage);
		*/
	}
}

void StartScreenStage::Render()
{
}
