//Nombres y apellidos: Fredy Goolberth Quispe Neira
//Curso: Computación gráfica, CCOMP7-1

#ifndef GLAD_GL_IMPLEMENTATION
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#endif

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif

#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <cmath>
#include <vector>
#include <thread>
#include <map>
#include <chrono>
#include <string>
#include <filesystem>
#include <algorithm>
#include <random>
#include <ctime>
#include "mylib/rubik.h"
#include "mylib/shader.h"
#include "mylib/camera.h"

using std::cout;
using std::endl;
using std::vector;
using std::map;
using std::pair;
using std::string;
using std::time;
using std::srand;
using std::rand;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void simulation(vector<string> movements);
vector<string> randomMovements(int n_movs);
vector<string> rubik_solver(vector<string> movements);

string path = std::filesystem::absolute(__FILE__).string() + "/../";
string shader_path = path + "mylib/shader_code/";

GLFWwindow* window;
const int SCR_WIDTH = 900;
const int SCR_HEIGHT = 900;
int delay = 1000; // Microseconds
int counter = 0;

// Rubik variables
bool movsEnabled = true;
Rubik* rubik;
const float move_step = 10.0f;
rubik_side::Side side;
bool moved = true;
bool counter_clockwise = true;

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
float lightShininess = 5.0f;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    srand(time(NULL));

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

    // glad: load all OpenGL function pointers
    gladLoadGL(glfwGetProcAddress);

    string light_shader = shader_path + "4.2.lighting_maps.vs";
    string light_frag = shader_path + "4.2.lighting_maps.fs";
    Shader lighting_shader(light_shader.c_str(), light_frag.c_str());
    
    // Create drawables
    rubik= new Rubik;

    // Apply movements
    vector<string> movs = randomMovements(100);
    rubik->apply_movements(movs);

    glPointSize(5.0f);
    glLineWidth(3.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //GL_FILL:GL_LINE

    GLuint color_location = glGetUniformLocation(lighting_shader.ID, "color");
    lighting_shader.setInt("material.diffuse", 1);
    lighting_shader.setInt("material.specular", 1);

    // Simulation of rubik movements
    vector<string> solution = rubik_solver(movs);
    std::thread(simulation, solution).detach();
    
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(0.6, 0.6, 0.6, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Transforms
        if (!moved)
            if (rubik->rotate_side(side, counter_clockwise, move_step))
                moved = true;
    
        rubik->rotation_centroid(Vec4f(1.0f, 1.0f, 0.0f, 1.0f), 0.5f);

        // be sure to activate shader when setting uniforms/drawing objects
        lighting_shader.use();
        lighting_shader.setVec3("light.position", lightPos);
        lighting_shader.setVec3("viewPos", camera.get_position());
        // light properties
        lighting_shader.setVec3("light.ambient", lightAmbient);
        lighting_shader.setVec3("light.diffuse", lightDiffuse);
        lighting_shader.setVec3("light.specular", lightSpecular);
        // material properties
        lighting_shader.setFloat("material.shininess", lightShininess);
        // View
        glm::mat4 view = camera.getViewMatrix();
        lighting_shader.setMat4("view", view);
        // Projection
        glm::mat4 projection = glm::perspective(glm::radians(camera.get_zoom()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        lighting_shader.setMat4("projection", projection);
        // Draw
        rubik->draw(color_location);

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

    if (key == GLFW_KEY_F && (action == GLFW_PRESS || action == GLFW_REPEAT))
        camera.processKeyboard2(camera_movement::LEFT, deltaTime);
    if (key == GLFW_KEY_H && (action == GLFW_PRESS || action == GLFW_REPEAT))
        camera.processKeyboard2(camera_movement::RIGHT, deltaTime);
    if (key == GLFW_KEY_T && (action == GLFW_PRESS || action == GLFW_REPEAT))
        camera.processKeyboard2(camera_movement::UP, deltaTime);
    if (key == GLFW_KEY_G && (action == GLFW_PRESS || action == GLFW_REPEAT))
        camera.processKeyboard2(camera_movement::DOWN, deltaTime);
    
    if (movsEnabled){ // Disable movement when simulation is running
        if (key == GLFW_KEY_KP_4 && action == GLFW_PRESS && moved){
            side = rubik_side::L;
            moved = false;
        }
        if (key == GLFW_KEY_KP_6 && action == GLFW_PRESS && moved){
            side = rubik_side::R;
            moved = false;
        }
        if (key == GLFW_KEY_KP_8 && action == GLFW_PRESS && moved){
            side = rubik_side::U;
            moved = false;
        }
        if (key == GLFW_KEY_KP_2 && action == GLFW_PRESS && moved){
            side = rubik_side::D;
            moved = false;
        }
        if (key == GLFW_KEY_KP_7 && action == GLFW_PRESS && moved){
            side = rubik_side::F;
            moved = false;
        }
        if (key == GLFW_KEY_KP_9 && action == GLFW_PRESS && moved){
            side = rubik_side::B;
            moved = false;
        }
        if (key == GLFW_KEY_KP_1 && action == GLFW_PRESS && moved){
            side = rubik_side::M;
            moved = false;
        }
        if (key == GLFW_KEY_KP_5 && action == GLFW_PRESS && moved){
            side = rubik_side::E;
            moved = false;
        }
        if (key == GLFW_KEY_KP_3 && action == GLFW_PRESS && moved){
            side = rubik_side::S;
            moved = false;
        }
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

void simulation(vector<string> movements){
    movsEnabled = false;
    int i = 0;
    while (i < movements.size()){
        if (moved){
            side = rubik_movs[movements[i]].first;
            counter_clockwise = rubik_movs[movements[i++]].second;
            moved = false;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
    movsEnabled = true;
}

vector<string> randomMovements(int n_movs){
    vector<string> valid_movs = {"U+", "U-", "D+", "D-", "L+", "L-", "R+", "R-", "F+", "F-", "B+", "B-", "M+", "M-", "E+", "E-", "S+", "S-"};
    vector<string> movs;
    for (int i = 0; i < n_movs; i++){
        int mov = rand() % valid_movs.size();
        movs.push_back(valid_movs[mov]);
    }
    return movs;
}

vector<string> rubik_solver(vector<string> movements){
    std::reverse(movements.begin(), movements.end());
    for (auto &movement : movements){
        movement[1] = (movement[1] == '+') ? '-' : '+';
    }
    return movements;
}