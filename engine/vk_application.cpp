#include "vk_application.hpp"

#include "camera.hpp"
#include <vk_descriptors.h>
#include "lve_device.hpp"
#include "lve_renderer.hpp"
#include <vk_ui_overlay.h>


namespace lve {

    VulkanApplication::VulkanApplication(const std::string &name, bool vsync) : Application(name) {
        // Note: the order of creation matters
        lveDevice = std::make_unique<LveDevice>(*window_);
        lveRenderer = std::make_unique<LveRenderer>(*window_, *lveDevice, vsync);
        descriptorAllocator = std::make_unique<DescriptorAllocator>(lveDevice->device());

//        settings.overlay = settings.overlay && (!benchmark.active);
//        if (settings.overlay)
        {
            uiOverlay = std::make_unique<UIOverlay>(window_.get(), lveDevice.get(), lveRenderer->getSwapChain(), VK_SAMPLE_COUNT_1_BIT);
        }

        camera_ = std::make_unique<Camera>();
        camera_->type = Camera::CameraType::lookat;
        camera_->setPosition(glm::vec3(0.0f, 0.0f, -1.0f));
        camera_->setRotation(glm::vec3(159, -150, 0));
        auto extent = window_->getExtent();
        camera_->setPerspective(60.0f, (float)extent.width / (float)extent.height, 0.001f, 256.0f);
    }


    VulkanApplication::~VulkanApplication() {
        // Note: the order of destroying matters
        uiOverlay.reset();
        descriptorAllocator.reset();
        lveRenderer.reset();
        lveDevice.reset();
    }


    int VulkanApplication::run() {
        auto status = Application::run();
        // wait idle so all subsequent operations (like cleanup) don't have to do so.
        lveDevice->waitIdle();
        return status;
    }


    void VulkanApplication::update() {
        if (/*settings.overlay && */ uiOverlay->isVisible())
            uiOverlay->showOptionsWindow(frameRate.get(), [this]() { fillUI(); });

        // todo: compare performance to that of "buildCommandBuffer () + submitFrame()"
        if (auto commandBuffer = lveRenderer->beginFrame()) {
            lveRenderer->beginSwapChainRenderPass(commandBuffer);
            // order here matters
//            simpleRenderSystem.renderGameObjects(frameInfo);
//            pointLightSystem.render(frameInfo);
            draw(commandBuffer);

            if (/*settings.overlay && */ uiOverlay->isVisible())
                uiOverlay->draw(commandBuffer);

            lveRenderer->endSwapChainRenderPass(commandBuffer);
            lveRenderer->endFrame();
        }
    }


    void VulkanApplication::onResize(uint32_t width, uint32_t height) {
        auto extent = window_->getExtent();
        camera_->setPerspective(60.0f, (float)extent.width / (float)extent.height, 0.001f, 256.0f);

        //if (/*settings.overlay*/)
        uiOverlay->onResize(width, height);

        viewChanged();
    }

    void VulkanApplication::onKeyEvent(const KeyEvent &event) {
        if (/*settings.overlay && */ uiOverlay->isVisible() && uiOverlay->onKeyEvent(event))
            return; // the event has already captured and processed.

        if (event.get_action() == KeyAction::Down) {
            switch (event.get_code()) {
                case Key::W:
                    camera_->keys.up = true;
                    break;
                case Key::S:
                    camera_->keys.down = true;
                    break;
                case Key::A:
                    camera_->keys.left = true;
                    break;
                case Key::D:
                    camera_->keys.right = true;
                    break;
                case Key::P:
                    paused = !paused;
                    break;
                case Key::Space:
//                    if (settings.overlay)
                    {
                        // Toggle GUI elements when tap or clicking outside the GUI windows
                        uiOverlay->setVisible(!uiOverlay->isVisible());
                    }
                    break;
                default:
                    break;
            }
        } else if (event.get_action() == KeyAction::Up) {
            switch (event.get_code()) {
                case Key::W:
                    camera_->keys.up = false;
                    break;
                case Key::S:
                    camera_->keys.down = false;
                    break;
                case Key::A:
                    camera_->keys.left = false;
                    break;
                case Key::D:
                    camera_->keys.right = false;
                    break;
                default:
                    break;
            }
        }

        Application::onKeyEvent(event);
    }


    void VulkanApplication::onMouseEvent(const MouseEvent& event) {
        if (/*settings.overlay && */ uiOverlay->onMouseEvent(event)) {
            mousePos = glm::vec2(event.get_pos_x(), event.get_pos_y());
            return; // the event has already captured and processed.
        }

        auto x = event.get_pos_x();
        auto y = event.get_pos_y();
        int32_t dx = static_cast<int32_t>(mousePos.x - x);
        int32_t dy = static_cast<int32_t>(mousePos.y - y);
        if (mouse_buttons_.left) {
            camera_->rotate(glm::vec3(dy * camera_->rotationSpeed, -dx * camera_->rotationSpeed, 0.0f));
            viewChanged();
        }
        if (mouse_buttons_.right) {
            camera_->translate(glm::vec3(-0.0f, 0.0f, dy * .005f));
            viewChanged();
        }
        if (mouse_buttons_.middle) {
            camera_->translate(glm::vec3(-dx * 0.005f, -dy * 0.005f, 0.0f));
            viewChanged();
        }
        mousePos = glm::vec2((float)x, (float)y);
        
        if (event.get_action() == MouseAction::Down) {
            switch (event.get_button()) {
                case MouseButton::Left:
                    mouse_buttons_.left = true;
                    break;
                case MouseButton::Right:
                    mouse_buttons_.right = true;
                    break;
                case MouseButton::Middle:
                    mouse_buttons_.middle = true;
                    break;
                default:
                    break;
            }
        } else if (event.get_action() == MouseAction::Up) {
            switch (event.get_button()) {
                case MouseButton::Left:
                    mouse_buttons_.left = false;
                    break;
                case MouseButton::Right:
                    mouse_buttons_.right = false;
                    break;
                case MouseButton::Middle:
                    mouse_buttons_.middle = false;
                    break;
                default:
                    break;
            }
        }

        Application::onMouseEvent(event);
    }

}  // namespace lve
