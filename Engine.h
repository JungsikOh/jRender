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
    shared_ptr<Model> m_ground;
    shared_ptr<Model> m_mainObj; 
    shared_ptr<Model> m_lightSphere[MAX_LIGHTS];
    shared_ptr<Model> m_skybox;
    shared_ptr<Model> m_cursorSphere;
    shared_ptr<Model> m_screenSquare;

    BoundingSphere m_mainBoundingSphere; // 물체의 여러 조작을 위한 Bounding Sphere

    bool m_usePerspectiveProjection = true;

    // 거울
    shared_ptr<Model> m_mirror;
    DirectX::SimpleMath::Plane m_mirrorPlane;
    float m_mirrorAlpha = 1.0f; // Opacity

    // For GUI 
    Quaternion q = Quaternion::CreateFromAxisAngle(Vector3(1.0f, 0.0f, 0.0f), 0.0f);
    Vector3 rotationGUI = {0.0f, 0.0f, 0.0f};

    // 거울이 아닌 물체들의 리스트 (for문으로 그리기 위함)
    vector<shared_ptr<Model>> m_basicList;
};

} // namespace hlab
