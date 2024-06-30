#pragma once
#include <directxtk/SimpleMath.h>
#include <string>
#include <vector>

#include "MeshData.h"
#include "Vertex.h"
#include "D3D11Utils.h"

namespace jRenderer {
using DirectX::SimpleMath::Vector2;

class GeometryGenerator {
  public:
    static MeshData MakeSquare(const float scale = 1.0f, const Vector2 texScale = Vector2(1.0f));
    static MeshData MakeBox(const float scale = 1.0f, bool invertNormal = false);
    static MeshData MakeSphere(const float radius, const int numSlices,
                               const int numStacks,
                               const Vector2 texScale = Vector2(1.0f));
};
} // namespace jRenderer