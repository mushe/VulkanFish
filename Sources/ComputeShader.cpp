#include "ComputeShader.hpp"
#include "Util.hpp"

void ComputeShader::Init(VkDevice* device, VkPhysicalDevice* physicalDevice, uint32_t particleNum, std::vector<VkBuffer> shaderStorageBuffers, VkCommandPool* commandPool)
{
    _device = device;
    _physicalDevice = physicalDevice;
    _N = particleNum;
    _shaderStorageBuffers = shaderStorageBuffers;
    _commandPool = commandPool;
    
    CreateComputeDescriptorSetLayout();
    CreateComputePipeline();
    CreateComputeUniformBuffers();
    CreateComputeDescriptorPool();
    CreateComputeDescriptorSets();
    CreateComputeCommandBuffers();
}



void ComputeShader::Execute(uint32_t frame, VkSemaphore* computeFinishedSemaphore, VkFence* computeInFlightFence, VkQueue queue)
{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // Compute submission
    vkWaitForFences(*_device, 1, computeInFlightFence, VK_TRUE, UINT64_MAX);

    
    
    ParticleParameters ubo{};
    ubo.MAX_SPEED = _params.MAX_SPEED;
    ubo.ATTRACTION = _params.ATTRACTION;
    ubo.WALL_AVOIDANCE = _params.WALL_AVOIDANCE;
    ubo.ATTRACTION_DISTANCE = _params.ATTRACTION_DISTANCE;
    ubo.ALIGNMENT_DISTANCE = _params.ALIGNMENT_DISTANCE;
    ubo.ALIGNMENT = _params.ALIGNMENT;
    ubo.AVOIDANCE_DISTANCE = _params.AVOIDANCE_DISTANCE;
    ubo.AVOIDANCE = _params.AVOIDANCE;
    ubo.VORTEX_FORCE = _params.VORTEX_FORCE;
    memcpy(_computeUniformBuffersMapped[frame], &ubo, sizeof(ubo));
    
    
    

    vkResetFences(*_device, 1, computeInFlightFence);

    vkResetCommandBuffer(_computeCommandBuffers[frame], /*VkCommandBufferResetFlagBits*/ 0);
    
    
    
    
    
    
    //RecordComputeCommandBuffer();
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    assert(vkBeginCommandBuffer(_computeCommandBuffers[frame], &beginInfo) == VK_SUCCESS);

    vkCmdBindPipeline(_computeCommandBuffers[frame], VK_PIPELINE_BIND_POINT_COMPUTE, _computePipeline);

    vkCmdBindDescriptorSets(_computeCommandBuffers[frame], VK_PIPELINE_BIND_POINT_COMPUTE, _computePipelineLayout, 0, 1, &_computeDescriptorSets[frame], 0, nullptr);

    vkCmdDispatch(_computeCommandBuffers[frame], _N / 256, 1, 1);

    assert(vkEndCommandBuffer(_computeCommandBuffers[frame]) == VK_SUCCESS);
    
    
    
    
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_computeCommandBuffers[frame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = computeFinishedSemaphore;

    assert(vkQueueSubmit(queue, 1, &submitInfo, *computeInFlightFence) == VK_SUCCESS);
}


void ComputeShader::Release()
{
    
}


void ComputeShader::CreateComputeDescriptorSetLayout()
{
    std::array<VkDescriptorSetLayoutBinding, 3> layoutBindings{};
    layoutBindings[0].binding = 0;
    layoutBindings[0].descriptorCount = 1;
    layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBindings[0].pImmutableSamplers = nullptr;
    layoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    layoutBindings[1].binding = 1;
    layoutBindings[1].descriptorCount = 1;
    layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBindings[1].pImmutableSamplers = nullptr;
    layoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    layoutBindings[2].binding = 2;
    layoutBindings[2].descriptorCount = 1;
    layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBindings[2].pImmutableSamplers = nullptr;
    layoutBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 3;
    layoutInfo.pBindings = layoutBindings.data();

    assert(vkCreateDescriptorSetLayout(*_device, &layoutInfo, nullptr, &_computeDescriptorSetLayout) == VK_SUCCESS);
}


void ComputeShader::CreateComputePipeline()
{
    auto computeShaderCode = Util::ReadFile("../Shaders/compute.spv");

    VkShaderModule computeShaderModule = Util::CreateShaderModule(*_device, computeShaderCode);

    VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
    computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageInfo.module = computeShaderModule;
    computeShaderStageInfo.pName = "main";

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &_computeDescriptorSetLayout;

    assert(vkCreatePipelineLayout(*_device, &pipelineLayoutInfo, nullptr, &_computePipelineLayout) == VK_SUCCESS);

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = _computePipelineLayout;
    pipelineInfo.stage = computeShaderStageInfo;

    assert(vkCreateComputePipelines(*_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_computePipeline) == VK_SUCCESS);

    vkDestroyShaderModule(*_device, computeShaderModule, nullptr);
    
}


void ComputeShader::CreateComputeUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(ParticleParameters);

    _computeUniformBuffers.resize(MAX_FRAMES);
    _computeUniformBuffersMemory.resize(MAX_FRAMES);
    _computeUniformBuffersMapped.resize(MAX_FRAMES);

    for (size_t i = 0; i < MAX_FRAMES; i++)
    {
        Util::CreateBuffer(*_device, *_physicalDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _computeUniformBuffers[i], _computeUniformBuffersMemory[i]);

        vkMapMemory(*_device, _computeUniformBuffersMemory[i], 0, bufferSize, 0, &_computeUniformBuffersMapped[i]);
    }
}

void ComputeShader::CreateComputeDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES);
    
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES) * 2;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES);

    assert(vkCreateDescriptorPool(*_device, &poolInfo, nullptr, &_computeDescriptorPool) == VK_SUCCESS);
}

void ComputeShader::CreateComputeDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES, _computeDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = _computeDescriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES);
    allocInfo.pSetLayouts = layouts.data();

    _computeDescriptorSets.resize(MAX_FRAMES);
    
    assert(vkAllocateDescriptorSets(*_device, &allocInfo, _computeDescriptorSets.data()) == VK_SUCCESS);

    for (size_t i = 0; i < MAX_FRAMES; i++)
    {
        VkDescriptorBufferInfo uniformBufferInfo{};
        uniformBufferInfo.buffer = _computeUniformBuffers[i];
        uniformBufferInfo.offset = 0;
        uniformBufferInfo.range = sizeof(ParticleParameters);

        std::array<VkWriteDescriptorSet, 3> descriptorWrites{};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = _computeDescriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &uniformBufferInfo;

        VkDescriptorBufferInfo storageBufferInfoLastFrame{};
        storageBufferInfoLastFrame.buffer = _shaderStorageBuffers[(i - 1) % MAX_FRAMES];
        storageBufferInfoLastFrame.offset = 0;
        storageBufferInfoLastFrame.range = sizeof(InstanceParameters) * _N;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = _computeDescriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &storageBufferInfoLastFrame;

        VkDescriptorBufferInfo storageBufferInfoCurrentFrame{};
        storageBufferInfoCurrentFrame.buffer = _shaderStorageBuffers[i];
        storageBufferInfoCurrentFrame.offset = 0;
        storageBufferInfoCurrentFrame.range = sizeof(InstanceParameters) * _N;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = _computeDescriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pBufferInfo = &storageBufferInfoCurrentFrame;

        vkUpdateDescriptorSets(*_device, 3, descriptorWrites.data(), 0, nullptr);
    }
}


void ComputeShader::CreateComputeCommandBuffers()
{
    _computeCommandBuffers.resize(MAX_FRAMES);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = *_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)_computeCommandBuffers.size();

    assert(vkAllocateCommandBuffers(*_device, &allocInfo, _computeCommandBuffers.data()) == VK_SUCCESS);
}

void ComputeShader::SetParameters(ParticleParameters params)
{ 
    _params = params;
}

