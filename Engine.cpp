#include "Engine.h"

#include <DirectXCollision.h> // 구와 광선 충돌 계산에 사용
#include <directxtk/DDSTextureLoader.h>
#include <directxtk/SimpleMath.h>
#include <random>
#include <tuple>
#include <vector>

#include "GeometryGenerator.h"
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
    AppBase::InitCubemaps(L"Assets/CubeMap/", L"blueroomEnvHDR.dds",
                          L"blueroomSpecularHDR.dds", L"blueroomDiffuseHDR.dds",
                          L"blueroomBrdf.dds");

    // 후처리용 박스
    {
        MeshData screenBox = GeometryGenerator::MakeSquare();
        m_screenSquare =
            make_shared<Model>(m_device, m_context, vector{screenBox}, 0);
    }

    // 렌더 패스 그리기용 박스
    {
        MeshData screenSquare = GeometryGenerator::MakeSquare(0.2f);
        for (int i = 0; i < 4; i++) {
            m_screenRenderPass[i] = make_shared<Model>(m_device, m_context,
                                                       vector{screenSquare}, 0);
            m_screenRenderPass[i]->UpdateWorldRow(Matrix::CreateTranslation(
                Vector3(-0.75f, 0.7f - (0.4f * (float)i), 0.0f)));
        }

        for (int i = 0; i < 4; i++) {
            m_screenRenderPass[i]->UpdateConstantBuffers(m_device, m_context);
        }
    }

    // Skybox object
    {
        MeshData skyBoxMesh = GeometryGenerator::MakeBox(25.0f);
        std::reverse(skyBoxMesh.indices.begin(), skyBoxMesh.indices.end());
        m_skybox =
            make_shared<Model>(m_device, m_context, vector{skyBoxMesh}, 0);

        m_skybox->m_materialConstsCPU.albedoFactor = Vector3(1.0f);
        m_skybox->m_materialConstsCPU.roughnessFactor = 0.3f;
        m_skybox->m_materialConstsCPU.metallicFactor = 0.8f;
        m_skybox->UpdateWorldRow(
            Matrix::CreateTranslation(Vector3(0.0f, 0.0f, 0.0f)));
    }

    // 바닥 오브젝트
    {
        MeshData box = GeometryGenerator::MakeBox(2.0f, false);
        MeshData ground = GeometryGenerator::MakeSquare(4.0f);
        ground.albedoTextureFilename =
            "Assets/Bricks075A/Bricks075A_1K-JPG_Color.jpg";
        ground.normalTextureFilename =
            "Assets/Bricks075A/Bricks075A_1K-JPG_NormalDX.jpg";

        m_ground[0] =
            make_shared<Model>(m_device, m_context, vector{ground}, 0);
        m_ground[0]->UpdateWorldRow(
            Matrix::CreateRotationX(1.0f / 2.0f * 3.141592f) *
            Matrix::CreateTranslation(Vector3(0.0f, -2.5f, 0.0f)));
        m_ground[0]->m_materialConstsCPU.albedoFactor =
            Vector3(0.4f, 0.5f, 0.2f);
                                      
        m_basicList.push_back(m_ground[0]);

        m_ground[1] = make_shared<Model>(m_device, m_context, vector{box}, 0);
        m_ground[1]->m_materialConstsCPU.roughnessFactor = 0.3f;
        m_ground[1]->m_materialConstsCPU.metallicFactor = 0.8f;
        m_ground[1]->UpdateWorldRow(
            Matrix::CreateTranslation(Vector3(0.0f, 0.0f, 5.0f)));
        m_ground[1]->m_materialConstsCPU.albedoFactor =
            Vector3(0.1f, 0.1f, 0.3f);

        m_basicList.push_back(m_ground[1]);

        m_ground[2] = make_shared<Model>(m_device, m_context, vector{box}, 0);
        m_ground[2]->UpdateWorldRow(
            Matrix::CreateTranslation(Vector3(5.0f, 0.0f, 0.0f)));
        m_ground[2]->m_materialConstsCPU.albedoFactor =
            Vector3(0.8f, 0.1f, 0.3f);

        m_basicList.push_back(m_ground[2]);
    }

    // Main Object
    {
        auto meshes = Model::ReadFromFile("Assets/DamagedHelmet/",
                                          "DamagedHelmet.gltf", false);

        // Vector3 center(0.0f, 0.5f, 0.0f);
        // m_mainObj = make_shared<Model>(m_device, m_context, meshes);
        /*auto meshes = GeometryGenerator::MakeBox(0.2f);*/

        Vector3 center(0.0f, 0.5f, 1.0f);
        m_mainObj = make_shared<Model>(m_device, m_context, vector{meshes}, 0);

        m_mainObj->m_materialConstsCPU.invertNormalMapY = true; // GLTF는 true로
        m_mainObj->m_materialConstsCPU.albedoFactor = Vector3(0.9f, 0.2f, 0.2f);
        m_mainObj->m_materialConstsCPU.roughnessFactor = 0.3f;
        m_mainObj->m_materialConstsCPU.metallicFactor = 0.8f;
        m_mainObj->UpdateWorldRow(Matrix::CreateTranslation(center));
        m_mainObj->m_castShadow = true;

        for (int i = 0; i < 1; i++) {
            m_mainObj->m_instancedConstsCPU.instanceMat[i] =
                Vector3(-1.0f * 0.8f * (float)i, 1.0f * 0.8f * (float)i,
                        1.0f * 0.5f * (float)i);
        }

        // 물체 감지를 위한 Bounding Sphere 생성
        m_mainBoundingSphere = BoundingSphere(center, 0.5f);
        m_mainObj->UpdateConstantBuffers(m_device, m_context);

        m_basicList.push_back(m_mainObj);
    }

    // 구 obj
    {
        /*auto meshes = Model::ReadFromFile("Assets/MetalRoughSpheres/",
                                          "MetalRoughSpheresNoTextures.gltf",
           false);   */

        auto meshes = GeometryGenerator::MakeSphere(0.3f, 50, 50);
        //meshes.albedoTextureFilename =
        //    "Assets/rustediron/rustediron2_basecolor.png";
        //meshes.roughnessTextureFilename =
        //    "Assets/rustediron/rustediron2_roughness.png";
        //meshes.metallicTextureFilename =
        //    "Assets/rustediron/rustediron2_metallic.png";
        //meshes.normalTextureFilename =
        //    "Assets/rustediron/rustediron2_normal.png";

        Vector3 center(-3.5f, 0.5f, 0.0f);
        m_boxObj = make_shared<Model>(m_device, m_context, vector{meshes}, 0);
        m_boxObj->m_materialConstsCPU.albedoFactor = Vector3(0.8f);
        m_boxObj->m_materialConstsCPU.roughnessFactor = 1.0f;
        m_boxObj->m_materialConstsCPU.metallicFactor = 0.2f;
        m_boxObj->UpdateWorldRow(
            Matrix::CreateScale(5.0f) *
            Matrix::CreateRotationY(1.0f / 2.0f * 3.141592f) *
            Matrix::CreateTranslation(center));
        m_boxObj->m_materialConstsCPU.invertNormalMapY = false; // GLTF는 true로

        m_boxObj->UpdateConstantBuffers(m_device, m_context);

        m_basicList.push_back(m_boxObj);
    }

    // light setting
    {
        // 조명 0은 고정
        m_globalConstsCPU.lights[0].radiance = Vector3(5.0f);
        m_globalConstsCPU.lights[0].position = Vector3(0.0f, 2.0f, 0.0f);
        m_globalConstsCPU.lights[0].direction = Vector3(0.0f, -1.0f, 0.0f);
        m_globalConstsCPU.lights[0].spotPower = 1.0f;
        m_globalConstsCPU.lights[0].radius = 0.02f;
        m_globalConstsCPU.lights[0].lightColor = Vector3(1.0f, 1.0f, 1.0f);
        m_globalConstsCPU.lights[0].type =
            LIGHT_SPOT | LIGHT_SHADOW; // Point with shadow

        //// 조명 1의 위치와 방향은 Update()에서 설정
        m_globalConstsCPU.lights[1].radiance = Vector3(5.0f);
        m_globalConstsCPU.lights[1].spotPower = 1.0f;
        m_globalConstsCPU.lights[1].fallOffEnd = 20.0f;
        m_globalConstsCPU.lights[1].position = Vector3(-0.5f, 1.2f, 0.0f);
        m_globalConstsCPU.lights[1].direction = Vector3(0.5f, -1.5f, 0.0f);
        m_globalConstsCPU.lights[1].lightColor = Vector3(0.5f, 1.0f, 1.0f);
        m_globalConstsCPU.lights[1].type =
            LIGHT_DIRECTIONAL | LIGHT_SHADOW; // Point with shadow

        // 조명 2는 꺼놓음
        m_globalConstsCPU.lights[2].radiance = Vector3(5.0f);
        m_globalConstsCPU.lights[2].position = Vector3(-1.3f, -0.4f, -1.0f);
        m_globalConstsCPU.lights[2].spotPower = 1.0f;
        m_globalConstsCPU.lights[2].radius = 0.02f;
        m_globalConstsCPU.lights[2].lightColor = Vector3(1.0f, 1.0f, 1.0f);
        m_globalConstsCPU.lights[2].type =
            LIGHT_POINT | LIGHT_SHADOW; // Point with shadow
    }
    // 조명 그리기
    {
        for (int i = 0; i < MAX_LIGHTS; i++) {
            MeshData sphere = GeometryGenerator::MakeSphere(1.0f, 20, 20);
            m_lightSphere[i] =
                make_shared<Model>(m_device, m_context, vector{sphere}, 0);
            m_lightSphere[i]->UpdateWorldRow(Matrix::CreateTranslation(
                m_globalConstsCPU.lights[i].position));
            m_lightSphere[i]->m_materialConstsCPU.albedoFactor =
                m_globalConstsCPU.lights[i].lightColor;
            m_lightSphere[i]->m_materialConstsCPU.emissionFactor =
                Vector3(1.0f, 0.0f, 0.0f);
            m_lightSphere[i]->m_castShadow = false;

            if (m_globalConstsCPU.lights[i].type == LIGHT_OFF)
                m_lightSphere[i]->m_isVisible = false;

            m_basicList.push_back(m_lightSphere[i]);
        }
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

    // 조명 시점 그리기
    for (int i = 0; i < MAX_LIGHTS; i++) {
        const auto &light = m_globalConstsCPU.lights[i];
        if (light.type & LIGHT_SHADOW) {

            Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
            // 만약 빛의 방향과 upDir의 dot 연산이 -1에 가깝다면, 그것은 둘의
            // 사이각이 180도에 가깝다는 이야기로, upDir의 방향을
            // 수정해줘야한다.
            if (abs(up.Dot(light.direction) + 1.0f) < 1e-5)
                up = Vector3(1.0f, 0.0f, 0.0f);

            // https://learn.microsoft.com/ko-kr/windows/win32/api/directxmath/nf-directxmath-xmmatrixperspectivefovlh
            Matrix lightProjRow = XMMatrixPerspectiveFovLH(
                XMConvertToRadians(120.0f), 1.0f, 0.01f, 25.0f);
            // Matrix lightProjRow = XMMatrixOrthographicOffCenterLH(
            //         -10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 100.0f);
            //  lightProjRow =
            //  XMMatrixOrthographicLH(20.0f, 20.0f, 1.0f, 7.5f);

            // https://learn.microsoft.com/ko-kr/windows/win32/api/directxmath/nf-directxmath-xmmatrixlookatlh
            Vector3 targetVec = (light.position + light.direction);
            targetVec.Normalize();
            Matrix lightViewRow =
                XMMatrixLookAtLH(light.position, targetVec, up);
                  
            m_shadowGlobalConstsCPU[i].eyeWorld = light.position;
            m_shadowGlobalConstsCPU[i].view = lightViewRow.Transpose();
            m_shadowGlobalConstsCPU[i].proj = lightProjRow.Transpose();
            m_shadowGlobalConstsCPU[i].invProj = 
                lightProjRow.Invert().Transpose();
            m_shadowGlobalConstsCPU[i].viewProj =
                (lightViewRow * lightProjRow).Transpose();                 

            if (light.type & LIGHT_POINT) {
                Matrix pointLightProjRow = XMMatrixPerspectiveFovLH(
                    XMConvertToRadians(90.0f), 1.0f, 1.0f, 50.0f);
                Vector3 directions[6] = {
                    {1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f},
                    {0.0f, 1.0f, 0.0f}, {0.0f, -1.0f, 0.0f},
                    {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}};
                Vector3 upDir[6] = {{0.0f, 1.0f, 0.0f},  {0.0f, 1.0f, 0.0f},
                                    {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 1.0f},
                                    {0.0f, 1.0f, 1.0f},  {0.0f, 1.0f, 0.0f}};

                for (int face = 0; face < 6; ++face) {
                    lightViewRow = XMMatrixLookAtLH(
                        light.position, light.position + directions[face],
                        upDir[face]);
                    m_pointLightTransformCPU[i].shadowViewProj[face] =
                        (lightViewRow * pointLightProjRow).Transpose();
                }
                D3D11Utils::UpdateBuffer(m_device, m_context,
                                         m_pointLightTransformCPU[i],
                                         m_pointLightTransformGPU[i]);
            }
            // for (int x = 0; x < 4; x++) // loop 3 times for three lines
            //{
            //     for (int y = 0; y < 4;
            //          y++) // loop for the three elements on the line
            //     {
            //         Matrix temp =
            //         m_pointLightTransformCPU[2].shadowViewProj[1]; cout <<
            //         temp.m[x][y] << " "; // display the
            //                                      // current element
            //                                      // out of
            //                                      // the array
            //     }
            //     cout << endl; // when the inner loop is done, go to a new
            //     line
            // }

            D3D11Utils::UpdateBuffer(m_device, m_context,
                                     m_shadowGlobalConstsCPU[i],
                                     m_shadowGlobalConstsGPU[i]);

            // 그림자를 실제로 렌더링할 때 필요
            m_globalConstsCPU.lights[i].viewProj =
                m_shadowGlobalConstsCPU[i].viewProj;
            m_globalConstsCPU.lights[i].invProj =
                m_shadowGlobalConstsCPU[i].invProj;
        }
    }

    // 조명의 위치 반영
    for (int i = 0; i < MAX_LIGHTS; i++) {
        m_lightSphere[i]->UpdateWorldRow(
            Matrix::CreateScale(
                std::max(0.01f, m_globalConstsCPU.lights[i].radius)) *
            Matrix::CreateTranslation(m_globalConstsCPU.lights[i].position));
    }

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
    const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    // for cubemap texture
    vector<ID3D11ShaderResourceView *> commonSRVs = {
        m_specularSRV.Get(), m_irradianceSRV.Get(), m_envSRV.Get(),
        m_brdfSRV.Get()};
    m_context->PSSetShaderResources(10, UINT(commonSRVs.size()),
                                    commonSRVs.data());
    AppBase::SetGlobalConsts(m_globalConstsGPU);       

    vector<ID3D11RenderTargetView *> RTVs = {m_resolvedRTV.Get()};

    // Cubemap을 위한 stencil 모양 찍기 및 나머지 부분에다가 큐브맵 그리기
    m_context->ClearDepthStencilView(m_depthStencilView.Get(),
                                     D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                     1.0f, 0);
    m_context->OMSetRenderTargets(0, NULL, m_depthStencilView.Get());
    AppBase::SetPipelineState(Graphics::stencilMaskPSO);
    for (auto &i : m_basicList) {
        i->Render(m_context);
    }   
    m_context->ClearRenderTargetView(m_cubeMapRTV.Get(), clearColor);
    m_context->OMSetRenderTargets(1, m_cubeMapRTV.GetAddressOf(),
                                  m_depthStencilView.Get());
    AppBase::SetPipelineState(Graphics::reflectSolidPSO);
    m_skybox->Render(m_context);
     
    // deferred lighting을 위한 G-Buffer 생성
    if (true) {
        AppBase::SetPipelineState(Graphics::gBufferPSO);
        m_gBuffer.PreRender(m_context);
        for (auto &i : m_basicList) {
            i->Render(m_context);
        }
    } 

    vector<ID3D11ShaderResourceView *> deferredLightingSRVs = {
        m_gBuffer.GetColorView(), m_gBuffer.GetNormalView(), 
        m_gBuffer.GetSpecPowerView(), m_gBuffer.GetDepthView()};

    // 1. SSAO texture 만들기
    m_context->ClearDepthStencilView(m_depthStencilView.Get(),
                                     D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                     1.0f, 0);
    m_context->ClearRenderTargetView(m_ssaoRTV.Get(), clearColor);
    m_context->OMSetRenderTargets(1, m_ssaoRTV.GetAddressOf(),
                                  m_depthStencilView.Get());
    AppBase::SetPipelineState(Graphics::ssaoPSO); 
    m_context->PSGetConstantBuffers(2, 1, m_kernelSamplesGPU.GetAddressOf());
    m_context->PSSetShaderResources(5, UINT(deferredLightingSRVs.size()),
                                    deferredLightingSRVs.data());
    m_context->PSSetShaderResources(9, 1, m_ssaoNoiseSRV.GetAddressOf());
    m_screenSquare->Render(m_context);

    // 2. SSAO texture Blur
    m_context->ClearDepthStencilView(m_depthStencilView.Get(),
                                     D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                     1.0f, 0);
    m_context->ClearRenderTargetView(m_ssaoBlurRTV.Get(), clearColor);
    m_context->OMSetRenderTargets(1, m_ssaoBlurRTV.GetAddressOf(),
                                  m_depthStencilView.Get());
    AppBase::SetPipelineState(Graphics::ssaoBlurPSO);
    m_context->PSSetShaderResources(5, 1, m_ssaoSRV.GetAddressOf());
    m_screenSquare->Render(m_context); 

    // AmbientEmission Pass
    AppBase::SetPipelineState(Graphics::ambientEmissionPSO);
    m_context->ClearRenderTargetView(m_resolvedRTV.Get(), clearColor);
    m_context->ClearDepthStencilView(m_depthStencilView.Get(),
                                     D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                     1.0f, 0);
    m_context->OMSetRenderTargets(1, m_resolvedRTV.GetAddressOf(),
                                  m_depthStencilView.Get());
    m_context->PSSetShaderResources(5, UINT(deferredLightingSRVs.size()),
                                    deferredLightingSRVs.data());
    m_context->PSSetShaderResources(9, 1, m_ssaoBlurSRV.GetAddressOf());
    m_screenSquare->Render(m_context);

    // deferred lighting 
    AppBase::SetPipelineState(Graphics::deferredLightingPSO);
    m_context->ClearDepthStencilView(m_depthStencilView.Get(), 
                                     D3D11_CLEAR_DEPTH,
                                     1.0f, 0);
    m_context->OMSetRenderTargets(1, m_resolvedRTV.GetAddressOf(),
                                  m_depthStencilView.Get());
    m_context->PSSetShaderResources(5, UINT(deferredLightingSRVs.size()),
                                    deferredLightingSRVs.data());
    m_screenSquare->Render(m_context);

    m_context->ClearRenderTargetView(m_backBufferRTV.Get(), clearColor);
    m_context->OMSetRenderTargets(1, m_backBufferRTV.GetAddressOf(), NULL);

    vector<ID3D11ShaderResourceView *> postEffectSRVs = {
        m_resolvedSRV.Get(), m_gBuffer.GetDepthView(), m_cubeMapSRV.Get()};

    AppBase::SetPipelineState(Graphics::postEffectsPSO);
    AppBase::SetGlobalConsts(m_globalConstsGPU);
    m_context->PSSetConstantBuffers(2, 1,
                                    m_postEffectsConstsGPU.GetAddressOf());
    m_context->PSSetShaderResources(5, UINT(postEffectSRVs.size()),
                                    postEffectSRVs.data());
    m_screenSquare->Render(m_context);

    // Render Pass
    AppBase::SetPipelineState(Graphics::renderPassPSO);
    AppBase::SetGlobalConsts(m_globalConstsGPU);
    for (int i = 0; i < 3; i++) {
        m_context->PSSetShaderResources(5, 1, &deferredLightingSRVs[i]);
        m_screenRenderPass[i]->Render(m_context);
    }
    m_context->PSSetShaderResources(5, 1, m_ssaoBlurSRV.GetAddressOf());
    m_screenRenderPass[3]->Render(m_context);
}

void Engine::UpdateGUI() {
    ImGui::SetNextItemOpen(false, ImGuiCond_Once);
    if (ImGui::TreeNode("General")) {
        ImGui::Checkbox("Use FPV", &m_camera.m_useFirstPersonView);
        ImGui::Checkbox("Wireframe", &m_drawAsWire);
        ImGui::TreePop();
    }
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Post-Processing")) {
        int flag = 0;
        flag += ImGui::RadioButton("Render", &m_postEffectsConstsCPU.mode, 1);
        ImGui::SameLine();
        flag += ImGui::RadioButton("Depth", &m_postEffectsConstsCPU.mode, 2);
        flag += ImGui::CheckboxFlags("Edge Detection",
                                     &m_postEffectsConstsCPU.edge, 1);
        flag += ImGui::CheckboxFlags("SSAO", &m_globalConstsCPU.SSAO, 1);
        flag += ImGui::CheckboxFlags("IBL", &m_globalConstsCPU.IBL, 1);
        flag += ImGui::SliderFloat(
            "strengthIBL", &m_globalConstsCPU.strengthIBL, 1e-3, 10.0f);
        flag += ImGui::SliderFloat(
            "DepthScale", &m_postEffectsConstsCPU.depthScale, 1e-3, 1.0f);
        flag += ImGui::SliderFloat(
            "GammaScale", &m_postEffectsConstsCPU.gammaScale, 1e-3, 10.0f);
        flag += ImGui::SliderFloat(
            "FogStrength", &m_postEffectsConstsCPU.fogStrength, 0.0f, 10.0f);
        flag += ImGui::SliderFloat("Exposure", &m_postEffectsConstsCPU.exposure,
                                   0.0f, 10.0f); 

        if (flag)
            D3D11Utils::UpdateBuffer(m_device, m_context,
                                     m_postEffectsConstsCPU,
                                     m_postEffectsConstsGPU);

        ImGui::TreePop();
    }
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("BOX")) {
        int flag = 0;
        flag += ImGui::CheckboxFlags(
            "Normal Map", &m_ground[0]->m_materialConstsCPU.useNormalMap, 1);

        if (flag) {
            m_ground[0]->UpdateConstantBuffers(m_device, m_context);
        }

        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("obj1")) {
        int flag = 0;
        // Move
        Vector3 transition = m_mainObj->m_worldRow.Translation();
        m_mainObj->m_worldRow.Translation(Vector3(0.0f));
        ImGui::SliderFloat3("Position", &transition.x, -5.0f, 5.0f);

        // Rotation
        ImGui::SliderFloat3("Roation", &rotationGUI.x, 0.0f, 1.0f);

        m_mainObj->UpdateWorldRow(m_mainObj->m_worldRow *
                                  Matrix::CreateRotationY(rotationGUI.y) *
                                  Matrix::CreateRotationX(-rotationGUI.x) *
                                  Matrix::CreateRotationZ(rotationGUI.z) *
                                  Matrix::CreateTranslation(transition));
        m_mainBoundingSphere.Center = m_mainObj->m_worldRow.Translation();

        flag += ImGui::CheckboxFlags(
            "Normal Map", &m_mainObj->m_materialConstsCPU.useNormalMap, 1);

        if (flag) {
            m_mainObj->UpdateConstantBuffers(m_device, m_context);
        }

        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Light0")) {

        ImGui::SliderFloat3("Position", &m_globalConstsCPU.lights[0].position.x,
                            -10.0f, 10.0f);
        ImGui::ColorEdit3("Color", &m_globalConstsCPU.lights[0].lightColor.x,
                          0);

        ImGui::TreePop();
    }
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Light1")) {
        ImGui::SliderFloat3("direction",
                            &m_globalConstsCPU.lights[1].direction.x, -10.0f,
                            10.0f);
        ImGui::ColorEdit3("Color", &m_globalConstsCPU.lights[1].lightColor.x,
                          0);

        ImGui::TreePop();
    }
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Light2")) {
        int flag = 0;
        flag += ImGui::SliderFloat3(
            "Position", &m_globalConstsCPU.lights[2].position.x, -10.0f, 10.0f);
        flag += ImGui::ColorEdit3("Color",
                                  &m_globalConstsCPU.lights[2].lightColor.x, 0);

        if (flag) {
            D3D11Utils::UpdateBuffer(m_device, m_context, m_globalConstsCPU,
                                     m_globalConstsGPU);
        }

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