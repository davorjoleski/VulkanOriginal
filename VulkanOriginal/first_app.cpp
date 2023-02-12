#include "first_app.hpp"
#include <vulkan/vulkan_core.h>
#include <memory>

#include "keyboard_movement_controller.hpp"
#include "lve_buffer.hpp"
#include "lve_camera.hpp"
#include "point_light_system.hpp"
#include "simple_render_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <chrono>
#include <stdexcept>
#include "lve_frame_info.hpp"
#include "simple_render_system.hpp"
#include "point_light_system.hpp"
#include "lve_Texture.hpp"

namespace lve {

    FirstApp::FirstApp() {
        globalPool =
            LveDescriptorPool::Builder(lveDevice)
            .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();
        loadGameObjects();
    }

    FirstApp::~FirstApp() {}

    void FirstApp::run() {
        std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<LveBuffer>(
                lveDevice,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffers[i]->map();
        }

      Texture  texture = Texture(lveDevice, "C:\\Users\\Davor\\Desktop\\VulkanOriginal\\VulkanOriginal\\texture\\viking_room.png");

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.sampler = texture.getSampler();
        imageInfo.imageView = texture.getImageView();
        imageInfo.imageLayout = texture.getImageLayout();

        auto globalSetLayout =
            LveDescriptorSetLayout::Builder(lveDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            LveDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .writeImage(1, &imageInfo)
                .build(globalDescriptorSets[i]);
        }

        SimpleRenderSystem simpleRenderSystem{
            lveDevice,
            lveRenderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout() };
        PointLightSystem pointLightSystem{
            lveDevice,
            lveRenderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout() };
        LveCamera camera{};

        auto viewerObject = LveGameObject::createGameObject();
        viewerObject.transform.translation.z = -2.5f;
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();
        while (!lveWindow.shouldClose()) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime =
                std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = lveRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

            if (auto commandBuffer = lveRenderer.beginFrame()) {
                int frameIndex = lveRenderer.getFrameIndex();
                FrameInfo frameInfo{
                    frameIndex,
                    frameTime,
                    commandBuffer,
                    camera,
                    globalDescriptorSets[frameIndex],
                    gameObjects };

                // update
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
                pointLightSystem.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // render
                lveRenderer.beginSwapChainRenderPass(commandBuffer);

                // order here matters
                simpleRenderSystem.renderGameObjects(frameInfo);
                pointLightSystem.render(frameInfo);

                lveRenderer.endSwapChainRenderPass(commandBuffer);
                lveRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(lveDevice.device());
    }
    void FirstApp::loadGameObjects() {
        std::shared_ptr<LveModel> lveModel =
            LveModel::createModelFromFile(lveDevice, "C:\\Users\\Davor\\Desktop\\VulkanOriginal\\VulkanOriginal\\models\\flat_vase.obj");
        auto flatVase = LveGameObject::createGameObject();
        flatVase.model = lveModel;
        flatVase.transform.translation = { -.5f, .5f, 0.f };
        flatVase.transform.scale = { 3.f, 1.5f, 3.f };
        gameObjects.emplace(flatVase.getId(), std::move(flatVase)); //to move game obj into the map

        lveModel = LveModel::createModelFromFile(lveDevice, "C:\\Users\\Davor\\Desktop\\VulkanOriginal\\VulkanOriginal\\models\\smooth_vase.obj");
        auto smoothVase = LveGameObject::createGameObject();
        smoothVase.model = lveModel;
        smoothVase.transform.translation = { .5f, .5f, 0.f };
        smoothVase.transform.scale = { 3.f, 1.5f, 3.f };
        gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));


        lveModel = LveModel::createModelFromFile(lveDevice, "C:\\Users\\Davor\\Desktop\\VulkanOriginal\\VulkanOriginal\\models\\pat.obj");
        auto pat = LveGameObject::createGameObject();
        pat.model = lveModel;
        pat.transform.translation = { .5f, .5f, 0.f };
        pat.transform.scale = { 3.f, 1.5f, 3.f };
        gameObjects.emplace(pat.getId(), std::move(pat));



 


        std::vector<glm::vec3> lightColors{
              {1.f, .1f, .1f},
              {.1f, .1f, 1.f},
              {.1f, 1.f, .1f},
              {1.f, 1.f, .1f},
              {.1f, 1.f, 1.f},
              {1.f, 1.f, 1.f}
              //
        };

        for (int i = 0; i < lightColors.size(); i++) {
            auto pointLight = LveGameObject::makePointLight(0.2f);
            pointLight.color = lightColors[i];
            auto rotateLight = glm::rotate(
                glm::mat4(1.f),
                (i * glm::two_pi<float>()) / lightColors.size(),
                { 0.f, 1.f, 0.f });
            pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(1.f, 0.0f, 0.f, -0.f) * sin(30.0f));
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }
   

    }
 





}  // namespace lve