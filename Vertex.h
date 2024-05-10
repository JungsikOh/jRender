#pragma once

#include <directxtk/SimpleMath.h>
#include <vector>

namespace jRenderer {

using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;

struct Vertex {
    Vector3 position;
    Vector3 normalModel;
    Vector2 texcoord;
    Vector3 tangentModel;
};

} // namespace jRenderer