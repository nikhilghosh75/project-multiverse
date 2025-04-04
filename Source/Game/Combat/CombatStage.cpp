#include "CombatStage.h"

#include "Core/RunManager.h"
#include "UI/HUD/CombatHUD.h"
#include "Temp/TempDefeatStage.h"
#include "Temp/TempVictoryStage.h"

#include "tracy/Tracy.hpp"

// TODO: Maybe change this to somewhere else
std::vector<std::vector<glm::vec2>> enemyPositions =
{
	{ },
	{ glm::vec2(0.8, 0.5) },
	{ glm::vec2(0.75, 0.3), glm::vec2(0.85, 0.6) }
};

CombatStage::CombatStage(EncounterInfo& info)
{
	blocksInput = true;
	blocksRendering = true;

	playerCharacter = new PlayerCharacter(RunManager::GetPlayerState());
	companions = RunManager::GetPlayerState()->companions;

	for (int i = 0; i < info.enemies.size(); i++)
	{
		glm::vec2 position = enemyPositions[info.enemies.size()][i];
		EnemyCharacter* enemyCharacter = new EnemyCharacter(info.enemies[i].character);
		enemyCharacter->baseScreenPosition = position;
		enemies.push_back(enemyCharacter);
	}

	currentTurnCharacter = playerCharacter;
	currentTurnCharacter->OnTurnStart(this);

	CombatHUD::Initialize();
	RunManager::OnBattleStart(this);
}

CombatStage::~CombatStage()
{
	CombatHUD::Cleanup();

	delete playerCharacter;
	
	for (int i = 0; i < enemies.size(); i++)
	{
		delete enemies[i];
	}
}

void CombatStage::Update()
{
	ZoneScoped;
	currentTurnCharacter->OnTurnUpdate(this);

	playerCharacter->Render();

	for (int i = 0; i < companions.size(); i++)
	{
		companions[i]->Render();
	}

	for (int i = 0; i < enemies.size(); i++)
	{
		enemies[i]->Render();
	}

	CombatHUD::Render(this);

	ProcessBattleOver();
}

void CombatStage::Render()
{

}

void CombatStage::AdvanceTurn()
{
	do
	{
		if (currentTurnCharacter == playerCharacter)
		{
			if (companions.size() != 0)
			{
				currentTurnCharacter = companions[0];
			}
			else
			{
				currentTurnCharacter = enemies[0];
			}
		}
		else if (currentTurnCharacter->type == CharacterType::Companion)
		{
			for (int i = 0; i < companions.size(); i++)
			{
				if (currentTurnCharacter == companions[i])
				{
					if (i == companions.size() - 1)
					{
						currentTurnCharacter = enemies[0];
					}
					else
					{
						currentTurnCharacter = companions[i + 1];
					}
					break;
				}
			}
		}
		else
		{
			for (int i = 0; i < enemies.size(); i++)
			{
				if (currentTurnCharacter == enemies[i])
				{
					if (i == enemies.size() - 1)
					{
						currentTurnCharacter = playerCharacter;
					}
					else
					{
						currentTurnCharacter = enemies[i + 1];
					}
					break;
				}
			}
		}
	} while (currentTurnCharacter->IsDead());

	CombatHUD::GetCurrentStage()->OnTurnAdvanced(this);
	currentTurnCharacter->OnTurnStart(this);
}

std::vector<glm::vec2>& CombatStage::GetEnemyPositions()
{
	return enemyPositions[enemies.size()];
}

std::vector<EnemyCharacter*>& CombatStage::GetEnemyCharacters()
{
	return enemies;
}

std::vector<CompanionCharacter*>& CombatStage::GetCompanionCharacters()
{
	return companions;
}

void CombatStage::ProcessBattleOver()
{
	if (playerCharacter->IsDead())
	{
		OnBattleLost();
	}
	else
	{
		bool allEnemiesDead = true;
		for (int i = 0; i < enemies.size(); i++)
		{
			if (!enemies[i]->IsDead())
			{
				allEnemiesDead = false;
				break;
			}
		}

		if (allEnemiesDead)
		{
			OnBattleWon();
		}
	}
}

void CombatStage::OnBattleLost()
{
	// Go to Death Screen
	StageManager::AddStage(new TempDefeatStage());
}

void CombatStage::OnBattleWon()
{
	RunManager::OnBattleOver(this);

	StageManager::AddStage(new TempVictoryStage());
}

