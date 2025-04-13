#include "ProjectileActionVisual.h"
#include "Combat/Characters/Character.h"
#include "Core/Time.h"
#include "ImageRenderer.h"
#include "ScreenCoordinate.h"
#include "MathUtils.h"

const float PROJECTILE_ORDER = 50.f;

ProjectileActionVisual::ProjectileActionVisual(const std::string& texturePath, glm::vec2 _textureSize, float time)
	: textureSize(_textureSize), visualTime(time)
{
	projectileTexture = new Texture(texturePath);
}

ProjectileActionVisual::ProjectileActionVisual(const rapidjson::Value& v)
{
	textureSize.x = v["texture_size_x"].GetFloat();
	textureSize.y = v["texture_size_y"].GetFloat();
	projectileTexture = new Texture(v["projectile_texture"].GetString());
	visualTime = v["visual_time"].GetFloat();
}

ProjectileActionVisual::~ProjectileActionVisual()
{
	delete projectileTexture;
}

void ProjectileActionVisual::Start(Action* action, Character* executor, Character* target)
{
	startPosition = executor->baseScreenPosition;
	endPosition = target->baseScreenPosition;

	currentTime = 0;
}

void ProjectileActionVisual::Update(Action* action, Character* executor, Character* target)
{
	float positionX = Math::Lerp(startPosition.x, endPosition.x, currentTime / visualTime);
	float positionY = Math::Lerp(startPosition.y, endPosition.y, currentTime / visualTime);

	ScreenCoordinate screenPosition = ScreenCoordinate(glm::vec2(0, 0), glm::vec2(positionX, positionY));
	Rect rect = ScreenCoordinate::CreateRect(screenPosition, textureSize, glm::vec2(0.5, 0.5));
	
	ImageRenderingOptions options;
	options.keepAspectRatio = true;

	ImageRenderer::Get()->AddImage(projectileTexture, rect, PROJECTILE_ORDER, options);

	currentTime += Time::GetDeltaTime();
}

float ProjectileActionVisual::GetVisualTime() const
{
	return visualTime;
}


