#include "ImGuiWrapper.hpp"

void ImGuiWrapper::Init(GLFWwindow* window, VkInstance &instance, VkDevice& device, VkPhysicalDevice& physicalDevice, VkRenderPass& renderPass, VkQueue & queue, VkCommandPool& commandPool)
{
    VkDescriptorPoolSize pool_sizes[] =
    {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
    };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    assert(vkCreateDescriptorPool(device, &pool_info, nullptr, &_descriptorPool) == VK_SUCCESS);
    
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForVulkan(window, true);

    ImGui_ImplVulkan_InitInfo info{};
    info.Instance = instance;
    info.PhysicalDevice = physicalDevice;
    info.Device = device;
    info.QueueFamily = 0;
    info.Queue = queue;
    info.DescriptorPool = _descriptorPool;
    info.MinImageCount = 2;
    info.ImageCount = 2;
    ImGui_ImplVulkan_Init(&info, renderPass);
    
    
    // font
    VkCommandBufferAllocateInfo commandAI
    {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        nullptr, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1
    };
    VkCommandBufferBeginInfo beginInfo
    {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };

    VkCommandBuffer command;
    vkAllocateCommandBuffers(device, &commandAI, &command);
    vkBeginCommandBuffer(command, &beginInfo);
    ImGui_ImplVulkan_CreateFontsTexture(command);

    vkEndCommandBuffer(command);

    VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr };
    submitInfo.pCommandBuffers = &command;
    submitInfo.commandBufferCount = 1;
    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

    vkDeviceWaitIdle(device);
    vkFreeCommandBuffers(device, commandPool, 1, &command);
}

void ImGuiWrapper::BeginFrame(std::string guiName)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin(guiName.c_str());
}

void ImGuiWrapper::EndFrame(VkCommandBuffer& commandBufferToDraw)
{
    ImGui::End();
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBufferToDraw);
}

void ImGuiWrapper::ShowFPS()
{
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::Text("%.1f FPS", io.Framerate);
}
