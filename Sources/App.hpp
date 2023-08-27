#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <vector>
#include <set>
#include <random>

#include "ComputeShader.hpp"
#include "InstancingRenderer.hpp"
#include "ImGuiWrapper.hpp"


class App
{

public:
    App(){}
    void Run();
    
private:
    // particle count (N)
    const uint32_t N_desired = 30000;
    const uint32_t N = N_desired - N_desired % 256;
    
    const float FIELD_SCALE = 1.0f;
    const int MAX_FRAMES = 2;
    const uint32_t WINDOW_WIDTH = 1300;
    const uint32_t WINDOW_HEIGHT = 800;

    GLFWwindow* window;
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VkDevice device;

    VkQueue instancingQueue;
    VkQueue computeQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkRenderPass renderPass;
    VkCommandPool commandPool;
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;
    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> computeSemaphores;
    std::vector<VkSemaphore> instancingSemaphores;
    std::vector<VkSemaphore> renderingSemaphores;
    std::vector<VkFence> instancingFences;
    std::vector<VkFence> computeFences;
    
    uint32_t frameIndex = 0;
    uint32_t imageIndex = 0;

    
    // shareing buffer between compute shader and instancing shader
    std::vector<VkBuffer> sharingBuffers;
    std::vector<VkDeviceMemory> sharingBuffersMemory;
        
    
    // gui parameters
    float cameraFov = 45.0f;
    glm::vec4 cameraPos = glm::vec4(1.2f,  FIELD_SCALE/2.0f, FIELD_SCALE/2.0f, 0.0f);
    glm::vec4 cameraCenter = glm::vec4(FIELD_SCALE/2.0f,FIELD_SCALE/2.0f,FIELD_SCALE/2.0f, 0.0f);
    const float PARAM_MULTIPLY = 10000.0f;
    float MAX_SPEED = 0.0018f * PARAM_MULTIPLY;
    float WALL_AVOIDANCE = 0.00002f * PARAM_MULTIPLY;
    float ATTRACTION = 0.00042f * PARAM_MULTIPLY;
    float ATTRACTION_DISTANCE = 0.05f;
    float ALIGNMENT = 0.0036f * PARAM_MULTIPLY;
    float ALIGNMENT_DISTANCE = 0.05f;
    float AVOIDANCE = 0.0002f * PARAM_MULTIPLY;
    float AVOIDANCE_DISTANCE = 0.015f;
    float VORTEX_FORCE = 0.0f * PARAM_MULTIPLY;
    
    
    ComputeShader computeShader;
    InstancingRenderer instancingRenderer;
    ImGuiWrapper imGuiWrapper;
    
    
    void InitWindow();
    void MainLoop();
    
    void InitVulkan();
    void InitInstance();
    void InitSurface();
    void InitPhysicalDevice();
    void InitLogicalDevice();
    void InitSwapChain();
    void InitImageViews();
    void InitRenderPass();
    void InitFramebuffers();
    void InitCommandPool();
    void InitSharingBuffers();
    void InitDepthImage();
    void InitCommandBuffers();
    void InitFenceAndSemaphores();
    
    void RenderBegin();
    void RenderEnd();
    void RenderGUI();
    
    void Finalize();
};
