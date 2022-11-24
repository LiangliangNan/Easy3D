/********************************************************************
 * Copyright (C) 2015-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) 2000-2005 INRIA - Project ALICE
 *
 * The code in this file is partly from OGF/Graphite (2.0 alpha-4) with
 * modifications and enhancement:
 *      https://gforge.inria.fr/forum/forum.php?forum_id=11459
 * The original code was distributed under the GNU GPL license.
 ********************************************************************/

#include <easy3d/util/string.h>

#include <algorithm>
#include <cstdarg>
#include <iomanip>
#include <cmath>
#include <codecvt>


namespace easy3d {

    namespace string {

        void split(const std::string &in, char separator, std::vector<std::string> &out, bool skip_empty_fields) {
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

        bool split(const std::string &in, char separator, std::string &left, std::string &right) {
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

        std::string join(const std::vector<std::string> &in, char separator) {
            std::string result;
            for (const auto& str : in) {
                if (result.length() != 0)
                    result += separator;
                result += str;
            }
            return result;
        }

        std::string join(const std::vector<std::string> &in, const std::string &separator) {
            std::string result;
            for (const auto& str : in) {
                if (result.length() != 0)
                    result += separator;
                result += str;
            }
            return result;
        }


        void replace(std::string &in, const std::string &old_substr, const std::string &new_substr) {
            std::size_t pos = in.find(old_substr);
            if (pos != std::string::npos) {
                in.replace(pos, old_substr.length(), new_substr);
            }
        }

#if 1

        std::string to_lowercase(const std::string &str) {
            std::string lowcase_str(str);
            std::transform(str.begin(), str.end(), lowcase_str.begin(), tolower);
            return lowcase_str;
        }

        std::string to_uppercase(const std::string &str) {
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

        std::string quote(const std::string &s, char quotes) {
            return to_string(quotes) + s + to_string(quotes);
        }

        bool starts_with(const std::string &haystack, const std::string &needle) {
            return haystack.compare(0, needle.length(), needle) == 0;
        }

        bool ends_with(const std::string &haystack, const std::string &needle) {
            size_t l1 = haystack.length();
            size_t l2 = needle.length();
            return l1 > l2 && haystack.compare(l1 - l2, l1, needle) == 0;
        }

        std::string to_string(int v, int width, char fill) {
            std::ostringstream string_stream;
            string_stream << std::setfill(fill) << std::setw(width) << v;
            return string_stream.str();
        }


        void append_v(std::string &dst, const char *format, va_list ap) {
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
                    dst.append(fixed_buffer, result);
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
            char *variable_buffer = new char[variable_buffer_size];

            // Restore the va_list before we use it again.
            va_copy(backup_ap, ap);
            result = vsnprintf(variable_buffer, variable_buffer_size, format, backup_ap);
            va_end(backup_ap);

            if (result >= 0 && result < variable_buffer_size) {
                dst.append(variable_buffer, result);
            }
            delete[] variable_buffer;
        }


        std::string printf(const char *format, ...) {
            va_list ap;
            va_start(ap, format);
            std::string result;
            append_v(result, format, ap);
            va_end(ap);
            return result;
        }

        void appendf(std::string &dst, const char *format, ...) {
            va_list ap;
            va_start(ap, format);
            append_v(dst, format, ap);
            va_end(ap);
        }


        // format example: "Fri Jan 09 11:39:32 2015"
        std::string date_time() {
            time_t now = ::time(nullptr); /* get current time; same as: time(&now)  */
            struct tm *timeinfo = localtime(&now);
            std::string tstr = asctime(timeinfo);

            //return tstr;
            return tstr.substr(0, tstr.length() - 1); // discard the terminating null-character

            // use strftime() to define your own stamp format
            //size_t strftime(char *strDest, size_t maxsize, const char *format, const struct tm *timeptr);

            // get a precise timestamp as a string
//            const auto now = std::chrono::system_clock::now();
//            const auto now_as_time_t = std::chrono::system_clock::to_time_t(now);
//            const auto now_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
//            std::stringstream time_stream;
//            time_stream << std::put_time(std::localtime(&now_as_time_t), "%Y%m%d %T")
//                        << '.' << std::setfill('0') << std::setw(3) << now_in_ms.count();
//            time_str_ = time_stream.str();
        }


        std::string time(double time, int num_digits) {
            if (std::isnan(time) || std::isinf(time))
                return "inf";

            std::string suffix = "ms";
            if (time > 1000) {
                time /= 1000;
                suffix = "s";
                if (time > 60) {
                    time /= 60;
                    suffix = "m";
                    if (time > 60) {
                        time /= 60;
                        suffix = "h";
                        if (time > 12) {
                            time /= 12;
                            suffix = "d";
                        }
                    }
                }
            }

            std::ostringstream os;
            os << std::setprecision(num_digits) << std::fixed << time << suffix;

            return os.str();
        }


        std::wstring to_wstring(const std::string &str) {
            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
            return converter.from_bytes(str);
        }

        std::string from_wstring(const std::wstring &wstr) {
            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
            return converter.to_bytes(wstr);
        }

    }

}