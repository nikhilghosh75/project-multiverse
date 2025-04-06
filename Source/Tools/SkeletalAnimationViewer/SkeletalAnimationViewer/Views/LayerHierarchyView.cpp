#include "LayerHierarchyView.h"

#include "../SkeletalAnimationLoader.h"

#include "imgui.h"

LayerHierarchyView::LayerHierarchyView()
	: SkeletalAnimationTab("Layer Hierarchy", false)
{
}

void LayerHierarchyView::Render()
{
	PhotoshopAPI::LayeredFile<uint8_t>* layeredFile = SkeletalAnimationLoader::Get()->layeredFile;
	for (int i = 0; i < layeredFile->m_Layers.size(); i++)
	{
		RenderHierarchy(layeredFile->m_Layers[i].get());
	}
}

void LayerHierarchyView::RenderHierarchy(PhotoshopAPI::Layer<uint8_t>* layer)
{
	PhotoshopAPI::GroupLayer<uint8_t>* groupLayer = dynamic_cast<PhotoshopAPI::GroupLayer<uint8_t>*>(layer);

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;

	if (layer->m_LayerName == currentLayer)
	{
		flags |= ImGuiTreeNodeFlags_Selected;
	}

	if (groupLayer != nullptr)
	{
		bool nodeOpen = ImGui::TreeNodeEx(layer->m_LayerName.c_str(), flags);
		if (ImGui::IsItemClicked() && currentLayer != layer->m_LayerName)
		{
			if (imageView != nullptr)
			{
				SkeletalAnimationTabSystem::Get()->RemoveTab(imageView);
				delete imageView;
			}

			currentLayer = layer->m_LayerName;
			imageView = new LayerImageView(currentLayer);
			imageView->onTabClose = [this]() { this->imageView = nullptr; };
			SkeletalAnimationTabSystem::Get()->AddTab(imageView);
		}

		if (nodeOpen)
		{
			for (int i = 0; i < groupLayer->m_Layers.size(); i++)
			{
				RenderHierarchy(groupLayer->m_Layers[i].get());
			}
			ImGui::TreePop();
		}
	}
	else
	{
		flags |= ImGuiTreeNodeFlags_Leaf;
		ImGui::TreeNodeEx(layer->m_LayerName.c_str(), flags);

		if (ImGui::IsItemClicked())
		{
			if (imageView != nullptr)
			{
				SkeletalAnimationTabSystem::Get()->RemoveTab(imageView);
				delete imageView;
			}

			currentLayer = layer->m_LayerName;
			imageView = new LayerImageView(currentLayer);
			imageView->onTabClose = [this]() { this->imageView = nullptr; this->currentLayer = "UNUSED"; };
			SkeletalAnimationTabSystem::Get()->AddTab(imageView);
		}

		ImGui::TreePop();
	}
}
