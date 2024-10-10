#pragma once
#include "Action.h"
#include "glm/glm.hpp"

class CrashActionVisual : public ActionVisual
{
public:
	CrashActionVisual(float crashTime);
	CrashActionVisual(const rapidjson::Value& v);

	void Start(Action* action, Character* executor, Character* target);

	void Update(Action* action, Character* executor, Character* target);

	float GetVisualTime() const;

private:
	float crashTime;
	float currentTime;

	glm::vec2 targetOffset;
};