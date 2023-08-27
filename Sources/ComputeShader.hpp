#pragma once
#include <stdio.h>
#include <vulkan/vulkan.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

struct ParticleParameters
{
    float MAX_SPEED;
    float ATTRACTION;
    float WALL_AVOIDANCE;
    float ATTRACTION_DISTANCE;
    float ALIGNMENT_DISTANCE;
    float ALIGNMENT;
    float AVOIDANCE_DISTANCE;
    float AVOIDANCE;
    float VORTEX_FORCE;
};

struct InstanceParameters
{
    alignas(16) glm::vec3 pos;
    alignas(16) glm::vec3 vel;
    alignas(16) glm::vec3 rgb;
};

class ComputeShader
{

public:
    void Init(VkDevice* device, VkPhysicalDevice* physicalDevice, uint32_t particleNum, std::vector<VkBuffer> shaderStorageBuffers, VkCommandPool* _commandPool);
    void Execute(uint32_t frame, VkSemaphore* computeFinishedSemaphore, VkFence* computeInFlightFence, VkQueue queue);
    void Release();
    
    void SetParameters(ParticleParameters params);
    
private:
    VkDevice* _device;
    VkPhysicalDevice* _physicalDevice;
    VkCommandPool* _commandPool;
    
    const int MAX_FRAMES = 2;
    uint32_t _N = 0;
    
    VkDescriptorSetLayout _computeDescriptorSetLayout;
    VkPipelineLayout _computePipelineLayout;
    VkPipeline _computePipeline;
    VkDescriptorPool _computeDescriptorPool;
    std::vector<VkDescriptorSet> _computeDescriptorSets;
    
    std::vector<VkBuffer> _computeUniformBuffers;
    std::vector<VkDeviceMemory> _computeUniformBuffersMemory;
    std::vector<void*> _computeUniformBuffersMapped;
    
    std::vector<VkBuffer> _shaderStorageBuffers;
    std::vector<VkCommandBuffer> _computeCommandBuffers;
    
    
    void CreateComputeDescriptorSetLayout();
    void CreateComputePipeline();
    void CreateComputeUniformBuffers();
    void CreateComputeDescriptorPool();
    void CreateComputeDescriptorSets();
    void CreateComputeCommandBuffers();
    
    
    float MAX_SPEED = 0.0018f;
    float WALL_AVOIDANCE = 0.00002f;
    float ATTRACTION = 0.00042f;
    float ATTRACTION_DISTANCE = 0.05f;
    float ALIGNMENT = 0.0036f;
    float ALIGNMENT_DISTANCE = 0.05f;
    float AVOIDANCE = 0.0002f;
    float AVOIDANCE_DISTANCE = 0.015f;
    float VORTEX_FORCE = 0.0f;
    ParticleParameters _params;
};
