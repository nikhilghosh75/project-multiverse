#pragma once
#include "glm/glm.hpp"
#include "Rect.h"
#include "RenderPipeline.h"
#include "Texture.h"

class ImageRenderingOptions
{
public:
	ImageRenderingOptions();

	bool keepAspectRatio;
};

class ImageRenderer
{
	static inline ImageRenderingOptions defaultOptions;

public:
	ImageRenderer();
	~ImageRenderer();

	static ImageRenderer* Get();

	void AddImage(Texture* texture, Rect rect, ImageRenderingOptions options = defaultOptions);

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
	
	void CreatePipeline();
	void CreateDescriptorSetLayout();
	void CreateDescriptorPool();
	void CreateDescriptorSets();

	void UpdateDescriptorSets();
	void PopulateBuffers();
	void DispatchCommands();

	static inline ImageRenderer* instance;

	RenderPipeline pipeline;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptorSet;
	VkDescriptorSetLayout descriptorSetLayout;
	// std::vector<VkDescriptorSet> descriptorSets;

	std::vector<Vertex> vertices;
	Texture* currentTexture;
};