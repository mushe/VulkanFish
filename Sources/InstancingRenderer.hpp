#pragma once
#include <vulkan/vulkan.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>


class InstancingRenderer
{
private:
    
    struct UniformBufferObject
    {
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };
    
    struct InstanceParameters
    {
        alignas(16) glm::vec3 pos;
        alignas(16) glm::vec3 vel;
        alignas(16) glm::vec3 rgb;
    };
    
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 texCoord;
    };

    const std::vector<Vertex> vertices =
    {
        {{-0.12f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.12f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.12f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{-0.12f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
    };

    const std::vector<uint16_t> indices =
    {
        0, 1, 2, 2, 3, 0,
    };
    
    const int MAX_FRAMES = 2;
    uint32_t _N = 0;
    
    const float FIELD_SCALE = 1.0f;
    
    void CreateDescriptorSetLayout();
    void CreateGraphicsPipeline();
    
    void CreateVertexBuffer();
    void CreateIndexBuffer();
    
    void CreateTextureImage();
    void CreateTextureImageView();
    void CreateTextureSampler();
    
    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    
    void CreateUniformBuffers();
    void CreateDescriptorPool();
    void CreateDescriptorSets();
    
    VkDevice* _device;
    VkPhysicalDevice* _physicalDevice;
    VkRenderPass* _renderPass;
    VkCommandPool* _commandPool;
    VkQueue* _queue;
    std::vector<VkBuffer> _sharingBuffers;
    
    VkDescriptorSetLayout _descriptorSetLayout;
    VkPipelineLayout _pipelineLayout;
    VkPipeline _pipeline;
    
    VkBuffer _vertexBuffer;
    VkDeviceMemory _vertexBufferMemory;
    VkBuffer _indexBuffer;
    VkDeviceMemory _indexBufferMemory;
    
    VkImage _textureImage;
    VkDeviceMemory _textureImageMemory;
    VkImageView _textureImageView;
    VkSampler _textureSampler;
    
    std::vector<VkBuffer> _uniformBuffers;
    std::vector<VkDeviceMemory> _uniformBuffersMemory;
    std::vector<void*> _uniformBuffersMapped;
    std::vector<VkBuffer> _instanceUniformBuffers;
    std::vector<VkDeviceMemory> _instanceUniformBuffersMemory;
    std::vector<void*> _instanceUniformBuffersMapped;

    VkDescriptorPool _descriptorPool;
    std::vector<VkDescriptorSet> _descriptorSets;
    
    
public:
    void Init(VkDevice* device, VkPhysicalDevice* physicalDevice, VkRenderPass* renderPass, VkCommandPool* commandPool, VkQueue* queue, uint32_t particleNum, std::vector<VkBuffer> _sharingBuffers);
    void Draw(uint32_t frame, VkCommandBuffer& commandBuffer);
    void Release();
    
    float cameraFov = 45.0f;
    glm::vec4 cameraPos = glm::vec4(1.2f,  FIELD_SCALE/2.0f, FIELD_SCALE/2.0f, 0.0f);
    glm::vec4 cameraCenter = glm::vec4(FIELD_SCALE/2.0f,FIELD_SCALE/2.0f,FIELD_SCALE/2.0f, 0.0f);
};
