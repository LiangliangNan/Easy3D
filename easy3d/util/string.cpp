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


#include <easy3d/util/string.h>

#include <algorithm>
#include <cstdarg>
#include <iostream>


namespace string {

	void split_string(
		const std::string& in,
		char separator,
		std::vector<std::string>& out,
		bool skip_empty_fields
	) {
		size_t length = in.length();
		size_t start = 0;
		while (start < length) {
			size_t end = in.find(separator, start);
			if (end == std::string::npos) {
				end = length;
			}
			if (!skip_empty_fields || (end - start > 0)) {
				out.push_back(in.substr(start, end - start));
			}
			start = end + 1;
		}
	}

	bool split_string(
		const std::string& in,
		char separator,
		std::string& left,
		std::string& right
	) {
		size_t p = in.find(separator);
		if (p == std::string::npos) {
			left = "";
			right = "";
			return false;
		}
		left = in.substr(0, p);
		right = in.substr(p + 1, in.length() - p);
		return true;
	}

	std::string join_strings(
		const std::vector<std::string>& in,
		char separator
	) {
		std::string result;
		for (unsigned int i = 0; i < in.size(); i++) {
			if (result.length() != 0) {
				result += separator;
			}
			result += in[i];
		}
		return result;
	}

	std::string join_strings(
		const std::vector<std::string>& in,
		const std::string& separator
	) {
		std::string result;
		for (unsigned int i = 0; i < in.size(); i++) {
			if (result.length() != 0) {
				result += separator;
			}
			result += in[i];
		}
		return result;
	}


	void replace_substring(
		std::string& in,
		const std::string& old_substr,
		const std::string& new_substr
	) {
		//std::cout << "n\n\before -----\n" << in << std::endl;
		std::size_t pos = in.find(old_substr);
		if (pos != std::string::npos) {
			in.replace(pos, old_substr.length(), new_substr);
		}
		//std::cout << "n\n\after -----\n" << in << std::endl;
	}

#if 1
	std::string to_lowercase(const std::string& str)
	{
		std::string lowcase_str(str);
		std::transform(str.begin(), str.end(), lowcase_str.begin(), tolower);
		return lowcase_str;
	}

	std::string to_uppercase(const std::string& str)
	{
		std::string upcase_str(str);
		std::transform(str.begin(), str.end(), upcase_str.begin(), toupper);
		return upcase_str;
	}
#else
	std::string to_lowercase(const std::string& in) {
		std::string s = in;
		for (unsigned int i = 0; i < s.length(); i++) {
			s[i] = char(tolower(s[i]));
		}
		return s;
	}

	std::string to_uppercase(const std::string& in) {
		std::string s = in;
		for (unsigned int i = 0; i < s.length(); i++) {
			s[i] = char(toupper(s[i]));
		}
		return s;
	}
#endif

	std::string quote(const std::string& s, char quotes) {
		return char_to_string(quotes) + s + char_to_string(quotes);
	}

	bool string_starts_with(
		const std::string& haystack, const std::string& needle
	) {
		return haystack.compare(0, needle.length(), needle) == 0;
	}

	bool string_ends_with(
		const std::string& haystack, const std::string& needle
	) {
		size_t l1 = haystack.length();
		size_t l2 = needle.length();
		return l1 > l2 && haystack.compare(l1 - l2, l1, needle) == 0;
	}

	/********************************************************************/

	void string_append_v(std::string* dst, const char* format, va_list ap) {
		// First try with a small fixed size buffer.
		static const int kFixedBufferSize = 1024;
		char fixed_buffer[kFixedBufferSize];

		// It is possible for methods that use a va_list to invalidate
		// the data in it upon use.  The fix is to make a copy
		// of the structure before using it and use that copy instead.
		va_list backup_ap;
		va_copy(backup_ap, ap);
		int result = vsnprintf(fixed_buffer, kFixedBufferSize, format, backup_ap);
		va_end(backup_ap);

		if (result < kFixedBufferSize) {
			if (result >= 0) {
				// Normal case - everything fits.
				dst->append(fixed_buffer, result);
				return;
			}

#ifdef _MSC_VER
			// Error or MSVC running out of space.  MSVC 8.0 and higher
			// can be asked about space needed with the special idiom below:
			va_copy(backup_ap, ap);
			result = vsnprintf(nullptr, 0, format, backup_ap);
			va_end(backup_ap);
#endif

			if (result < 0) {
				// Just an error.
				return;
			}
		}

		// Increase the buffer size to the size requested by vsnprintf,
		// plus one for the closing \0.
		const int variable_buffer_size = result + 1;
		char* variable_buffer = new char[variable_buffer_size];

		// Restore the va_list before we use it again.
		va_copy(backup_ap, ap);
		result =
			vsnprintf(variable_buffer, variable_buffer_size, format, backup_ap);
		va_end(backup_ap);

		if (result >= 0 && result < variable_buffer_size) {
			dst->append(variable_buffer, result);
		}
		delete[] variable_buffer;
	}


	std::string string_printf(const char* format, ...) {
		va_list ap;
		va_start(ap, format);
		std::string result;
		string_append_v(&result, format, ap);
		va_end(ap);
		return result;
	}

	void string_appendf(std::string* dst, const char* format, ...) {
		va_list ap;
		va_start(ap, format);
		string_append_v(dst, format, ap);
		va_end(ap);
	}


	// format example: "Fri Jan 09 11:39:32 2015"
	std::string from_current_time() {
        time_t now = time(nullptr); /* get current time; same as: time(&now)  */
		struct tm* timeinfo = localtime(&now);
		std::string tstr = asctime(timeinfo);

		//return tstr;
		return tstr.substr(0, tstr.length() - 1); // discard the terminating null-character

		// use strftime() to define your own stamp format
		//size_t strftime(char *strDest, size_t maxsize, const char *format, const struct tm *timeptr);
	}
}

