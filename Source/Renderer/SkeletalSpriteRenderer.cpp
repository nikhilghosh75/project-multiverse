#include "SkeletalSpriteRenderer.h"

#include "MatrixUtils.h"

glm::mat4 Bone::GetLocalTransform() const
{
    return Matrix::TRS2D(localPosition, localRotation, glm::vec2(1, 1));
}

Skeleton::Skeleton()
{
}
