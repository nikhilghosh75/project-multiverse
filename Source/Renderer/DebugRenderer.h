#pragma once
#include "vulkan/vulkan.h"
#include "RenderPipeline.h"
#include "Rect.h"
#include "glm/glm.hpp"
#include <array>
#include <vector>

class DebugRenderer
{
public:
	DebugRenderer();
	~DebugRenderer();

	static DebugRenderer* Get();

	// In Rendering Space
	void AddBox(Rect rect);

private:
	static inline DebugRenderer* instance;

	void CreatePipeline();

	void CreateDescriptorSetLayout();
	void CreateDescriptorPool();
	void CreateDescriptorSets();

	void UpdateDescriptorSets();
	void PopulateBuffers();
	void DispatchCommands();

	void Render();

	RenderPipeline pipeline;

	static const int MAX_REQUESTS_IN_FLIGHT = 5;
	static const int MAX_VERTICES_IN_REQUEST = 128;

	unsigned int currentIndex = 0;

	std::array<VkBuffer, MAX_REQUESTS_IN_FLIGHT> vertexBuffers;
	std::array<VkDeviceMemory, MAX_REQUESTS_IN_FLIGHT> vertexBufferMemories;
	std::array<VkBuffer, MAX_REQUESTS_IN_FLIGHT> indexBuffers;
	std::array<VkDeviceMemory, MAX_REQUESTS_IN_FLIGHT> indexBufferMemories;

	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptorSet;
	VkDescriptorSetLayout descriptorSetLayout;

	class DebugVertex
	{
	public:
		glm::vec2 position;

		DebugVertex(glm::vec2 position);

		static VkVertexInputBindingDescription GetBindingDescription();
		static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
	};

	std::vector<DebugVertex> vertices;
	std::vector<uint16_t> indices;
};