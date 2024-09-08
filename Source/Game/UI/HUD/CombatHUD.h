#pragma once
#include "glm/glm.hpp"
#include <vector>

class CombatStage;
class Action;
class Character;
class Texture;
class VectorPainter;

class CombatHUDState
{
public:
	CombatHUDState();

	void StartExecuteAction(Action* action, CombatStage* combatStage);

	virtual void Render(CombatStage* stage) = 0;

	virtual void OnTurnAdvanced(CombatStage* stage) { }
	virtual void OnTargetSelected(CombatStage* stage, Character* character) { }

protected:
	Action* action;

private:
	bool isSelectingTarget;

	friend class CombatHUD;
};

class FloatingDamageNumber
{
public:
	FloatingDamageNumber(int damage, glm::vec2 startingPosition, float duration);

	int damage;
	glm::vec2 startingPosition;
	float startTime;
	float duration;
};

// TODO: Memory cleanup of CombatHUD
class CombatHUD
{
public:
	static void Initialize();

	static void Render(CombatStage* stage);

	static void SetCurrentState(CombatHUDState* newState);

	static CombatHUDState* GetCurrentStage();

	static void AddDamageNumber(FloatingDamageNumber damageNumber);

private:
	// Target Selection
	static void RenderTargetSelection(CombatStage* stage);
	static void RenderCrosshair(glm::vec2 position);
	static inline Texture* crosshairTexture;

	// Damage Numbers
	static void RenderDamageNumbers(CombatStage* stage);
	static inline std::vector<FloatingDamageNumber> damageNumbers;

	// Current Turn Triangle
	static void RenderCurrentTurnTriangle(CombatStage* stage);
	static inline Texture* triangleTexture;

	// Character HUDs
	static void RenderCharacterHUDs(CombatStage* stage);
	static void RenderCharacterHUD(CombatStage* stage, Character* character, VectorPainter* painter);

	static inline CombatHUDState* currentState;
	static inline CombatHUDState* previousState;
};