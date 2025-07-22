#pragma once

#include "RenderPipeline.h"
#include "RenderRequest.h"
#include "ScreenCoordinate.h"
#include "SkeletalSprite.h"

#include <mutex>

class SkeletalSpriteVertex
{
public:
	SkeletalSpriteVertex(glm::vec2 position, glm::vec2 uvCoordinate, float layer);

	glm::vec2 position;
	glm::vec2 uvCoordinate;
	float layer;

	static VkVertexInputBindingDescription GetBindingDescription();
	static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
};

class SkeletalSpriteRenderRequest : public RenderRequest
{
public:
	bool CanBeCombined(const RenderRequest* other) const override;

	void CombineWith(RenderRequest* other) override;

	void Render() override;
	void Clean() override;

	std::vector<SkeletalSpriteVertex> vertices;
	std::vector<unsigned int> indices;
	Texture* texture;

	static SkeletalSpriteRenderRequest* CreateRequest();
	static std::vector<RenderRequest*> GetRequestsThisFrame();

private:
	static const int MAX_SPRITE_REQUESTS = 24;

	// Requests can be accessed by both the rendering thread and the game thread
	static std::array<SkeletalSpriteRenderRequest, MAX_SPRITE_REQUESTS> requests;
	static std::mutex requestsMutex;

	static inline bool requestsArrayInitialized = false;
	static inline int lastIndex = 0;

	friend class SkeletalSpriteRenderer;
};

class SkeletalSpriteRenderer
{
public:
	SkeletalSpriteRenderer();
	~SkeletalSpriteRenderer();

	static SkeletalSpriteRenderer* Get();

	void AddSkeletalSprite(SkeletalSprite& sprite, ScreenCoordinate position, float scale);

	void RenderSkeletalSpriteRequest(SkeletalSpriteRenderRequest* request);

	// TODO: Add a convience function to convert a rect to a scale

private:
	static inline SkeletalSpriteRenderer* instance;

	static const int MAX_REQUESTS_IN_FLIGHT = 5;
	static const int MAX_VERTICES_IN_REQUEST = 2048;

	void CreateBuffers();
	void CreatePipeline();
	void CreateDescriptorSetLayout();
	void CreateDescriptorPool();
	void CreateDescriptorSets();

	void UpdateDescriptorSets();
	void PopulateBuffers(std::vector<SkeletalSpriteVertex>& vertices, std::vector<unsigned int>& indices);
	void DispatchCommands();

	std::array<VkBuffer, MAX_REQUESTS_IN_FLIGHT> vertexBuffers;
	std::array<VkDeviceMemory, MAX_REQUESTS_IN_FLIGHT> vertexBufferMemories;
	std::array<VkBuffer, MAX_REQUESTS_IN_FLIGHT> indexBuffers;
	std::array<VkDeviceMemory, MAX_REQUESTS_IN_FLIGHT> indexBufferMemories;

	std::array<VkCommandBuffer, MAX_REQUESTS_IN_FLIGHT> commandBuffers;

	unsigned int currentIndex = 0;
	unsigned int verticesCount = 0;
	unsigned int indicesCount;

	RenderPipeline pipeline;

	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptorSet;
	VkDescriptorSetLayout descriptorSetLayout;

	std::vector<SkeletalSpriteVertex> ComputeVertices(SkeletalSprite& sprite, ScreenCoordinate position, float scale);
	std::vector<unsigned int> ComputeIndices(SkeletalSprite& sprite);

	Texture* currentTexture;
};