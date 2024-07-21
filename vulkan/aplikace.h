#pragma once

#include "okno.h"
//#include "pipeline.h"
#include "ep_device.hpp"
//#include "ep_swap_chain.hpp"
#include "model.h"
#include "game_object.h"
#include"renderer.h"

#include<memory>
#include<vector>

namespace ep {
	class RenderApp {
	public:
		static constexpr int SIRKA = 800;
		static constexpr int VYSKA = 600;

		RenderApp();
		~RenderApp();

		RenderApp(const RenderApp&) = delete;
		RenderApp& operator=(const RenderApp&) = delete;

		void run();

	private:
		//void createPipelineLayout();
		//void createPipeline();
		//void createCommandBuffers();
		//void freeCommandBuffers();
		//void drawFrame();
		//void recreateSwapChain();
		//void recordCommandBuffer(int imageIndex);
		//void loadModels();
		void loadGameObjects();
		//void renderGameObjects(VkCommandBuffer commandBuffer);

		EpOkno epOkno{ 
			SIRKA, 
			VYSKA, 
			"Vulkan" };
		EpDevice epDevice{ 
			epOkno };
		MyRenderer myRenderer{ epOkno, epDevice };
		/*EpSwapChain epSwapChain{
			epDevice, 
			epOkno.getExtent() };*/
		//std::unique_ptr<EpSwapChain> epSwapChain;
		/*EnginePipeline enginePipeline{
			epDevice,
			"Dependencies/shaders/shader.vert.spv",
			"Dependencies/shaders/shader.frag.spv",
			EnginePipeline::defaultPipelineConfigInfo(SIRKA, VYSKA) };*/
		//std::unique_ptr<EnginePipeline> enginePipeline;	//cpp smart pointer - odpovida za spravu pameti
		//VkPipelineLayout pipelineLayout;
		//std::vector<VkCommandBuffer> commandBuffers;
		std::unique_ptr<EpModel> epModel;
		std::vector<EpGameObject> gameObjects;
	};
}