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

    // Main Object
    {
        auto meshes = Model::ReadFromFile("Assets/DamagedHelmet/",
                                          "DamagedHelmet.gltf", true);

        Vector3 center(0.0f, 1.0f, 0.0f);
        m_mainObj = make_shared<Model>(m_device, m_context, meshes);
        m_mainObj->m_materialConstsCPU.invertNormalMapY = true; // GLTF는 true로
        m_mainObj->m_materialConstsCPU.albedoFactor = Vector3(0.2f, 1.0f, 0.5f);
        m_mainObj->m_materialConstsCPU.roughnessFactor = 0.3f;
        m_mainObj->m_materialConstsCPU.metallicFactor = 0.8f;
        m_mainObj->UpdateWorldRow(Matrix::CreateTranslation(center));

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
        m_globalConstsCPU.lights[1].lightColor = Vector3(1.0f, 0.1f, 0.1f);
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

    for (auto &i : m_basicList) {
        i->UpdateConstantBuffers(m_device, m_context);
    }
}

void Engine::Render() {
    AppBase::SetMainViewport();

    // m_context->VSSetSamplers(0, UINT(Graphics::sampleStates.size()),
    //                          Graphics::sampleStates.data());
    // m_context->PSSetSamplers(0, UINT(Graphics::sampleStates.size()),
    //                          Graphics::sampleStates.data());

    // for cubemap texture
    // vector<ID3D11ShaderResourceView *> commonSRVs = {m_brdfSRV.Get()};
    // m_context->PSSetShaderResources(10,
    //                                UINT(commonSRVs.size(),
    //                                commonSRVs.data()));

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

    AppBase::SetPipelineState(Graphics::defaultSolidPSO);

    AppBase::SetGlobalConsts(m_globalConstsGPU);

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
    if (ImGui::TreeNode("Frag")) {
        ImGui::SliderFloat3("Position", &translationGUI.x, -5.0f, 5.0f);
        m_mainObj->UpdateWorldRow(Matrix::CreateTranslation(translationGUI));
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