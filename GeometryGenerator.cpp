#include "GeometryGenerator.h"

namespace jRenderer {

using namespace DirectX;
using namespace DirectX::SimpleMath;

MeshData GeometryGenerator::MakeSquare(const float scale,
                                       const Vector2 texScale) {

    MeshData meshData;

    Vertex v;
    vector<Vector3> positions;
    vector<Vector3> colors;
    vector<Vector3> normals;
    vector<Vector2> texcoords;

    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));

    positions.push_back(Vector3(-1.0f, 1.0f, 0.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, 0.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, 0.0f) * scale);
    positions.push_back(Vector3(-1.0f, -1.0f, 0.0f) * scale);

    // Texture Coordinates (Direct3D 9)
    // https://learn.microsoft.com/en-us/windows/win32/direct3d9/texture-coordinates
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    meshData.indices = {0, 1, 2, 0, 2, 3};
    for (UINT i = 0; i < positions.size(); i++) {
        Vertex v;
        v.position = positions[i];
        v.normalModel = normals[i];
        v.texcoord = texcoords[i];
        v.tangentModel = Vector3(1.0f, 0.0f, 0.0f);

        meshData.vertices.push_back(v);
    }

    return meshData;
}

MeshData GeometryGenerator::MakeBox(const float scale, bool invertNormal) {

    MeshData meshData;

    vector<Vector3> positions;
    vector<Vector3> colors;
    vector<Vector3> normals;
    vector<Vector2> texcoords; // 텍스춰 좌표
    vector<Vector3> tangents;

    Vector3 edge1;
    Vector3 edge2;
    Vector2 deltaTex1;
    Vector2 deltaTex2;
    Vector3 tangent;
    float flag = 1.0f;

    if (invertNormal)
        flag = -1.0f;
    // 윗면
    positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
    colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f) * flag);
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f) * flag);
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f) * flag);
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f) * flag);
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    edge1 = Vector3(-1.0f, 1.0f, 1.0f) - Vector3(1.0f, 1.0f, -1.0f);
    edge2 = Vector3(1.0f, 1.0f, 1.0f) - Vector3(1.0f, 1.0f, -1.0f);
    deltaTex1 = Vector2(1.0f, 0.0f) - Vector2(0.0f, 0.0f);
    deltaTex2 = Vector2(1.0f, 1.0f) - Vector2(0.0f, 0.0f);
    tangent = D3D11Utils::GetTangent(edge1, edge2, deltaTex1, deltaTex2);
    for (int i = 0; i < 4; i++) {
        tangents.push_back(tangent);
    }

    // 아랫면
    positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
    colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f) * flag);
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f) * flag);
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f) * flag);
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f) * flag);
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    edge1 = Vector3(1.0f, -1.0f, -1.0f) - Vector3(-1.0f, -1.0f, -1.0f);
    edge2 = Vector3(1.0f, -1.0f, 1.0f) - Vector3(-1.0f, -1.0f, -1.0f);
    deltaTex1 = Vector2(1.0f, 0.0f) - Vector2(0.0f, 0.0f);
    deltaTex2 = Vector2(1.0f, 1.0f) - Vector2(0.0f, 0.0f);
    tangent = D3D11Utils::GetTangent(edge1, edge2, deltaTex1, deltaTex2);
    for (int i = 0; i < 4; i++) {
        tangents.push_back(tangent);
    }

    // 앞면
    positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f) * flag);
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f) * flag);
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f) * flag);
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f) * flag);
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    edge1 = Vector3(-1.0f, 1.0f, -1.0f) - Vector3(-1.0f, -1.0f, -1.0f);
    edge2 = Vector3(1.0f, 1.0f, -1.0f) - Vector3(-1.0f, -1.0f, -1.0f);
    deltaTex1 = Vector2(1.0f, 0.0f) - Vector2(0.0f, 0.0f);
    deltaTex2 = Vector2(1.0f, 1.0f) - Vector2(0.0f, 0.0f);
    tangent = D3D11Utils::GetTangent(edge1, edge2, deltaTex1, deltaTex2);
    for (int i = 0; i < 4; i++) {
        tangents.push_back(tangent);
    }

    // 뒷면
    positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
    colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f) * flag);
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f) * flag);
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f) * flag);
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f) * flag);
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    edge1 = Vector3(1.0f, -1.0f, 1.0f) - Vector3(-1.0f, -1.0f, 1.0f);
    edge2 = Vector3(1.0f, 1.0f, 1.0f) - Vector3(-1.0f, -1.0f, 1.0f);
    deltaTex1 = Vector2(1.0f, 0.0f) - Vector2(0.0f, 0.0f);
    deltaTex2 = Vector2(1.0f, 1.0f) - Vector2(0.0f, 0.0f);
    tangent = D3D11Utils::GetTangent(edge1, edge2, deltaTex1, deltaTex2);
    for (int i = 0; i < 4; i++) {
        tangents.push_back(tangent);
    }

    // 왼쪽
    positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
    colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f) * flag);
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f) * flag);
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f) * flag);
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f) * flag);
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    edge1 = Vector3(-1.0f, 1.0f, 1.0f) - Vector3(-1.0f, -1.0f, 1.0f);
    edge2 = Vector3(-1.0f, 1.0f, -1.0f) - Vector3(-1.0f, -1.0f, 1.0f);
    deltaTex1 = Vector2(1.0f, 0.0f) - Vector2(0.0f, 0.0f);
    deltaTex2 = Vector2(1.0f, 1.0f) - Vector2(0.0f, 0.0f);
    tangent = D3D11Utils::GetTangent(edge1, edge2, deltaTex1, deltaTex2);
    for (int i = 0; i < 4; i++) {
        tangents.push_back(tangent);
    }

    // 오른쪽
    positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
    colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f) * flag);
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f) * flag);
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f) * flag);
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f) * flag);
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    edge1 = Vector3(1.0f, -1.0f, -1.0f) - Vector3(1.0f, -1.0f, 1.0f);
    edge2 = Vector3(1.0f, 1.0f, -1.0f) - Vector3(1.0f, -1.0f, 1.0f);
    deltaTex1 = Vector2(1.0f, 0.0f) - Vector2(0.0f, 0.0f);
    deltaTex2 = Vector2(1.0f, 1.0f) - Vector2(0.0f, 0.0f);
    tangent = D3D11Utils::GetTangent(edge1, edge2, deltaTex1, deltaTex2);
    for (int i = 0; i < 4; i++) {
        tangents.push_back(tangent);
    }

    for (UINT i = 0; i < positions.size(); i++) {
        Vertex v;
        v.position = positions[i];
        v.normalModel = normals[i];
        v.texcoord = texcoords[i];
        v.tangentModel = tangents[i];

        meshData.vertices.push_back(v);
    }

    meshData.indices = {
        0,  1,  2,  0,  2,  3, // 윗면
        4,  5,  6,  4,  6,
        7, // 아랫면
        8,  9,  10, 8,  10,
        11, // 앞면
        12, 13, 14, 12, 14,
        15, // 뒷면
        16, 17, 18, 16, 18,
        19, // 왼쪽면
        20, 21, 22, 20, 22,
        23 // 오른면
    };

    return meshData;
}

MeshData GeometryGenerator::MakeSphere(const float radius, const int numSlices,
                                       const int numStacks,
                                       const Vector2 texScale) {
    const float dTheta = -XM_2PI / float(numSlices);
    const float dPhi = -XM_PI / float(numStacks);

    MeshData meshData;

    std::vector<Vertex> &vertices = meshData.vertices;

    for (size_t j = 0; j <= numStacks; j++) {

        // stack에 쌓일 수록 점점회전시켜서 완성하는 구조
        Vector3 stackStartPoint = Vector3::Transform(
            Vector3(0.0f, -radius, 0.0f), Matrix::CreateRotationZ(dPhi * j));

        for (size_t i = 0; i <= numSlices; i++) {
            Vertex v;

            v.position = Vector3::Transform(
                stackStartPoint, Matrix::CreateRotationY(dTheta * float(i)));

            v.normalModel = v.position; // 원점이 구의 중심
            v.normalModel.Normalize();

            v.texcoord =
                Vector2(float(i) / numSlices, 1.0f - float(j) / numStacks) *
                texScale;

            // texcoord가 위로 갈수록 증가
            Vector3 biTangent = Vector3(0.0f, 1.0f, 0.0f);

            // normalOrth의 도출
            // n - proj(n)의 공식을 코드로 바꾼 것이다.
            Vector3 normalOrth =
                v.normalModel - biTangent.Dot(v.normalModel) * v.normalModel;
            normalOrth.Normalize();

            v.tangentModel = biTangent.Cross(normalOrth);
            v.tangentModel.Normalize();

            vertices.push_back(v);
        }
    }

    vector<uint32_t> &indices = meshData.indices;

    for (int j = 0; j < numStacks; j++) {

        const int offset = (numSlices + 1) * j;

        for (int i = 0; i < numSlices; i++) {
            indices.push_back(offset + i);
            indices.push_back(offset + i + numSlices + 1);
            indices.push_back(offset + i + 1 + numSlices + 1);

            indices.push_back(offset + i);
            indices.push_back(offset + i + 1 + numSlices + 1);
            indices.push_back(offset + i + 1);
        }
    }
    return meshData;
}

} // namespace jRenderer