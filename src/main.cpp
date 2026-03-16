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
void updateProjection();
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
unsigned int screen_width = 800;
unsigned int screen_height = 600;

// Camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
Camera camera(cameraPos);
float lastX = screen_width / 2.0f;
float lastY = screen_height / 2.0f;

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
glm::vec3 mirrorCenterPos(0.0f, 0.5f, -4.5f);
glm::vec3 cubeColor(1.0f, 0.5f, 0.31f);
glm::vec3 framebufferColor(0.2f, 0.3f, 0.3f);
// light things
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
glm::vec3 lightDirection(-0.5f, -0.2f, -0.3f);
float lightConstant = 1.0f;
float lightLinear = 0.09f;
float lightQuadratic = 0.032f;
float flashlightInnerCutoff = 12.5f;
float flashlightOuterCutoff = 17.5f;
unsigned int uboMatrices;
unsigned int framebufferMSSA;
unsigned int textureColorBufferMSSA;
unsigned int rboMSSA;
unsigned int intermediateFBO;
unsigned int screenTexture;

//////////////////

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 8);

    GLFWwindow *window = glfwCreateWindow(screen_width, screen_height, "LearnOpenGL", NULL, NULL);
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

    std::vector<std::string>
        faces = {"assets/textures/skybox/right.jpg", "assets/textures/skybox/left.jpg", "assets/textures/skybox/top.jpg", "assets/textures/skybox/bottom.jpg", "assets/textures/skybox/front.jpg", "assets/textures/skybox/back.jpg"};
    // Models
    stbi_set_flip_vertically_on_load(true);
    Model planetModel("assets/objects/planet/planet.obj");
    Model rockModel("assets/objects/rock/rock.obj");
    // Implementation
    Shader shaderQuad("assets/shaders/framebuffersSimpleQuad.vert", "assets/shaders/framebuffersSimpleQuad.frag");
    Shader shaderJustNormals("assets/shaders/geometryShaderNormals.vert", "assets/shaders/geometryShaderNormals.frag", "assets/shaders/geometryShaderNormals.geom");
    Shader shaderInstancingArrays("assets/shaders/instancingArrays.vert", "assets/shaders/instancingArrays.frag");
    Shader shaderPlanet("assets/shaders/instancingPlanet.vert", "assets/shaders/instancingPlanet.frag");
    Shader shaderInstancingRocks("assets/shaders/instancingRealRocks.vert", "assets/shaders/instancingRealRocks.frag");
    // Set shader programs use the same values
    unsigned int uniformBlockIndexJustNormals = glGetUniformBlockIndex(shaderJustNormals.ID, "Matrices");
    unsigned int uniformBlockIndexPlanet = glGetUniformBlockIndex(shaderPlanet.ID, "Matrices");
    unsigned int uniformBlockIndexInstancingRocks = glGetUniformBlockIndex(shaderInstancingRocks.ID, "Matrices");

    glUniformBlockBinding(shaderJustNormals.ID, uniformBlockIndexJustNormals, 0);
    glUniformBlockBinding(shaderPlanet.ID, uniformBlockIndexPlanet, 0);
    glUniformBlockBinding(shaderInstancingRocks.ID, uniformBlockIndexInstancingRocks, 0);
    // quadVAO
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
    // UBO's
    // unsigned int uboMatrices;
    glGenBuffers(1, &uboMatrices);

    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

    // framebuffers
    // unsigned int framebufferMSSA;
    glGenFramebuffers(1, &framebufferMSSA);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferMSSA);

    // Attach a color buffer texture to the framebuffer's color attachment point
    // unsigned int textureColorBufferMSSA;
    glGenTextures(1, &textureColorBufferMSSA);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMSSA);
    int samples = 4; // Number of samples for multisampling
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, screen_width, screen_height, GL_TRUE);

    // Dont use
    // glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMSSA, 0);

    // buffer for depth and stencil attachment // obviusly using texture this approach is for sampling data from depth and stencil buffer, if we dont need to sample we can use renderbuffer which is faster
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, screen_width, screen_height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, textureColorBufferMSSA, 0);

    // attachments
    // renderbuffer object for depth and stencil attachment (we won't be sampling these)
    // unsigned int rboMSSA;
    glGenRenderbuffers(1, &rboMSSA);
    glBindRenderbuffer(GL_RENDERBUFFER, rboMSSA);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, screen_width, screen_height);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboMSSA);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    // glDeleteFramebuffers(1, &framebufferMSSA);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // THe intemediate framebuffer for resolving multisampled framebuffer
    // unsigned int intermediateFBO;
    glGenFramebuffers(1, &intermediateFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
    // unsigned int screenTexture;
    glGenTextures(1, &screenTexture);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screen_width, screen_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Intermediate framebuffer is not complete!" << std::endl;
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
    glEnable(GL_MULTISAMPLE);

    // mouse input, set cursor to center of screen
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glfwSwapBuffers(window);
    glfwShowWindow(window);
    glfwSetCursorPos(window, screen_width / 2.0, screen_height / 2.0);

    // model default
    glm::mat4 model = glm::mat4(1.0f);

    // now view
    glm::mat4 view = camera.GetViewMatrix();
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    // defined positions for instancing
    unsigned int amount = 1000;
    glm::mat4 *modelMatrices = new glm::mat4[amount];
    srand(0);
    float radius = 15.0f;
    float offset = 2.5f;
    for (unsigned int i = 0; i < amount; i++)
    {
        model = glm::mat4(1.0f);
        // model = glm::rotate(model, glm::radians(rotationSpeed * static_cast<float>(glfwGetTime())), glm::vec3(0.0f, 1.0f, 0.0f));
        // 1. translation: displace along circle with 'radius' in range [-offset, offset]
        float angle = (float)i / (float)amount * 360.0f;
        float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float x = sin(angle) * radius + displacement;
        float y = displacement * 0.4f; // keep height of field smaller compared to width of x and z
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float z = cos(angle) * radius + displacement;
        model = glm::translate(model, glm::vec3(x, y, z));
        model = glm::translate(model, mirrorCenterPos); // translate to center of planet

        // 2. scale: Scale between 0.05 and 0.25f
        float scale = (rand() % 20) / 100.0f + 0.05f;
        model = glm::scale(model, glm::vec3(scale));

        // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
        float rotAngle = static_cast<float>(rand() % 360);
        model = glm::rotate(model, glm::radians(rotAngle), glm::vec3(0.4f, 0.6f, 0.8f));

        // 4. now add to list of matrices
        modelMatrices[i] = model;
    }

    // buffer for instancing
    unsigned int modelsByRockVBO;
    glGenBuffers(1, &modelsByRockVBO);
    glBindBuffer(GL_ARRAY_BUFFER, modelsByRockVBO);
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);
    // glBindBuffer(GL_ARRAY_BUFFER, 0); We need the data for the vertex attribute pointers, dont unbind
    // glEnableVertexAttribArray(3);
    // glVertexAttribPointer(3, )
    for (unsigned int i = 0; i < rockModel.meshes.size(); i++)
    {
        unsigned int VAO = rockModel.meshes[i].VAO;
        glBindVertexArray(VAO);
        // glBindBuffer(GL_ARRAY_BUFFER, modelsByRockVBO); // bind it if was unbinded it
        // set attribute pointers for matrix (4 times vec4)
        std::size_t vec4Size = sizeof(glm::vec4);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * static_cast<GLsizei>(vec4Size), (void *)0);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * static_cast<GLsizei>(vec4Size), (void *)(1 * vec4Size));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * static_cast<GLsizei>(vec4Size), (void *)(2 * vec4Size));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * static_cast<GLsizei>(vec4Size), (void *)(3 * vec4Size));

        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);

        glBindVertexArray(0);
    }

    // setting uniforms that won't change in the render loop
    updateProjection();

    while (!glfwWindowShouldClose(window))
    {
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
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferMSSA);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glClearColor(framebufferColor.r, framebufferColor.g, framebufferColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Draw
        model = glm::mat4(1.0f);
        model = glm::translate(model, mirrorCenterPos);
        model = glm::rotate(model, glm::radians(rotationSpeed * static_cast<float>(glfwGetTime())), glm::vec3(0.0f, 1.0f, 0.0f));
        shaderPlanet.use();
        shaderPlanet.setMat4("model", model);
        // material
        shaderPlanet.setFloat("material.shininess", 8.0f);
        // directional light
        glm::vec3 lightAmbient = glm::vec3(0.05f, 0.05f, 0.05f);
        glm::vec3 lightDiffuse = glm::vec3(0.9f, 0.9f, 0.9f);
        glm::vec3 lightSpecular = glm::vec3(0.5f, 0.5f, 0.5f);
        shaderPlanet.setVec3("directionalLight.direction", lightDirection);
        shaderPlanet.setVec3("directionalLight.ambient", lightAmbient);
        shaderPlanet.setVec3("directionalLight.diffuse", lightDiffuse);
        shaderPlanet.setVec3("directionalLight.specular", lightSpecular);
        // draw planet
        planetModel.Draw(shaderPlanet);
        // draw rocks
        shaderInstancingRocks.use();
        shaderInstancingRocks.setMat4("model", model);
        // material
        shaderInstancingRocks.setFloat("material.shininess", 16.0f);
        // directional light
        shaderInstancingRocks.setVec3("directionalLight.direction", lightDirection);
        shaderInstancingRocks.setVec3("directionalLight.ambient", lightAmbient);
        shaderInstancingRocks.setVec3("directionalLight.diffuse", lightDiffuse);
        shaderInstancingRocks.setVec3("directionalLight.specular", lightSpecular);
        shaderInstancingRocks.setInt("texture_diffuse1", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rockModel.textures_loaded[0].id);
        for (unsigned int i = 0; i < rockModel.meshes.size(); i++)
        {
            glBindVertexArray(rockModel.meshes[i].VAO);
            glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(rockModel.meshes[i].indices.size()), GL_UNSIGNED_INT, 0, amount);
        }
        // rockModel.Draw(shaderInstancingRocks);
        // quads

        // Blit framebuffer first
        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferMSSA);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);
        glBlitFramebuffer(0, 0, screen_width, screen_height, 0, 0, screen_width, screen_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        // Now the window's framebuffer default
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(framebufferColor.r, framebufferColor.g, framebufferColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // by default
        // force to not show the quad in wireframe
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        shaderQuad.use();
        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, screenTexture);
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

void updateProjection()
{
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    float ratio = static_cast<float>(screen_width) / static_cast<float>(screen_height);
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), ratio, 0.1f, 100.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
}

void framebuffer_size_callback([[maybe_unused]] GLFWwindow *window, int width, int height)
{
    screen_width = width;
    screen_height = height;
    updateProjection();
    glViewport(0, 0, screen_width, screen_height);
    // Volvemos a configurar las texturas y el framebuffer para el nuevo tamaño
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferMSSA);

    // Actualizar la textura color buffer asociada al framebuffer de MSAA
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMSSA);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, screen_width, screen_height, GL_TRUE);

    // Actualizamos el renderbuffer de profundidad y stencil
    glBindRenderbuffer(GL_RENDERBUFFER, rboMSSA);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, screen_width, screen_height);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Actualizamos la textura del framebuffer intermedio
    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screen_width, screen_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
    ImGui::ColorEdit3("Framebuffer Color", (float *)&framebufferColor);
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