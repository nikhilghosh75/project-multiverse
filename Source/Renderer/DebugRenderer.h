#pragma once
#include "vulkan/vulkan.h"
#include "RenderPipeline.h"
#include "RenderRequest.h"
#include "Rect.h"
#include "glm/glm.hpp"

#include <array>
#include <mutex>
#include <vector>

class DebugRenderRequest : public RenderRequest
{
public:
	bool CanBeCombined(const RenderRequest* other) const override;

	void CombineWith(RenderRequest* other) override;

	void Render(VkCommandBuffer buffer) override;
	void Clean() override;

	static DebugRenderRequest* CreateRequest();

	std::vector<Rect> rects;

	static std::vector<RenderRequest*> GetRequestsThisFrame();
private:
	static const int MAX_DEBUG_REQUESTS = 50;

	// Requests can be accessed by both the rendering thread and the game thread
	static std::array<DebugRenderRequest, MAX_DEBUG_REQUESTS> requests;
	static std::mutex requestsMutex;

	static inline bool requestsArrayInitialized = false;
	static inline int lastIndex = 0;

	friend class DebugRenderer;
};

class DebugRenderer
{
public:
	DebugRenderer();
	~DebugRenderer();

	static DebugRenderer* Get();

	// In Rendering Space
	void AddBox(Rect rect);

	void RenderDebugRequest(DebugRenderRequest* request, VkCommandBuffer commandBuffer);
private:
	static inline DebugRenderer* instance;

	void PopulateWithBox(Rect rect);

	void CreatePipeline();

	void CreateDescriptorSetLayout();
	void CreateDescriptorPool();
	void CreateDescriptorSets();

	void UpdateDescriptorSets();
	void PopulateBuffers();
	void DispatchCommands(VkCommandBuffer commandBuffer);

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