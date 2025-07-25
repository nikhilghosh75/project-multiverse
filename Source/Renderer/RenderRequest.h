#pragma once

/*
* Render Requests are not dynamically instantiated. They are a base class for all render request
*/
class RenderRequest
{
public:
	virtual bool CanBeCombined(const RenderRequest* other) const = 0;

	virtual void CombineWith(RenderRequest* other) = 0;

	virtual void Render() = 0;
	virtual void Clean() = 0;

	bool isActive = false; // Does the request have data that is actively used
	bool isProcessing = false; // Is the request being processed by the RenderingManager
	float renderingOrder = 0.f;
};