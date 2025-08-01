#include "TempBoneView.h"

#include "../SkeletalAnimationLoader.h"

TempBoneView::TempBoneView()
	: SkeletalAnimationTab("Temp Bone View", false)
{
}

void TempBoneView::Render()
{
	float& angle = SkeletalAnimationLoader::Get()->sprite.skeleton.bones[1].localRotation;
	ImGui::SliderFloat("Angle", &angle, 0, 6);
}
