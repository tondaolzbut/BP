#include "model.h"
#include"ep_utils.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include<tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <cassert>
#include <cstring>
#include<iostream>
#include<unordered_map>

namespace std {
    template <>
    struct hash<ep::EpModel::Vertex> {
        size_t operator()(ep::EpModel::Vertex const& vertex) const {
            size_t seed = 0;
            ep::hashCombine(seed, vertex.position, vertex.barva, vertex.normal, vertex.uv);
            return seed;
        }
    };
}

namespace ep {

    EpModel::EpModel(EpDevice& device, const EpModel::Builder &builder) : epDevice{ device } {
        //createVertexBuffers(vertices);
        createVertexBuffers(builder.vertices);
        createIndexBuffers(builder.indices);
    }

    EpModel::~EpModel() {
        vkDestroyBuffer(epDevice.device(), vertexBuffer, nullptr);
        vkFreeMemory(epDevice.device(), vertexBufferMemory, nullptr); //allocation callback

        if (hasIndexBuffer) {
            vkDestroyBuffer(epDevice.device(), indexBuffer, nullptr);
            vkFreeMemory(epDevice.device(), indexBufferMemory, nullptr); //allocation callback
        }
    }

    std::unique_ptr<EpModel> EpModel::createModelFromFile(EpDevice& device, const std::string& filepath) {
        Builder builder{};
        builder.loadModel(filepath);
        std::cout << "pocet vrcholu:" << builder.vertices.size() << "\n";
        return std::make_unique<EpModel>(device, builder);
    }

    void EpModel::createVertexBuffers(const std::vector<Vertex>& vertices) {
        vertexCount = static_cast<uint32_t>(vertices.size());           //pocet vrcholu
        assert(vertexCount >= 3 && "Vertex count must be at least 3");  //vynuceni alespon 3 vrcholu (trojuhelnik)
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;    //velikost bufferu KRAT pocet vrcholu -> bytes
        
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        
        epDevice.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, //chceme mit pamet dostupnou z cpu (pro zapis)
            stagingBuffer,                                                               //.. cpu - host, gpu - device
            stagingBufferMemory);

        void* data;                                      //.offset      //.no mapped vk memory flags
        vkMapMemory(epDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data); //vytvari region host pameti odpovidajici device pameti
        memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        //bere data vrcholuu a kopiruje je do host regionu -> Flush() automaticky
        vkUnmapMemory(epDevice.device(), stagingBufferMemory); //u hosta (cpu) uz nepotrebujeme

        epDevice.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,  //buffer pro vrcholy
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            vertexBuffer,                                                               //.. cpu - host, gpu - device
            vertexBufferMemory);

        epDevice.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

        vkDestroyBuffer(epDevice.device(), stagingBuffer, nullptr);
        vkFreeMemory(epDevice.device(), stagingBufferMemory, nullptr); //allocation callback
    }

    void EpModel::createIndexBuffers(const std::vector<uint32_t>& indices) {
        indexCount = static_cast<uint32_t>(indices.size());           //pocet vrcholu
        hasIndexBuffer = indexCount > 0;

        if (!hasIndexBuffer) {
            return;
        }

        VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;    //velikost bufferu KRAT pocet vrcholu -> bytes
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        epDevice.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, //chceme mit pamet dostupnou z cpu (pro zapis)
            stagingBuffer,                                                               //.. cpu - host, gpu - device
            stagingBufferMemory);

        void* data;                                      //.offset      //.no mapped vk memory flags
        vkMapMemory(epDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data); //vytvari region host pameti odpovidajici device pameti
        memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
        //bere data vrcholuu a kopiruje je do host regionu -> Flush() automaticky
        vkUnmapMemory(epDevice.device(), stagingBufferMemory); //u hosta (cpu) uz nepotrebujeme

        epDevice.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,  //buffer pro vrcholy
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            indexBuffer,                                                               //.. cpu - host, gpu - device
            indexBufferMemory);

        epDevice.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

        vkDestroyBuffer(epDevice.device(), stagingBuffer, nullptr);
        vkFreeMemory(epDevice.device(), stagingBufferMemory, nullptr); //allocation callback
    }

    void EpModel::draw(VkCommandBuffer commandBuffer) {
        if (hasIndexBuffer) {
            vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0); //indexed draw
        }
        else {
            vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0); //instance, 1. vertex index, 1. instance index
        }
            
    }

    void EpModel::bind(VkCommandBuffer commandBuffer) {
        VkBuffer buffers[] = { vertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets); //record to command buffer 1 vertex buffer
    
        if (hasIndexBuffer) {
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32); //pro male modely staci 16
        }
    }

    std::vector<VkVertexInputBindingDescription> EpModel::Vertex::getBindingDescriptions() {     //POPISUJE JEDEN VERTEX BUFFER
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> EpModel::Vertex::getAttributeDescriptions() { //POPISUJE ATRIBUTY
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2); //pocet atributu
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; //protoze vec3 na pozici
        attributeDescriptions[0].offset = offsetof(Vertex, position);;

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, barva);
        return attributeDescriptions;
    }

    void EpModel::Builder::loadModel(const std::string& filepath) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
            throw std::runtime_error(warn + err);
        }

        vertices.clear();
        indices.clear();

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};

                if (index.vertex_index >= 0) {
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2],
                    };

                    vertex.barva = {
                        attrib.colors[3 * index.vertex_index + 0],
                        attrib.colors[3 * index.vertex_index + 1],
                        attrib.colors[3 * index.vertex_index + 2],
                    };
                }

                if (index.normal_index >= 0) {
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2],
                    };
                }

                if (index.texcoord_index >= 0) {
                    vertex.uv = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1],
                    };
                }

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
    }
}