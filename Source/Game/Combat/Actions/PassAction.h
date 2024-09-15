#pragma once
#include "Action.h"

class PassAction : public Action
{
public:
	PassAction();

	std::string GetDisplayName() const;

	void SetFromJson(const rapidjson::Document& data);
};