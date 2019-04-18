/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work, 
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan. 
*           Easy3D: a lightweight, easy-to-use, and efficient C++ 
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef EASY3D_LINE_STREAM_STREAM_H
#define EASY3D_LINE_STREAM_STREAM_H

#include <iostream>
#include <sstream>



namespace easy3d {

namespace io {


	class LineInputStream {
	public:
        LineInputStream(std::istream& in) : in_(in), line_in_(nullptr) {}
		~LineInputStream() {
            delete line_in_;
            line_in_ = nullptr ;
		}

		bool eof() const { return in_.eof() ; }
        bool eol() const { return line_in_ == nullptr || line_in_->eof(); }
		bool fail() const { return in_.fail() || line_in_->fail(); }
		bool ok() const { return !fail(); }

		void get_line() {
			getline(in_, buffer_);
			delete line_in_ ; 
			line_in_ = new std::istringstream(buffer_) ;
		}

		std::istream& line() { 
            assert(line_in_ != nullptr) ;
			return *line_in_ ; 
		}

		const std::string& current_line() const {
			return buffer_;
		}

		template <class T> LineInputStream& operator>>(T& param) {
			*line_in_ >> param;
			return *this;
		}

	private:
		std::istream& in_ ;
		std::istringstream* line_in_ ;
		std::string buffer_;
	} ;


} // namespace io

} // namespace easy3d


#endif
