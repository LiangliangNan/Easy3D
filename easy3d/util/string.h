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

#ifndef EASY3D_UTIL_STRING_H
#define EASY3D_UTIL_STRING_H

#include <string>
#include <sstream>
#include <iomanip>
#include <vector>


namespace easy3d {

	/*
	 * \brief String manipulation utilities.
	 */
	namespace string {

		/**
		 * \brief Splits a string into parts
		 * \details Splits the string \p in into a list of substrings \p out
		 * wherever \p separator occurs.
		 * \param[in] in the input string to split
		 * \param[in] separator the separator character
		 * \param[in] out the resulting list of substrings
		 * \param[in] skip_empty_fields specifies whether empty parts should
		 * be ignored and not stored in list \p out (this is true by default).
		 * \see join_strings()
		 */
		void split_string(
			const std::string& in,
			char separator,
			std::vector<std::string>& out,
			bool skip_empty_fields = true
		);

		/**
		 * \brief Splits a string into two parts.
		 * \param[in] in the input string to split
		 * \param[in] separator the separator character
		 * \param[in] left the part of the input string on the left
		 *   of the separator or the empty string if the separator
		 *   did not appear in the input string
		 * \param[in] right the right of the input string on the left
		 *   of the separator or the empty string if the separator
		 *   did not appear in the input string
		 * \retval true if the separator was found in the input string
		 * \retval false otherwise
		 */
		bool split_string(
			const std::string& in,
			char separator,
			std::string& left,
			std::string& right
		);

		/**
		 * \brief Join multiple strings
		 * \details Joins all the strings in list \p in into a single string
		 * with each element separated by the given \p separator character.
		 * \param[in] in the list of strings to join
		 * \param[in] separator the separator character
		 * \return the joined string
		 * \see split_string()
		 */
		std::string join_strings(
			const std::vector<std::string>& in,
			char separator
		);

		/**
		 * \brief Join multiple strings
		 * \details Joins all the strings in list \p in into a single string
		 * with each element separated by the given \p separator string.
		 * \param[in] in the list of strings to join
		 * \param[in] separator the separator string (can be an empty string)
		 * \return the joined string
		 * \see split_string()
		 */
		std::string join_strings(
			const std::vector<std::string>& in,
			const std::string& separator
		);

		/**
		 * \brief Replaces the old sub-string by a new sub-string
		 */
		void replace_substring(
			std::string& in,
			const std::string& old_substr,
			const std::string& new_substr
		);

		/**
		 * \brief Converts a string to lowercase
		 * \details The conversion is done in place in the string \p s.
		 * \param[in,out] s The string to convert
		 * \see to_uppercase()
		 */
		std::string to_lowercase(const std::string& s);

		/**
		 * \brief Converts a string to uppercase
		 * \details The conversion is done in place in the string \p s.
		 * \param[in,out] s The string to convert
		 * \see to_lowercase()
		 */
		std::string to_uppercase(const std::string& s);

		/**
		 * \brief Creates a one char string
		 * \param[in] c the character to convert to a string
		 * \return a string that contains character \p c
		 */
		inline std::string char_to_string(char c) {
			char s[2];
			s[0] = c;
			s[1] = '\0';
			return std::string(s);
		}

		/**
		 * \brief Adds quotes to a string
		 * \details Adds character \p quote at the beginning and the end of
		 * string \p s and returns the resulting string.
		 * \param[in] s the string to quote
		 * \param[in] quotes the quoting char (default is '"')
		 * \return the quoted string
		 */
		std::string quote(const std::string& s, char quotes = '\"');

		/**
		 * \brief Checks if a string starts with a substring
		 * \param[in] haystack the input string
		 * \param[in] needle the substring to check
		 * \return \c true if \p haystack starts with \p needle, \c false
		 * otherwise.
		 */
		bool string_starts_with(const std::string& haystack, const std::string& needle);

		/**
		 * \brief Checks if a string ends with a substring
		 * \param[in] haystack the input string
		 * \param[in] needle the substring to check
		 * \return \c true if \p haystack ends with \p needle, \c false
		 * otherwise.
		 */
		bool string_ends_with(const std::string& haystack, const std::string& needle);

		/**
		 * \brief Converts an integer v to a string. Different from the std::to_string, this method results in a
		 * fixed-width string by filling the missing parts with character 'fill'.
		 */
		std::string from_integer(int value, int width, char fill);

		/**
		 * Return a C++ string and work like printf.
		 */
		std::string string_printf(const char* format, ...);

		/**
		 * @brief Append the resulting string to a supplied string.
		 */
		void string_appendf(std::string* dst, const char* format, ...);

		/**
		 * @brief Get the time string, e.g., "Fri Jan 09 11:39:32 2015"
		 */
		std::string from_current_time();

		/**
		 * @brief Convert from std::string to std::wstring.
		 */
        std::wstring to_wstring(const std::string& str);

        /**
         * @brief Convert from std::wstring to std::string.
         */
        std::string to_string(const std::wstring& wstr);

	}

}


#endif  // EASY3D_UTIL_STRING_H

