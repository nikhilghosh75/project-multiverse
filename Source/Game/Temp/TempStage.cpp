#include "TempStage.h"
#include "ImageRenderer.h"
#include "FontRenderer.h"
#include "Rect.h"

TempStage::TempStage()
{
	texture = new Texture("Data/Sprites/Player/Wolvey.png");
}

void TempStage::Update()
{
	ImageRenderer::Get()->AddImage(texture, Rect(0.1f, 0.2f, 0.1f, 0.2f), 0);
	ImageRenderer::Get()->AddImage(texture, Rect(0.3f, 0.4f, 0.1f, 0.2f), 0);
	ImageRenderer::Get()->AddImage(texture, Rect(0.1f, 0.2f, 0.3f, 0.4f), 0);
	ImageRenderer::Get()->AddImage(texture, Rect(0.3f, 0.4f, 0.3f, 0.4f), 0);
	ImageRenderer::Get()->AddImage(texture, Rect(0.5f, 0.6f, 0.1f, 0.2f), 0);
	ImageRenderer::Get()->AddImage(texture, Rect(0.5f, 0.6f, 0.1f, 0.2f), 0);
	ImageRenderer::Get()->AddImage(texture, Rect(0.7f, 0.8f, 0.3f, 0.4f), 0);
	ImageRenderer::Get()->AddImage(texture, Rect(0.7f, 0.8f, 0.3f, 0.4f), 0);
	ImageRenderer::Get()->AddImage(texture, Rect(0.1f, 0.2f, 0.5f, 0.6f), 0);
	ImageRenderer::Get()->AddImage(texture, Rect(0.3f, 0.4f, 0.5f, 0.6f), 0);
	ImageRenderer::Get()->AddImage(texture, Rect(0.1f, 0.2f, 0.7f, 0.8f), 0);
	ImageRenderer::Get()->AddImage(texture, Rect(0.3f, 0.4f, 0.7f, 0.8f), 0);
	ImageRenderer::Get()->AddImage(texture, Rect(0.5f, 0.6f, 0.5f, 0.6f), 0);
	ImageRenderer::Get()->AddImage(texture, Rect(0.7f, 0.8f, 0.5f, 0.6f), 0);
	ImageRenderer::Get()->AddImage(texture, Rect(0.5f, 0.6f, 0.7f, 0.8f), 0);
	ImageRenderer::Get()->AddImage(texture, Rect(0.7f, 0.8f, 0.7f, 0.8f), 0);
}

void TempStage::Render()
{
}
