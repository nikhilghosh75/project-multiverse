#pragma once
#include "Core/Stage.h"
#include "Texture.h"

class TempStage : public Stage
{
public:
	TempStage();

	void Update();

	void Render();

private:
	Texture* texture;
};