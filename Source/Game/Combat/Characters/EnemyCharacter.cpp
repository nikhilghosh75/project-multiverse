#include "EnemyCharacter.h"
#include "ScreenCoordinate.h"
#include "ImageRenderer.h"

EnemyCharacter::EnemyCharacter()
{
}

EnemyCharacter::EnemyCharacter(EnemyInfo& info, glm::vec2 _renderPosition)
{
	texture = info.texture;
	name = info.enemyName;

	renderPosition = _renderPosition;
}

void EnemyCharacter::Render()
{
	ScreenCoordinate playerPosition = ScreenCoordinate(glm::vec2(0, 0), renderPosition);
	Rect rect = ScreenCoordinate::CreateRect(playerPosition, glm::vec2(80, 120), glm::vec2(0.5, 0.5));

	ImageRenderingOptions options;
	options.keepAspectRatio = true;
	ImageRenderer::Get()->AddImage(texture, rect, options);
}
