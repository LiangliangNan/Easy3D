/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EASY3D_GUI_CANVAS_H
#define EASY3D_GUI_CANVAS_H

#include <vector>

#include "tools/tool_manager.h"

namespace easy3d {

    class Camera;
    class Model;

    namespace tools {
        class ToolManager;
    }

    // Canvas defined the basic interface for interactive tools
    class Canvas {
    public:
        Canvas() {
            tool_manager_ = new tools::ToolManager(this);
        }
        ~Canvas() { delete tool_manager_; }

        virtual Camera *camera() = 0;
        virtual const Camera *camera() const = 0;

        virtual const std::vector<Model*>& models() const = 0;

        tools::ToolManager* tool_manager() const { return tool_manager_; }

    private:
        tools::ToolManager* tool_manager_;
    };

}


#endif //EASY3D_GUI_CANVAS_H
