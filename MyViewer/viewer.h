#ifndef EASY_VIEWER_H
#define EASY_VIEWER_H

#include <vector>
#include <functional>
#include <unordered_map>

#include "vk_application.hpp"

namespace lve {


    class Viewer : public VulkanApplication {
    public:
        explicit Viewer(const std::string &name = "Viewer");
        ~Viewer();

        class PointsDrawable&    createPointsDrawable();
        class LinesDrawable&     createLinesDrawable();
        class TrianglesDrawable& createTrianglesDrawable();

        /**
         * A function type, which can be triggered by user pressing a shortcut key.
         */
        using Function = std::function<void(Viewer *viewer)>;

        /**
         * @brief Bind a function that will be triggered by the 'key'.
         * @details This operation will overwrite the previous function (if exist) bound to the same key.
         * @param func The function to be executed, which will be triggered by the shortcut key.
         * @param key The shortcut key.
         * @sa Function
         */
        void bind(Key key, const Function &func) { commands_[key] = func; }

        /** @brief (Virtual) Called when the camera view has changed */
        void viewChanged() override;

    protected:
        void draw(VkCommandBuffer commandBuffer) override;
        void fillUI() override;
        void onKeyEvent(const KeyEvent&) override;

    private:
        std::vector<std::unique_ptr<class Drawable> > drawables_;
        std::unordered_map<Key, Function>  commands_;
    };

}

#endif // VULKAN_VIEWER_H