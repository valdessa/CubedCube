#include <common.h>
#include <ogc/gx.h>
#include <renderer.h>

#include <camera.h>

#include <ogc/pad.h>


using namespace poyo;

Camera::Camera(cFVec3& position, cfloat speed) :   Camera(position, 0, -90, speed) {
    
}

Camera::Camera(cFVec3& position, cfloat& pitch, cfloat& yaw, cfloat speed) :
    position_(position), forward_(0, 0, 0), pitch_(pitch), yaw_(yaw), speed_(speed) {
    worldUp_ = glm::vec3(0, 1, 0);
    right_ = glm::vec3(1, 0, 0);
    up_ = glm::vec3(0, 1, 0);

    near_ = 0.1f;
    far_ = 300.0f;
    fov_ = 60.0f;
    aspectRatio_ = static_cast<float>(Renderer::ScreenWidth()) / static_cast<float>(Renderer::ScreenHeight());
    
    sensitivity_ = 1.0f;
}

Camera::~Camera() = default;

void Camera::updateCamera(float deltaTime) {
    // Read inputs from the left joystick (s8 type)
    float joystickLeftX = static_cast<float>(PAD_StickX(0)) / 128.0f; // Normalize between -1 and 1
    float joystickLeftY = static_cast<float>(PAD_StickY(0)) / 128.0f; // Normalize between -1 and 1

    // Update camera position based on left joystick movement
    if (joystickLeftY > 0.1f) { // Move forward
        position_ += forward_ * speed_ * deltaTime;
    }
    if (joystickLeftY < -0.1f) { // Move backward
        position_ -= forward_ * speed_ * deltaTime;
    }
    if (joystickLeftX > 0.1f) { // Move right
        position_ += right_ * speed_ * deltaTime;
    }
    if (joystickLeftX < -0.1f) { // Move left
        position_ -= right_ * speed_ * deltaTime;
    }

    if(PAD_ButtonsHeld(0) & PAD_BUTTON_Y) {
        position_ += up_ * speed_ * deltaTime;
    }
    if(PAD_ButtonsHeld(0) & PAD_BUTTON_X) {
        position_ -= up_ * speed_ * deltaTime;
    }

    // Read inputs from the right joystick (s8 type) for rotation
    float joystickRightX = static_cast<float>(PAD_SubStickX(0)) / 128.0f; // Normalize between -1 and 1
    float joystickRightY = static_cast<float>(PAD_SubStickY(0)) / 128.0f; // Normalize between -1 and 1

    // Apply dead zones to the right joystick for better control
    if (fabs(joystickRightX) > 0.1f) { // Check dead zone for right stick X-axis
        yaw_ += joystickRightX * sensitivity_; // Rotate along the Y-axis
    }
    if (fabs(joystickRightY) > 0.1f) { // Check dead zone for right stick Y-axis
        pitch_ += joystickRightY * sensitivity_; // Rotate along the X-axis (invert for natural control)
    }

    // Clamp pitch to avoid full rotations
    pitch_ = glm::clamp(pitch_, -89.0f, 89.0f);

    // Calculate new forward direction
    glm::vec3 front{
        cosf(glm::radians(yaw_)) * cosf(glm::radians(pitch_)),
        sinf(glm::radians(pitch_)),
        sinf(glm::radians(yaw_)) * cosf(glm::radians(pitch_))
    };
    forward_ = glm::normalize(front);
    right_ = glm::normalize(glm::cross(forward_, worldUp_));
    up_ = glm::normalize(glm::cross(right_, forward_));

    // Update camera settings for rendering
    auto center = position_ + forward_;
    // GRRLIB_Camera3dSettings(
    //     position_.x, position_.y, position_.z,
    //     up_.x, up_.y, up_.z,
    //     center.x, center.y, center.z); // View matrix

    Renderer::SetCameraSettings(position_, up_, center);
}

void Camera::setPosition(cFVec3& pos) {
    position_ = pos;
}

void Camera::setSpeed(cfloat& speed) {
    speed_ = speed;
}

cFVec3& Camera::getPosition() const {
    return position_;
}

cfloat& Camera::getPitch() const {
    return pitch_;
}

cfloat& Camera::getYaw() const {
    return yaw_;
}


