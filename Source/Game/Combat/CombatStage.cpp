#include "CombatStage.h"
#include "Core/RunManager.h"
#include "UI/HUD/CombatHUD.h"
#include "Temp/TempDefeatStage.h"
#include "Temp/TempVictoryStage.h"

// TODO: Maybe change this to somewhere else
std::vector<std::vector<glm::vec2>> enemyPositions =
{
	{ glm::vec2(0.8, 0.5) },
	{ glm::vec2(0.75, 0.3), glm::vec2(0.85, 0.6) }
};

CombatStage::CombatStage(EncounterInfo& info)
{
	blocksInput = true;
	blocksRendering = true;

	playerCharacter = new PlayerCharacter(RunManager::GetPlayerState());

	for (int i = 0; i < info.enemies.size(); i++)
	{
		glm::vec2 position = enemyPositions[info.enemies.size()][i];
		EnemyCharacter* enemyCharacter = new EnemyCharacter(info.enemies[i], position);
		enemies.push_back(enemyCharacter);
	}

	currentTurnCharacter = playerCharacter;

	CombatHUD::Initialize();
}

void CombatStage::Update()
{
	currentTurnCharacter->OnTurnUpdate(this);

	playerCharacter->Render();

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
			currentTurnCharacter = enemies[0];
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

void CombatStage::ProcessBattleOver()
{
	if (playerCharacter->IsDead())
	{
		// Go to Death Screen
		StageManager::AddStage(new TempDefeatStage());
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
			// Go to Victory Screen
			StageManager::AddStage(new TempVictoryStage());
		}
	}
}
