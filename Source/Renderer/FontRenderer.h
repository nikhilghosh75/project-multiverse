#pragma once
#include "Font.h"
#include "RenderRequest.h"
#include "RenderPipeline.h"

#include <array>
#include <mutex>

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
	void Clean() override;

	static FontRenderRequest* CreateRequest();

	struct TextRequest
	{
		std::string text;
		glm::vec2 position;
		int fontSize;
	};

	Font* font;

	std::vector<TextRequest> texts;

	static std::vector<RenderRequest*> GetRequestsThisFrame();
private:
	static const int MAX_FONT_REQUESTS = 50;

	// Requests can be accessed by both the rendering thread and the game thread
	static std::array<FontRenderRequest, MAX_FONT_REQUESTS> requests;
	static std::mutex requestsMutex;

	static inline bool requestsArrayInitialized = false;
	static inline int lastIndex = 0;

	friend class FontRenderer;
};

class FontRenderer
{
public:
	FontRenderer();
	~FontRenderer();

	static FontRenderer* Get();

	// position is relative to the screen, and is the top left of the rect
	void AddText(std::string text, glm::vec2 position, float renderingOrder, int fontSize = 16);

	void RenderFontRequest(FontRenderRequest* request);
private:
	static inline FontRenderer* instance;

	Font* defaultFont;

	class FontUniform
	{
	public:

	};

	void PopulateBufferWithTextRequest(FontRenderRequest::TextRequest& text);

	void CreatePipeline();
	void CreateCommandBuffers();

	void UpdateDescriptorSets();
	void PopulateBuffers();
	void DispatchCommands();

	std::vector<FontVertex> vertices;
	std::vector<unsigned int> indices;

	static const int MAX_REQUESTS_IN_FLIGHT = 5;
	static const int MAX_VERTICES_IN_REQUEST = 2048;

	unsigned int currentIndex = 0;
	
	std::array<VkBuffer, MAX_REQUESTS_IN_FLIGHT> fontVertexBuffers;
	std::array<VkDeviceMemory, MAX_REQUESTS_IN_FLIGHT> fontVertexBufferMemories;
	std::array<VkBuffer, MAX_REQUESTS_IN_FLIGHT> fontIndexBuffers;
	std::array<VkDeviceMemory, MAX_REQUESTS_IN_FLIGHT> fontIndexBufferMemories;

	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	std::vector<VkDescriptorSet> descriptorSets;
	VkSampler sampler;

	std::array<VkCommandBuffer, MAX_REQUESTS_IN_FLIGHT> commandBuffers;

	RenderPipeline pipeline;
};