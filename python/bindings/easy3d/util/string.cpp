#include <easy3d/util/string.h>

#include <memory>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_util_string(pybind11::module_& m)
{
	// easy3d::string::split(const std::string &, char, class std::vector<std::string> &, bool) file:easy3d/util/string.h line:39
	m.def("split", [](const std::string & a0, char const & a1, class std::vector<std::string> & a2) -> void { return easy3d::string::split(a0, a1, a2); }, "", pybind11::arg("in"), pybind11::arg("separator"), pybind11::arg("out"));
	m.def("split", (void (*)(const std::string &, char, class std::vector<std::string> &, bool)) &easy3d::string::split, "Splits a string into parts\n \n\n Splits the string  into a list of substrings \n wherever  occurs.\n \n\n the input string to split\n \n\n the separator character\n \n\n the resulting list of substrings\n \n\n specifies whether empty parts should\n be ignored and not stored in list  (this is true by default).\n \n\n join_strings()\n\nC++: easy3d::string::split(const std::string &, char, class std::vector<std::string> &, bool) --> void", pybind11::arg("in"), pybind11::arg("separator"), pybind11::arg("out"), pybind11::arg("skip_empty_fields"));

	// easy3d::string::split(const std::string &, char, std::string &, std::string &) file:easy3d/util/string.h line:54
	m.def("split", (bool (*)(const std::string &, char, std::string &, std::string &)) &easy3d::string::split, "Splits a string into two parts.\n \n\n the input string to split\n \n\n the separator character\n \n\n the part of the input string on the left\n   of the separator or the empty string if the separator\n   did not appear in the input string\n \n\n the right of the input string on the left\n   of the separator or the empty string if the separator\n   did not appear in the input string\n \n\n if the separator was found in the input string\n \n\n otherwise\n\nC++: easy3d::string::split(const std::string &, char, std::string &, std::string &) --> bool", pybind11::arg("in"), pybind11::arg("separator"), pybind11::arg("left"), pybind11::arg("right"));

	// easy3d::string::join(const class std::vector<std::string> &, char) file:easy3d/util/string.h line:65
	m.def("join", (std::string (*)(const class std::vector<std::string> &, char)) &easy3d::string::join, "Join multiple strings\n \n\n Joins all the strings in list  into a single string\n with each element separated by the given  character.\n \n\n the list of strings to join\n \n\n the separator character\n \n\n the joined string\n \n\n split_string()\n\nC++: easy3d::string::join(const class std::vector<std::string> &, char) --> std::string", pybind11::arg("in"), pybind11::arg("separator"));

	// easy3d::string::join(const class std::vector<std::string> &, const std::string &) file:easy3d/util/string.h line:76
	m.def("join", (std::string (*)(const class std::vector<std::string> &, const std::string &)) &easy3d::string::join, "Join multiple strings\n \n\n Joins all the strings in list  into a single string\n with each element separated by the given  string.\n \n\n the list of strings to join\n \n\n the separator string (can be an empty string)\n \n\n the joined string\n \n\n split_string()\n\nC++: easy3d::string::join(const class std::vector<std::string> &, const std::string &) --> std::string", pybind11::arg("in"), pybind11::arg("separator"));

	// easy3d::string::replace(std::string &, const std::string &, const std::string &) file:easy3d/util/string.h line:81
	m.def("replace", (void (*)(std::string &, const std::string &, const std::string &)) &easy3d::string::replace, "Replaces the old sub-string by a new sub-string\n\nC++: easy3d::string::replace(std::string &, const std::string &, const std::string &) --> void", pybind11::arg("in"), pybind11::arg("old_substr"), pybind11::arg("new_substr"));

	// easy3d::string::to_lowercase(const std::string &) file:easy3d/util/string.h line:89
	m.def("to_lowercase", (std::string (*)(const std::string &)) &easy3d::string::to_lowercase, "Converts a string to lowercase\n \n\n The conversion is done in place in the string \n \n\n The string to convert\n \n\n to_uppercase()\n\nC++: easy3d::string::to_lowercase(const std::string &) --> std::string", pybind11::arg("s"));

	// easy3d::string::to_uppercase(const std::string &) file:easy3d/util/string.h line:97
	m.def("to_uppercase", (std::string (*)(const std::string &)) &easy3d::string::to_uppercase, "Converts a string to uppercase\n \n\n The conversion is done in place in the string \n \n\n The string to convert\n \n\n to_lowercase()\n\nC++: easy3d::string::to_uppercase(const std::string &) --> std::string", pybind11::arg("s"));

	// easy3d::string::to_string(char) file:easy3d/util/string.h line:104
	m.def("to_string", (std::string (*)(char)) &easy3d::string::to_string, "Creates a one char string\n \n\n the character to convert to a string\n \n\n a string that contains character \n         \n\nC++: easy3d::string::to_string(char) --> std::string", pybind11::arg("c"));

	// easy3d::string::quote(const std::string &, char) file:easy3d/util/string.h line:117
	m.def("quote", [](const std::string & a0) -> std::string { return easy3d::string::quote(a0); }, "", pybind11::arg("s"));
	m.def("quote", (std::string (*)(const std::string &, char)) &easy3d::string::quote, "Adds quotes to a string\n \n\n Adds character  at the beginning and the end of\n string  and returns the resulting string.\n \n\n the string to quote\n \n\n the quoting char (default is '\"')\n \n\n the quoted string\n\nC++: easy3d::string::quote(const std::string &, char) --> std::string", pybind11::arg("s"), pybind11::arg("quotes"));

	// easy3d::string::starts_with(const std::string &, const std::string &) file:easy3d/util/string.h line:126
	m.def("starts_with", (bool (*)(const std::string &, const std::string &)) &easy3d::string::starts_with, "Checks if a string starts with a substring\n \n\n the input string\n \n\n the substring to check\n \n\n  if  starts with  \n otherwise.\n\nC++: easy3d::string::starts_with(const std::string &, const std::string &) --> bool", pybind11::arg("haystack"), pybind11::arg("needle"));

	// easy3d::string::ends_with(const std::string &, const std::string &) file:easy3d/util/string.h line:135
	m.def("ends_with", (bool (*)(const std::string &, const std::string &)) &easy3d::string::ends_with, "Checks if a string ends with a substring\n \n\n the input string\n \n\n the substring to check\n \n\n  if  ends with  \n otherwise.\n\nC++: easy3d::string::ends_with(const std::string &, const std::string &) --> bool", pybind11::arg("haystack"), pybind11::arg("needle"));

	// easy3d::string::to_string(int, int, char) file:easy3d/util/string.h line:142
	m.def("to_string", [](int const & a0, int const & a1) -> std::string { return easy3d::string::to_string(a0, a1); }, "", pybind11::arg("value"), pybind11::arg("width"));
	m.def("to_string", (std::string (*)(int, int, char)) &easy3d::string::to_string, "Converts an integer  to a string of a desired length.\n \n\n Different from the std::to_string(), this method results in a fixed-width string by filling the\n      missing parts with character \n         \n\nC++: easy3d::string::to_string(int, int, char) --> std::string", pybind11::arg("value"), pybind11::arg("width"), pybind11::arg("fill"));

	// easy3d::string::printf(const char *) file:easy3d/util/string.h line:147
	m.def("printf", [](const char * a0) -> std::string { return easy3d::string::printf(a0); }, "", pybind11::arg("format"));

	// easy3d::string::appendf(std::string &, const char *) file:easy3d/util/string.h line:152
	m.def("appendf", [](std::string & a0, const char * a1) -> void { return easy3d::string::appendf(a0, a1); }, "", pybind11::arg("dst"), pybind11::arg("format"));

	// easy3d::string::current_time() file:easy3d/util/string.h line:157
	m.def("current_time", (std::string (*)()) &easy3d::string::current_time, "Gets the time string, e.g., \"Fri Jan 09 11:39:32 2015\"\n\nC++: easy3d::string::current_time() --> std::string");

	// easy3d::string::current_time_detailed() file:easy3d/util/string.h line:162
	m.def("current_time_detailed", (std::string (*)()) &easy3d::string::current_time_detailed, "Gets the detailed current time as a string, e.g., \"2024-10-24-17-41-16-753\".\n \n\n Different from current_time(), this method also includes the millisecond part.\n\nC++: easy3d::string::current_time_detailed() --> std::string");

	// easy3d::string::time(double, int) file:easy3d/util/string.h line:168
	m.def("time", [](double const & a0) -> std::string { return easy3d::string::time(a0); }, "", pybind11::arg("time"));
	m.def("time", (std::string (*)(double, int)) &easy3d::string::time, "Converts  (in millisecond) into a string with the most suitable/readable unit.\n The result will look like e.g., 88ms, 2.3s, 1.7m, 0.1h.\n\nC++: easy3d::string::time(double, int) --> std::string", pybind11::arg("time"), pybind11::arg("num_digits"));

	// easy3d::string::to_wstring(const std::string &) file:easy3d/util/string.h line:173
	m.def("to_wstring", (std::wstring (*)(const std::string &)) &easy3d::string::to_wstring, "Converts from std::string to std::wstring.\n\nC++: easy3d::string::to_wstring(const std::string &) --> std::wstring", pybind11::arg("str"));

	// easy3d::string::from_wstring(const std::wstring &) file:easy3d/util/string.h line:178
	m.def("from_wstring", (std::string (*)(const std::wstring &)) &easy3d::string::from_wstring, "Converts from std::wstring to std::string.\n\nC++: easy3d::string::from_wstring(const std::wstring &) --> std::string", pybind11::arg("wstr"));

}
