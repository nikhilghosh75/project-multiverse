#include "EnemyCharacter.h"
#include "ScreenCoordinate.h"
#include "ImageRenderer.h"
#include "Core/Time.h"
#include "Combat/CombatStage.h"

const float decisionTime = 0.5f;
const float cooldownTime = 1.5f;

EnemyCharacter::EnemyCharacter()
{
	type = CharacterType::Enemy;
}

EnemyCharacter::EnemyCharacter(EnemyInfo& info, glm::vec2 _renderPosition)
{
	texture = info.texture;
	name = info.enemyName;

	type = CharacterType::Enemy;

	screenPosition = _renderPosition;

	actions.push_back(info.enemyAttack);

	health = info.startingHealth;
	maxHealth = info.startingHealth;
	defense = 0;
}

void EnemyCharacter::Render()
{
	ScreenCoordinate enemyPosition = ScreenCoordinate(glm::vec2(0, 0), screenPosition);
	Rect rect = ScreenCoordinate::CreateRect(enemyPosition, glm::vec2(80, 120), glm::vec2(0.5, 0.5));

	ImageRenderingOptions options;
	options.keepAspectRatio = true;
	ImageRenderer::Get()->AddImage(texture, rect, options);
}

void EnemyCharacter::OnTurnStart(CombatStage* stage)
{
	currentState = State::Deciding;
	timeLeftInState = decisionTime;
}

void EnemyCharacter::OnTurnUpdate(CombatStage* stage)
{
	timeLeftInState -= Time::GetDeltaTime();

	if (timeLeftInState < 0.f)
	{
		switch (currentState)
		{
		case State::Attacking:
			actions[0]->StartExecuteOnTarget(stage, this, stage->GetPlayerCharacter());
			currentState = State::Cooldown;
			timeLeftInState = cooldownTime;
			break;
		case State::Cooldown:
			if (timeLeftInState < 0)
			{
				stage->AdvanceTurn();
			}
			break;
		case State::Deciding:
			if (timeLeftInState < 0)
			{
				currentState = State::Attacking;
			}
		}
	}
}
