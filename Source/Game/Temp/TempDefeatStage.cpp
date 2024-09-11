#include "TempDefeatStage.h"
#include "FontRenderer.h"

TempDefeatStage::TempDefeatStage()
{

}

void TempDefeatStage::Update()
{
	FontRenderer::Get()->AddText("You have lost", glm::vec2(0, 0));
}

void TempDefeatStage::Render()
{
}
