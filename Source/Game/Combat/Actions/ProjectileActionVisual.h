#pragma once
#include "Action.h"
#include <glm/glm.hpp>

class ProjectileActionVisual : public ActionVisual
{
public:
	ProjectileActionVisual(Texture* texture, glm::vec2 pixelSize);
};