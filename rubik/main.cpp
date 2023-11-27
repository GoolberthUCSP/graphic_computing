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
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cmath>
#include <vector>
#include <thread>
#include <map>
#include <chrono>
#include <string>
#include <filesystem>
#include "mylib/rubik.h"
#include "mylib/shader.h"
#include "mylib/camera.h"

// solver: https://github.com/jamesmtexas/Rubik
#include "mylib/solver.hpp"
#include "Cube.hpp"
#include "Centers.hpp"
#include "Cross.hpp"
#include "Corners.hpp"
#include "Edges.hpp"
#include "OLL.hpp"
#include "PLL.hpp"

using std::cout;
using std::endl;
using std::vector;
using std::map;
using std::pair;
using std::string;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void simulation(vector<string> movements);

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

// Map movs variables
vector<string> map_movs;
string clockwise_str;


map<string, pair<rubik_side::Side, bool>> rubik_movs{
    {"U'", {rubik_side::U, true}},
    {"U", {rubik_side::U, false}},
    {"D'", {rubik_side::D, true}},
    {"D", {rubik_side::D, false}},
    {"R'", {rubik_side::R, true}},
    {"R", {rubik_side::R, false}},
    {"L'", {rubik_side::L, true}},
    {"L", {rubik_side::L, false}},
    {"F'", {rubik_side::F, true}},
    {"F", {rubik_side::F, false}},
    {"B'", {rubik_side::B, true}},
    {"B", {rubik_side::B, false}},
    {"S'", {rubik_side::M, true}},
    {"S", {rubik_side::M, false}},
    {"E'", {rubik_side::E, true}},
    {"E", {rubik_side::E, false}},
    {"M'", {rubik_side::S, true}},
    {"M", {rubik_side::S, false}}
};

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// lighting
glm::vec3 lightPos(0.0f, 1.0f, 0.0f);
glm::vec3 lightAmbient(0.5f, 0.5f, 0.5f);
glm::vec3 lightDiffuse(0.5f, 0.5f, 0.5f);
glm::vec3 lightSpecular(1.0f, 1.0f, 1.0f);
float lightShininess = 5.0f;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
	srand(time(nullptr));
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
    rubik = new Rubik;

    glPointSize(5.0f);
    glLineWidth(3.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //GL_FILL:GL_LINE


    GLuint color_location = glGetUniformLocation(lighting_shader.ID, "color");
    lighting_shader.setInt("material.diffuse", 1);
    lighting_shader.setInt("material.specular", 1);

    //Simulation of rubik movements
    //std::thread(simulation, movs).detach();
    
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
    
        
        //rubik->rotation_centroid(Vec4f(1.0f, 1.0f, 0.0f, 1.0f), 0.5f);

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

// Input

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
    
    if (key == GLFW_KEY_C && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        rubik = new Rubik;
        map_movs.clear();
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
        // Solver
		if (key == GLFW_KEY_Z && action == GLFW_PRESS){
			string movements;
			for (auto& mov : map_movs) {
				movements += mov + " ";
			}
			std::cout << "Movements: " << movements << "\n";
			
			solver::Cube myCube(false);
			movements = solver::format(movements);
			myCube.moves(movements, false);
			// myCube.output();
			solver::Centers::solveCenters(myCube);
			//std::cout << "Centers solved: " << myCube.solution << "\n";
			//myCube.output();
            solver::Cross::solveCross(myCube);
			//std::cout << "Cross solved\n";// << myCube.solution << "\n";
			//myCube.output();
			solver::Corners::solveCorners(myCube);
			//std::cout << "Corners solved\n";// << myCube.solution << "\n";
			//myCube.output();
			solver::Edges::solveEdges(myCube);
			//std::cout << "Edges solved\n";
			solver::OLL::solveOLL(myCube);
			//std::cout << "OLL solved\n";
			solver::PLL::solvePLL(myCube);
			//std::cout << "PLL solved\n";
			
			std::cout << "Solution: " << myCube.solution << "\n\n";
			
			std::thread(simulation, solver::split(myCube.solution)).detach();
            map_movs.clear();
        }

        // Movements by keyboard
		if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS && moved){
            counter_clockwise = true;
            clockwise_str = "'";
        } else if ( moved ) {
            counter_clockwise = false;
            clockwise_str = "";
        }
        if (key == GLFW_KEY_KP_4 && action == GLFW_PRESS && moved){
            side = rubik_side::L;
            moved = false;
            map_movs.push_back("L" + clockwise_str);
        }
        if (key == GLFW_KEY_KP_6 && action == GLFW_PRESS && moved){
            side = rubik_side::R;
            moved = false;
            map_movs.push_back("R" + clockwise_str);
        }
        if (key == GLFW_KEY_KP_8 && action == GLFW_PRESS && moved){
            side = rubik_side::U;
            moved = false;
            map_movs.push_back("U" + clockwise_str);
        }
        if (key == GLFW_KEY_KP_5 && action == GLFW_PRESS && moved){
            side = rubik_side::D;
            moved = false;
            map_movs.push_back("D" + clockwise_str);
        }
        if (key == GLFW_KEY_KP_7 && action == GLFW_PRESS && moved){
            side = rubik_side::F;
            moved = false;
            map_movs.push_back("F" + clockwise_str);
        }
        if (key == GLFW_KEY_KP_9 && action == GLFW_PRESS && moved){
            side = rubik_side::B;
            moved = false;
            map_movs.push_back("B" + clockwise_str);
        }
        if (key == GLFW_KEY_KP_1 && action == GLFW_PRESS && moved){
            side = rubik_side::M;
            moved = false;
            map_movs.push_back("S" + clockwise_str);
        }
        if (key == GLFW_KEY_KP_2 && action == GLFW_PRESS && moved){
            side = rubik_side::E;
            moved = false;
            map_movs.push_back("E" + clockwise_str);
        }
        if (key == GLFW_KEY_KP_3 && action == GLFW_PRESS && moved){
            side = rubik_side::S;
            moved = false;
            map_movs.push_back("M" + clockwise_str);
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
    int i = 0, n_movs = movements.size();
    while (i < n_movs){
        if (moved){
            side = rubik_movs[movements[i]].first;
            counter_clockwise = rubik_movs[movements[i]].second;
            moved = false;
            i++;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
    movsEnabled = true;
}