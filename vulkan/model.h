#pragma once

#include "ep_device.hpp"

#define GLM_FORCE_RADIANS   //uhly budou ocekavany v radianech
#define GLM_FORCE_DEPTH_ZERO_TO_ONE //glm bude ocekavat 0 - 1 oproti -1 - 1 (oproti opengl)
#include <glm/glm.hpp>

#include<memory>
#include <vector>

namespace ep {
    class EpModel { //úcel - prevzeti dat z modelu a alokovani pameti
    public:
        struct Vertex {
            glm::vec3 position;
            glm::vec3 barva;
            glm::vec3 normal{};
            glm::vec2 uv{};
    
            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

            bool operator==(const Vertex& other) const {
                return position == other.position && barva == other.barva && normal == other.normal &&
                    uv == other.uv;
            }
        };

        struct Builder {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};

            void loadModel(const std::string& filepath);
        };

        //EpModel(EpDevice& device, const std::vector<Vertex>& vertices);
        EpModel(EpDevice& device, const EpModel::Builder& builder);
        ~EpModel();

        EpModel(const EpModel&) = delete;
        EpModel& operator=(const EpModel&) = delete;

        static std::unique_ptr<EpModel> createModelFromFile(EpDevice& device, const std::string& filepath);

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

    private:
        void createVertexBuffers(const std::vector<Vertex>& vertices);
        void createIndexBuffers(const std::vector<uint32_t>& indices);

        EpDevice& epDevice;

        VkBuffer vertexBuffer;              //buffer..
        VkDeviceMemory vertexBufferMemory;  //.. a buffer memory jsou 2 ruzne objekty!!
        uint32_t vertexCount;

        bool hasIndexBuffer = false;       //chci moznost ignorovat
        VkBuffer indexBuffer;              //buffer..
        VkDeviceMemory indexBufferMemory;  //.. a buffer memory jsou 2 ruzne objekty!!
        uint32_t indexCount;
    };
}
