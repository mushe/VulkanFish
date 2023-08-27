#pragma once
#include <vulkan/vulkan.hpp>

#include <vector>
#include <iostream>
#include <fstream>

class Util
{
public:
    static std::vector<char> ReadFile(const std::string& filename);
    static VkShaderModule CreateShaderModule(VkDevice& device, const std::vector<char>& code);
    
    static uint32_t FindMemoryType(VkPhysicalDevice& physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
    
    static void CreateBuffer(VkDevice& device, VkPhysicalDevice& physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    
    static void CopyBuffer(VkDevice& device, VkCommandPool& commandPool, VkQueue& queue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    
    
    static VkCommandBuffer BeginSimpleCommand(VkDevice& device, VkCommandPool& commandPool);
    static void EndSimpleCommand(VkCommandBuffer& commandBuffer, VkDevice& device, VkCommandPool& commandPool, VkQueue& queue);
    
    static void CreateImage(VkDevice& device, VkPhysicalDevice& physicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    
    static VkImageView CreateImageView(VkDevice &device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
};
