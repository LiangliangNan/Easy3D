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

#ifndef EASY3D_UTIL_LINE_STREAM_H
#define EASY3D_UTIL_LINE_STREAM_H

#include <iostream>
#include <sstream>
#include <cassert>


namespace easy3d {

    /**
     * \brief File input/output functionalities.
     * \namespace easy3d::io
     */
    namespace io {

        /**
         * \brief Input stream class to operate on ASCII files.
         * \class LineInputStream easy3d/util/line_stream.h
         */
        class LineInputStream {
        public:
            /**
             * \brief Constructor.
             * \param in The input stream.
             */
            explicit LineInputStream(std::istream &in) : in_(in), line_in_(nullptr) {}
            /**
             * \brief Destructor.
             */
            ~LineInputStream() {
                delete line_in_;
                line_in_ = nullptr;
            }
            /**
             * \brief Check if the end of the file has been reached.
             * \return True if the end of the file has been reached, false otherwise.
             */
            bool eof() const { return in_.eof(); }
            /**
             * \brief Check if the end of the line has been reached.
             * \return True if the end of the line has been reached, false otherwise.
             */
            bool eol() const { return line_in_ == nullptr || line_in_->eof(); }
            /**
             * \brief Check if the stream has failed.
             * \return True if the stream has failed, false otherwise.
             */
            bool fail() const { return in_.fail() || line_in_->fail(); }
            /**
             * \brief Read the next line from the input stream.
             */
            void get_line() {
                getline(in_, buffer_);
                delete line_in_;
                line_in_ = new std::istringstream(buffer_);
            }
            /**
             * \brief Get the current line as an input stream.
             * \return The current line as an input stream.
             */
            std::istream &line() {
                assert(line_in_ != nullptr);
                return *line_in_;
            }
            /**
             * \brief Get the current line as a string.
             * \return The current line as a string.
             */
            const std::string &current_line() const {
                return buffer_;
            }
            /**
             * \brief Extract a value from the current line.
             * \tparam T The type of the value to extract.
             * \param param The value to extract.
             * \return A reference to the LineInputStream object.
             */
            template<class T>
            LineInputStream &operator>>(T &param) {
                *line_in_ >> param;
                return *this;
            }

        private:
            std::istream &in_;                ///< The input stream.
            std::istringstream *line_in_;     ///< The current line as an input stream.
            std::string buffer_;              ///< The buffer to store the current line.
        };


    } // namespace io

} // namespace easy3d


#endif  // EASY3D_UTIL_LINE_STREAM_H
