#pragma once

#include <vk_application.hpp>
#include <lve_game_object.hpp>


namespace lve {

    class FirstApp : public VulkanApplication {
    public:
        explicit FirstApp(const std::string &name = "FirstApp");
        ~FirstApp();

        int run() override;

    private:
        void loadGameObjects();

    private:
        LveGameObject::Map gameObjects;
    };
}  // namespace lve
