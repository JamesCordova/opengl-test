#include "learnopengl/camera.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
      MovementSpeed(SPEED),
      MouseSensitivity(SENSITIVITY),
      Zoom(ZOOM),
      Position(position),
      HorizontalFront(glm::vec3(0.0f, 0.0f, -1.0f)),
      WorldUp(up),
      Yaw(yaw),
      Pitch(pitch)
{
    updateCameraVectors();
}

Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
      MovementSpeed(SPEED),
      MouseSensitivity(SENSITIVITY),
      Zoom(ZOOM),
      Position(glm::vec3(posX, posY, posZ)),
      HorizontalFront(glm::vec3(0.0f, 0.0f, -1.0f)),
      WorldUp(glm::vec3(upX, upY, upZ)),
      Yaw(yaw),
      Pitch(pitch)
{
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix()
{
    return glm::lookAt(Position, Position + Front, Up);
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime, bool useHorizontalFront, bool useWorldUp)
{
    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD)
        Position += (useHorizontalFront ? HorizontalFront : Front) * velocity;
    if (direction == BACKWARD)
        Position -= (useHorizontalFront ? HorizontalFront : Front) * velocity;
    if (direction == LEFT)
        Position -= Right * velocity;
    if (direction == RIGHT)
        Position += Right * velocity;
    if (direction == UP)
        Position += (useWorldUp ? WorldUp : Up) * velocity;
    if (direction == DOWN)
        Position -= (useWorldUp ? WorldUp : Up) * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    if (constrainPitch)
    {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }
    if (Yaw > 360.0f)
        Yaw = 0.0f;
    
    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset)
{
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f)
        Zoom = 1.0f;
    if (Zoom > 89.0f)
        Zoom = 89.0f;
}

void Camera::ResetCamera()
{
    Position = glm::vec3(0.0f, 0.0f, 3.0f);
    Front = glm::vec3(0.0f, 0.0f, -1.0f);
    Up = glm::vec3(0.0f, 1.0f, 0.0f);
    Right = glm::vec3(1.0f, 0.0f, 0.0f);
    WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    Yaw = YAW;
    Pitch = PITCH;
    MovementSpeed = SPEED;
    MouseSensitivity = SENSITIVITY;
    Zoom = ZOOM;
}

void Camera::updateCameraVectors()
{
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    HorizontalFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}
