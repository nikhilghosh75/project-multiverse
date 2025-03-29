#include "Stage.h"

#include "UI/Main Menu/StartScreenStage.h"

#include "tracy/Tracy.hpp"

void StageManager::Initialize()
{
	stages.reserve(MAX_STAGES);
	stages.push_back(new StartScreenStage());
}

void StageManager::AddStage(Stage* stage)
{
	nextStage = stage;
}

void StageManager::PopStage()
{
	stagesDeletedThisFrame.push_back(stages[stages.size() - 1]);
	stages.pop_back();
}

void StageManager::Update()
{
	ZoneScoped;
	startOfFrameCount = stages.size() - 1;

	Stage* topStage = stages[stages.size() - 1];
	if (topStage)
	{
		topStage->Update();
	}
	else
	{
		// Quit the game
	}

	for (int i = 0; i < stagesDeletedThisFrame.size(); i++)
	{
		stagesDeletedThisFrame[i]->OnStateRemove();
		delete stagesDeletedThisFrame[i];
	}

	stagesDeletedThisFrame.clear();

	if (nextStage != nullptr)
	{
		nextStage->OnStateAdd();
		stages.push_back(nextStage);
		nextStage = nullptr;
	}
}
