#pragma once
#include "Character.h"
#include "Combat/EncounterInfo.h"
#include "glm/vec2.hpp"

class EnemyCharacter : public Character
{
public:
	EnemyCharacter();

	EnemyCharacter(EnemyInfo& info, glm::vec2 _renderPosition);

	void Render();

private:
	glm::vec2 renderPosition;
};