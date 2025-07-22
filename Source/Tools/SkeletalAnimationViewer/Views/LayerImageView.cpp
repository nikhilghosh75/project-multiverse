#include "LayerImageView.h"

#include "../SkeletalAnimationLoader.h"

#include "imgui.h"

LayerImageView::LayerImageView(std::string _layerName)
	: SkeletalAnimationTab(_layerName, true), layerName(_layerName)
{
}

void LayerImageView::Render()
{
	auto foundLayer = SkeletalAnimationLoader::Get()->FindLayerOfName(layerName);
	if (foundLayer == nullptr)
	{
		return;
	}

	Texture* texture = SkeletalAnimationLoader::Get()->sprite.texture;
	ImGui::Image(SkeletalAnimationLoader::Get()->textureDescriptorSet, ImVec2(texture->GetTextureWidth(), texture->GetTextureHeight()), ImVec2(foundLayer->uvRect.left, foundLayer->uvRect.bottom), ImVec2(foundLayer->uvRect.right, foundLayer->uvRect.top));
}
