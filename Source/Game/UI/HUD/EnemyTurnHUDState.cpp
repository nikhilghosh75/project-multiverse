#include "EnemyTurnHUDState.h"
#include "Combat/CombatStage.h"
#include "MainCombatHUDState.h"
#include "FontRenderer.h"

const float COMBAT_HUD_ORDER_START = 100.f;

EnemyTurnHUDState::EnemyTurnHUDState()
{

}

void EnemyTurnHUDState::Render(CombatStage* stage)
{
	Character* character = stage->GetCurrentTurnCharacter();

	if (character)
	{
		FontRenderer::Get()->AddText(character->name + " 's Turn", glm::vec2(-0.2f, 0.6f), COMBAT_HUD_ORDER_START + 0.1f, 60);

		if (character->type != CharacterType::Enemy)
		{
			CombatHUD::SetCurrentState(new MainCombatHUDState());
		}
	}
}
