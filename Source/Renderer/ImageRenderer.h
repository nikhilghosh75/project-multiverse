#pragma once
#include "glm/glm.hpp"
#include "Rect.h"
#include "RenderPipeline.h"
#include "RenderRequest.h"
#include "Texture.h"
#include <array>

class ImageRenderingOptions
{
public:
	ImageRenderingOptions();

	bool keepAspectRatio;
};

class ImageRenderingResult
{
public:
	Rect finalRect;
	bool rendered;
};

class ImageRenderRequest : public RenderRequest
{
public:
	bool CanBeCombined(const RenderRequest* other) const override;

	void CombineWith(RenderRequest* other) override;

	void Render() override;

	static ImageRenderRequest* CreateRequest();

	Texture* texture;
	std::vector<Rect> rects;

	static const int MAX_IMAGE_REQUESTS = 50;

private:
	static std::array<ImageRenderRequest, MAX_IMAGE_REQUESTS> requests;
	static inline bool requestsArrayInitialized = false;
	static inline int lastIndex = 0;
};

class ImageRenderer
{
	static inline ImageRenderingOptions defaultOptions;

public:
	ImageRenderer();
	~ImageRenderer();

	static ImageRenderer* Get();

	ImageRenderingResult AddImage(Texture* texture, Rect rect, ImageRenderingOptions options = defaultOptions);

	void Render();

private:
	class Vertex
	{
	public:
		glm::vec2 position;
		glm::vec2 uv;

		Vertex(glm::vec2 _position, glm::vec2 _uv);

		static VkVertexInputBindingDescription GetBindingDescription();
		static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
	};

	Rect FitRectToTexture(Rect currentRect);
	
	void CreateBuffers();
	void CreatePipeline();
	void CreateDescriptorSetLayout();
	void CreateDescriptorPool();
	void CreateDescriptorSets();

	void UpdateDescriptorSets();
	void PopulateBuffers();
	void DispatchCommands();

	static inline ImageRenderer* instance;

	RenderPipeline pipeline;

	static const int MAX_REQUESTS_IN_FLIGHT = 5;
	static const int MAX_VERTICES_IN_REQUEST = 512;

	unsigned int currentIndex = 0;

	std::array<VkBuffer, MAX_REQUESTS_IN_FLIGHT> vertexBuffers;
	std::array<VkDeviceMemory, MAX_REQUESTS_IN_FLIGHT> vertexBufferMemories;
	std::array<VkBuffer, MAX_REQUESTS_IN_FLIGHT> indexBuffers;
	std::array<VkDeviceMemory, MAX_REQUESTS_IN_FLIGHT> indexBufferMemories;

	std::array<VkCommandBuffer, MAX_REQUESTS_IN_FLIGHT> commandBuffers;

	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptorSet;
	VkDescriptorSetLayout descriptorSetLayout;

	std::vector<Vertex> vertices;
	Texture* currentTexture;
};