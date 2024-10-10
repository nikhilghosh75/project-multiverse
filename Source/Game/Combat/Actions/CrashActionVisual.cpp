#include "CrashActionVisual.h"
#include "Combat/Characters/Character.h"
#include "Core/Time.h"
#include "MathUtils.h"
#include "glm/glm.hpp"

CrashActionVisual::CrashActionVisual(float crashTime)
	: crashTime(crashTime), currentTime(0)
{

}

CrashActionVisual::CrashActionVisual(const rapidjson::Value& v)
	: currentTime(0)
{
	crashTime = v["crash_time"].GetFloat();
}

void CrashActionVisual::Start(Action* action, Character* executor, Character* target)
{
	currentTime = 0;
	targetOffset = target->baseScreenPosition - executor->baseScreenPosition;
}

void CrashActionVisual::Update(Action* action, Character* executor, Character* target)
{
	currentTime += Time::GetDeltaTime();
	

	if (currentTime < crashTime / 2)
	{
		float positionX = Math::Lerp(0, targetOffset.x, currentTime / (crashTime / 2));
		float positionY = Math::Lerp(0, targetOffset.y, currentTime / (crashTime / 2));
		executor->screenOffset = glm::vec2(positionX, positionY);
	}
	else if (currentTime < crashTime)
	{
		float t = currentTime / (crashTime / 2) - 1.f;
		float positionX = Math::Lerp(targetOffset.x, 0, t);
		float positionY = Math::Lerp(targetOffset.y, 0, t);
		executor->screenOffset = glm::vec2(positionX, positionY);
	}
	else
	{
		executor->screenOffset = glm::vec2(0, 0);
	}
}

float CrashActionVisual::GetVisualTime() const
{
	return crashTime;
}
