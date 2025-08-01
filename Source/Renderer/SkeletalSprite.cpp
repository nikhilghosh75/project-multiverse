#include "SkeletalSprite.h"

#include "MatrixUtils.h"

glm::mat4 Bone::GetLocalTransform() const
{
    return Matrix::TRS2D(localPosition, localRotation, glm::vec2(1, 1));
}

glm::mat4 Bone::GetAbsoluteTransform() const
{
    if (parent == nullptr)
    {
        return GetLocalTransform();
    }
    
    return parent->GetAbsoluteTransform() * GetLocalTransform();
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

SkeletalSprite::SkeletalSprite()
{
    texture = nullptr;
}

void SkeletalSprite::SkinVertices()
{
    for (Layer& layer : layers)
    {
        glm::vec2 layerOffset = glm::vec2(-layer.center.x + layer.width * 0.5f, layer.center.y + layer.height * 0.5f);
        for (SpriteVertex& vertex : layer.vertices)
        {
            glm::vec4 finalPosition = glm::vec4(0, 0, 0, 0);
            for (int i = 0; i < 4; i++)
            {
                if (vertex.weights[i].boneWeight > 0)
                {
                    Bone& bone = skeleton.bones[vertex.weights[i].boneIndex];
                    glm::mat4 transform = bone.GetAbsoluteTransform() * bone.inverseBindPose;
                    finalPosition += transform * glm::vec4(vertex.position - layerOffset, 0, 1) * vertex.weights[i].boneWeight;
                }
            }

            vertex.skinnedPosition = glm::vec2(finalPosition.x, finalPosition.y);
        }
    }
}
