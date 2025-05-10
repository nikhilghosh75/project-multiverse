#include "Quat.h"

#include "MathUtils.h"

Quat::Quat()
	: x(0), y(0), z(0), w(0)
{
}

Quat::Quat(float x, float y, float z, float w)
	: x(x), y(y), z(z), w(w)
{
}

glm::vec3 Quat::ToEulerAngles() const
{
	float sinrCosp = 2 * (w * x + y * z);
	float cosrCosp = 1 - 2 * (x * x + y * y);
	float roll = atan2f(sinrCosp, cosrCosp);

	float sinp = 2 * (w * y - z * x);
	float pitch = 0.f;
	if (abs(sinp) >= 1)
	{
		pitch = Math::CopySign(1.5707f, sinp);
	}
	else
	{
		pitch = asinf(sinp);
	}

	float sinyCosp = 2 * (w * z + x * y);
	float cosyCosp = 1 - 2 * (y * y + z * z);
	float yaw = atan2(sinyCosp, cosyCosp);

	return glm::vec3(roll, pitch, yaw);
}
