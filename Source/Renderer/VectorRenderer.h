#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "Color.h"
#include "Rect.h"
#include "RenderPipeline.h"
#include "ScreenCoordinate.h"
#include <array>

class VectorPainter
{
public:
	VectorPainter();
	VectorPainter(ScreenSpace space);

	void SetFillColor(Color color);

	void BeginPath();

	void ClosePath();

	void PointTo(glm::vec2 point);
	
	void LineTo(glm::vec2 point);

private:
	friend class VectorRenderer;

	enum class State
	{
		WaitingToBegin,
		WaitingForStartingPoint,
		Painting
	};

	class Path
	{
	public:
		Color color;
		std::vector<glm::vec2> points;

		Path() { }
	};

	ScreenSpace screenSpace;

	State state;
	std::vector<Path> paths;

	Color currentColor;
};

class VectorRenderer
{
public:
	VectorRenderer();
	~VectorRenderer();

	static VectorRenderer* Get();

	void SubmitPainter(const VectorPainter& painter);

private:
	class Vertex
	{
	public:
		glm::vec2 position;
		glm::vec4 color;
		float layer;

		Vertex(glm::vec2 _position, glm::vec4 _color, float _layer);

		static VkVertexInputBindingDescription GetBindingDescription();
		static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
	};

	void CreateBuffers();
	void CreatePipeline();
	void CreateDescriptorSetLayout();
	void CreateDescriptorPool();
	void CreateDescriptorSets();

	void UpdateDescriptorSets();
	void PopulateBuffers();
	void DispatchCommands();

	void Render();

	bool IsValidPath(const VectorPainter::Path& path);

	void SubmitPath(const VectorPainter::Path& path, ScreenSpace space);

	glm::vec2 ConvertToRenderSpace(glm::vec2 point, ScreenSpace space) const;

	static const int MAX_REQUESTS_IN_FLIGHT = 5;
	static const int MAX_VERTICES_IN_REQUEST = 4096;

	unsigned int currentIndex = 0;

	std::array<VkBuffer, MAX_REQUESTS_IN_FLIGHT> vertexBuffers;
	std::array<VkDeviceMemory, MAX_REQUESTS_IN_FLIGHT> vertexBufferMemories;
	std::array<VkBuffer, MAX_REQUESTS_IN_FLIGHT> indexBuffers;
	std::array<VkDeviceMemory, MAX_REQUESTS_IN_FLIGHT> indexBufferMemories;

	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptorSet;
	VkDescriptorSetLayout descriptorSetLayout;

	RenderPipeline pipeline;

	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;
	float currentLayer = 0;
	float maxLayers = 0;

	static inline VectorRenderer* instance;
};