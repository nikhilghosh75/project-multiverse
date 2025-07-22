#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

// Note: this assumes that the buffer exists for the entire duration of the YAMLFile class
class YAML
{
public:
	enum class ValueType :uint8_t
	{
		String,
		Int,
		Float,
		Group,
		List,
	};

	class Node
	{
	public:
		std::string_view GetName() const;

		ValueType GetType() const;

		bool IsString() const;
		bool IsInt() const;
		bool IsFloat() const;

		std::string_view GetString() const;
		int GetInt() const;
		float GetFloat() const;

	private:
		char* name;

		union
		{
			int stringOffset;
			int intValue;
			float floatValue;
		};

		int parentIndex;
		int childrenListStart;

		ValueType valueType;
		uint8_t nameLength;
		uint16_t valueLength;

		friend class YAML;
	};

	class Children;

	class ChildrenIterator
	{
		YAML* yaml;
		int index;

		friend class YAML::Children;

	public:
		ChildrenIterator& operator++();
		bool operator==(ChildrenIterator other);
		bool operator!=(ChildrenIterator other);

		Node& operator*();
	};

	class Children
	{
		YAML* yaml;
		int listStartindex;
	public:
		ChildrenIterator begin();
		ChildrenIterator end();

		friend class YAML;
	};

	YAML(void* buffer, size_t size);

	bool HasChildren(const Node* node) const;
	Children NodeChildren(const Node* node);
	
	size_t RootChildrenCount() const;
	size_t NodeCount() const;

	bool HasRootNode(const std::string& name);
	bool HasNodeRecursive(const std::string& node);

	bool HasChild(const Node* node, const std::string& name);
	Node* GetChild(const Node* node, const std::string& name);
	
	Node& operator[](int rootIndex);
	Node& operator[](const std::string& node);
private:
	void Populate(char* buffer, size_t size);

	int GetIndentationLevelOfNode(const Node& node, char* bufferStart);

	std::vector<unsigned int> childrenList;
	std::vector<unsigned int> rootChildrenIndices;
	std::vector<Node> nodes;

	enum class LineParseState
	{
		StartOfLine,
		Indentation,
		Name,
		BetweenValue,
		Value,
		SkippingToEndOfLine
	};
};