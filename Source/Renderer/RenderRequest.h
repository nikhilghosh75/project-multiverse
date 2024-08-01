#pragma once

enum class RenderRequestType : uint8_t
{
	Font,
	Image
};

class RenderRequest
{
public:
	virtual bool CanBeCombined(const RenderRequest* other) const = 0;

	virtual void CombineWith(RenderRequest* other) = 0;

	virtual void Render() = 0;

	RenderRequestType type;
	bool hasBeenSubmitted;
};