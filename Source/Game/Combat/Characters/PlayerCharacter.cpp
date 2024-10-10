#include "PlayerCharacter.h"
#include "ScreenCoordinate.h"
#include "Rect.h"
#include "ImageRenderer.h"
#include "Combat/Actions/GuardAction.h"
#include "Combat/Actions/GunAttack.h"
#include "Combat/Actions/MeleeAttack.h"
#include "Combat/Actions/PassAction.h"
#include "Combat/CombatStage.h"
#include "UI/HUD/CombatHUD.h"

const glm::vec2 playerScreenPosition = glm::vec2(0.2f, 0.4f);

PlayerCharacter::PlayerCharacter()
	: playerState(nullptr)
{
	texture = new Texture("Data/Sprites/Player/Wolvey.png");
	type = CharacterType::Player;

	baseScreenPosition = playerScreenPosition;
	screenOffset = glm::vec2(0, 0);

	AddTempAbilities();
}

PlayerCharacter::PlayerCharacter(PlayerState* _playerState)
	: playerState(_playerState)
{
	texture = new Texture("Data/Sprites/Player/Wolvey.png");
	type = CharacterType::Player;

	baseScreenPosition = playerScreenPosition;
	screenOffset = glm::vec2(0, 0);

	health = playerState->health;
	maxHealth = playerState->maxHealth;
	defense = 0;
	actionPointsPerTurn = playerState->apPerTurn;

	AddTempAbilities();
}

void PlayerCharacter::Render()
{
	// Render Player Sprite
	ScreenCoordinate playerPosition = ScreenCoordinate(glm::vec2(0, 0), playerScreenPosition + screenOffset);
	Rect rect = ScreenCoordinate::CreateRect(playerPosition, glm::vec2(80, 120), glm::vec2(0.5, 0.5));

	ImageRenderingOptions options;
	options.keepAspectRatio = true;
	ImageRenderer::Get()->AddImage(texture, rect, options);
}

void PlayerCharacter::EndAction(CombatStage* stage)
{
	CombatHUD::GetCurrentStage()->OnActionEnded(stage, this, currentAction);

	Character::EndAction(stage);
}

void PlayerCharacter::AddTempAbilities()
{
	actions.push_back(new MeleeAttack("Teleporting Sword (IO)", 9, 3, 3, "Data/Sprites/UI/Icons/Icon Teleporting Sword.png"));
	actions.push_back(new MeleeAttack("Standard Punch", 5, 1, 2, "Data/Sprites/UI/Icons/Icon Standard Punch.png"));
	actions.push_back(new GunAttack("Blubberbus (Curse of the Corsair)", 4, 2, 1, 1, 2, "Data/Sprites/UI/Icons/Icon Blubberbus.png"));
	actions.push_back(new GuardAction("Weak Guard", 2, 1, "Data/Sprites/UI/Icons/Icon Shield Small.png"));
	actions.push_back(new GuardAction("Strong Guard", 4, 2, "Data/Sprites/UI/Icons/Icon Shield Big.png"));
	actions.push_back(new PassAction());
}
