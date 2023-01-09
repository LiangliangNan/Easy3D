#include "first_app.hpp"

#include "lve_buffer.hpp"
#include "lve_camera.hpp"
#include "systems/point_light_system.hpp"
#include "systems/simple_render_system.hpp"

// std
#include <array>
#include <chrono>
#include <stdexcept>

// libs
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "vk_descriptors.h"
#include "lve_device.hpp"
#include "lve_renderer.hpp"
#include <lve_swap_chain.hpp>


namespace lve {

    FirstApp::FirstApp(const std::string &name) : VulkanApplication(name) {
        loadGameObjects();
    }

    FirstApp::~FirstApp() {
        gameObjects.clear();
    }

    int FirstApp::run() {
        std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<LveBuffer>(
                    *lveDevice,
                    sizeof(GlobalUbo),
                    1,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffers[i]->map();
        }

        std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        VkDescriptorSetLayout globalSetLayout;
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            descriptorAllocator->begin()
                    .bind(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .write(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &bufferInfo)
                    .build(globalDescriptorSets[i], globalSetLayout);
        }

        SimpleRenderSystem simpleRenderSystem{
                *lveDevice,
                lveRenderer->getSwapChainRenderPass(),
                globalSetLayout};
        PointLightSystem pointLightSystem{
                *lveDevice,
                lveRenderer->getSwapChainRenderPass(),
                globalSetLayout};
        LveCamera camera{};

        auto viewerObject = LveGameObject::createGameObject();
        viewerObject.transform.translation.z = -2.5f;
//CameraController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();
        while (!window_->shouldClose()) {
            Application::update();
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;
            //cameraController.moveInPlaneXZ(window_->getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = lveRenderer->getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

            // todo: compare performance to that of "buildCommandBuffer () + submitFrame()"
            if (auto commandBuffer = lveRenderer->beginFrame()) {
                int frameIndex = lveRenderer->getFrameIndex();
                FrameInfo frameInfo{
                        frameIndex,
                        frameTime,
                        commandBuffer,
                        camera,
                        globalDescriptorSets[frameIndex],
                        gameObjects};

                // update
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
                pointLightSystem.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // render
                lveRenderer->beginSwapChainRenderPass(commandBuffer);

                // order here matters
                simpleRenderSystem.renderGameObjects(frameInfo);
                pointLightSystem.render(frameInfo);

                lveRenderer->endSwapChainRenderPass(commandBuffer);
                lveRenderer->endFrame();
            }
            window_->processEvents();
        }

        lveDevice->waitIdle();
        return EXIT_SUCCESS;
    }

    void FirstApp::loadGameObjects() {
        std::shared_ptr<LveModel> lveModel =
                LveModel::createModelFromFile(*lveDevice, std::string(RESOURCE_DIR) + "models/flat_vase.obj");
        auto flatVase = LveGameObject::createGameObject();
        flatVase.model = lveModel;
        flatVase.transform.translation = {-.5f, .5f, 0.f};
        flatVase.transform.scale = {3.f, 1.5f, 3.f};
        gameObjects.emplace(flatVase.getId(), std::move(flatVase));

        lveModel = LveModel::createModelFromFile(*lveDevice, std::string(RESOURCE_DIR) + "models/smooth_vase.obj");
        auto smoothVase = LveGameObject::createGameObject();
        smoothVase.model = lveModel;
        smoothVase.transform.translation = {.5f, .5f, 0.f};
        smoothVase.transform.scale = {3.f, 1.5f, 3.f};
        gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

        lveModel = LveModel::createModelFromFile(*lveDevice, std::string(RESOURCE_DIR) + "models/quad.obj");
        auto floor = LveGameObject::createGameObject();
        floor.model = lveModel;
        floor.transform.translation = {0.f, .5f, 0.f};
        floor.transform.scale = {3.f, 1.f, 3.f};
        gameObjects.emplace(floor.getId(), std::move(floor));

        std::vector<glm::vec3> lightColors{
                {1.f, .1f, .1f},
                {.1f, .1f, 1.f},
                {.1f, 1.f, .1f},
                {1.f, 1.f, .1f},
                {.1f, 1.f, 1.f},
                {1.f, 1.f, 1.f}  //
        };

        for (int i = 0; i < lightColors.size(); i++) {
            auto pointLight = LveGameObject::makePointLight(0.2f);
            pointLight.color = lightColors[i];
            auto rotateLight = glm::rotate(
                    glm::mat4(1.f),
                    (i * glm::two_pi<float>()) / lightColors.size(),
                    {0.f, -1.f, 0.f});
            pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }
    }

}  // namespace lve
