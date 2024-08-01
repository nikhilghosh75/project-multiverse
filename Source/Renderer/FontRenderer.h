#pragma once
#include "Font.h"
#include "RenderRequest.h"
#include "RenderPipeline.h"
#include <array>

class FontVertex
{
public:
	FontVertex(glm::vec2 position, glm::vec2 uvCoordinate);

	glm::vec2 position;
	glm::vec2 uvCoordinate;

	static VkVertexInputBindingDescription GetBindingDescription();
	static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
};

class FontRenderRequest : public RenderRequest
{
public:
	bool CanBeCombined(const RenderRequest* other) const override;

	void CombineWith(RenderRequest* other) override;

	void Render() override;

	static FontRenderRequest* CreateRequest();

	Font* font;
	
	std::vector<FontVertex> vertices;
	std::vector<unsigned int> indices;
private:
	static const int MAX_FONT_REQUESTS = 50;

	static std::array<FontRenderRequest, MAX_FONT_REQUESTS> requests;
	static inline bool requestsArrayInitialized = false;
	static inline int lastIndex = 0;
};

class FontRenderer
{
public:
	FontRenderer();
	~FontRenderer();

	static FontRenderer* Get();

	// position is relative to the screen, and is the top left of the rect
	void AddText(std::string text, glm::vec2 position, int fontSize = 16);
private:
	static inline FontRenderer* instance;

	Font* defaultFont;

	class FontUniform
	{
	public:

	};

	void CreatePipeline();

	void UpdateDescriptorSets();
	void CreateBuffers();
	void DispatchCommands();

	void Render();

	std::vector<FontVertex> vertices;
	std::vector<unsigned int> indices;

	VkBuffer fontVertexBuffer;
	VkDeviceMemory fontVertexBufferMemory;
	VkBuffer fontIndexBuffer;
	VkDeviceMemory fontIndexBufferMemory;

	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	std::vector<VkDescriptorSet> descriptorSets;
	VkSampler sampler;

	VkSemaphore renderFinishedSemaphore;

	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	// RenderPipeline pipeline;
};