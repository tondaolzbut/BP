#pragma once

#include "ep_device.hpp"
#include "game_object.h"
#include "pipeline.h"

// std
#include <memory>
#include <vector>

namespace ep {
	class SimpleRenderSystem {
	public:
		SimpleRenderSystem(EpDevice& device, VkRenderPass renderPass);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<EpGameObject>& gameObjects);

	private:
		void createPipelineLayout();
		void createPipeline(VkRenderPass renderPass);

		EpDevice& epDevice;

		std::unique_ptr<EnginePipeline> enginePipeline;
		VkPipelineLayout pipelineLayout;
	};
}
