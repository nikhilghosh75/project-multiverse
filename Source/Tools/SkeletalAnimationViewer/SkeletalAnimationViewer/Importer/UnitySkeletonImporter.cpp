#include "UnitySkeletonImporter.h"

#include "AssertUtils.h"
#include "ErrorManager.h"
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

			Bone bone;
			bone.localPosition = ParsePosition(bonePosition);
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
	std::regex regex(R"(z:\s*([-+]?[0-9]*\.?[0-9]+),)");
	std::smatch match;

	std::string position = std::string(rotationString);
	if (std::regex_search(position, match, regex) && match.size() == 2) {
		return std::stof(match[1].str());
	}

	return 0.0f;
}
