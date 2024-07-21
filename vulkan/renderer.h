#pragma once

#include "ep_device.hpp"
#include "ep_swap_chain.hpp"
#include "okno.h"

// std
#include <cassert>
#include <memory>
#include <vector>

namespace ep {
    class MyRenderer {
    public:
        MyRenderer(EpOkno& okno, EpDevice& device);
        ~MyRenderer();

        MyRenderer(const MyRenderer&) = delete;
        MyRenderer& operator=(const MyRenderer&) = delete;

        VkRenderPass getSwapChainRenderPass() const { return epSwapChain->getRenderPass(); }
        bool isFrameInProgress() const { return isFrameStarted; }

        VkCommandBuffer getCurrentCommandBuffer() const {
            assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
            return commandBuffers[currentFrameIndex];
        }

        int getFrameIndex() const {
            assert(isFrameStarted && "Cannot get frame index when frame not in progress");
            return currentFrameIndex;
        }

        VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

    private:
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapChain();

        EpOkno& epOkno;
        EpDevice& epDevice;
        std::unique_ptr<EpSwapChain> epSwapChain;
        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex;
        int currentFrameIndex;
        bool isFrameStarted;
    };
}