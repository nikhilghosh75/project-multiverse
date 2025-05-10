#include "UnitySkeletonImporter.h"

#include "AssertUtils.h"
#include "ErrorManager.h"
#include "Quat.h"
#include "YAMLParser.h"

#include <filesystem>
#include <fstream>
#include <regex>

Skeleton UnitySkeletonImporter::Import(const std::string& filepath)
{
	Skeleton skeleton;

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

		// Get Document Pivot Point (which is a separate value)
		YAML::Node* documentPivotNode = yaml.GetChild(textureSettings, "documentPivot");
		glm::vec2 documentPivot = ParsePosition(documentPivotNode->GetString());

		// Get Document Size
		YAML::Node* documentSizeNode = yaml.GetChild(&importer, "documentSize");
		glm::vec2 documentSize = ParsePosition(documentSizeNode->GetString());

		// Get Bones
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

			skeleton.bones.push_back(bone);

			if (boneParentIndex != -1)
			{
				bonesToParentIndices.insert({ skeleton.bones.size() - 1, boneParentIndex});
			}
		}

		for (auto it : bonesToParentIndices)
		{
			skeleton.bones[it.first].parent = &skeleton.bones[it.second];
			skeleton.bones[it.second].children.push_back(&skeleton.bones[it.first]);
		}
	}
	else
	{
		ErrorManager::Get()->ReportError(ErrorSeverity::Error, "UnitySkeletonImporter::Import", "SkeletalAnimationViewer", 0, "File could not be opened");
	}

	return skeleton;
}

glm::vec2 UnitySkeletonImporter::ParsePosition(const std::string_view& positionString)
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

float UnitySkeletonImporter::ParseRotation(const std::string_view& rotationString)
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
