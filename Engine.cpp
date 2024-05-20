#include "Engine.h"

#include <DirectXCollision.h> // 구와 광선 충돌 계산에 사용
#include <directxtk/DDSTextureLoader.h>
#include <directxtk/SimpleMath.h>
#include <tuple>
#include <vector>

#include "GraphicsCommon.h"

namespace jRenderer {

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

Engine::Engine() : AppBase() {}

bool Engine::Initialize() {

    if (!AppBase::Initialize())
        return false;

    // SkyBox texture Init
    AppBase::InitCubemaps(L"Assets/SkyBox/", L"normalSkyEnvHDR.dds",
                          L"normalSkySpecularHDR.dds",
                          L"normalSkyDiffuseHDR.dds", L"normalSkyBrdf.dds");

    // Skybox object
    {
        MeshData skyBoxMesh;
        vector<Vector3> positions;
        vector<Vector3> colors;
        vector<Vector3> normals;
        vector<Vector2> texcoords;
        float scale = 25.0f;

        //// 윗면
        positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
        positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
        positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
        positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
        colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
        colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
        colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
        colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
        normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
        normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
        normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
        normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
        texcoords.push_back(Vector2(0.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 1.0f));
        texcoords.push_back(Vector2(0.0f, 1.0f));

        // 아랫면
        positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
        positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
        positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
        positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
        colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
        colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
        colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
        colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
        normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
        normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
        normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
        normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
        texcoords.push_back(Vector2(0.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 1.0f));
        texcoords.push_back(Vector2(0.0f, 1.0f));

        // 앞면
        positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
        positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
        positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
        positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
        colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
        colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
        colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
        colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
        normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
        normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
        normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
        normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
        texcoords.push_back(Vector2(0.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 1.0f));
        texcoords.push_back(Vector2(0.0f, 1.0f));

        // 뒷면
        positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
        positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
        positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
        positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
        colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
        colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
        colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
        colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
        normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
        normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
        normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
        normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
        texcoords.push_back(Vector2(0.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 1.0f));
        texcoords.push_back(Vector2(0.0f, 1.0f));

        // 왼면
        positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
        positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
        positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
        positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
        colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
        colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
        colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
        colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
        normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
        normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
        normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
        normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
        texcoords.push_back(Vector2(0.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 1.0f));
        texcoords.push_back(Vector2(0.0f, 1.0f));

        // 오른면
        positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
        positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
        positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
        positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
        // positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
        // positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
        // positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
        // positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
        colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
        colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
        colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
        colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
        normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
        normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
        normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
        normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
        texcoords.push_back(Vector2(0.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 1.0f));
        texcoords.push_back(Vector2(0.0f, 1.0f));

        for (UINT i = 0; i < positions.size(); i++) {
            Vertex v;
            v.position = positions[i];
            v.normalModel = normals[i];
            v.texcoord = texcoords[i];
            skyBoxMesh.vertices.push_back(v);
        }

        skyBoxMesh.indices = {
            0,  1,  2,  0,  2,
            3, // 윗면
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
        std::reverse(skyBoxMesh.indices.begin(), skyBoxMesh.indices.end());
        m_skybox = make_shared<Model>(m_device, m_context, vector{skyBoxMesh});
    }

    // Main Object
    {
        auto meshes = Model::ReadFromFile("Assets/DamagedHelmet/",
                                          "DamagedHelmet.gltf", false);

        Vector3 center(0.0f, 0.5f, 0.0f);
        m_mainObj = make_shared<Model>(m_device, m_context, meshes);
        m_mainObj->m_materialConstsCPU.invertNormalMapY = true; // GLTF는 true로
        m_mainObj->m_materialConstsCPU.albedoFactor = Vector3(1.0f);
        m_mainObj->m_materialConstsCPU.roughnessFactor = 0.3f;
        m_mainObj->m_materialConstsCPU.metallicFactor = 0.8f;
        m_mainObj->UpdateWorldRow(Matrix::CreateTranslation(center));

        // 물체 감지를 위한 Bounding Sphere 생성
        m_mainBoundingSphere = BoundingSphere(center, 0.5f);

        m_basicList.push_back(m_mainObj);
    }

    // light setting
    {
        // 조명 0은 고정
        m_globalConstsCPU.lights[0].radiance = Vector3(5.0f);
        m_globalConstsCPU.lights[0].position = Vector3(0.0f, 1.2f, 0.0f);
        m_globalConstsCPU.lights[0].direction = Vector3(0.0f, -1.0f, 0.0f);
        m_globalConstsCPU.lights[0].spotPower = 3.0f;
        m_globalConstsCPU.lights[0].radius = 0.02f;
        m_globalConstsCPU.lights[0].type =
            LIGHT_SPOT | LIGHT_SHADOW; // Point with shadow

        //// 조명 1의 위치와 방향은 Update()에서 설정
        m_globalConstsCPU.lights[1].radiance = Vector3(5.0f);
        m_globalConstsCPU.lights[1].position = Vector3(1.0f, 2.2f, 0.0f);
        m_globalConstsCPU.lights[1].spotPower = 3.0f;
        m_globalConstsCPU.lights[1].fallOffEnd = 20.0f;
        m_globalConstsCPU.lights[1].radius = 0.02f;
        m_globalConstsCPU.lights[1].lightColor = Vector3(1.0f, 1.0f, 1.0f);
        m_globalConstsCPU.lights[1].type =
            LIGHT_SPOT | LIGHT_SHADOW; // Point with shadow

        // 조명 2는 꺼놓음
        m_globalConstsCPU.lights[2].type = LIGHT_OFF;
    }
    return true;
}

void Engine::Update(float dt) {
    // camera moving
    m_camera.UpdateKeyboard(dt, m_keyPressed);

    const Vector3 eyeWorld = m_camera.GetEyePos();
    // const Matrix reflectRow = Matrix::CreateReflection(m_mirrorPlane); 
    const Matrix viewRow = m_camera.GetViewRow();
    const Matrix projRow = m_camera.GetProjRow();

    // Update Global ConstantBuffer
    AppBase::UpdateGlobalConstants(eyeWorld, viewRow, projRow);

    // 조명의 위치 반영
    // for (int i = 0; i < MAX_LIGHTS; i++) {
    //    m_lightSphere[i]->UpdateWorldRow(
    //        Matrix::CreateScale(
    //            std::max(0.01f, m_globalConstsCPU.lights[i].radius)) *
    //        Matrix::CreateTranslation(m_globalConstsCPU.lights[i].position));
    //}

    static float prevRatio = 0.0f;
    static Vector3 prevPos(0.0f);
    static Vector3 prevVector(0.0f);
    q = Quaternion::CreateFromAxisAngle(Vector3(1.0f, 0.0f, 0.0f), 0.0f);
    Vector3 dragTranslation(0.0f);

    // Object Moving using Mouse
    if (m_leftButton) {
        Vector3 cursorNdcNear = Vector3(m_cursorNdcX, m_cursorNdcY, 0.0f);
        Vector3 cursorNdcFar = Vector3(m_cursorNdcX, m_cursorNdcY, 1.0f);

        Matrix invProjToView = (viewRow * projRow).Invert();

        // NDC(Proj) -> World(View)
        Vector3 cursorWorldNear =
            Vector3::Transform(cursorNdcNear, invProjToView);
        Vector3 cursorWorldFar =
            Vector3::Transform(cursorNdcFar, invProjToView);
        // cursor direction
        Vector3 dir = (cursorWorldFar - cursorWorldNear);
        dir.Normalize();

        // 광선을 만들어서 해당 광선이 닿는다면 커서가 물체랑 닿는 것을 의미.
        SimpleMath::Ray cursorRay = SimpleMath::Ray(cursorWorldNear, dir);
        float dist = 0.0f;
        m_selected = cursorRay.Intersects(m_mainBoundingSphere, dist);

        if (m_selected) {
            Vector3 pickPoint = cursorWorldNear + dist * dir;
            if (m_dragStartFlag) {
                m_dragStartFlag = false;

                prevRatio = dist / (cursorWorldFar - cursorWorldNear).Length();
                prevPos = pickPoint;
            } else {
                Vector3 newPos = cursorWorldNear +
                                 prevRatio * (cursorWorldFar - cursorWorldNear);

                if ((newPos - prevPos).Length() > 1e-3) {
                    dragTranslation = newPos - prevPos;
                    prevPos = newPos;
                }
            }
        }
    }

    // Object Roation using Mouse
    if (m_rightButton) {
        Vector3 cursorNdcNear = Vector3(m_cursorNdcX, m_cursorNdcY, 0.0f);
        Vector3 cursorNdcFar = Vector3(m_cursorNdcX, m_cursorNdcY, 1.0f);

        Matrix invProjToView = (viewRow * projRow).Invert();

        // NDC(Proj) -> World(View)
        Vector3 cursorWorldNear =
            Vector3::Transform(cursorNdcNear, invProjToView);
        Vector3 cursorWorldFar =
            Vector3::Transform(cursorNdcFar, invProjToView);
        // cursor direction
        Vector3 dir = (cursorWorldFar - cursorWorldNear);
        dir.Normalize();

        // Make Ray for checking to hand obj and mouse.
        SimpleMath::Ray cursorRay = SimpleMath::Ray(cursorWorldNear, dir);
        float dist = 0.0f;
        m_selected = cursorRay.Intersects(m_mainBoundingSphere, dist);

        if (m_selected) {
            Vector3 pickPoint = cursorWorldNear + dist * dir;
            if (m_dragStartFlag) {
                m_dragStartFlag = false;

                prevRatio = dist / (cursorWorldFar - cursorWorldNear).Length();
                prevVector = pickPoint - m_mainBoundingSphere.Center;
                prevVector.Normalize();
            } else {
                Vector3 newVector = pickPoint - m_mainBoundingSphere.Center;
                newVector.Normalize();

                float angle = std::acos(newVector.Dot(prevVector)) *
                              (180.0 / 3.141592) * 0.1f;
                Vector3 axis = prevVector.Cross(newVector);
                axis.Normalize();
                if ((newVector - prevVector).Length() > 1e-3) {
                    q = SimpleMath::Quaternion::CreateFromAxisAngle(axis,
                                                                    angle);
                    prevVector = newVector;
                }
            }
        }
    }

    Vector3 transition = m_mainObj->m_worldRow.Translation();
    m_mainObj->m_worldRow.Translation(Vector3(0.0f));
    m_mainObj->UpdateWorldRow(
        m_mainObj->m_worldRow * Matrix::CreateFromQuaternion(q) *
        Matrix::CreateTranslation(dragTranslation + transition));
    m_mainBoundingSphere.Center = m_mainObj->m_worldRow.Translation();

    for (auto &i : m_basicList) {
        i->UpdateConstantBuffers(m_device, m_context);
    }
}

void Engine::Render() {
    AppBase::SetMainViewport();

    m_context->VSSetSamplers(0, UINT(Graphics::sampleStates.size()),
                             Graphics::sampleStates.data());
    m_context->PSSetSamplers(0, UINT(Graphics::sampleStates.size()),
                             Graphics::sampleStates.data());

    // for cubemap texture
    vector<ID3D11ShaderResourceView *> commonSRVs = {
        m_specularSRV.Get(), m_irradianceSRV.Get(), m_envSRV.Get(),
        m_brdfSRV.Get()};
    m_context->PSSetShaderResources(10, UINT(commonSRVs.size()),
                                    commonSRVs.data());

    const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    vector<ID3D11RenderTargetView *> RTVs = {m_backBufferRTV.Get()};
    for (size_t i = 0; i < RTVs.size(); i++) {
        m_context->ClearRenderTargetView(RTVs[i], clearColor);
    }
    m_context->ClearDepthStencilView(m_depthStencilView.Get(),
                                     D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                     1.0f, 0);
    m_context->OMSetRenderTargets(UINT(RTVs.size()), RTVs.data(),
                                  m_depthStencilView.Get());

    AppBase::SetGlobalConsts(m_globalConstsGPU);

    // skybox
    AppBase::SetPipelineState(Graphics::skyboxSolidPSO);
    //m_skybox->Render(m_context);

    AppBase::SetPipelineState(Graphics::defaultSolidPSO);

    for (auto &i : m_basicList) {
        i->Render(m_context);
    }
}

void Engine::UpdateGUI() {
    ImGui::SetNextItemOpen(false, ImGuiCond_Once);
    if (ImGui::TreeNode("General")) {
        ImGui::Checkbox("Use FPV", &m_camera.m_useFirstPersonView);
        ImGui::Checkbox("Wireframe", &m_drawAsWire);
        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("obj1")) {
        // Move
        Vector3 transition = m_mainObj->m_worldRow.Translation();
        m_mainObj->m_worldRow.Translation(Vector3(0.0f));
        ImGui::SliderFloat3("Position", &transition.x, -5.0f, 5.0f);

        // Rotation
        ImGui::SliderFloat3("Roation", &rotationGUI.x, -1.0f, 1.0f);

        m_mainObj->UpdateWorldRow(m_mainObj->m_worldRow *
                                  Matrix::CreateRotationY(rotationGUI.y) *
                                  Matrix::CreateRotationX(-rotationGUI.x) *
                                  Matrix::CreateRotationZ(rotationGUI.z) *
                                  Matrix::CreateTranslation(transition));
        m_mainBoundingSphere.Center = m_mainObj->m_worldRow.Translation();

        int flag = 0;
        flag += ImGui::CheckboxFlags("Normal Map",
                             &m_mainObj->m_materialConstsCPU.useNormalMap, 1);

        if (flag) {
            m_mainObj->UpdateConstantBuffers(m_device, m_context);
        }

        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Light0")) {
        ImGui::SliderFloat3("Position", &m_globalConstsCPU.lights[0].position.x,
                            -1.0f, 1.0f);
        ImGui::SliderFloat("Spot Power", &m_globalConstsCPU.lights[0].spotPower,
                           0.0f, 32.0f);
        ImGui::ColorEdit3("Color", &m_globalConstsCPU.lights[0].lightColor.x,
                          0);

        ImGui::TreePop();
    }

    // ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    // if (ImGui::TreeNode("Light")) {
    //     // ImGui::SliderFloat3("Position",
    //     // &m_globalConstsCPU.lights[0].position.x,
    //     //                     -5.0f, 5.0f);
    //     ImGui::SliderFloat("Radius", &m_globalConstsCPU.lights[1].radius,
    //     0.0f,
    //                        0.1f);
    //     ImGui::TreePop();
    // }
}

} // namespace jRenderer