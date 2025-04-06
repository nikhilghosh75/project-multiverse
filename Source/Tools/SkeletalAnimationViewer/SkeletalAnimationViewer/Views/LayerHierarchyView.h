#pragma once

#include "LayerImageView.h"
#include "../SkeletalAnimationTab.h"

#include "PhotoshopAPI/include/PhotoshopAPI.h"

class LayerHierarchyView : public SkeletalAnimationTab
{
public:
	LayerHierarchyView();

	void Render() override;

	std::string currentLayer = "UNUSED";
	LayerImageView* imageView = nullptr;
private:
	void RenderHierarchy(PhotoshopAPI::Layer<uint8_t>* layer);
};