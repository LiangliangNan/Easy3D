/** \cond LICENSE
 * Copyright (C) 2015-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) 2000-2005 INRIA - Project ALICE
 *
 * The code in this file is partly from OGF/Graphite (2.0 alpha-4) with
 * modifications and enhancement:
 *      https://gforge.inria.fr/forum/forum.php?forum_id=11459
 * The original code was distributed under the GNU GPL license.
 \endcond */

#ifndef EASY3D_UTIL_STRING_H
#define EASY3D_UTIL_STRING_H

#include <string>
#include <sstream>
#include <vector>


namespace easy3d {

    /**
     * \brief String manipulation utilities.
     * \namespace easy3d::string
     */
    namespace string {

        /**
         * \brief Splits a string into parts
         * \details Splits the string \p in into a list of substrings \p out
         * wherever \p separator occurs.
         * \param[in] in the input string to split
         * \param[in] separator the separator character
         * \param[out] out receives substrings appended to its existing contents
         * \param skip_empty_fields whether empty parts should
         * be ignored and not stored in list \p out (this is true by default).
         * A trailing empty field is not emitted, even when \p skip_empty_fields is false.
         * \see join()
         */
        void split(const std::string &in, char separator, std::vector<std::string> &out, bool skip_empty_fields = true);

        /**
         * \brief Splits a string into two parts.
         * \param[in] in the input string to split
         * \param[in] separator the separator character
         * \param[out] left the part of the input string on the left
         *   of the separator or the empty string if the separator
         *   did not appear in the input string
         * \param[out] right the part of the input string on the right
         *   of the separator or the empty string if the separator
         *   did not appear in the input string
         * \retval true if the separator was found in the input string
         * \retval false otherwise
         * \details Splits at the first occurrence of \p separator.
         */
        bool split(const std::string &in, char separator, std::string &left, std::string &right);

        /**
         * \brief Join multiple strings
         * \details Joins strings in \p in using \p separator. A separator is added only when the accumulated
         *          result is non-empty, so leading empty elements are omitted; an all-empty input yields an
         *          empty string.
         * \param[in] in the list of strings to join
         * \param[in] separator the separator character
         * \return the joined string
         * \see split()
         */
        std::string join(const std::vector<std::string> &in, char separator);

        /**
         * \brief Join multiple strings
         * \details Joins strings in \p in using \p separator. A separator is added only when the accumulated
         *          result is non-empty, so leading empty elements are omitted; an all-empty input yields an
         *          empty string.
         * \param[in] in the list of strings to join
         * \param[in] separator the separator string (can be an empty string)
         * \return the joined string
         * \see split()
         */
        std::string join(const std::vector<std::string> &in, const std::string &separator);

        /**
         * \brief Replaces the first occurrence of a substring.
         * \param[in,out] in String to modify.
         * \param old_substr Substring to find.
         * \param new_substr Replacement text.
         */
        void replace(std::string &in, const std::string &old_substr, const std::string &new_substr);

        /**
         * \brief Converts a string to lowercase
         * \details Returns a converted copy; the input string is unchanged.
         * \param s String to convert.
         * \return Lowercase copy of \p s.
         * \see to_uppercase()
         */
        std::string to_lowercase(const std::string &s);

        /**
         * \brief Converts a string to uppercase
         * \details Returns a converted copy; the input string is unchanged.
         * \param s String to convert.
         * \return Uppercase copy of \p s.
         * \see to_lowercase()
         */
        std::string to_uppercase(const std::string &s);

        /**
         * \brief Creates a one char string
         * \param[in] c the character to convert to a string
         * \return a string that contains character \p c
         */
        inline std::string to_string(char c) {
            const char s[2] = { c, '\0' };
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
        std::string quote(const std::string &s, char quotes = '\"');

        /**
         * \brief Checks if a string starts with a substring
         * \param[in] haystack the input string
         * \param[in] needle the substring to check
         * \return \c true if \p haystack starts with \p needle, \c false
         * otherwise.
         */
        bool starts_with(const std::string &haystack, const std::string &needle);

        /**
         * \brief Checks if a string ends with a substring
         * \param[in] haystack the input string
         * \param[in] needle the substring to check
         * \return \c true if \p haystack ends with \p needle, \c false
         * otherwise.
         * \details Returns false when \p needle is empty or its length is greater than or equal to that of
         *          \p haystack.
         */
        bool ends_with(const std::string &haystack, const std::string &needle);

        /**
         * \brief Converts an integer \p value to a string of a desired length.
         * \details Unlike std::to_string(), this method pads to at least \p width characters using \p fill; wider
         *          values are not truncated.
         * \param value Integer to format.
         * \param width Minimum output width.
         * \param fill Padding character (defaults to '0').
         * \return Formatted integer.
         */
        std::string to_string(int value, int width, char fill = '0');

        /**
         * Format a string using printf-style conversion specifiers.
         * \param format Format string followed by the corresponding variadic arguments.
         * \return Formatted string; an empty string is returned if formatting fails.
         */
        std::string printf(const char *format, ...);

        /**
         * \brief Append printf-style formatted text to a string.
         * \param dst Destination string.
         * \param format Format string followed by the corresponding variadic arguments.
         */
        void appendf(std::string &dst, const char *format, ...);

        /**
         * \brief Get the current local time as a human-readable string.
         * \return Local time formatted like \c "Fri\ Jan\ 09\ 11:39:32\ 2015".
         */
        std::string current_time();
        /**
         * \brief Get the current local time with millisecond precision.
         * \return Timestamp formatted as \c YYYY-MM-DD-HH-MM-SS-mmm.
         */
        std::string current_time_detailed();

        /**
         * \brief Format a duration in milliseconds using a readable unit.
         * \param time Duration in milliseconds.
         * \param num_digits Number of digits after the decimal point.
         * \return Duration formatted in milliseconds, seconds, minutes, hours, or days.
         * \details The implementation switches from hours to days after 12 hours and divides by 12.
         */
        std::string time(double time, int num_digits = 1);

        /**
         * \brief Convert a UTF-8 string to a wide string.
         * \param str UTF-8 input.
         * \return Converted wide string.
         */
        std::wstring to_wstring(const std::string &str);

        /**
         * \brief Convert a wide string to UTF-8.
         * \param wstr Wide-string input.
         * \return UTF-8 string.
         */
        std::string from_wstring(const std::wstring &wstr);

    }

}


#endif  // EASY3D_UTIL_STRING_H
