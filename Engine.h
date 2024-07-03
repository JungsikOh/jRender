#pragma once

#include <algorithm>
#include <directxtk/SimpleMath.h> 
#include <iostream>
#include <memory>

#include "AppBase.h"
#include "Model.h"

namespace jRenderer {

using DirectX::BoundingSphere;
using DirectX::SimpleMath::Vector3;

class Engine : public AppBase {
  public:
    Engine();

    virtual bool Initialize() override;
    virtual void UpdateGUI() override;
    virtual void Update(float dt) override;
    virtual void Render() override;

    void UpdateLights(float dt);

  protected:
    shared_ptr<Model> m_ground[3];


    shared_ptr<Model> m_mainObj; 
    shared_ptr<Model> m_boxObj;
    shared_ptr<Model> m_lightSphere[MAX_LIGHTS];
    shared_ptr<Model> m_skybox;
    shared_ptr<Model> m_cursorSphere;
    shared_ptr<Model> m_screenSquare;

    // render pass
    shared_ptr<Model> m_screenRenderPass[4];

    BoundingSphere m_mainBoundingSphere; // ��ü�� ���� ������ ���� Bounding Sphere

    bool m_usePerspectiveProjection = true;

    // �ſ�
    shared_ptr<Model> m_mirror;
    DirectX::SimpleMath::Plane m_mirrorPlane;
    float m_mirrorAlpha = 1.0f; // Opacity

    // For GUI 
    Quaternion q = Quaternion::CreateFromAxisAngle(Vector3(1.0f, 0.0f, 0.0f), 0.0f);
    Vector3 rotationGUI = {0.0f, 0.0f, 0.0f};

    // �ſ��� �ƴ� ��ü���� ����Ʈ (for������ �׸��� ����)
    vector<shared_ptr<Model>> m_basicList;
};

} // namespace hlab
