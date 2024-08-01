#include "Stage.h"
#include "UI/Main Menu/StartScreenStage.h"

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
	popBackCount++;
}

void StageManager::Update()
{
	Stage* topStage = stages[stages.size() - 1];
	if (topStage)
	{
		topStage->Update();
	}
	else
	{
		// Quit the game
	}

	for (int i = 0; i < popBackCount; i++)
	{
		stages[stages.size() - 1]->OnStateRemove();
		delete stages[stages.size() - 1];
		stages.pop_back();
	}

	if (nextStage != nullptr)
	{
		nextStage->OnStateAdd();
		stages.push_back(nextStage);
		nextStage = nullptr;
	}
}
