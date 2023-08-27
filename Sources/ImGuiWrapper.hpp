#pragma once

#include <string>

#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

class ImGuiWrapper
{
private:
    VkDescriptorPool _descriptorPool;
public:
    void Init(GLFWwindow* window, VkInstance &instance, VkDevice& device, VkPhysicalDevice& physicalDevice, VkRenderPass& renderPass, VkQueue & queue, VkCommandPool& commandPool);
    void BeginFrame(std::string guiName);
    void EndFrame(VkCommandBuffer& commandBufferToDraw);
    void ShowFPS();
};
