#include "LayerImageView.h"

#include "../SkeletalAnimationLoader.h"

#include "imgui.h"

LayerImageView::LayerImageView(std::string _layerName)
	: SkeletalAnimationTab(_layerName, true), layerName(_layerName)
{
}

void LayerImageView::Render()
{
	auto foundLayer = SkeletalAnimationLoader::Get()->layers.find(layerName);
	if (foundLayer == SkeletalAnimationLoader::Get()->layers.end())
	{
		return;
	}

	LayerInfo& currentLayerInfo = (*foundLayer).second;

	ImGui::Image((ImTextureID)currentLayerInfo.descriptorSet, GetTextureDimensions(currentLayerInfo.texture));
}
