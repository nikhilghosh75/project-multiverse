#include "UnitySkeletonImporter.h"

#include "../SkeletalAnimationLoader.h"

#include "AssertUtils.h"
#include "ErrorManager.h"
#include "Quat.h"
#include "YAMLParser.h"

#include <filesystem>
#include <fstream>
#include <regex>

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

		debugInfo.boneNameToIndex.insert({ (std::string)boneName, skeleton.bones.size() });
		skeleton.bones.push_back(bone);

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

void ParseRig(YAML& yaml, Skeleton& skeleton, SkeletonDebugInfo& debugInfo)
{
	YAML::Node& importer = yaml["ScriptedImporter"];

	// Get Rig Data
	YAML::Node* spriteSettings = yaml.GetChild(&importer, "spriteImportData");
	YAML::Node* rigSpriteSettings = yaml.GetChild(&importer, "rigSpriteImportData");

	LayerInfo* layerInfo = nullptr;
	
	// Each layer has their own set of indices to refer to bones
	std::vector<int> absoluteBoneIndicesOfLocalLayers;

	int i = 0;

	// Iterate over all the sprites
	for (auto child = yaml.NodeChildren(rigSpriteSettings).begin(); child != yaml.NodeChildren(rigSpriteSettings).end(); ++child)
	{
		YAML::Node& node = *child;
		std::string_view name = (*child).GetName();
		if (name == "name")
		{
			std::string layerName = (std::string)(*child).GetString();
			std::optional<std::string> fullLayerName = SkeletalAnimationLoader::Get()->FindFullNameOfLayer(layerName);
			if (fullLayerName)
			{
				layerInfo = &(SkeletalAnimationLoader::Get()->layers[*fullLayerName]);
			}
			else
			{
				ErrorManager::Get()->ReportError(ErrorSeverity::Error, "UnitySkeletalAnimation::ParseRig", "SkeletalAnimationViewer", 0, "LayerInfo cannot be found");
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
				layerInfo->spriteVertices = ParseSpriteVertices(yaml, &(*child));

				for (SpriteVertex vertex : layerInfo->spriteVertices)
				{
					for (BoneWeight weight : vertex.weights)
					{
						if (weight.boneWeight > 0)
						{
							weight.boneIndex = absoluteBoneIndicesOfLocalLayers[weight.boneIndex];
						}
					}
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
