/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
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
 ********************************************************************/

#ifndef EASY3D_FILEIO_TRANSLATOR_H
#define EASY3D_FILEIO_TRANSLATOR_H

#include <easy3d/core/types.h>


namespace easy3d {

    /**
     * \brief Manages the translation of all the models during the file IO.
     * \class Translator easy3d/fileio/translator.h
    */
	class Translator
	{
	public:

        /**
         * \brief Returns the instance of the translator
         */
       static Translator* instance();

       enum Status {
           DISABLED, // default behavior: no translation will be recorded and applied to the model.
           TRANSLATE_USE_FIRST_POINT,  // translates the current model w.r.t. its first vertex (also recorded).
           TRANSLATE_USE_LAST_KNOWN_OFFSET  // translates the current model using the last known translation
       };

       /**
        * \brief Sets the status of the translator.
        * \param s The new status to be set.
        */
        void set_status(Status s) { status_ = s; }

        /**
         * \brief Checks the status of the translator.
         * \return The status of the translator.
         */
        Status status() const { return status_; }

        /**
         * \brief Sets the translation vector. If enabled, this translation will be applied to models loaded later on.
         * \param t The translation vector.
         */
        void set_translation(const dvec3& t) { translation_ = t; }

        /**
         * \brief Gets the translation vector.
         * \return The translation vector.
         */
        const dvec3& translation() const { return translation_; }

    private:
        Translator() : status_(DISABLED) {}

    private:
        Status status_;
        dvec3  translation_;
	};

} // namespace easy3d

#endif // EASY3D_FILEIO_TRANSLATOR_H
