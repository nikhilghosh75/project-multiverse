#include "TempVictoryStage.h"
#include "FontRenderer.h"

TempVictoryStage::TempVictoryStage()
{

}

void TempVictoryStage::Update()
{
	FontRenderer::Get()->AddText("You have won", glm::vec2(0, 0));
}

void TempVictoryStage::Render()
{
}
