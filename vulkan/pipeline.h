#pragma once

#include "ep_device.hpp"


#include <string>
#include <vector>

namespace ep {
	struct PipelineConfigInfo { //NASTAVENI FAZI VYKRESLOVACIHO RETEZCE
		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo() = default;

		//VkViewport viewport;
		//VkRect2D scissor;
		//VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;

		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicStateInfo;

		VkPipelineLayout pipelineLayout = nullptr;	//specifikujeme mimo funkci
		VkRenderPass renderPass = nullptr;			//specifikujeme mimo funkci
		uint32_t subpass = 0;
	};

	class EnginePipeline {
		public:
			EnginePipeline(	//KONSTRUKTOR
				EpDevice& device,
				const std::string& vertCesta,
				const std::string& fragCesta,
				const PipelineConfigInfo& configInfo);
			~EnginePipeline();

			EnginePipeline(const EnginePipeline&) = delete;
			EnginePipeline& operator=(const EnginePipeline&) = delete;
			EnginePipeline() = default;

			void bind(VkCommandBuffer commandBuffer);

			static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

		private:
			static std::vector<char> readFile(const std::string& cesta);
			void vytvorPipeline(
				const std::string& vertCesta,
				const std::string& fragCesta,
				const PipelineConfigInfo& configInfo);
	

		void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

		EpDevice& epDevice;
		VkPipeline graphicsPipeline;
		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;
	};
}