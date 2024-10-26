#pragma once
#include "Action.h"
#include <glm/glm.hpp>

class ProjectileActionVisual : public ActionVisual
{
public:
	ProjectileActionVisual(const std::string& texture, glm::vec2 textureSize, float time);
	ProjectileActionVisual(const rapidjson::Value& v);

	~ProjectileActionVisual();

	void Start(Action* action, Character* executor, Character* target);

	void Update(Action* action, Character* executor, Character* target);

	float GetVisualTime() const;

private:
	float visualTime;
	
	Texture* projectileTexture;
	glm::vec2 textureSize;

	float currentTime;

	glm::vec2 startPosition;
	glm::vec2 endPosition;
};