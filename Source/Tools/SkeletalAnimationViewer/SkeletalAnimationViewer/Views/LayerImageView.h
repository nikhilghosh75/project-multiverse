#pragma once

#include "../SkeletalAnimationTab.h"

#include "PhotoshopAPI/include/PhotoshopAPI.h"

class LayerImageView : public SkeletalAnimationTab
{
public:
	LayerImageView(std::string layerName);

	void Render() override;
private:
	std::string layerName;
};