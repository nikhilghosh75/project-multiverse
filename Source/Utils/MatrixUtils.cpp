#include "MatrixUtils.h"

glm::mat3 Matrix::TRS2D(glm::vec2 translation, float rotation, glm::vec2 scale)
{
    float cosTheta = cos(rotation);
    float sinTheta = sin(rotation);

    glm::mat3 transform;

    transform[0][0] = cosTheta * scale.x;
    transform[0][1] = -sinTheta * scale.y;
    transform[0][2] = translation.x;

    transform[1][0] = sinTheta * scale.x;
    transform[1][1] = cosTheta * scale.y;
    transform[1][2] = translation.y;

    transform[2][0] = 0.0f;
    transform[2][1] = 0.0f;
    transform[2][2] = 1.0f;

    return transform;
}
