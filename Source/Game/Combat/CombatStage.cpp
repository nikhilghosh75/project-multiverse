#include "CombatStage.h"
#include "Core/RunManager.h"
#include "UI/HUD/CombatHUD.h"

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

	CombatHUD::Initialize();
}

void CombatStage::Update()
{
	playerCharacter->Render();

	for (int i = 0; i < enemies.size(); i++)
	{
		enemies[i]->Render();
	}

	CombatHUD::Render(this);
}

void CombatStage::Render()
{

}
