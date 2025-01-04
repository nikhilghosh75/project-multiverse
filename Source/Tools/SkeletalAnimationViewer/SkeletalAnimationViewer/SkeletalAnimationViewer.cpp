#include "PhotoshopAPI/include/PhotoshopAPI.h"

#include "Device.h"
#include "ImGuiDevice.h"
#include "Texture.h"
#include "Window.h"

#include "ImageRenderer.h"
#include "RenderManager.h"

#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"

#include <unordered_map>

ImVec2 GetTextureDimensions(Texture* texture)
{
	return ImVec2(texture->GetTextureWidth(), texture->GetTextureHeight());
}

uint8_t* CombineChannelData(uint8_t* redChannel, uint8_t* greenChannel, uint8_t* blueChannel, uint8_t* alphaChannel, uint32_t width, uint32_t height)
{
	uint8_t* data = (uint8_t*)malloc(width * height * 4);

	for (uint32_t i = 0; i < width * height; i++)
	{
		data[i * 4 + 0] = redChannel[i];
		data[i * 4 + 1] = greenChannel[i];
		data[i * 4 + 2] = blueChannel[i];
		data[i * 4 + 3] = alphaChannel[i];
	}

	return data;
}

class LayerInfo
{
public:
	Texture* texture;
	VkDescriptorSet descriptorSet;
	float centerX;
	float centerY;
	float width;
	float height;
};

std::unordered_map<std::string, LayerInfo> layers;
std::string currentLayer = "UNUSED";

VkImage offscreenImage;
VkImageView offscreenImageView;
VkFramebuffer offscreenFramebuffer;
VkDeviceMemory offscreenMemory;
VkSampler offscreenSampler;
VkRenderPass offscreenRenderPass;
ImTextureID offscreenImguiTexture;

void RegisterLayer(PhotoshopAPI::Layer<uint8_t>* layer)
{
	PhotoshopAPI::ImageLayer<uint8_t>* imageLayer = dynamic_cast<PhotoshopAPI::ImageLayer<uint8_t>*>(layer);
	PhotoshopAPI::GroupLayer<uint8_t>* groupLayer = dynamic_cast<PhotoshopAPI::GroupLayer<uint8_t>*>(layer);

	if (imageLayer != nullptr)
	{
		// Split Image by Channel
		std::vector<uint8_t> redChannel = imageLayer->getChannel(PhotoshopAPI::Enum::ChannelID::Red, true);
		std::vector<uint8_t> greenChannel = imageLayer->getChannel(PhotoshopAPI::Enum::ChannelID::Green, true);
		std::vector<uint8_t> blueChannel = imageLayer->getChannel(PhotoshopAPI::Enum::ChannelID::Blue, true);
		std::vector<uint8_t> alphaChannel = imageLayer->getChannel(PhotoshopAPI::Enum::ChannelID::Alpha, true);

		uint8_t* data = CombineChannelData(redChannel.data(), greenChannel.data(), blueChannel.data(), alphaChannel.data(), layer->m_Width, layer->m_Height);
		Texture* texture = new Texture(data, layer->m_Width, layer->m_Height);
		texture->TransitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		LayerInfo layerInfo;
		layerInfo.texture = texture;
		layerInfo.descriptorSet = ImGui_ImplVulkan_AddTexture(texture->GetSampler(), texture->GetImageView(), texture->GetImageLayout());
		layerInfo.centerX = imageLayer->m_CenterX;
		layerInfo.centerY = imageLayer->m_CenterY;
		layerInfo.width = imageLayer->m_Width;
		layerInfo.height = imageLayer->m_Height;

		layers.insert({ layer->m_LayerName, layerInfo });
	}
	else if (groupLayer != nullptr)
	{
		for (int i = 0; i < groupLayer->m_Layers.size(); i++)
		{
			RegisterLayer(groupLayer->m_Layers[i].get());
		}
	}
}

void SetupLayerInfos(PhotoshopAPI::LayeredFile<uint8_t>* layeredFile)
{
	for (int i = 0; i < layeredFile->m_Layers.size(); i++)
	{
		RegisterLayer(layeredFile->m_Layers[i].get());
	}
}

void RenderLayerHierarchy(PhotoshopAPI::Layer<uint8_t>* layer)
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
		if (ImGui::IsItemClicked())
		{
			currentLayer = layer->m_LayerName;
		}

		if (nodeOpen)
		{
			for (int i = 0; i < groupLayer->m_Layers.size(); i++)
			{
				RenderLayerHierarchy(groupLayer->m_Layers[i].get());
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
			currentLayer = layer->m_LayerName;
		}

		ImGui::TreePop();
	}
}

void RenderLayersHierarchyWindow(PhotoshopAPI::LayeredFile<uint8_t>* layeredFile)
{
	ImGui::Begin("Layers");
	for (int i = 0; i < layeredFile->m_Layers.size(); i++)
	{
		RenderLayerHierarchy(layeredFile->m_Layers[i].get());
	}
	ImGui::End();
}

void RenderLayerImage()
{
	if (currentLayer == "UNUSED")
	{
		return;
	}

	auto foundLayer = layers.find(currentLayer);
	if (foundLayer == layers.end())
	{
		return;
	}

	LayerInfo& currentLayerInfo = (*foundLayer).second;

	ImGui::Begin("Layer Image");

	ImGui::Image((ImTextureID)currentLayerInfo.descriptorSet, GetTextureDimensions(currentLayerInfo.texture));

	ImGui::End();
}

uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(Device::Get()->GetPhysicalDevice(), &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	return 0;
}

void SetupMainView()
{
	unsigned int width = 5000;
	unsigned int height = 5000;

	// Create image
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = VK_FORMAT_B8G8R8A8_SRGB; 
	imageInfo.extent = { width, height, 1 };
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; 

	vkCreateImage(Device::Get()->GetVulkanDevice(), &imageInfo, nullptr, &offscreenImage);

	// Allocate and bind memory
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(Device::Get()->GetVulkanDevice(), offscreenImage, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vkAllocateMemory(Device::Get()->GetVulkanDevice(), &allocInfo, nullptr, &offscreenMemory);
	vkBindImageMemory(Device::Get()->GetVulkanDevice(), offscreenImage, offscreenMemory, 0);

	// Create image view
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = offscreenImage;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	vkCreateImageView(Device::Get()->GetVulkanDevice(), &viewInfo, nullptr, &offscreenImageView);

	// Create Render Pass
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = Device::Get()->GetSwapChainFormat();
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(Device::Get()->GetVulkanDevice(), &renderPassInfo, nullptr, &offscreenRenderPass) != VK_SUCCESS)
	{
		// TODO: Output the following error code
		// "Vulkan, failed to create render pass"
		exit(0);
	}

	// Create framebuffer
	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = offscreenRenderPass;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.pAttachments = &offscreenImageView;
	framebufferInfo.width = width;
	framebufferInfo.height = height;
	framebufferInfo.layers = 1;

	vkCreateFramebuffer(Device::Get()->GetVulkanDevice(), &framebufferInfo, nullptr, &offscreenFramebuffer);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR; // Use linear filtering for magnification
	samplerInfo.minFilter = VK_FILTER_LINEAR; // Use linear filtering for minification
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // Clamp texture coordinates
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.anisotropyEnable = VK_TRUE; // Enable anisotropic filtering if supported
	samplerInfo.maxAnisotropy = 16.0f; // Adjust as needed; check device limits
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // Border color for out-of-range coordinates
	samplerInfo.unnormalizedCoordinates = VK_FALSE; // Use normalized texture coordinates (0.0 to 1.0)
	samplerInfo.compareEnable = VK_FALSE; // Disable comparison for shadow maps
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // Use linear interpolation for mipmaps
	samplerInfo.mipLodBias = 0.0f; // No bias
	samplerInfo.minLod = 0.0f; // Minimum level of detail
	samplerInfo.maxLod = 0.0f; // Maximum level of detail (set to 0 for no mipmapping)

	vkCreateSampler(Device::Get()->GetVulkanDevice(), &samplerInfo, nullptr, &offscreenSampler);

	Device::Get()->TransitionImageLayout(offscreenImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	offscreenImguiTexture = ImGui_ImplVulkan_AddTexture(offscreenSampler, offscreenImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void RenderMainView()
{
	Device::Get()->TransitionImageLayout(offscreenImage, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	int halfWidth = 2500;
	int halfHeight = 2500;
	unsigned int width = 5000;
	unsigned int height = 5000;

	RenderPass renderPass;
	renderPass.extents = { width, height };
	renderPass.framebuffer = offscreenFramebuffer;
	renderPass.pass = offscreenRenderPass;

	Device::Get()->SetOverrideRenderPass(renderPass);

	// Render all of the images in the file one at a time
	for (auto& layer : layers)
	{
		Rect rect(halfHeight + layer.second.centerY + layer.second.height, halfHeight + layer.second.centerY,
			halfWidth + layer.second.centerX, halfWidth + layer.second.centerX + layer.second.width);

		rect.top /= height;
		rect.bottom /= height;
		rect.left /= width;
		rect.right /= width;

		ImageRenderer::Get()->AddImage(layer.second.texture, rect);
	}

	Device::Get()->ClearOverrideRenderPass();

	Device::Get()->TransitionImageLayout(offscreenImage, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	ImGui::Begin("Main View");

	ImGui::Image(offscreenImguiTexture, ImVec2(400, 400));

	ImGui::End();
}

int main()
{
	PhotoshopAPI::File inputFile = PhotoshopAPI::File("C:/Users/debgh/OneDrive/Documents/Unity Projects/Dreamwillow/Assets/Art/Characters/Player/PlayerHorizSprite.psb");

	std::unique_ptr<PhotoshopAPI::PhotoshopFile> file = std::make_unique<PhotoshopAPI::PhotoshopFile>();
	PhotoshopAPI::ProgressCallback callback;
	file->read(inputFile, callback);

	PhotoshopAPI::LayeredFile<uint8_t>* layeredFile = new PhotoshopAPI::LayeredFile<uint8_t>(file);

	Window window;

	ImGuiDevice device;
	device.Setup(Window::GetWindowHandle(), Device::Get());

	RenderManager renderingManager;
	renderingManager.Setup();

	SetupLayerInfos(layeredFile);
	SetupMainView();

	while (window.windowRunning)
	{
		window.Process();
		Device::Get()->StartFrame();
		device.StartFrame();
		renderingManager.StartFrame();

		RenderMainView();
		RenderLayersHierarchyWindow(layeredFile);
		RenderLayerImage();

		renderingManager.EndFrame();
		device.EndFrame();
		Device::Get()->EndFrame();
	}
}