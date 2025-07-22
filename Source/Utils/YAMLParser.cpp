#include "YAMLParser.h"

#include "ErrorManager.h"

#include <charconv>

std::string_view YAML::Node::GetName() const
{
	return std::string_view(name, nameLength);
}

YAML::ValueType YAML::Node::GetType() const
{
	return valueType;
}

bool YAML::Node::IsString() const
{
	return valueType == ValueType::String;
}

bool YAML::Node::IsInt() const
{
	return valueType == ValueType::Int;
}

bool YAML::Node::IsFloat() const
{
	return valueType == ValueType::Float;
}

std::string_view YAML::Node::GetString() const
{
	return std::string_view(name + stringOffset, valueLength);
}

int YAML::Node::GetInt() const
{
	return intValue;
}

float YAML::Node::GetFloat() const
{
	if (valueType == ValueType::Int)
	{
		return (float)intValue;
	}

	return floatValue;
}

YAML::ChildrenIterator& YAML::ChildrenIterator::operator++()
{
	index++;
	return *this;
}

bool YAML::ChildrenIterator::operator==(ChildrenIterator other)
{
	return index == other.index;
}

bool YAML::ChildrenIterator::operator!=(ChildrenIterator other)
{
	return index != other.index;
}

YAML::Node& YAML::ChildrenIterator::operator*()
{
	int indexOfChild = yaml->childrenList[index];
	return yaml->nodes[indexOfChild];
}

YAML::ChildrenIterator YAML::Children::begin()
{
	YAML::ChildrenIterator it;
	it.yaml = yaml;
	it.index = listStartindex + 1;

	return it;
}

YAML::ChildrenIterator YAML::Children::end()
{
	if (listStartindex == -1)
	{
		YAML::ChildrenIterator it;
		it.yaml = yaml;
		it.index = listStartindex + 1;

		return it;
	}

	int lengthOfChildrenList = yaml->childrenList[listStartindex];

	YAML::ChildrenIterator it;
	it.yaml = yaml;
	it.index = listStartindex + 1 + lengthOfChildrenList;

	return it;
}

YAML::YAML(void* buffer, size_t size)
{
	Populate((char*)buffer, size);
}

bool YAML::HasChildren(const Node* node) const
{
	return node->childrenListStart == -1;
}

YAML::Children YAML::NodeChildren(const Node* node)
{
	YAML::Children children;
	children.yaml = this;
	children.listStartindex = node->childrenListStart;

	return children;
}

size_t YAML::RootChildrenCount() const
{
	return rootChildrenIndices.size();
}

size_t YAML::NodeCount() const
{
	return nodes.size();
}

bool YAML::HasRootNode(const std::string& name)
{
	for (int i = 0; i < rootChildrenIndices.size(); i++)
	{
		Node& node = nodes[rootChildrenIndices[i]];
		if (node.GetName() == name)
		{
			return true;
		}
	}

	return false;
}

bool YAML::HasNodeRecursive(const std::string& name)
{
	for (int i = 0; i < nodes.size(); i++)
	{
		if (nodes[i].GetName() == name)
		{
			return true;
		}
	}

	return false;
}

bool YAML::HasChild(const Node* node, const std::string& name)
{
	for (const auto child : NodeChildren(node))
	{
		if (child.GetName() == name)
		{
			return true;
		}
	}

	return false;
}

YAML::Node* YAML::GetChild(const Node* node, const std::string& name)
{
	for (auto& child : NodeChildren(node))
	{
		if (child.GetName() == name)
		{
			return &child;
		}
	}

	return &nodes[0];
}

YAML::Node& YAML::operator[](int rootIndex)
{
	return nodes[rootChildrenIndices[rootIndex]];
}

YAML::Node& YAML::operator[](const std::string& name)
{
	for (int i = 0; i < rootChildrenIndices.size(); i++)
	{
		Node& node = nodes[rootChildrenIndices[i]];
		if (node.GetName() == name)
		{
			return node;
		}
	}

	return nodes[0];
}

void YAML::Populate(char* buffer, size_t size)
{
	LineParseState parseState = LineParseState::StartOfLine;

	int indentation = 0;
	int nameStartIndex = 0;
	int nameEndIndex = 0;
	int valueStartIndex = 0;
	int valueEndIndex = 0;

	std::vector<int> currentNodeStackIndices;

	int lines = 0;

	size_t i = 0;
	while (i < size)
	{
		switch (parseState)
		{
		case LineParseState::StartOfLine:
			lines++;

			indentation = 0;
			nameStartIndex = 0;
			nameEndIndex = 0;
			valueStartIndex = 0;
			valueEndIndex = 0;

			while (buffer[i] == ' ' || buffer[i] == '-')
			{
				indentation++;
				i++;
			}
			indentation /= 2;
			currentNodeStackIndices.resize(indentation);
			parseState = LineParseState::Name;
			break;
		case LineParseState::Name:
			nameStartIndex = i;
			while (buffer[i] != ':' && buffer[i] != '-' && buffer[i] != '\n' && buffer[i] != '{')
			{
				i++;
			}

			if (buffer[i] == '\n')
			{
				i++;
				parseState = LineParseState::StartOfLine;
			}
			else if (buffer[i] == '-' || buffer[i] == '{')
			{
				// If this happens, we have an anonymous node, so we leave the name blank
				nameStartIndex = i;
				nameEndIndex = i;
				parseState = LineParseState::BetweenValue;
			}
			else if (buffer[i] == ':')
			{
				nameEndIndex = i;
				parseState = LineParseState::BetweenValue;
			}
			else
			{
				ErrorManager::Get()->ReportError(ErrorSeverity::Warning, "YAMLParser::Populate", "Utils", buffer[i], "YAML Parser found a character in the name it could not understand");
				parseState = LineParseState::StartOfLine;
			}
			break;
		case LineParseState::BetweenValue:
			while (!std::isalnum(buffer[i]) && buffer[i] != '\n' && buffer[i] != '-' && buffer[i] != '{' && buffer[i] != '<')
			{
				i++;
			}
			if (buffer[i] == '\n')
			{
				// If this happens, it's likely it has no value and is instead a group node
				i++;
				parseState = LineParseState::StartOfLine;
				
				Node node;
				node.name = buffer + nameStartIndex;
				node.parentIndex = currentNodeStackIndices.size() == 0 ? -1 : currentNodeStackIndices.back();
				node.childrenListStart = -1;
				node.valueType = ValueType::Group;
				node.nameLength = nameEndIndex - nameStartIndex;
				node.valueLength = 0;

				nodes.push_back(node);
				currentNodeStackIndices.push_back(nodes.size() - 1);
			}
			else
			{
				parseState = LineParseState::Value;
			}
			break;
		case LineParseState::Value:
			ValueType type = ValueType::Int;
			valueStartIndex = i;
			while (buffer[i] != '\n')
			{
				if (std::isalpha(buffer[i]))
				{
					type = ValueType::String;
				}
				else if (buffer[i] == '.' && type == ValueType::Int)
				{
					type = ValueType::Float;
				}
				i++;
			}
			valueEndIndex = i - 1;
			
			Node node;
			node.name = buffer + nameStartIndex;
			node.parentIndex = currentNodeStackIndices.size() == 0 ? -1 : currentNodeStackIndices.back();
			node.childrenListStart = -1;
			node.valueType = type;
			node.nameLength = nameEndIndex - nameStartIndex;
			node.valueLength = valueEndIndex - valueStartIndex + 1;
			
			if (type == ValueType::Int)
			{
				std::string_view stringView(buffer + valueStartIndex, valueEndIndex - valueStartIndex + 1);
				std::from_chars(stringView.data(), stringView.data() + stringView.length(), node.intValue);
			}
			else if (type == ValueType::Float)
			{
				std::string_view stringView(buffer + valueStartIndex, valueEndIndex - valueStartIndex + 1);
				std::from_chars(stringView.data(), stringView.data() + stringView.length(), node.floatValue);
			}
			else if (type == ValueType::String)
			{
				node.stringOffset = static_cast<uint16_t>(valueStartIndex - nameStartIndex);
			}

			nodes.push_back(node);
			currentNodeStackIndices.push_back(nodes.size() - 1);

			i++;
			parseState = LineParseState::StartOfLine;
			break;
		}
	}

	// Populate the children list
	for (int i = 0; i < nodes.size(); i++)
	{
		int indentationLevel = GetIndentationLevelOfNode(nodes[i], buffer);
		int numChildren = 0;

		if (indentationLevel == 0)
		{
			rootChildrenIndices.push_back(i);
		}

		std::vector<int> childIndices;
		for (int j = i + 1; j < nodes.size(); j++)
		{
			int childIndentationLevel = GetIndentationLevelOfNode(nodes[j], buffer);
			if (childIndentationLevel == indentationLevel)
			{
				break;
			}
			if (childIndentationLevel == indentationLevel + 1)
			{
				childIndices.push_back(j);
				numChildren++;
			}
		}

		if (numChildren == 0)
		{
			nodes[i].childrenListStart = -1;
		}
		else
		{
			nodes[i].childrenListStart = childrenList.size();

			childrenList.push_back(childIndices.size());

			for (int childIndex : childIndices)
			{
				childrenList.push_back(childIndex);
			}
		}
	}
}

int YAML::GetIndentationLevelOfNode(const Node& node, char* bufferStart)
{
	if (node.name == bufferStart)
	{
		return 0;
	}

	int indentation = 0;
	while (*((char*)(node.name - indentation)) != '\n')
	{
		indentation++;
	}
	indentation /= 2;

	return indentation;
}
