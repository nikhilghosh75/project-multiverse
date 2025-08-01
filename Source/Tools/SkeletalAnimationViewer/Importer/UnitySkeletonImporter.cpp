#include "UnitySkeletonImporter.h"

#include "../SkeletalAnimationLoader.h"

#include "AssertUtils.h"
#include "ErrorManager.h"
#include "GeometryUtils.h"
#include "Quat.h"
#include "StringUtils.h"
#include "YAMLParser.h"

#include <filesystem>
#include <fstream>
#include <regex>
#include <unordered_set>

glm::vec2 ParsePosition(const std::string_view& positionString);
float ParseRotation(const std::string_view& rotationString);

void ParseBones(YAML& yaml, Skeleton& skeleton, SkeletonDebugInfo& debugInfo, glm::vec2 documentSize)
{
	YAML::Node& importer = yaml["ScriptedImporter"];

	// Get Document Pivot Point (which is a separate value)
	YAML::Node* textureSettings = yaml.GetChild(&importer, "textureImporterSettings");
	YAML::Node* documentPivotNode = yaml.GetChild(textureSettings, "documentPivot");
	glm::vec2 documentPivot = ParsePosition(documentPivotNode->GetString());

	ASSERT(yaml.HasChild(&importer, "characterData"));
	YAML::Node* characterData = yaml.GetChild(&importer, "characterData");

	ASSERT(yaml.HasChild(characterData, "bones"));
	YAML::Node* bones = yaml.GetChild(characterData, "bones");

	std::unordered_map<int, int> bonesToParentIndices;

	for (auto child = yaml.NodeChildren(bones).begin(); child != yaml.NodeChildren(bones).end(); ++child)
	{
		// Each bone element has 5 children underneath the "bones" object

		ASSERT((*child).GetName() == "name");
		ASSERT((*child).GetType() == YAML::ValueType::String);
		std::string_view boneName = (*child).GetString();
		++child;

		ASSERT((*child).GetName() == "position");
		ASSERT((*child).GetType() == YAML::ValueType::String);
		std::string_view bonePosition = (*child).GetString();
		++child;

		ASSERT((*child).GetName() == "rotation");
		ASSERT((*child).GetType() == YAML::ValueType::String);
		std::string_view boneRotation = (*child).GetString();
		++child;

		ASSERT((*child).GetName() == "length");
		ASSERT((*child).GetType() == YAML::ValueType::Float);
		float boneLength = (*child).GetFloat();
		++child;

		ASSERT((*child).GetName() == "parentId");
		ASSERT((*child).GetType() == YAML::ValueType::Int);
		int boneParentIndex = (*child).GetInt();

		// Since we store all positions as pixel positions relative to the center
		// we need to transform it when we parse
		glm::vec2 localPosition;

		if (boneParentIndex == -1)
		{
			glm::vec2 pivotedLocalPosition = ParsePosition(bonePosition);
			localPosition = pivotedLocalPosition - glm::vec2((0.5 - documentPivot.x) * documentSize.x, (0.5 - documentPivot.y) * documentSize.y);
		}
		else
		{
			localPosition = ParsePosition(bonePosition);
		}

		Bone bone;
		bone.localPosition = localPosition;
		bone.localRotation = ParseRotation(boneRotation);
		bone.length = boneLength;
		bone.parent = nullptr;

		debugInfo.boneNameToIndex.insert({ (std::string)boneName, skeleton.bones.size() });
		skeleton.bones.push_back(bone);

		// If the bone has a parent, store it and figure out the bone's positions after the vector is properly allocated
		if (boneParentIndex != -1)
		{
			bonesToParentIndices.insert({ skeleton.bones.size() - 1, boneParentIndex });
		}
	}
	
	for (auto it : bonesToParentIndices)
	{
		skeleton.bones[it.first].parent = &skeleton.bones[it.second];
		skeleton.bones[it.second].children.push_back(&skeleton.bones[it.first]);
	}

	// Calculate the inverse bind poses of all the bones
	for (int i = 0; i < skeleton.bones.size(); i++)
	{
		if (skeleton.bones[i].parent != nullptr)
		{
			skeleton.bones[i].inverseBindPose = glm::inverse(skeleton.bones[i].parent->GetAbsoluteTransform()
				* skeleton.bones[i].GetLocalTransform());
		}
		else
		{
			skeleton.bones[i].inverseBindPose = glm::inverse(skeleton.bones[i].GetLocalTransform());
		}
	}
}

std::vector<SpriteVertex> ParseSpriteVertices(YAML yaml, YAML::Node* parent)
{
	std::vector<SpriteVertex> spriteVertices;

	for (auto child = yaml.NodeChildren(parent).begin(); child != yaml.NodeChildren(parent).end(); ++child)
	{
		SpriteVertex vertex;

		vertex.position = ParsePosition((*child).GetString());
		++child;

		YAML::ChildrenIterator boneIterator = yaml.NodeChildren(&(*child)).begin();

		vertex.weights[0].boneWeight = (*boneIterator).GetFloat();
		++boneIterator;

		vertex.weights[1].boneWeight = (*boneIterator).GetFloat();
		++boneIterator;

		vertex.weights[2].boneWeight = (*boneIterator).GetFloat();
		++boneIterator;

		vertex.weights[3].boneWeight = (*boneIterator).GetFloat();
		++boneIterator;

		vertex.weights[0].boneIndex = (*boneIterator).GetInt();
		++boneIterator;

		vertex.weights[1].boneIndex = (*boneIterator).GetInt();
		++boneIterator;

		vertex.weights[2].boneIndex = (*boneIterator).GetInt();
		++boneIterator;

		vertex.weights[3].boneIndex = (*boneIterator).GetInt();
		++boneIterator;

		spriteVertices.push_back(vertex);
	}

	return spriteVertices;
}

void CalculateUVPositions(SkeletalSprite::Layer* layerInfo, std::vector<SpriteVertex>& vertices, float textureWidth, float textureHeight)
{
	for (SpriteVertex& vertex : vertices)
	{
		float x = (layerInfo->uvRect.left * textureWidth + vertex.position).x / textureWidth;
		float y = (layerInfo->uvRect.bottom * textureHeight - vertex.position).y / textureHeight;
		vertex.uv = glm::vec2(x, y);
	}
}

using Edge = std::pair<int, int>;

std::vector<Edge> ParseEdges(YAML yaml, YAML::Node* parent)
{
	std::vector<Edge> edges;

	for (auto child = yaml.NodeChildren(parent).begin(); child != yaml.NodeChildren(parent).end(); ++child)
	{
		std::regex regex(R"(x:\s*([0-9]*),\s*y:\s*([0-9]*))");
		std::smatch match;

		std::string str = std::string((*child).GetString());
		if (std::regex_search(str, match, regex) && match.size() == 3) 
		{
			edges.push_back({ atoi(match[1].str().c_str()), atoi(match[2].str().c_str()) });
		}
	}

	return edges;
}

std::vector<int> ParseIndices(YAML yaml, YAML::Node* node)
{
	std::vector<int> indices;

	std::string_view str = node->GetString();

	for (int i = 0; i + 7 < str.size(); i += 8)
	{
		uint8_t b0, b1, b2, b3; 
		std::from_chars(str.data() + i + 0, str.data() + i + 2, b0, 16);
		std::from_chars(str.data() + i + 2, str.data() + i + 4, b1, 16);
		std::from_chars(str.data() + i + 4, str.data() + i + 6, b2, 16);
		std::from_chars(str.data() + i + 6, str.data() + i + 8, b3, 16);

		uint32_t value = value = (b0) | (b1 << 8) | (b2 << 16) | (b3 << 24);
		indices.push_back(value);
	}

	return indices;
}

std::vector<std::array<int, 3>> TriangulateLoop(const std::vector<int>& loop, const std::vector<SpriteVertex>& vertices)
{
	std::vector<std::array<int, 3>> result;
	std::vector<int> polygon = loop;

	while (polygon.size() >= 3) 
	{
		bool ear_found = false;
		for (size_t i = 0; i < polygon.size(); ++i) 
		{
			int previousIndex = polygon[(i + polygon.size() - 1) % polygon.size()];
			int currentIndex = polygon[i];
			int nextIndex = polygon[(i + 1) % polygon.size()];

			glm::vec2 p0 = vertices[previousIndex].position;
			glm::vec2 p1 = vertices[currentIndex].position;
			glm::vec2 p2 = vertices[nextIndex].position;

			if (!Geometry::DoesCornerBendOutwards(p0, p1, p2))
			{
				continue;
			}

			// Check no other point is inside this triangle
			bool containsOther = false;
			for (int j = 0; j < polygon.size(); ++j) 
			{
				if (j == previousIndex || j == currentIndex || j == nextIndex)
				{
					continue;
				}

				if (Geometry::IsPointInTriangle(vertices[polygon[j]].position, p0, p1, p2)) 
				{
					containsOther = true;
					break;
				}
			}

			if (containsOther)
			{
				continue;
			}

			// We found an ear
			result.push_back({ previousIndex, currentIndex, nextIndex });
			polygon.erase(polygon.begin() + i);
			ear_found = true;
			break;
		}

		if (!ear_found) 
		{
			// Bad polygon (e.g. it might be self-intersecting)
			break;
		}
	}

	return result;
}

std::vector<std::vector<int>> ExtractLoopsFromEdges(std::vector<Edge>& edges)
{
	// We create a undirected adjacency list
	std::unordered_map<int, std::vector<int>> adjacency;
	for (const Edge& edge : edges) 
	{
		adjacency[edge.first].push_back(edge.second);
		adjacency[edge.second].push_back(edge.first); 
	}

	std::unordered_set<int> visitedNodes;
	std::vector<std::vector<int>> loops;

	for (const auto& [start, neighbors] : adjacency) 
	{
		if (visitedNodes.find(start) != visitedNodes.end())
		{
			continue;
		}

		std::vector<int> loop;
		int current = start;
		int previous = -1;

		while (true) 
		{
			loop.push_back(current);
			visitedNodes.insert(current);

			const std::vector<int>& neighbors = adjacency[current];
			int next = -1;

			for (int neighbor : neighbors) 
			{
				if (neighbor != previous) 
				{
					next = neighbor;
					break;
				}
			}

			if (next == -1 || next == start)
			{
				break;
			}

			previous = current;
			current = next;

			if (visitedNodes.count(current))
			{
				break;
			}
		}

		// If there's a duplicate closing vertex, remove it
		if (loop.size() >= 3 && loop.front() == loop.back())
		{
			loop.pop_back();
		}

		if (loop.size() >= 3)
		{
			loops.push_back(loop);
		}
	}

	return loops;
}


void ParseRig(YAML& yaml, Skeleton& skeleton, SkeletonDebugInfo& debugInfo)
{
	YAML::Node& importer = yaml["ScriptedImporter"];

	// Get Rig Data
	YAML::Node* spriteSettings = yaml.GetChild(&importer, "spriteImportData");
	YAML::Node* rigSpriteSettings = yaml.GetChild(&importer, "rigSpriteImportData");
	YAML::Node* rigPSDLayersNode = yaml.GetChild(&importer, "rigPSDLayers");

	SkeletalSprite::Layer* layerInfo = nullptr;
	
	// Each layer has their own set of indices to refer to bones
	std::vector<int> absoluteBoneIndicesOfLocalLayers;

	float textureWidth = SkeletalAnimationLoader::Get()->sprite.texture->GetTextureWidth();
	float textureHeight = SkeletalAnimationLoader::Get()->sprite.texture->GetTextureHeight();
	
	// Some layers will have the same name but different parent, so we want to differentiate them.
	struct RigPSDLayer
	{
		std::string_view layerName;
		std::string_view spriteName;
		int parentIndex;
		std::string fullName;
	};
	std::vector<RigPSDLayer> rigPSDLayers;

	for (auto child = yaml.NodeChildren(rigPSDLayersNode).begin(); child != yaml.NodeChildren(rigPSDLayersNode).end(); ++child)
	{
		std::string_view layerName = (*child).GetString();
		++child;

		std::string_view spriteName = (*child).GetString();
		++child;
		++child;

		int parentIndex = (*child).GetInt();
		++child;
		++child;
		++child;

		int currentParentIndex = parentIndex;
		std::string fullName = (std::string)layerName;
		String::ReplaceAll(fullName, '_', ' ');
		String::ReplaceAll(fullName, "\\", "");
		if (fullName[fullName.size() - 1] == '\"')
		{
			fullName.pop_back();
		}

		int i = 0;
		while (currentParentIndex != -1 && i < 10)
		{
			fullName = (std::string)rigPSDLayers[currentParentIndex].layerName + "/" + fullName;
			currentParentIndex = rigPSDLayers[currentParentIndex].parentIndex;
			i++;
		}

		rigPSDLayers.push_back({ layerName, spriteName, parentIndex, fullName });
	}

	// Iterate over all the sprites
	for (auto child = yaml.NodeChildren(rigSpriteSettings).begin(); child != yaml.NodeChildren(rigSpriteSettings).end(); ++child)
	{
		std::string_view name = (*child).GetName();
		if (name == "name")
		{
			std::string layerName = (std::string)(*child).GetString();

			// layerInfo = SkeletalAnimationLoader::Get()->FindLayerOfName(layerName);
			// if (layerInfo == nullptr)
			{
				// This layer has a similar name as another node
				// Use the rigPSDLayers vector to find it
				for (int i = 0; i < rigPSDLayers.size(); i++)
				{
					if (rigPSDLayers[i].spriteName == layerName)
					{
						layerInfo = SkeletalAnimationLoader::Get()->FindLayerOfName(rigPSDLayers[i].fullName);
						break;
					}
				}

				if (layerInfo == nullptr)
				{
					ErrorManager::Get()->ReportError(ErrorSeverity::Error, "UnitySkeletalAnimation::ParseRig", "SkeletalAnimationViewer", 0, "LayerInfo cannot be found");
				}
			}
		}
		else if (name == "spriteBone")
		{
			absoluteBoneIndicesOfLocalLayers.clear();

			for (auto grandchild = yaml.NodeChildren(&(*child)).begin(); grandchild != yaml.NodeChildren(&(*child)).end(); ++grandchild)
			{
				std::string_view boneName = (*grandchild).GetString();
				auto foundBoneIndex = debugInfo.boneNameToIndex.find((std::string)boneName);
				if (foundBoneIndex != debugInfo.boneNameToIndex.end())
				{
					absoluteBoneIndicesOfLocalLayers.push_back((*foundBoneIndex).second);
				}
				else
				{
					absoluteBoneIndicesOfLocalLayers.push_back(-1);
				}

				++grandchild;
				++grandchild;
				++grandchild;
				++grandchild;
			}
		}
		else if (name == "vertices")
		{
			if (layerInfo != nullptr)
			{
				layerInfo->vertices = ParseSpriteVertices(yaml, &(*child));
				CalculateUVPositions(layerInfo, layerInfo->vertices, textureWidth, textureHeight);

				for (SpriteVertex& vertex : layerInfo->vertices)
				{
					for (BoneWeight& weight : vertex.weights)
					{
						if (weight.boneWeight > 0)
						{
							weight.boneIndex = absoluteBoneIndicesOfLocalLayers[weight.boneIndex];
						}
					}
				}
			}
		}
		else if (name == "indices")
		{
			if (layerInfo != nullptr)
			{
				layerInfo->indices = ParseIndices(yaml, &(*child));
				debugInfo.indicesSize += layerInfo->indices.size();
			}
		}
		else if (name == "edges")
		{
			if (layerInfo != nullptr)
			{
				std::vector<Edge> edges = ParseEdges(yaml, &(*child));

				// Triangulate those edges
				std::vector<std::vector<int>> loops = ExtractLoopsFromEdges(edges);
				std::vector<std::array<int, 3>> triangles;

				for (std::vector<int>& loop : loops)
				{
					std::vector<std::array<int, 3>> loopTriangles = TriangulateLoop(loop, layerInfo->vertices);
					triangles.insert(triangles.end(), loopTriangles.begin(), loopTriangles.end());
				}
			}
		}
	}
}

void UnitySkeletonImporter::Import(const std::string& filepath, Skeleton& skeleton, SkeletonDebugInfo& debugInfo)
{
	std::filesystem::path path(filepath);
	std::size_t size = std::filesystem::file_size(path);

	std::ifstream file(filepath, std::ios::binary | std::ios::ate);
	file.seekg(0, std::ios::beg);

	std::vector<char> buffer(size);
	if (file.read(buffer.data(), size))
	{
		YAML yaml(buffer.data(), size);

		ASSERT(yaml.HasRootNode("ScriptedImporter"));
		YAML::Node& importer = yaml["ScriptedImporter"];

		// Get Texture Pivot Point
		YAML::Node* textureSettings = yaml.GetChild(&importer, "textureImporterSettings");

		YAML::Node* spritePivotNode = yaml.GetChild(textureSettings, "spritePivot");
		glm::vec2 spritePivot = ParsePosition(spritePivotNode->GetString());

		// Get Document Size
		YAML::Node* documentSizeNode = yaml.GetChild(&importer, "documentSize");
		glm::vec2 documentSize = ParsePosition(documentSizeNode->GetString());

		ParseBones(yaml, skeleton, debugInfo, documentSize);
		ParseRig(yaml, skeleton, debugInfo);
	}
	else
	{
		ErrorManager::Get()->ReportError(ErrorSeverity::Error, "UnitySkeletonImporter::Import", "SkeletalAnimationViewer", 0, "File could not be opened");
	}
}

glm::vec2 ParsePosition(const std::string_view& positionString)
{
	std::regex regex(R"(x:\s*([-+]?[0-9]*\.?[0-9]+),\s*y:\s*([-+]?[0-9]*\.?[0-9]+))");
	std::smatch match;

	std::string position = std::string(positionString);
	if (std::regex_search(position, match, regex) && match.size() == 3) {
		float x = std::stof(match[1].str());
		float y = std::stof(match[2].str());
		return glm::vec2(x, y);
	}

	return glm::vec2(0.0f);
}

float ParseRotation(const std::string_view& rotationString)
{
	std::regex regex(R"(x:\s*([-+]?[0-9]*\.?[0-9]+),\s*y:\s*([-+]?[0-9]*\.?[0-9]+),\s*z:\s*([-+]?[0-9]*\.?[0-9]+),\s*w:\s*([-+]?[0-9]*\.?[0-9]+))");
	std::smatch match;

	std::string position = std::string(rotationString);
	if (std::regex_search(position, match, regex) && match.size() == 5) {
		Quat quat(atof(match[1].str().c_str()), atof(match[2].str().c_str()), atof(match[3].str().c_str()), atof(match[4].str().c_str()));

		return quat.ToEulerAngles().z;
	}

	return 0.0f;
}
