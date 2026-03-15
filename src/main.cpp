// project headers
#include "learnopengl/camera.h"
#include "learnopengl/errorReporting.h"
#include "learnopengl/model.h"
#include "learnopengl/shader.h"

// third-party libraries
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <stb_image.h>

// standard library
#include <iostream>
#include <map>

namespace FileSystem
{
    std::string getPath(const std::string &relativePath)
    {
        return "/" + relativePath;
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void DrawScene(Shader &normalShader, unsigned int planeVAO, unsigned int floorTexture, unsigned int cubeVAO, unsigned int cubeTexture, Shader &singleColorShader, unsigned int vegetationVAO, unsigned int transparentTexture, std::vector<glm::vec3> &vegetation);
void DrawReflectiveCube(Shader &reflectiveContainerShader, unsigned int containerReflectVAO, unsigned int cubemapTexture);
void DrawRefractiveCube(Shader &refractiveContainerShader, unsigned int containerRefractVAO, unsigned int cubemapTexture);
void DrawSkybox(Shader &skyboxShader, unsigned int skyboxVAO, unsigned int cubemapTexture);
void DrawPoints(Shader &pointShader, unsigned int pointsVAO);
void DrawColoredCube(Shader &fragCoordShader, unsigned int containerReflectVAO);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
unsigned int loadCubemap(std::vector<std::string> faces);

void toggleCursor(GLFWwindow *window);
void toggleWireframe(GLFWwindow *window);
void calculateDeltaTime();
// For ImGui
void imgui_frame_init(GLFWwindow *window);
void imgui_frame_update();
void imgui_frame_new();
void imgui_frame_render();
void imgui_frame_shutdown();

// settings
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;

// Camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
Camera camera(cameraPos);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

// deltatime
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f; // time of last frame

// Game state
bool cursorEnabled = false;
bool wireframeEnabled = false;

////////////////
// Temporary variables needed
// animation
float rotationSpeed = 50.0f; // degrees per second
glm::vec3 mirrorCenterPos(2.5f, 0.5f, -1.5f);
glm::vec3 cubeColor(1.0f, 0.5f, 0.31f);
// light things
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
glm::vec3 lightDirection(-0.2f, -1.0f, -0.3f);
float lightConstant = 1.0f;
float lightLinear = 0.09f;
float lightQuadratic = 0.032f;
float flashlightInnerCutoff = 12.5f;
float flashlightOuterCutoff = 17.5f;
//////////////////

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    // callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    enableReportGlErrors();

    // Test output
    std::cout << "Hello, OpenGL!" << std::endl;

    // Imgui setup
    imgui_frame_init(window);

    // data for all the vertices

    float cubeVertices[] = {
        // positions          // texture Coords
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // back face

        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // front face

        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, // left face

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, // right face

        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // bottom face

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f // top face
    };

    float containerReflectiveVertices[] = {
        // positions         // normals
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, // back face

        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, // front face

        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, // left face

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, // right face

        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, // bottom face

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f // top face
    };

    float planeVertices[] = {
        // positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
        5.0f, -0.5f, 5.0f, 2.0f, 0.0f,
        -5.0f, -0.5f, -5.0f, 0.0f, 2.0f,
        -5.0f, -0.5f, 5.0f, 0.0f, 0.0f,

        5.0f, -0.5f, 5.0f, 2.0f, 0.0f,
        5.0f, -0.5f, -5.0f, 2.0f, 2.0f,
        -5.0f, -0.5f, -5.0f, 0.0f, 2.0f};

    float transparentVertices[] = {
        // positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
        0.0f, 0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f, 1.0f,
        1.0f, -0.5f, 0.0f, 1.0f, 1.0f,

        0.0f, 0.5f, 0.0f, 0.0f, 0.0f,
        1.0f, -0.5f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.5f, 0.0f, 1.0f, 0.0f};

    std::vector<glm::vec3> vegetation;
    vegetation.push_back(glm::vec3(-1.5f, 0.0f, -0.48f));
    vegetation.push_back(glm::vec3(1.5f, 0.0f, 0.51f));
    vegetation.push_back(glm::vec3(0.0f, 0.0f, 0.7f));
    vegetation.push_back(glm::vec3(-0.3f, 0.0f, -2.3f));
    vegetation.push_back(glm::vec3(0.5f, 0.0f, -0.6f));

    float quadVertices[] = {
        // positions   // texCoords
        -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f};

    float skyboxVertices[] = {
        // positions
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f};

    float vertices[] = {
        0.0f, 0.0f, 0.0f,  // point at center
        -0.5f, 0.0f, 0.0f, // point left
        0.5f, 0.0f, 0.0f,  // point right
        0.0f, 0.5f, 0.0f,  // point top
        0.0f, -0.5f, 0.0f  // point bottom
    };

    float geometryPoints[] = {
        -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, // top-left
        0.5f, 0.5f, 0.0f, 1.0f, 0.0f,  // top-right
        0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // bottom-right
        -0.5f, -0.5f, 1.0f, 1.0f, 0.0f // bottom-left
    };

    float instancingQuadVertices[] = {
        // positions     // colors
        -0.05f, 0.05f, 1.0f, 0.0f, 0.0f,
        -0.05f, -0.05f, 0.0f, 0.0f, 1.0f,
        0.05f, -0.05f, 0.0f, 1.0f, 0.0f,

        -0.05f, 0.05f, 1.0f, 0.0f, 0.0f,
        0.05f, -0.05f, 0.0f, 1.0f, 0.0f,
        0.05f, 0.05f, 0.0f, 1.0f, 1.0f // 
    };

    std::vector<std::string>
        faces = {"assets/textures/skybox/right.jpg", "assets/textures/skybox/left.jpg", "assets/textures/skybox/top.jpg", "assets/textures/skybox/bottom.jpg", "assets/textures/skybox/front.jpg", "assets/textures/skybox/back.jpg"};
    // Models
    stbi_set_flip_vertically_on_load(true);
    Model backpackModel("assets/objects/backpack/backpack.obj");
    // Implementation
    Shader shaderRed("assets/shaders/advancedGLSLRed.vert", "assets/shaders/advancedGLSLRed.frag");
    Shader shaderGreen("assets/shaders/advancedGLSLGreen.vert", "assets/shaders/advancedGLSLGreen.frag");
    Shader shaderBlue("assets/shaders/advancedGLSLBlue.vert", "assets/shaders/advancedGLSLBlue.frag");
    Shader shaderYellow("assets/shaders/advancedGLSLYellow.vert", "assets/shaders/advancedGLSLYellow.frag");
    Shader shaderQuad("assets/shaders/framebuffersSimpleQuad.vert", "assets/shaders/framebuffersSimpleQuad.frag");
    Shader shaderUsingGeom("assets/shaders/geometryShaderExplode.vert", "assets/shaders/geometryShaderExplode.frag", "assets/shaders/geometryShaderExplode.geom");
    Shader shaderJustNormals("assets/shaders/geometryShaderNormals.vert", "assets/shaders/geometryShaderNormals.frag", "assets/shaders/geometryShaderNormals.geom");
    Shader shaderInstancingQuad("assets/shaders/instancingQuad.vert", "assets/shaders/instancingQuad.frag");

    // Set shader programs use the same values
    unsigned int uniformBlockIndexRed = glGetUniformBlockIndex(shaderRed.ID, "Matrices");
    unsigned int uniformBlockIndexGreen = glGetUniformBlockIndex(shaderGreen.ID, "Matrices");
    unsigned int uniformBlockIndexBlue = glGetUniformBlockIndex(shaderBlue.ID, "Matrices");
    unsigned int uniformBlockIndexYellow = glGetUniformBlockIndex(shaderYellow.ID, "Matrices");
    unsigned int uniformBlockIndexUsingGeom = glGetUniformBlockIndex(shaderUsingGeom.ID, "Matrices");
    unsigned int uniformBlockIndexJustNormals = glGetUniformBlockIndex(shaderJustNormals.ID, "Matrices");

    glUniformBlockBinding(shaderRed.ID, uniformBlockIndexRed, 0);
    glUniformBlockBinding(shaderGreen.ID, uniformBlockIndexGreen, 0);
    glUniformBlockBinding(shaderBlue.ID, uniformBlockIndexBlue, 0);
    glUniformBlockBinding(shaderYellow.ID, uniformBlockIndexYellow, 0);
    glUniformBlockBinding(shaderUsingGeom.ID, uniformBlockIndexUsingGeom, 0);
    glUniformBlockBinding(shaderJustNormals.ID, uniformBlockIndexJustNormals, 0);

    // cubeVAO
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glBindVertexArray(0);
    // plane VAO
    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glBindVertexArray(0);
    // vegetation VAO
    unsigned int vegetationVAO, vegetationVBO;
    glGenVertexArrays(1, &vegetationVAO);
    glGenBuffers(1, &vegetationVBO);
    glBindVertexArray(vegetationVAO);
    glBindBuffer(GL_ARRAY_BUFFER, vegetationVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), &transparentVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glBindVertexArray(0);
    // quad VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glBindVertexArray(0);
    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glBindVertexArray(0);
    // Reflective container VAO
    unsigned int containerReflectVAO, containerReflectVBO;
    glGenVertexArrays(1, &containerReflectVAO);
    glGenBuffers(1, &containerReflectVBO);
    glBindVertexArray(containerReflectVAO);
    glBindBuffer(GL_ARRAY_BUFFER, containerReflectVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(containerReflectiveVertices), &containerReflectiveVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glBindVertexArray(0);
    // points VAO VBO
    unsigned int pointsVAO, pointsVBO;
    glGenVertexArrays(1, &pointsVAO);
    glGenBuffers(1, &pointsVBO);
    glBindVertexArray(pointsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glBindVertexArray(0);
    // geometry points VAO, VBO
    unsigned int geometryPointsVAO, geometryPointsVBO;
    glGenVertexArrays(1, &geometryPointsVAO);
    glGenBuffers(1, &geometryPointsVBO);
    glBindVertexArray(geometryPointsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, geometryPointsVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(geometryPoints), &geometryPoints, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(2 * sizeof(float)));
    glBindVertexArray(0);
    // instancing quad VAO, VBO
    unsigned int instancingQuadVAO, instancingQuadVBO;
    glGenVertexArrays(1, &instancingQuadVAO);
    glGenBuffers(1, &instancingQuadVBO);
    glBindVertexArray(instancingQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, instancingQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(instancingQuadVertices), &instancingQuadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(2 * sizeof(float)));
    glBindVertexArray(0);
    // UBO's
    unsigned int uboMatrices;
    glGenBuffers(1, &uboMatrices);

    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

    // Load Textures
    // stbi_set_flip_vertically_on_load(true);
    // unsigned int cubeTexture = loadTexture("assets/textures/marble.jpg");

    // framebuffers
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // Attach a color buffer texture to the framebuffer's color attachment point
    unsigned int textureColorBuffer;
    glGenTextures(1, &textureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);

    // buffer for depth and stencil attachment // obviusly using texture this approach is for sampling data from depth and stencil buffer, if we dont need to sample we can use renderbuffer which is faster
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, textureColorBuffer, 0);

    // attachments
    // renderbuffer object for depth and stencil attachment (we won't be sampling these)
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    // glDeleteFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // finish
    // Configure global opengl state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    // glDepthMask(GL_FALSE);
    glDepthFunc(GL_LESS); // change depth function so depth test passes when values are equal to depth buffer's content
    // blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // backface culling
    glEnable(GL_CULL_FACE);
    // glCullFace(GL_FRONT);
    glEnable(GL_PROGRAM_POINT_SIZE);

    // mouse input, set cursor to center of screen
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glfwSwapBuffers(window);
    glfwShowWindow(window);
    glfwSetCursorPos(window, SCR_WIDTH / 2.0, SCR_HEIGHT / 2.0);

    // now view
    glm::mat4 view = camera.GetViewMatrix();
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    // defined
    glm::mat4 model = glm::mat4(1.0f);
    glm::vec3 redCubePos(-0.75f, 0.75f, 0.0f);
    glm::vec3 greenCubePos(0.75f, 0.75f, 0.0f);
    glm::vec3 blueCubePos(-0.75f, -0.75f, 0.0f);
    glm::vec3 yellowCubePos(0.75f, -0.75f, 0.0f);
    glm::vec2 translation[100];
    int index = 0;
    float offset = 0.1f;
    for (int y = -10; y < 10; y += 2)
    {
        for (int x = -10; x < 10; x += 2)
        {
            glm::vec2 pos;
            pos.x = static_cast<float>(x) / 10.0f + offset;
            pos.y = static_cast<float>(y) / 10.0f + offset;
            translation[index++] = pos;
        }
    }
    shaderInstancingQuad.use();
    for (unsigned int i = 0; i < 100; i++)
    {
        shaderInstancingQuad.setVec2("offsets[" + std::to_string(i) + "]", translation[i]);
    }

    while (!glfwWindowShouldClose(window))
    {
        // setting uniforms that won't change in the render loop
        glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
        processInput(window);
        calculateDeltaTime();
        // FPS counter
        std::string currentFps = std::to_string(1.0f / deltaTime) + " FPS";

        // now view
        glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
        view = camera.GetViewMatrix();
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // Rendering
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Draw
        // glBindVertexArray(cubeVAO);
        // shaderRed.use();
        // model = glm::mat4(1.0f);
        // model = glm::translate(model, redCubePos);
        // shaderRed.setMat4("model", model);
        // glDrawArrays(GL_TRIANGLES, 0, 36);
        // shaderGreen.use();
        // model = glm::mat4(1.0f);
        // model = glm::translate(model, greenCubePos);
        // shaderGreen.setMat4("model", model);
        // glDrawArrays(GL_TRIANGLES, 0, 36);
        // shaderBlue.use();
        // model = glm::mat4(1.0f);
        // model = glm::translate(model, blueCubePos);
        // shaderBlue.setMat4("model", model);
        // glDrawArrays(GL_TRIANGLES, 0, 36);
        // shaderYellow.use();
        // model = glm::mat4(1.0f);
        // model = glm::translate(model, yellowCubePos);
        // shaderYellow.setMat4("model", model);
        // glDrawArrays(GL_TRIANGLES, 0, 36);
        shaderUsingGeom.use();
        model = glm::mat4(1.0f);
        shaderUsingGeom.setMat4("model", model);
        // shaderUsingGeom.setFloat("time", static_cast<float>(glfwGetTime()));
        // for testing normals, set constant so no position change
        shaderUsingGeom.setFloat("time", glm::radians(90.0f));
        backpackModel.Draw(shaderUsingGeom);
        shaderJustNormals.use();
        shaderJustNormals.setMat4("model", model);
        backpackModel.Draw(shaderJustNormals);

        // quads
        shaderInstancingQuad.use();
        glBindVertexArray(instancingQuadVAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 100);

        // Now the window's framebuffer default
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.2f, 0.3f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // by default
        // force to not show the quad in wireframe
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        shaderQuad.use();
        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // needed to update based on bool
        glPolygonMode(GL_FRONT_AND_BACK, wireframeEnabled ? GL_LINE : GL_FILL);

        // new frame for imgui
        imgui_frame_new();

        // ImGui frame update
        imgui_frame_update();

        // Rendering imgui
        // (Your code clears your framebuffer, renders your other stuff etc.)
        imgui_frame_render();
        // (Your code calls glfwSwapBuffers() etc.)

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    imgui_frame_shutdown();

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback([[maybe_unused]] GLFWwindow *window, int width, int height)
{
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    glViewport(0, 0, width, height);
}

void mouse_callback([[maybe_unused]] GLFWwindow *window, double xpos, double ypos)
{
    if (cursorEnabled)
    {
        // prevent sudden jump when enabling mouse input
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        return;
    }
    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos); // reversed since y-coordinates go from bottom
    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback([[maybe_unused]] GLFWwindow *window, [[maybe_unused]] double xoffset, double yoffset)
{
    if (cursorEnabled)
        return;
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        camera.ResetCamera();

    // togglers
    toggleCursor(window);
    toggleWireframe(window);
}

unsigned int loadTexture(const char *path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        else
            format = GL_RGB; // default to RGB if unknown

        // wrapping and filtering options
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // load texture data
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
    }
    stbi_image_free(data);

    return textureID;
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        else
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void toggleCursor(GLFWwindow *window)
{
    static bool nPrev = false;
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS && !nPrev)
    {
        cursorEnabled = !cursorEnabled;
        glfwSetInputMode(window, GLFW_CURSOR,
                         cursorEnabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }
    nPrev = glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS;
}

void toggleWireframe(GLFWwindow *window)
{
    static bool vPrev = false;
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS && !vPrev)
    {
        wireframeEnabled = !wireframeEnabled;
        glPolygonMode(GL_FRONT_AND_BACK, wireframeEnabled ? GL_LINE : GL_FILL);
    }
    vPrev = glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS;
}

void calculateDeltaTime()
{
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

void imgui_frame_init(GLFWwindow *window)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // IF using Docking Branch
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true); // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();
}

void imgui_frame_new()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void imgui_frame_update()
{
    ImGui::ShowDemoWindow(); // Show demo window! :)
    // another
    ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.
    // FPS counting
    ImGui::Text("Calculated: %.3f ms/frame (%.1f FPS)", deltaTime * 1000.0f, 1.0f / deltaTime);
    ImGui::SliderFloat("Rotation Speed", &rotationSpeed, 0.0f, 360.0f);
    ImGui::SliderFloat("Camera Speed", &camera.MovementSpeed, 0.0f, 250.0f);
    ImGui::SliderFloat("Mouse Sensitivity", &camera.MouseSensitivity, 0.0f, 1.0f);
    // ImGui::ColorEdit3("Cube Color", (float *)&cubeColor);
    // ImGui::SliderFloat3("Cube position", (float *)&cubePos, -20.0f, 20.0f);
    // // 3 coloredits and 1 slider float
    // ImGui::ColorEdit3("Ambient Color", (float *)&cubeAmbientColor);
    // ImGui::ColorEdit3("Diffuse Color", (float *)&cubeDiffuseColor);
    // ImGui::ColorEdit3("Specular Color", (float *)&cubeSpecularColor);
    // ImGui::SliderFloat("Shininess", &shininess, 1.0f, 256.0f);
    ImGui::Text("Light properties:");
    ImGui::SliderFloat3("Light direction", (float *)&lightDirection, -1.0f, 1.0f);
    ImGui::SliderFloat("Flashlight Inner cutoff", &flashlightInnerCutoff, 0.0f, 360.0f);
    ImGui::SliderFloat("Flashlight Outer cutoff", &flashlightOuterCutoff, 0.0f, 360.0f);
    ImGui::InputFloat("Light constant", &lightConstant);
    ImGui::InputFloat("Light linear", &lightLinear);
    ImGui::InputFloat("Light quadratic", &lightQuadratic);
    ImGui::End();
}

void imgui_frame_render()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void imgui_frame_shutdown()
{
    // Cleanup imgui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}