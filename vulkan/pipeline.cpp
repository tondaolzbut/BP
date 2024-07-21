#include "pipeline.h"
#include "model.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cassert>

namespace ep {

    EnginePipeline::EnginePipeline(	//KONSTRUKTOR
        EpDevice& device,
        const std::string& vertCesta,
        const std::string& fragCesta,
        const PipelineConfigInfo& configInfo)
        : epDevice{device} {
        vytvorPipeline(vertCesta, fragCesta, configInfo);
    }

    EnginePipeline::~EnginePipeline() {
        vkDestroyShaderModule(epDevice.device(), vertShaderModule, nullptr);
        vkDestroyShaderModule(epDevice.device(), fragShaderModule, nullptr);
        vkDestroyPipeline(epDevice.device(), graphicsPipeline, nullptr);
    }

    std::vector<char> EnginePipeline::readFile(const std::string& cesta) {
        std::ifstream file{ cesta, std::ios::ate | std::ios::binary }; //ate - vyhledá konec souboru (uzitecné na odhadnutí velikosti)

        if (!file.is_open()) { //kontrola otevrení souboru
            throw std::runtime_error("nepovedlo se otevrit: " + cesta);
        }

        size_t velikost = static_cast<size_t>(file.tellg()); //jsme uz na konci (viz ate), takze vrací velikost souboru
        std::vector<char> buffer(velikost); //buffer znakuu

        file.seekg(0); //posuneme se na zacatek souboru - chceme cist
        file.read(buffer.data(), velikost); //co chceme cist a kolik toho chceme precist

        file.close();
        return buffer;
    }

    void EnginePipeline::vytvorPipeline(
        const std::string& vertCesta, const std::string& fragCesta, const PipelineConfigInfo& configInfo) {
        auto vertKod = readFile(vertCesta);
        auto fragKod = readFile(fragCesta);

        assert(
            configInfo.pipelineLayout != VK_NULL_HANDLE &&
            "Cannot create graphics pipeline: no pipelineLayout provided in configInfo");
        assert(
            configInfo.renderPass != VK_NULL_HANDLE &&
            "Cannot create graphics pipeline: no renderPass provided in configInfo");

        createShaderModule(vertKod, &vertShaderModule);
        createShaderModule(fragKod, &fragShaderModule);

        VkPipelineShaderStageCreateInfo shaderStages[2] = {};
        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;     //typ shaderu
        shaderStages[0].module = vertShaderModule;
        shaderStages[0].pName = "main"; //entry function ve vertex shaderu
        shaderStages[0].flags = 0;
        shaderStages[0].pNext = nullptr;
        shaderStages[0].pSpecializationInfo = nullptr;

        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;   //typ shaderu
        shaderStages[1].module = fragShaderModule;
        shaderStages[1].pName = "main";
        shaderStages[1].flags = 0;
        shaderStages[1].pNext = nullptr;
        shaderStages[1].pSpecializationInfo = nullptr;

        auto bindingDescriptions = EpModel::Vertex::getBindingDescriptions();
        auto attributeDescriptions = EpModel::Vertex::getAttributeDescriptions();
        
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{}; //specifikujeme jak interpretujeme vertex buffer data
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

        VkPipelineViewportStateCreateInfo viewportInfo{};
        viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO; //transformace a orezavani kombinujeme do teto promenne
        viewportInfo.viewportCount = 1;                      //muzeme pouzit vice transformaci nebo orezavani
        viewportInfo.pViewports = nullptr;
        viewportInfo.scissorCount = 1;
        viewportInfo.pScissors = nullptr;        //BLOK PRESUNUT VIZ NIZE

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;   //VYTVORENI PIPELINE OBJEKTU
        pipelineInfo.stageCount = 2;                                            //kolik mame shaderu
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
        pipelineInfo.pViewportState = &viewportInfo;
        pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
        pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
        pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
        pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
        pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;               //Optional

        pipelineInfo.layout = configInfo.pipelineLayout;
        pipelineInfo.renderPass = configInfo.renderPass;
        pipelineInfo.subpass = configInfo.subpass;

        pipelineInfo.basePipelineIndex = -1;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(
            epDevice.device(),
            VK_NULL_HANDLE,
            1,
            &pipelineInfo,
            nullptr,
            &graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("nepovedlo se vytvorit graficky retezec");
        }
    }
    void EnginePipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule) {
        VkShaderModuleCreateInfo createInfo{};   
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data()); //ve vulkanu casto volame funkce s ukazatelem na structs s hodnotami misto hodne parametru

        if (vkCreateShaderModule(epDevice.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("nepovedlo se vytvorit shader modul");
        }
    }

    void EnginePipeline::bind(VkCommandBuffer commandBuffer) {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    }                                    //specifikujeme GRAPHICS pipeline  (oproti COMPUTE A RAY_TRACING_KHR)

    //PipelineConfigInfo EnginePipeline::defaultPipelineConfigInfo(uint32_t width, uint32_t height) {
    //    PipelineConfigInfo configInfo{};
    void EnginePipeline::defaultPipelineConfigInfo(PipelineConfigInfo& configInfo) {

        configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; //BUDEME SHLUKOVAT PO TRECH VRCHOLECH
        configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE; //jestli pouzivame strips viz VkPrimitiveTechnology

        /*configInfo.viewport.x = 0.0f;
        configInfo.viewport.y = 0.0f;
        configInfo.viewport.width = static_cast<float>(width);  //muzeme definovat transformace
        configInfo.viewport.height = static_cast<float>(height);
        configInfo.viewport.minDepth = 0.0f;                    //muzeme definovat orezavani
        configInfo.viewport.maxDepth = 1.0f;

        configInfo.scissor.offset = { 0, 0 };
        configInfo.scissor.extent = { width, height };*/

        /*configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO; //transformace a orezavani kombinujeme do teto promenne
        configInfo.viewportInfo.viewportCount = 1;                      //muzeme pouzit vice transformaci nebo orezavani
        configInfo.viewportInfo.pViewports = &configInfo.viewport;
        configInfo.viewportInfo.scissorCount = 1;
        configInfo.viewportInfo.pScissors = &configInfo.scissor; */  //PRESUNUTO VYSE

        configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO; //NASTAVENI RASTERIZACE
        configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;           //hodnoty Z mimo interval 0 a 1 se srazi do tohoto intervalu 
        configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;    //pokud chceme vyuzit pouze prvnich par kroku grafickeho retezce
        configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        configInfo.rasterizationInfo.lineWidth = 1.0f;
        configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;          //muze ignorovat polygony ktere jsou k nam zady
        configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;   //poradi ve kterem jsou ulozeny vrcholy urcuji zda je polygon zady k nam
        configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;            //moznost upravit hloubku:
        configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
        configInfo.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
        configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional

        configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO; //sprava okrajuu polygonuu - MSAA
        configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
        configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        configInfo.multisampleInfo.minSampleShading = 1.0f;           // Optional
        configInfo.multisampleInfo.pSampleMask = nullptr;             // Optional
        configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
        configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional

        configInfo.colorBlendAttachment.colorWriteMask =                //jak kombinujeme barvy ve framebufferu
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        configInfo.colorBlendAttachment.blendEnable = VK_FALSE;         //jak michame barvy prichozi s barvami co už jsou ve framebufferu
        configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
        configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
        configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
        configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
        configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
        configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

        configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
        configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
        configInfo.colorBlendInfo.attachmentCount = 1;
        configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
        configInfo.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
        configInfo.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
        configInfo.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
        configInfo.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

        configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO; //pro kazdy pixel si pamatuje ktery fragment je nejblize
        configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        configInfo.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
        configInfo.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
        configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
        configInfo.depthStencilInfo.front = {};  // Optional
        configInfo.depthStencilInfo.back = {};   // Optional

        configInfo.dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
        configInfo.dynamicStateInfo.dynamicStateCount =
            static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
        configInfo.dynamicStateInfo.flags = 0;

        //return configInfo;
    }
}