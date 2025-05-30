#include "SkeletalSprite.h"

#include "MatrixUtils.h"

glm::mat4 Bone::GetLocalTransform() const
{
    return Matrix::TRS2D(localPosition, localRotation, glm::vec2(1, 1));
}

glm::vec2 Bone::GetAbsolutePosition() const
{
    if (parent != nullptr)
    {
        float parentRotation = parent->GetAbsoluteRotation();
        glm::vec2 temp = glm::vec2(
            cosf(parentRotation) * localPosition.x - sinf(parentRotation) * localPosition.y,
            sinf(parentRotation) * localPosition.x + cosf(parentRotation) * localPosition.y);
        return parent->GetAbsolutePosition() + temp;
    }

    return localPosition;
}

float Bone::GetAbsoluteRotation() const
{
    if (parent != nullptr)
    {
        return parent->GetAbsoluteRotation() + localRotation;
    }
    return localRotation;
}

Skeleton::Skeleton()
{
}
