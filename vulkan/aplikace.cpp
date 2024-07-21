#include "aplikace.h"
#include<stdexcept>
#include<array>
#include <cassert>
#include <glm/gtc/constants.hpp>
#include"render_system.h"

#include<chrono>
#include<iostream>

#define GLM_FORCE_RADIANS   //uhly budou ocekavany v radianech
#define GLM_FORCE_DEPTH_ZERO_TO_ONE //glm bude ocekavat 0 - 1 oproti -1 - 1 (oproti opengl)
#include <glm/glm.hpp>

namespace ep {
    /*struct SomePushConstantData {

        glm::mat2 transform{ 1.f };
        glm::vec2 offset;
        alignas(16) glm::vec3 barva;
    };*/

    RenderApp::RenderApp() {
        //loadModels();
        loadGameObjects();
        //createPipelineLayout();
        //recreateSwapChain();
        //createCommandBuffers();
        //createPipeline();
        
    }
    RenderApp::~RenderApp() { }

    void RenderApp::run() {
        SimpleRenderSystem simpleRenderSystem{ epDevice, myRenderer.getSwapChainRenderPass() };

        /*
        //PREDCYKLUS
        glfwPollEvents();

        if (auto commandBuffer = myRenderer.beginFrame()) {
            myRenderer.beginSwapChainRenderPass(commandBuffer);
            simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects); // Assuming this renders your game objects
            myRenderer.endSwapChainRenderPass(commandBuffer);
            myRenderer.endFrame();
        }
        //KONEC PREDCYKLU
        */

        // Timing variables
        auto startTime = std::chrono::high_resolution_clock::now();
        int frameCount = 0;

        // Main rendering loop
        while (!epOkno.oknoZavreno() && frameCount < 5000) { 
            // Handle window events
            glfwPollEvents();

            // Render frame using Vulkan
            if (auto commandBuffer = myRenderer.beginFrame()) {
                myRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects); // Assuming this renders your game objects
                myRenderer.endSwapChainRenderPass(commandBuffer);
                myRenderer.endFrame();
                frameCount++;
            }
        }

        // Calculate total time taken
        auto endTime = std::chrono::high_resolution_clock::now();
        float totalTime = std::chrono::duration<float, std::chrono::seconds::period>(endTime - startTime).count();

        // Output time taken
        std::cout << "Vykreslovani trvalo " << totalTime << " vterin." << std::endl;

        vkDeviceWaitIdle(epDevice.device()); //aby nevyskocily validation layer chyby pri vypnuti okna (ceka na ukonceni GPU operaci)
    }
    
    std::unique_ptr<EpModel> createCubeModel(EpDevice& device, glm::vec3 offset) {
        EpModel::Builder modelBuilder{};
        modelBuilder.vertices = {
            // left face (white)
            {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
            {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},

            // right face (yellow)
            {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
            {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},

            // top face (orange, remember y axis points down)
            {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
            {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},

            // bottom face (red)
            {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
            {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},

            // nose face (blue)
            {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
            {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},

            // tail face (green)
            {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
            {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        };
        for (auto& v : modelBuilder.vertices) {
            v.position += offset;
        }

        modelBuilder.indices = { 0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
                                12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21 };

        //return std::make_unique<EpModel>(device, modelBuilder);
        return std::make_unique<EpModel>(device, modelBuilder);
    }

    void RenderApp::loadGameObjects() {
        //std::shared_ptr<EpModel> epModel = createCubeModel(epDevice, { .0f, .0f, .0f });
        std::shared_ptr<EpModel> epModel = EpModel::createModelFromFile(epDevice, "models/colored_cube.obj");
        auto cube = EpGameObject::createGameObject();
        cube.model = epModel;
        cube.transform.translation = { .0f, .0f, .5f };
        cube.transform.scale = glm::vec3(0.9f); //kosticka
        gameObjects.push_back(std::move(cube));
    }

}