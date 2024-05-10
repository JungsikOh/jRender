#include "Camera.h"

#include <iostream>

namespace jRenderer {

using namespace DirectX;

// Return View Matrix about Camera. (Row major)
// you can think it is View frustum obj matrix.
Matrix Camera::GetViewRow() {

    // When rendering, it makes 'View' Matrix.
    // In this example, 'upDir' is fixed about Y
    // View Transformation is like that virtual world move on other way.
    return Matrix::CreateTranslation(-this->m_position) *
           Matrix::CreateRotationY(-this->m_yaw) *
           Matrix::CreateRotationX(this->m_pitch);
    // why it calc about translation, rotationY and rotationX?
    // 기본적으로 뷰 프러스텀의 Z방향은 나를 향하게 되야 한다. 그러므로,
    // 로테이션 Y를 통해서 m_yaw를 기준으로 돌리는 것이다. 그렇다면 왜?
    // -m_position을 하는 것일까,
}

Vector3 Camera::GetEyePos() { return m_position; }

void Camera::UpdateViewDir() {
    m_viewDir = Vector3::Transform(Vector3(0.0f, 0.0f, 1.0f),
                                   Matrix::CreateRotationY(this->m_yaw));
    m_rightDir = m_upDir.Cross(m_viewDir);
}

void Camera::UpdateKeyboard(const float dt, bool const keyPressed[256]) {
    if (m_useFirstPersonView) {
        if (keyPressed['W'])
            MoveForward(dt);
        if (keyPressed['S'])
            MoveForward(-dt);
        if (keyPressed['D'])
            MoveRight(dt);
        if (keyPressed['A'])
            MoveRight(-dt);
        if (keyPressed['E'])
            MoveUp(dt);
        if (keyPressed['Q'])
            MoveUp(-dt);
    }
}

void Camera::UpdateMouse(float mouseNdcX, float mouseNdcY) {
    // m_yaw = mouseNdcX * DirectX::XM_2PI;: 이 코드는 마우스의 X 좌표에 따라
    // 카메라의 수평 회전인 yaw를 계산합니다.
    //  마우스의 X 좌표 값이 -1부터 1까지의 범위에 있으며, 이를 전체 360도(0부터
    //  2π까지의 라디안 값)에 대응시키기 위해 DirectX::XM_2PI를 곱해줍니다. 이는
    //  마우스의 좌우 이동에 따라 카메라가 좌우로 회전하도록 합니다.

    // m_pitch = mouseNdcY * DirectX::XM_PIDIV2;: 이 코드는 마우스의 Y 좌표에
    // 따라 카메라의 수직 회전인 pitch를 계산합니다. 마우스의 Y 좌표 값이 -1부터
    // 1까지의 범위에 있으며, 이를 90도(π/2 라디안)에 대응시키기 위해
    // DirectX::XM_PIDIV2를 곱해줍니다. 이는 마우스의 상하 이동에 따라 카메라가
    // 위아래로 회전하도록 합니다.
    if (m_useFirstPersonView) {
        // calc how much roatate a mouse.
        m_yaw = mouseNdcX * DirectX::XM_2PI;      // 좌우 360도
        m_pitch = mouseNdcY * DirectX::XM_PIDIV2; // 위 아래 90도

        UpdateViewDir();
    }
}

void Camera::MoveForward(float dt) {
    // 이동 후의 위치 = 현재 위치 + 이동방향 * 속도 * 시간차이
    m_position += m_viewDir * m_speed * dt;
}

void Camera::MoveUp(float dt) { m_position += m_upDir * m_speed * dt; }

void Camera::MoveRight(float dt) { m_position += m_rightDir * m_speed * dt; }

void Camera::SetAspectRatio(float aspect) { m_aspect = aspect; }

Matrix Camera::GetProjRow() {
    return m_usePerspectiveProjection
               ? XMMatrixPerspectiveFovLH(XMConvertToRadians(m_projFovAngleY),
                                          m_aspect, m_nearZ, m_farZ)
               : XMMatrixOrthographicOffCenterLH(-m_aspect, m_aspect, -1.0f,
                                                 1.0f, m_nearZ, m_farZ);
}

} // namespace jRenderer