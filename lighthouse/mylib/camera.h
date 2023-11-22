#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.3f;
const float ZOOM = 45.0f;
namespace camera_movement {
    enum CamMov {FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN};
}

using glm::vec3;

struct Camera{
    vec3 position;
    vec3 front;
    vec3 up;
    vec3 right;
    vec3 worldUp;
    float yaw;
    float pitch;
    float speed;
    float sensitivity;
    float zoom;

    Camera(vec3 position = vec3(0.0f, 0.0f, 0.0f), vec3 up = vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : worldUp(up), yaw(yaw), pitch(pitch), speed(SPEED), sensitivity(SENSITIVITY), zoom(ZOOM){
            this->position = position;
            front = vec3(0.0f, 0.0f, -1.0f);
            this->updateCameraVectors();
    }

    void updateCameraVectors(){
        vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        this->front = glm::normalize(front);
        this->right = glm::normalize(glm::cross(this->front, this->worldUp));
        this->up = glm::normalize(glm::cross(this->right, this->front));
    }

    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true){
        xoffset *= this->sensitivity;
        yoffset *= this->sensitivity;
        yaw += xoffset;
        pitch += yoffset;

        if (constrainPitch){
            if (pitch > 89.0f)
                pitch = 89.0f;
            if (pitch < -89.0f)
                pitch = -89.0f;
        }
        this->updateCameraVectors();
    }

    void processMouseScroll(float yoffset){
        this->zoom -= yoffset;
        if (this->zoom <= 1.0f)
            this->zoom = 1.0f;
        if (this->zoom >= 45.0f)
            this->zoom = 45.0f;
    }

    void processKeyboard(camera_movement::CamMov direction, float deltaTime){
        float velocity = this->speed * deltaTime;
        if (direction == camera_movement::FORWARD)
            this->position += this->front * velocity;
        if (direction == camera_movement::BACKWARD)
            this->position -= this->front * velocity;
        if (direction == camera_movement::LEFT)
            this->position -= this->right * velocity;
        if (direction == camera_movement::RIGHT)
            this->position += this->right * velocity;
        if (direction == camera_movement::UP)
            this->position += this->up * velocity;
        if (direction == camera_movement::DOWN)
            this->position -= this->up * velocity;
    }

    glm::mat4 getViewMatrix(){
        return glm::lookAt(this->position, this->position + this->front, this->up);
    }
};

#endif