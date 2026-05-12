// project headers
#include "learnopengl/camera.h"
#include "learnopengl/errorReporting.h"
#include "learnopengl/model.h"
#include "learnopengl/shader.h"

// third-party libraries
#include <ft2build.h>
#include FT_FREETYPE_H

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
#include <random>
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
void renderScene(Shader &shader);
void renderCube();
void renderQuad();
void renderSphere();
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path, bool gammaCorrection = false);
unsigned int loadEnvironmentHDRMap(const char *path);
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
unsigned int screen_width = 1280;
unsigned int screen_height = 720;

// Camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
Camera camera(cameraPos);
float lastX = screen_width / 2.0f;
float lastY = screen_height / 2.0f;

// deltatime
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f; // time of last frame

// util
glm::mat4 projection;
int kernelSize = 32;
float hemisphereRadius = 0.5f;
float ambientOcclusionBias = 0.025f;
float occlusionPower = 1.0f;

// Game state
bool cursorEnabled = false;
bool wireframeEnabled = false;
bool faceCullingEnabled = true;
bool gammaEnabled = true;
bool ssaoEnabled = true;
bool useSimpleTangent = false;
bool blurSsaoEnabled = true;
////////////////
// Temporary variables needed
// animation
float rotationSpeed = 50.0f; // degrees per second
glm::vec3 mirrorCenterPos(0.0f, 0.5f, -4.5f);
glm::vec3 cubeColor(0.9f, 0.9f, 0.9f);
float mettalicValue = 0.0f;
float roughnessValue = 0.0f;
float aoValue = 1.0f;
int nrRows = 7;
int nrColumns = 7;
float spacing = 2.5f;
glm::vec3 framebufferColor(0.2f, 0.3f, 0.3f);
// light things
glm::vec3 lightPos(0.5f, 1.0f, 0.3f);
glm::vec3 lightDirection(-0.5f, -0.2f, -0.3f);
glm::vec3 lightAmbient(0.25f, 0.25f, 0.25f);
glm::vec3 lightDiffuse(0.9f, 0.9f, 0.9f);
glm::vec3 lightSpecular(0.5f, 0.5f, 0.5f);
float shininess = 32.0f;
float heightScale = 0.1f;
glm::vec3 lightColor(1.0f);
float lightConstant = 1.0f;
float lightLinear = 0.7f;
float lightQuadratic = 1.8f;
float flashlightInnerCutoff = 12.5f;
float flashlightOuterCutoff = 17.5f;
float near_plane = 1.0f;
float far_plane = 25.0f;
unsigned int uboMatrices;
unsigned int screenTexture;
bool textureGammaCorrected = false;
int hdrMode = 0;
int bloomType = 0;
int bloomIterations = 10;
float exposure = 1.0f;
float gammaFactor = 2.2f;
//////////////////
unsigned int framebufferHDR;
unsigned int colorBufferHDR[2];
unsigned int rboDepthHDR;
unsigned int gBufferFBO;
/////
unsigned int gPosition, gNormal, gAlbedoSpec;
unsigned int gRboDepth;
/////
unsigned int pingPongFBO[2];
unsigned int pingPongColorBuffer[2];
unsigned int ssaoColorBuffer;
unsigned int ssaoBlurPassColorBuffer;
////
//////////////////
unsigned int planeVAO;

float ourLerp(float a, float b, float f)
{
    return a + f * (b - a);
}

/////////// Temporal: Not recomended
struct Character
{
    unsigned int TextureID;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    unsigned int Advance;
};

// text renderer member
std::map<char, Character> Characters;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 4);

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

    [[maybe_unused]] float quadVertices[] = {
        // positions   // texCoords
        -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f};

    [[maybe_unused]] float skyboxVertices[] = {
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

    [[maybe_unused]] float planeVertices[] = {
        // positions            // normals         // texcoords
        25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 0.0f,
        -25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 25.0f,
        -25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,

        -25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 25.0f,
        25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 0.0f,
        25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 25.0f};
    std::vector<std::string>
        faces = {"assets/textures/skybox/right.jpg", "assets/textures/skybox/left.jpg", "assets/textures/skybox/top.jpg", "assets/textures/skybox/bottom.jpg", "assets/textures/skybox/front.jpg", "assets/textures/skybox/back.jpg"};
    // Models
    stbi_set_flip_vertically_on_load(true);
    // Model backpackModel("assets/objects/backpack/backpack.obj");
    
    // Rusted iron PBR IBL Specular
    unsigned int rustedIronAlbedo = loadTexture("assets/textures/pbr/rusted_iron/albedo.png");
    unsigned int rustedIronNormal = loadTexture("assets/textures/pbr/rusted_iron/normal.png");
    unsigned int rustedIronMetallic = loadTexture("assets/textures/pbr/rusted_iron/metallic.png");
    unsigned int rustedIronRoughness = loadTexture("assets/textures/pbr/rusted_iron/roughness.png");
    unsigned int rustedIronAo = loadTexture("assets/textures/pbr/rusted_iron/ao.png");

    // Gold PBR IBL Specular
    [[maybe_unused]] unsigned int goldAlbedo = loadTexture("assets/textures/pbr/gold/albedo.png");
    [[maybe_unused]] unsigned int goldNormal = loadTexture("assets/textures/pbr/gold/normal.png");
    [[maybe_unused]] unsigned int goldMetallic = loadTexture("assets/textures/pbr/gold/metallic.png");
    [[maybe_unused]] unsigned int goldRoughness = loadTexture("assets/textures/pbr/gold/roughness.png");
    [[maybe_unused]] unsigned int goldAo = loadTexture("assets/textures/pbr/gold/ao.png");

    // Grass PBR IBL Specular
    [[maybe_unused]] unsigned int grassAlbedo = loadTexture("assets/textures/pbr/grass/albedo.png");
    [[maybe_unused]] unsigned int grassNormal = loadTexture("assets/textures/pbr/grass/normal.png");
    [[maybe_unused]] unsigned int grassMetallic = loadTexture("assets/textures/pbr/grass/metallic.png");
    [[maybe_unused]] unsigned int grassRoughness = loadTexture("assets/textures/pbr/grass/roughness.png");
    [[maybe_unused]] unsigned int grassAo = loadTexture("assets/textures/pbr/grass/ao.png");

    // Plastic PBR IBL Specular
    [[maybe_unused]] unsigned int plasticAlbedo = loadTexture("assets/textures/pbr/plastic/albedo.png");
    [[maybe_unused]] unsigned int plasticNormal = loadTexture("assets/textures/pbr/plastic/normal.png");
    [[maybe_unused]] unsigned int plasticMetallic = loadTexture("assets/textures/pbr/plastic/metallic.png");
    [[maybe_unused]] unsigned int plasticRoughness = loadTexture("assets/textures/pbr/plastic/roughness.png");
    [[maybe_unused]] unsigned int plasticAo = loadTexture("assets/textures/pbr/plastic/ao.png");

    // Wall PBR IBL Specular
    [[maybe_unused]] unsigned int wallAlbedo = loadTexture("assets/textures/pbr/wall/albedo.png");
    [[maybe_unused]] unsigned int wallNormal = loadTexture("assets/textures/pbr/wall/normal.png");
    [[maybe_unused]] unsigned int wallMetallic = loadTexture("assets/textures/pbr/wall/metallic.png");
    [[maybe_unused]] unsigned int wallRoughness = loadTexture("assets/textures/pbr/wall/roughness.png");
    [[maybe_unused]] unsigned int wallAo = loadTexture("assets/textures/pbr/wall/ao.png");

    unsigned int hdrTexture = loadEnvironmentHDRMap("assets/textures/hdr/newport_loft.hdr");
    // Implementation
    Shader shaderGBufferPass("assets/shaders/ssaoGBufferWhiteScene.vert", "assets/shaders/ssaoGBufferWhiteScene.frag");
    Shader shaderPBRDirectLighting("assets/shaders/iblPbrDirectLightingSpecularDiffuse.vert", "assets/shaders/iblPbrDirectLightingSpecularDiffuse.frag");
    Shader shaderPBRTextured("assets/shaders/iblPbrDirectLightingSpecularDiffuseTextured.vert", "assets/shaders/iblPbrDirectLightingSpecularDiffuseTextured.frag");
    Shader shaderEquirectangularToCube("assets/shaders/iblDiffuseCubemap.vert", "assets/shaders/iblDiffuseCubemap.frag");
    Shader shaderSpecularSkyboxTest("assets/shaders/iblSpecularSkyboxTest.vert", "assets/shaders/iblSpecularSkyboxTest.frag");
    Shader shaderSkybox("assets/shaders/iblDiffuseSkybox.vert", "assets/shaders/iblDiffuseSkybox.frag");
    Shader shaderIrradianceConvolution("assets/shaders/iblDiffuseCubemapConvolution.vert", "assets/shaders/iblDiffuseCubemapConvolution.frag");
    Shader shaderPrefilterCubemapPerRoughness("assets/shaders/iblPrefilterCubeMapPerRoughness.vert", "assets/shaders/iblPrefilterCubeMapPerRoughness.frag");
    Shader shaderBrdfPrecomputing("assets/shaders/iblSpecularBRDFPrecomputed.vert", "assets/shaders/iblSpecularBRDFPrecomputed.frag");
    Shader shaderRenderQuad("assets/shaders/framebuffersSimpleQuad.vert", "assets/shaders/framebuffersSimpleQuad.frag");

    // Configure shader for debug quad
    shaderGBufferPass.use();
    shaderGBufferPass.setInt("texture_diffuse1", 0);
    shaderGBufferPass.setInt("texture_specular1", 1);

    shaderPBRDirectLighting.use();
    shaderPBRDirectLighting.setInt("irradianceMap", 0);
    shaderPBRDirectLighting.setInt("prefilterMap", 1);
    shaderPBRDirectLighting.setInt("brdfLUT", 2);
    shaderPBRDirectLighting.setInt("albedoMap", 3);
    shaderPBRDirectLighting.setInt("normalMap", 4);
    shaderPBRDirectLighting.setInt("metallicMap", 5);
    shaderPBRDirectLighting.setInt("roughnessMap", 6);
    shaderPBRDirectLighting.setInt("aoMap", 7);

    shaderPBRTextured.use();
    shaderPBRTextured.setInt("irradianceMap", 0);
    shaderPBRTextured.setInt("prefilterMap", 1);
    shaderPBRTextured.setInt("brdfLUT", 2);
    shaderPBRTextured.setInt("albedoMap", 3);
    shaderPBRTextured.setInt("normalMap", 4);
    shaderPBRTextured.setInt("metallicMap", 5);
    shaderPBRTextured.setInt("roughnessMap", 6);
    shaderPBRTextured.setInt("aoMap", 7);
    // Bind uniform block to binding point
    GLuint matricesIndex = glGetUniformBlockIndex(shaderGBufferPass.ID, "Matrices");
    glUniformBlockBinding(shaderGBufferPass.ID, matricesIndex, 0);

    // pbr: setup framebuffer IBL Diffuse
    unsigned int captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    int widthCapture = 512, heightCapture = 512;
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, widthCapture, heightCapture);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    // pbr: ibl cubemap to render and attach to framebuffer
    unsigned int envCubemap;
    glGenTextures(1, &envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        // note that we store each face with 16 bit floating point values
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, widthCapture, heightCapture, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    // wrapping
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // filtering
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // pbr: ibl set up projection and view matrices, 6 faces directions
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] =
    {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
    };

    // convert from HDR equirectangular map
    shaderEquirectangularToCube.use();
    shaderEquirectangularToCube.setInt("equirectangularMap", 0);
    shaderEquirectangularToCube.setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);

    glViewport(0, 0, widthCapture, heightCapture);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; ++i)
    {
        shaderEquirectangularToCube.setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // PBR: IBL irradiance map
    unsigned int widthMapConvolution = 32, heightMapConvolution = 32;
    unsigned int irradianceMap;
    glGenTextures(1, &irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, widthMapConvolution, heightMapConvolution, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    // wrapping
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // filtering
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, widthMapConvolution, heightMapConvolution);

    shaderIrradianceConvolution.use();
    shaderIrradianceConvolution.setInt("environmentMap", 0);
    shaderIrradianceConvolution.setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glViewport(0, 0, widthMapConvolution, heightMapConvolution);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; ++i)
    {
        shaderIrradianceConvolution.setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // IBL PBR Mapping yeah Prefilterd environment cubemap
    int baseFilterMapWidth = 128, baseFilterMapHeight = 128;
    unsigned int prefilterMap;
    glGenTextures(1, &prefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    for (unsigned int i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, baseFilterMapWidth, baseFilterMapHeight, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // Prefilter Run to Cubemap processed per roughness
    shaderPrefilterCubemapPerRoughness.use();
    shaderPrefilterCubemapPerRoughness.setInt("environmentMap", 0);
    shaderPrefilterCubemapPerRoughness.setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
    {
        // resize framebuffer based on mip map level
        unsigned int mipWidth = static_cast<int>(baseFilterMapWidth * pow(0.5, mip)); 
        unsigned int mipHeight = static_cast<int>(baseFilterMapHeight * pow(0.5, mip));
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        shaderPrefilterCubemapPerRoughness.setFloat("roughness", roughness);
        for (unsigned int i = 0; i < 6; ++i)
        {
            shaderPrefilterCubemapPerRoughness.setMat4("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            renderCube();
        } 
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // IBL PBR Brdf Lut precomputing
    unsigned int brdfLUTTexture;
    glGenTextures(1, &brdfLUTTexture);

    // pre allocate memory
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
    unsigned int brdfLUTWidth = 512, brdfLUTHeight = 512;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, brdfLUTWidth, brdfLUTHeight, 0, GL_RG, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, brdfLUTWidth, brdfLUTHeight);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

    glViewport(0, 0, brdfLUTWidth, brdfLUTHeight);
    shaderBrdfPrecomputing.use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderQuad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    

    // UBO's
    // unsigned int uboMatrices;
    glGenBuffers(1, &uboMatrices);

    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));
    // framebuffer for HDR
    glGenFramebuffers(1, &framebufferHDR);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferHDR);
    // color buffer for hdr

    glGenTextures(2, colorBufferHDR);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBufferHDR[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL);
        // filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // wrapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBufferHDR[i], 0);
    }

    unsigned int colorAttachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, colorAttachments);

    // depth buffer, just as normal

    glGenRenderbuffers(1, &rboDepthHDR);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepthHDR);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screen_width, screen_height);

    // the Attachment phase|
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepthHDR);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete\n";
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // another buffer for ping pong blur
    glGenFramebuffers(2, pingPongFBO);
    glGenTextures(2, pingPongColorBuffer);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingPongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingPongColorBuffer[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL);
        // filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // wrapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingPongColorBuffer[i], 0);
    }
    // G Buffer
    glGenFramebuffers(1, &gBufferFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);

    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

    // telling framebuffer
    unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, attachments);

    // rbo and then
    glGenRenderbuffers(1, &gRboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, gRboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screen_width, screen_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gRboDepth);

    // check
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete\n";
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ssao FBO, using it to take a quad and operate over it
    unsigned int ssaoFBO;
    glGenFramebuffers(1, &ssaoFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

    glGenTextures(1, &ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screen_width, screen_height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete\n";
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned int ssaoBlurPassFBO;
    glGenFramebuffers(1, &ssaoBlurPassFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurPassFBO);
    glGenTextures(1, &ssaoBlurPassColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoBlurPassColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screen_width, screen_height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoBlurPassColorBuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete\n";
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // generate sample kernel
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
    std::default_random_engine generator;
    std::vector<glm::vec3> ssaoKernel;
    for (unsigned int i = 0; i < 64; i++)
    {
        glm::vec3 sample(randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator));
        sample = glm::normalize(sample);   // now is a circle or semicircle
        sample *= randomFloats(generator); // now is part of the volume instead of just surface
        float scale = (float)i / 64.0f;
        // scale samples to make sample near the center with lerp
        scale = ourLerp(0.1f, 1.0f, scale * scale); // quadratic
        sample *= scale;
        ssaoKernel.push_back(sample);
    }

    // noise values
    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++)
    {
        glm::vec3 noise(randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator) * 2.0f - 1.0f, 0.0f); // around Z axis
        ssaoNoise.push_back(noise);
    }
    // noise texture
    unsigned int noiseTexture;
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // object positions
    std::vector<glm::vec3> objectPositions;
    objectPositions.push_back(glm::vec3(-3.0, -0.5, -3.0));
    objectPositions.push_back(glm::vec3(0.0, -0.5, -3.0));
    objectPositions.push_back(glm::vec3(3.0, -0.5, -3.0));
    objectPositions.push_back(glm::vec3(-3.0, -0.5, 0.0));
    objectPositions.push_back(glm::vec3(0.0, -0.5, 0.0));
    objectPositions.push_back(glm::vec3(3.0, -0.5, 0.0));
    objectPositions.push_back(glm::vec3(-3.0, -0.5, 3.0));
    objectPositions.push_back(glm::vec3(0.0, -0.5, 3.0));
    objectPositions.push_back(glm::vec3(3.0, -0.5, 3.0));

    // Freetype boilerplate
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init Freetype Library\n";
        return -1;
    }

    FT_Face face;
    if (FT_New_Face(ft, "assets/fonts/Antonio-Bold.ttf", 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load Font\n";
        return -1;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);
    // test
    if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
    {
        std::cout << "ERROR::FREETYPE: Failed to load Glyph\n";
        return -1;
    }

    // map 128 characters
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    for (unsigned char c = 0; c < 128; c++)
    {
        // load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYPE: Failed to load Glyph\n";
            continue;
        }
        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D
        (
            GL_TEXTURE_2D, 
            0, 
            GL_RED, 
            face->glyph->bitmap.width, 
            face->glyph->bitmap.rows, 
            0, 
            GL_RED, 
            GL_UNSIGNED_BYTE, 
            face->glyph->bitmap.buffer
        );
        // Wrapping 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // Filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };

        Characters.insert(std::pair<char, Character>(c, character));
    }
    // Clear resources
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // positions
    glm::vec3 lightPositions[] = {
        glm::vec3(-10.0f, 10.0f, 10.0f),
        glm::vec3(10.0f, 10.0f, 10.0f),
        glm::vec3(-10.0f, -10.0f, 10.0f),
        glm::vec3(10.0f, -10.0f, 10.0f)};

    glm::vec3 lightColors[] = {
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f)};

    // finish
    // Configure global opengl state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    // glDepthMask(GL_FALSE);
    glDepthFunc(GL_LESS); // change depth function so depth test passes when values are equal to depth buffer's content
    // blending
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // backface culling
    glEnable(GL_CULL_FACE);
    // glCullFace(GL_FRONT);
    glEnable(GL_PROGRAM_POINT_SIZE);
    // glEnable(GL_MULTISAMPLE);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // mouse input, set cursor to center of screen
    glViewport(0, 0, screen_width, screen_height);
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

        // Rendering - render to default framebuffer
        glClearColor(0.01f, 0.01f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Disable culling to render cube from inside
        // glDisable(GL_CULL_FACE);

        shaderPBRDirectLighting.use();

        shaderPBRDirectLighting.setVec3("viewPos", camera.Position);
        shaderPBRDirectLighting.setVec3("albedo", cubeColor);
        shaderPBRDirectLighting.setFloat("ao", aoValue); // its obligatory needed to be 1.0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

        // for (int i = 0; i < nrRows * nrColumns; i++)
        // {
        //     shaderPBRDirectLighting.setFloat("metallic", mettalicValue);
        //     shaderPBRDirectLighting.setFloat("roughness", roughnessValue);
        //     model = glm::mat4(1.0f);
        //     model = glm::translate(model, glm::vec3((i % nrColumns) * (spacing), (int)(i / nrRows) * (spacing), 0.0f));
        //     shaderPBRDirectLighting.setMat4("model", model);
        //     renderSphere();
        // }
        model = glm::mat4(1.0f);
        // render rows*column number of spheres with varying metallic/roughness values scaled by rows and columns respectively
        for (int row = 0; row < nrRows; ++row)
        {
            shaderPBRDirectLighting.setFloat("metallic", (float)row / (float)nrRows);
            for (int col = 0; col < nrColumns; ++col)
            {
                // we clamp the roughness to 0.025 - 1.0 as perfectly smooth surfaces (roughness of 0.0) tend to look a bit off
                // on direct lighting.
                shaderPBRDirectLighting.setFloat("roughness", glm::clamp((float)col / (float)nrColumns, 0.05f, 1.0f));

                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(
                    (float)(col - (nrColumns / 2)) * spacing,
                    (float)(row - (nrRows / 2)) * spacing,
                    -2.0f
                ));
                shaderPBRDirectLighting.setMat4("model", model);
                // shaderPBRDirectLighting.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
                renderSphere();
            }
        }

        // render lights
        for (int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); i++)
        {
            glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(static_cast<float>(glfwGetTime()) * 5.0f) * 5.0f, 0.0f, 0.0f);
            newPos = lightPositions[i];
            shaderPBRDirectLighting.setVec3("lightPositions[" + std::to_string(i) + "]", newPos);
            shaderPBRDirectLighting.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);

            model = glm::mat4(1.0f);
            model = glm::translate(model, newPos);
            model = glm::scale(model, glm::vec3(0.5f));
            shaderPBRDirectLighting.setMat4("model", model);
            // shaderPBRDirectLighting.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
            renderSphere();
        }

        // PBR IBL shader
        shaderPBRTextured.use();
        shaderPBRTextured.setVec3("viewPos", camera.Position);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

        // Rusted iron
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, rustedIronAlbedo);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, rustedIronNormal);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, rustedIronMetallic);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, rustedIronRoughness);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, rustedIronAo);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-5.0f, 0.0f, 2.0f));
        shaderPBRTextured.setMat4("model", model);
        renderSphere();

        // Gold
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, goldAlbedo);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, goldNormal);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, goldMetallic);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, goldRoughness);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, goldAo);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-3.0f, 0.0f, 2.0f));
        shaderPBRTextured.setMat4("model", model);
        renderSphere();

        // Grass
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, grassAlbedo);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, grassNormal);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, grassMetallic);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, grassRoughness);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, grassAo);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0f));
        shaderPBRTextured.setMat4("model", model);
        renderSphere();

        // Plastic
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, plasticAlbedo);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, plasticNormal);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, plasticMetallic);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, plasticRoughness);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, plasticAo);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.0f, 0.0f, 2.0f));
        shaderPBRTextured.setMat4("model", model);
        renderSphere();

        // Wall
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, wallAlbedo);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, wallNormal);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, wallMetallic);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, wallRoughness);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, wallAo);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(3.0f, 0.0f, 2.0f));
        shaderPBRTextured.setMat4("model", model);
        renderSphere();

        // skybox
        glDepthFunc(GL_LEQUAL);
        glCullFace(GL_FRONT);
        shaderSkybox.use();
        shaderSkybox.setMat4("view", view);
        shaderSkybox.setMat4("projection", projection);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
        renderCube();
        glCullFace(GL_BACK);
        glDepthFunc(GL_LESS);


        // new frame for imgui
        imgui_frame_new();

        // ImGui frame update
        imgui_frame_update();

        // Rendering imgui
        // (Your code clears your framebuffer, renders your other stuff etc.)
        imgui_frame_render();
        // (Your code calls glfwSwapBuffers() etc.)
        faceCullingEnabled ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, wireframeEnabled ? GL_LINE : GL_FILL);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    imgui_frame_shutdown();

    glfwTerminate();
    return 0;
}

void updateProjection()
{
    // Restrict if the window gets minimized
    if (screen_height == 0)
        return;

    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    float ratio = static_cast<float>(screen_width) / static_cast<float>(screen_height);
    projection = glm::perspective(glm::radians(camera.Zoom), ratio, 0.1f, 100.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void renderScene(Shader &shader)
{
    // room cube

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(5.0f));
    shader.setMat4("model", model);
    glCullFace(GL_FRONT);
    shader.setInt("reverse_normals", 1);
    renderCube();
    shader.setInt("reverse_normals", 0);
    glCullFace(GL_BACK);
    // cubes
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(4.0f, -3.5f, 0.0));
    model = glm::scale(model, glm::vec3(0.5f));
    shader.setMat4("model", model);
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.0f, 3.0f, 1.0f));
    model = glm::scale(model, glm::vec3(0.75f));
    shader.setMat4("model", model);
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-3.0f, -1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.5f));
    shader.setMat4("model", model);
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.5f, 1.0f, 1.5f));
    model = glm::scale(model, glm::vec3(0.5f));
    shader.setMat4("model", model);
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.5f, 2.0f, -3.0));
    model = glm::rotate(model, glm::radians(rotationSpeed * static_cast<float>(glfwGetTime())), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    model = glm::scale(model, glm::vec3(0.75f));
    shader.setMat4("model", model);
    renderCube();
}

unsigned int sphereVAO = 0;
unsigned int indexCount;
void renderSphere()
{
    if (sphereVAO == 0)
    {
        glGenVertexArrays(1, &sphereVAO);
        std::vector<float> data;
        unsigned int vbo, ebo;
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;

        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.1415926535f;

        for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
        {
            for (unsigned int y = 0; y <= X_SEGMENTS; ++y)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                positions.push_back(glm::vec3(xPos, yPos, zPos));
                uv.push_back(glm::vec2(xSegment, ySegment));
                normals.push_back(glm::vec3(xPos, yPos, zPos));
            }
        }

        bool oddRow = false;
        for (int y = 0; y < Y_SEGMENTS; ++y)
        {
            if (!oddRow)
            {
                for (int x = 0; x <= X_SEGMENTS; ++x)
                {
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + 1 + x);
                }
            }
            else
            {
                for (int x = X_SEGMENTS; x >= 0; --x)
                {
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + 1 + x);
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        }
        indexCount = static_cast<unsigned int>(indices.size());
        // data layouts
        for (unsigned int i = 0; i < positions.size(); i++)
        {
            data.push_back(positions[i].x);
            data.push_back(positions[i].y);
            data.push_back(positions[i].z);
            if (normals.size() > 0)
            {
                data.push_back(normals[i].x);
                data.push_back(normals[i].y);
                data.push_back(normals[i].z);
            }
            if (uv.size() > 0)
            {
                data.push_back(uv[i].x);
                data.push_back(uv[i].y);
            }
        }
        // data buffers
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        unsigned int stride = (3 + 2 + 3) * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void *)(6 * sizeof(float)));
    }
    // draw
    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
}

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // top-right
            1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,  // bottom-right
            1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // top-right
            -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,  // top-left
            // front face
            -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
            1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // bottom-right
            1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // top-right
            1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // top-right
            -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  // top-left
            -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-right
            -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top-left
            -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
            -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-right
                                                                // right face
            1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,     // top-left
            1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // bottom-right
            1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,    // top-right
            1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // bottom-right
            1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,     // top-left
            1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,    // bottom-left
            // bottom face
            -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,  // top-left
            1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom-left
            1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom-left
            -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
            -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
            1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // bottom-right
            1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,  // top-right
            1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // bottom-right
            -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
            -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f   // bottom-left
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        glBindVertexArray(cubeVAO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    }
    // render cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        // positions
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,
            1.0f,
            0.0f,
            0.0f,
            1.0f,
            -1.0f,
            -1.0f,
            0.0f,
            0.0f,
            0.0f,
            1.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,
            1.0f,
            -1.0f,
            0.0f,
            1.0f,
            0.0f,
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void framebuffer_size_callback([[maybe_unused]] GLFWwindow *window, int width, int height)
{
    // restrict if the window gets minimized
    if (width == 0 || height == 0)
        return;

    screen_width = width;
    screen_height = height;
    updateProjection();
    glViewport(0, 0, screen_width, screen_height);
    // Volvemos a configurar las texturas y el framebuffer para el nuevo tamaño
    // glBindFramebuffer(GL_FRAMEBUFFER, framebufferHDR);
    // glViewport(0, 0, screen_width, screen_height);

    // Actualizar el G buffer
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL);

    glBindRenderbuffer(GL_RENDERBUFFER, gRboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screen_width, screen_height);

    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screen_width, screen_height, 0, GL_RED, GL_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, ssaoBlurPassColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screen_width, screen_height, 0, GL_RED, GL_FLOAT, NULL);
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
    updateProjection();
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

unsigned int loadTexture(const char *path, bool gammaCorrection)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);

    if (!data)
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        return 0;
    }

    GLenum internalFormat;
    GLenum dataFormat;
    if (nrComponents == 1)
    {
        dataFormat = GL_RED;
        internalFormat = GL_RED;
    }
    else if (nrComponents == 4)
    {
        internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
        dataFormat = GL_RGBA;
    }
    else // if (nrComponents == 3)
    {
        internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
        dataFormat = GL_RGB;
    } // default to RGB if unknown

    // wrapping and filtering options
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, dataFormat == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, dataFormat == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load texture data
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    return textureID;
}

unsigned int loadEnvironmentHDRMap(const char *path)
{
    int width, height, nrComponents;
    float *data = stbi_loadf(path, &width, &height, &nrComponents, 0);
    if (!data)
    {
        std::cout << "Texture failed to load HDR image at path: " << path << std::endl;
        return 0;
    }
    unsigned int hdrTextureID;
    glGenTextures(1, &hdrTextureID);
    glBindTexture(GL_TEXTURE_2D, hdrTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

    // wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return hdrTextureID;
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
    if (ImGui::CollapsingHeader("Camera/Player"))
    {
        // FPS counting
        ImGui::Text("Calculated: %.3f ms/frame (%.1f FPS)", deltaTime * 1000.0f, 1.0f / deltaTime);
        ImGui::SliderFloat("Rotation Speed", &rotationSpeed, 0.0f, 360.0f);
        ImGui::SliderFloat("Camera Speed", &camera.MovementSpeed, 0.0f, 250.0f);
        ImGui::SliderFloat("Mouse Sensitivity", &camera.MouseSensitivity, 0.0f, 1.0f);
    }
    if (ImGui::CollapsingHeader("Result"))
    {
        ImGui::Checkbox("Gamma Correction", &gammaEnabled);
        ImGui::SliderFloat("Gamma Factor", &gammaFactor, 1.0f, 5.0f);
        const char *toneMappingOptions[] = {"Disabled", "Reinhard Tone Mapping", "Exposure Tone Mapping"};
        ImGui::Combo("Tone Mapping algorithm", &hdrMode, toneMappingOptions, IM_COUNTOF(toneMappingOptions));
        ImGui::SliderFloat("Exposure value", &exposure, 0.0f, 20.0f);
        const char *bloomOptions[] = {"Disabled", "Blurred threshold"};
        ImGui::Combo("Bloom type", &bloomType, bloomOptions, IM_COUNTOF(bloomOptions));
        ImGui::SliderInt("Bloom iterations", &bloomIterations, 1, 100);
        ImGui::Checkbox("SSAO", &ssaoEnabled);
        ImGui::Checkbox("Back-Face Culling ", &faceCullingEnabled);
    }
    if (ImGui::CollapsingHeader("Material"))
    {
        ImGui::ColorEdit3("Color", (float *)&cubeColor);
        ImGui::SliderFloat("Roughness", &roughnessValue, 0.0f, 1.0f);
        ImGui::SliderFloat("Metallic", &mettalicValue, 0.0f, 1.0f);
        ImGui::SliderFloat("Ambient Oclussion", &aoValue, 0.0f, 1.0f);
    }
    if (ImGui::CollapsingHeader("Light properties"))
    {
        ImGui::SliderFloat3("Light position", (float *)&lightPos, -20.0f, 20.0f);
        ImGui::SliderFloat3("Light direction", (float *)&lightDirection, -1.0f, 1.0f);
        ImGui::ColorEdit3("Light Color", (float *)&lightColor);
        ImGui::ColorEdit3("Light ambient", (float *)&lightAmbient);
        ImGui::ColorEdit3("Light diffuse", (float *)&lightDiffuse);
        ImGui::ColorEdit3("Light specular", (float *)&lightSpecular);
        ImGui::SliderFloat("Shininess", &shininess, 0.1f, 512.0f);
        ImGui::SliderFloat("Height Scale", &heightScale, 0.0f, 2.0f);
        ImGui::SliderFloat("Flashlight Inner cutoff", &flashlightInnerCutoff, 0.0f, 360.0f);
        ImGui::SliderFloat("Flashlight Outer cutoff", &flashlightOuterCutoff, 0.0f, 360.0f);
        ImGui::InputFloat("Light constant", &lightConstant);
        ImGui::InputFloat("Light linear", &lightLinear);
        ImGui::InputFloat("Light quadratic", &lightQuadratic);
    }
    if (ImGui::CollapsingHeader("Screen Space Ambient Occlussion"))
    {
        ImGui::SliderInt("KernelSize", &kernelSize, 0, 64);
        ImGui::SliderFloat("Sphere radius", &hemisphereRadius, 0.0f, 2.0f);
        ImGui::SliderFloat("Bias", &ambientOcclusionBias, 0.0f, 0.05f, "%.6f");
        ImGui::SliderFloat("Occlusion Power", &occlusionPower, 0.1f, 2.0f);
        ImGui::Checkbox("Simple Tangent(no noise)", &useSimpleTangent);
        ImGui::Checkbox("Blur SSAO result", &blurSsaoEnabled);
    }
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