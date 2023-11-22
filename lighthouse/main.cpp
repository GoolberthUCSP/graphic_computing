//Nombres y apellidos: Fredy Goolberth Quispe Neira
//Curso: Computación gráfica, CCOMP7-1

#ifndef GLAD_GL_IMPLEMENTATION
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#endif

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#endif

#include <iostream>
#include <cmath>
#include <vector>
#include <thread>
#include <map>
#include <chrono>
#include <filesystem>
#include <string>
#include "mylib/shader.h"
#include "mylib/camera.h"
#include "mylib/lighthouse.h"

using std::cout;
using std::endl;
using std::vector;
using std::map;
using std::pair;
using std::string;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

GLFWwindow* window;
const int SCR_WIDTH = 900;
const int SCR_HEIGHT = 900;
int delay = 1000; // Microseconds

string path = std::filesystem::absolute(__FILE__).string() + "/../";
string shader_path = path + "mylib/shader_code/";

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// lighting
glm::vec3 lightPos(1.0f, 1.0f, 2.0f);
glm::vec3 lightAmbient(0.2f, 0.2f, 0.2f);
glm::vec3 lightDiffuse(0.5f, 0.5f, 0.5f);
glm::vec3 lightSpecular(1.0f, 1.0f, 1.0f);

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    gladLoadGL(glfwGetProcAddress);

    string light_shader = shader_path + "4.2.lighting_maps.vs";
    string light_frag = shader_path + "4.2.lighting_maps.fs";
    Shader lighting_shader(light_shader.c_str(), light_frag.c_str());
    
    // Create drawables
    Lighthouse lighthouse;

    // GL global config
    glPointSize(3.0f);
    glLineWidth(2.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //GL_FILL:GL_LINE
    
    lighting_shader.use();

    GLuint color_location = glGetUniformLocation(lighting_shader.ID, "color");
    lighting_shader.setInt("material.diffuse", 1);
    lighting_shader.setInt("material.specular", 1);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(0.7, 0.7, 0.7, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Transforms
        lighthouse.rotate_lantern(5.0f);
        Vec4f lightpos = lighthouse.get_lantern_position()+ lighthouse.get_light_direction()*0.3;
        lightPos = vec3(lightpos[0], lightpos[1], lightpos[2]);

        // be sure to activate shader when setting uniforms/drawing objects
        lighting_shader.use();
        lighting_shader.setVec3("light.position", lightPos);
        lighting_shader.setVec3("viewPos", camera.position);
        // light properties
        lighting_shader.setVec3("light.ambient", lightAmbient);
        lighting_shader.setVec3("light.diffuse", lightDiffuse);
        lighting_shader.setVec3("light.specular", lightSpecular);
        // material properties
        lighting_shader.setFloat("material.shininess", 35.0f);
        // View
        glm::mat4 view = camera.getViewMatrix();
        lighting_shader.setMat4("view", view);
        // Projection
        glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        lighting_shader.setMat4("projection", projection);
        // Draw
        lighthouse.draw(color_location);

        glfwSwapBuffers(window);
        glfwPollEvents();

        std::this_thread::sleep_for(std::chrono::microseconds(delay));
    }

    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        camera.processKeyboard(camera_movement::LEFT, deltaTime);
    }
    if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        camera.processKeyboard(camera_movement::RIGHT, deltaTime);
    }
    if (key == GLFW_KEY_Q && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        camera.processKeyboard(camera_movement::FORWARD, deltaTime);
    }
    if (key == GLFW_KEY_E && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        camera.processKeyboard(camera_movement::BACKWARD, deltaTime);
    }
    if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        camera.processKeyboard(camera_movement::UP, deltaTime);
    }
    if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        camera.processKeyboard(camera_movement::DOWN, deltaTime);
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.processMouseScroll(float(yoffset));
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 

    lastX = xpos;
    lastY = ypos;

    camera.processMouseMovement(xoffset, yoffset);
}