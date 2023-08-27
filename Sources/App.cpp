#include "App.hpp"
#include "Util.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include "stb_image.h"


void App::Run()
{
    InitWindow();
    InitVulkan();
    
    
    imGuiWrapper.Init(window, instance, device,  physicalDevice, renderPass, instancingQueue, commandPool);
    
    computeShader.Init(&device, &physicalDevice, N, sharingBuffers, &commandPool);
    
    instancingRenderer.Init(&device, &physicalDevice, &renderPass, &commandPool, &instancingQueue, N, sharingBuffers);
    
    
    // loop every frame
    MainLoop();
    
    Finalize();
}


void App::MainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        // frame polling and input
        glfwPollEvents();
        if(glfwGetKey(window, GLFW_KEY_ESCAPE))break;
        
        
        // execute compute shader
        ParticleParameters p;
        p.MAX_SPEED = MAX_SPEED / PARAM_MULTIPLY;
        p.ATTRACTION = ATTRACTION / PARAM_MULTIPLY;
        p.WALL_AVOIDANCE = WALL_AVOIDANCE / PARAM_MULTIPLY;
        p.ATTRACTION_DISTANCE = ATTRACTION_DISTANCE;
        p.ALIGNMENT_DISTANCE = ALIGNMENT_DISTANCE;
        p.ALIGNMENT = ALIGNMENT / PARAM_MULTIPLY;
        p.AVOIDANCE_DISTANCE = AVOIDANCE_DISTANCE;
        p.AVOIDANCE = AVOIDANCE / PARAM_MULTIPLY;
        p.VORTEX_FORCE = VORTEX_FORCE / PARAM_MULTIPLY;
        computeShader.SetParameters(p);
        
        computeShader.Execute(frameIndex, &computeSemaphores[frameIndex], &computeFences[frameIndex], computeQueue);
        
        
        // render instanced fish and GUI
        RenderBegin();
        
        instancingRenderer.Draw(frameIndex, commandBuffers[frameIndex]);
        RenderGUI();
        
        RenderEnd();
    }

    vkDeviceWaitIdle(device);
}


void App::InitWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan Fish", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
}


void App::InitVulkan()
{
    InitInstance();
    InitSurface();
    InitPhysicalDevice();
    InitLogicalDevice();
    InitSwapChain();
    
    InitImageViews();
    InitRenderPass();
    InitCommandPool();
    InitSharingBuffers();
    InitDepthImage();
    InitFramebuffers();
    InitCommandBuffers();
    InitFenceAndSemaphores();
}


void App::InitInstance()
{
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
 
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    assert(vkCreateInstance(&createInfo, nullptr, &instance) == VK_SUCCESS);
}


void App::InitSurface()
{
    assert(glfwCreateWindowSurface(instance, window, nullptr, &surface) == VK_SUCCESS);
}



void App::InitPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    assert(deviceCount != 0);

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    physicalDevice = devices[0];

    assert(physicalDevice != VK_NULL_HANDLE);
}



void App::InitLogicalDevice()
{
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = 0;
    queueCreateInfo.queueCount = 1;
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    queueCreateInfos.push_back(queueCreateInfo);
    
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    const std::vector<const char*> enabledExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
    createInfo.ppEnabledExtensionNames = enabledExtensions.data();
    createInfo.enabledLayerCount = 0;

    assert(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) == VK_SUCCESS);

    vkGetDeviceQueue(device, 0, 0, &instancingQueue);
    vkGetDeviceQueue(device, 0, 0, &computeQueue);
    vkGetDeviceQueue(device, 0, 0, &presentQueue);
}



void App::InitSwapChain()
{
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    assert(formatCount != 0);
    
    std::vector<VkSurfaceFormatKHR> formats;
    formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
    assert(presentModeCount != 0);
    
    std::vector<VkPresentModeKHR> presentModes;
    presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());
    
    VkSurfaceFormatKHR surfaceFormat;
    surfaceFormat.format = VK_FORMAT_B8G8R8A8_SRGB;
    surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    uint32_t imageCount = capabilities.minImageCount + 1;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = capabilities.currentExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    createInfo.clipped = VK_TRUE;

    assert(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) == VK_SUCCESS);

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = capabilities.currentExtent;
}



void App::InitImageViews()
{
    swapChainImageViews.resize(swapChainImages.size());

    for (uint32_t i = 0; i < swapChainImages.size(); i++)
    {
        swapChainImageViews[i] = Util::CreateImageView(device, swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}



void App::InitRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = VK_FORMAT_D32_SFLOAT;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    assert(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) == VK_SUCCESS);
}


void App::InitFramebuffers()
{
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++)
    {
        std::array<VkImageView, 2> attachments =
        {
            swapChainImageViews[i],
            depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        assert(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) == VK_SUCCESS);
    }
}


void App::InitCommandPool()
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = 0;

    assert(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) == VK_SUCCESS);
}


void App::InitSharingBuffers()
{
    // Initialize particles
    std::default_random_engine rndEngine((unsigned)time(nullptr));
    std::uniform_real_distribution<float> rndDist(0.0f, FIELD_SCALE);
    std::uniform_real_distribution<float> rNorm(-1.0f, 1.0f);

    std::vector<InstanceParameters> particles(N);
    for (auto& particle : particles)
    {
        particle.pos = glm::vec3(rndDist(rndEngine) * FIELD_SCALE, rndDist(rndEngine) * FIELD_SCALE,  rndDist(rndEngine) * FIELD_SCALE);
        particle.vel = glm::vec3(rNorm(rndEngine), rNorm(rndEngine),  rNorm(rndEngine)) * 0.003f;
        particle.rgb = glm::vec3(rndDist(rndEngine), rndDist(rndEngine),  rndDist(rndEngine));
    }

    VkDeviceSize bufferSize = sizeof(InstanceParameters) * N;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    Util::CreateBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, particles.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    sharingBuffers.resize(MAX_FRAMES);
    sharingBuffersMemory.resize(MAX_FRAMES);

    for (size_t i = 0; i < MAX_FRAMES; i++)
    {
        Util::CreateBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sharingBuffers[i], sharingBuffersMemory[i]);
        Util::CopyBuffer(device, commandPool, instancingQueue, stagingBuffer, sharingBuffers[i], bufferSize);
    }

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}


void App::InitDepthImage()
{
    Util::CreateImage(device, physicalDevice, swapChainExtent.width, swapChainExtent.height, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
    depthImageView = Util::CreateImageView(device, depthImage, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT);
}


void App::InitCommandBuffers()
{
    commandBuffers.resize(MAX_FRAMES);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

    assert(vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) == VK_SUCCESS);
}


void App::InitFenceAndSemaphores()
{
    computeSemaphores.resize(MAX_FRAMES);
    instancingSemaphores.resize(MAX_FRAMES);
    renderingSemaphores.resize(MAX_FRAMES);
    instancingFences.resize(MAX_FRAMES);
    computeFences.resize(MAX_FRAMES);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < MAX_FRAMES; i++)
    {
        assert(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &instancingSemaphores[i]) == VK_SUCCESS);
        assert(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderingSemaphores[i]) == VK_SUCCESS);
        assert(vkCreateFence(device, &fenceInfo, nullptr, &instancingFences[i]) == VK_SUCCESS);
        assert(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &computeSemaphores[i]) == VK_SUCCESS);
        assert(vkCreateFence(device, &fenceInfo, nullptr, &computeFences[i]) == VK_SUCCESS);
    }
}


void App::RenderBegin()
{
    vkWaitForFences(device, 1, &instancingFences[frameIndex], VK_TRUE, UINT64_MAX);
    assert(vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, instancingSemaphores[frameIndex], VK_NULL_HANDLE, &imageIndex) == VK_SUCCESS);

    vkResetFences(device, 1, &instancingFences[frameIndex]);
    vkResetCommandBuffer(commandBuffers[frameIndex], 0);
    
    VkCommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    assert(vkBeginCommandBuffer(commandBuffers[frameIndex], &commandBufferBeginInfo) == VK_SUCCESS);

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassBeginInfo.renderArea.offset = {0, 0};
    renderPassBeginInfo.renderArea.extent = swapChainExtent;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.002f, 0.0f, 0.02f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassBeginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffers[frameIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = swapChainExtent.width;
    viewport.height = swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffers[frameIndex], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffers[frameIndex], 0, 1, &scissor);
}


void App::RenderEnd()
{
    vkCmdEndRenderPass(commandBuffers[frameIndex]);
    assert(vkEndCommandBuffer(commandBuffers[frameIndex]) == VK_SUCCESS);
    
    
    VkSemaphore waitSemaphores[] = { computeSemaphores[frameIndex], instancingSemaphores[frameIndex] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 2;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[frameIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderingSemaphores[frameIndex];
    assert(vkQueueSubmit(instancingQueue, 1, &submitInfo, instancingFences[frameIndex]) == VK_SUCCESS);


    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderingSemaphores[frameIndex];

    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    assert(vkQueuePresentKHR(presentQueue, &presentInfo) == VK_SUCCESS);
    
    frameIndex = (frameIndex + 1) % MAX_FRAMES;
}


void App::RenderGUI()
{
    imGuiWrapper.BeginFrame("Vulkan Fish");
    {
        imGuiWrapper.ShowFPS();
        ImGui::Text("%i Fishes", N);
        
        ImGui::SliderFloat("MAX_SPEED", (float*)&MAX_SPEED, 0.001f, 300.0f);
        ImGui::SliderFloat("ATTRACTION", (float*)&ATTRACTION, 0.001f, 300.0f);
        ImGui::SliderFloat("ATTRACTION_DISTANCE", (float*)&ATTRACTION_DISTANCE, 0.001f, 0.3f);
        ImGui::SliderFloat("ALIGNMENT", (float*)&ALIGNMENT, 0.001f, 300.0f);
        ImGui::SliderFloat("ALIGNMENT_DISTANCE", (float*)&ALIGNMENT_DISTANCE, 0.001f, 0.3f);
        ImGui::SliderFloat("AVOIDANCE", (float*)&AVOIDANCE, 0.001f, 300.0f);
        ImGui::SliderFloat("AVOIDANCE_DISTANCE", (float*)&AVOIDANCE_DISTANCE, 0.001f, 0.3f);
        ImGui::SliderFloat("WALL_AVOIDANCE", (float*)&WALL_AVOIDANCE, 0.001f, 1000.0f);
        ImGui::SliderFloat("VORTEX_FORCE", (float*)&VORTEX_FORCE, 0.0f, 3.0f);
        ImGui::SliderFloat3("Camera LookAt", (float*)&cameraCenter, -1.0, 1.0f);
        ImGui::SliderFloat3("Camera Pos", (float*)&cameraPos, -2.5f, 2.5f);
        ImGui::SliderFloat("Camera FOV", (float*)&cameraFov, 0.0f, 180.0f);
    }
    imGuiWrapper.EndFrame(commandBuffers[frameIndex]);
    
    instancingRenderer.cameraFov = cameraFov;
    instancingRenderer.cameraCenter = cameraCenter;
    instancingRenderer.cameraPos = cameraPos;
}


void App::Finalize()
{
    vkDestroyImageView(device, depthImageView, nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthImageMemory, nullptr);
    for (auto framebuffer : swapChainFramebuffers) vkDestroyFramebuffer(device, framebuffer, nullptr);
    for (auto imageView : swapChainImageViews) vkDestroyImageView(device, imageView, nullptr);
    vkDestroySwapchainKHR(device, swapChain, nullptr);

    computeShader.Release();
    instancingRenderer.Release();
    vkDestroyRenderPass(device, renderPass, nullptr);


    for (int i = 0; i < MAX_FRAMES; i++)
    {
        vkDestroySemaphore(device, renderingSemaphores[i], nullptr);
        vkDestroySemaphore(device, instancingSemaphores[i], nullptr);
        vkDestroyFence(device, instancingFences[i], nullptr);
    }

    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}
