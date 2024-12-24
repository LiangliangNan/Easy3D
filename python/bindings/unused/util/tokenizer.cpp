#include <easy3d/util/tokenizer.h>

#include <memory>

#include <pybind11/pybind11.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif


void bind_easy3d_util_tokenizer(pybind11::module_& m)
{
	{ // easy3d::Tokenizer file:easy3d/util/tokenizer.h line:51
		pybind11::class_<easy3d::Tokenizer, std::shared_ptr<easy3d::Tokenizer>> cl(m, "Tokenizer", "A class for tokenizing input character streams.\n \n\n\n     ");
		cl.def( pybind11::init( [](const char * a0){ return new easy3d::Tokenizer(a0); } ), "doc" , pybind11::arg("src"));
		cl.def( pybind11::init<const char *, const char *>(), pybind11::arg("src"), pybind11::arg("delims") );

		cl.def("setConsumeWS", (void (easy3d::Tokenizer::*)(bool)) &easy3d::Tokenizer::setConsumeWS, "C++: easy3d::Tokenizer::setConsumeWS(bool) --> void", pybind11::arg("ws"));
		cl.def("isWhitespace", (bool (easy3d::Tokenizer::*)(const char)) &easy3d::Tokenizer::isWhitespace, "C++: easy3d::Tokenizer::isWhitespace(const char) --> bool", pybind11::arg("c"));
		cl.def("isQuote", (bool (easy3d::Tokenizer::*)(const char)) &easy3d::Tokenizer::isQuote, "C++: easy3d::Tokenizer::isQuote(const char) --> bool", pybind11::arg("c"));
		cl.def("isEOL", (bool (easy3d::Tokenizer::*)(const char)) &easy3d::Tokenizer::isEOL, "C++: easy3d::Tokenizer::isEOL(const char) --> bool", pybind11::arg("c"));
		cl.def("isTerm", (bool (easy3d::Tokenizer::*)(const char)) &easy3d::Tokenizer::isTerm, "C++: easy3d::Tokenizer::isTerm(const char) --> bool", pybind11::arg("c"));
		cl.def("isDelim", (bool (easy3d::Tokenizer::*)(const char)) &easy3d::Tokenizer::isDelim, "C++: easy3d::Tokenizer::isDelim(const char) --> bool", pybind11::arg("c"));
		cl.def("atEOF", (bool (easy3d::Tokenizer::*)()) &easy3d::Tokenizer::atEOF, "C++: easy3d::Tokenizer::atEOF() --> bool");
		cl.def("consumeWhitespace", (char (easy3d::Tokenizer::*)()) &easy3d::Tokenizer::consumeWhitespace, "C++: easy3d::Tokenizer::consumeWhitespace() --> char");
		cl.def("consumeOneDelim", (char (easy3d::Tokenizer::*)()) &easy3d::Tokenizer::consumeOneDelim, "C++: easy3d::Tokenizer::consumeOneDelim() --> char");
		cl.def("consumeToEOL", (void (easy3d::Tokenizer::*)()) &easy3d::Tokenizer::consumeToEOL, "C++: easy3d::Tokenizer::consumeToEOL() --> void");
		cl.def("readToken", (bool (easy3d::Tokenizer::*)()) &easy3d::Tokenizer::readToken, "C++: easy3d::Tokenizer::readToken() --> bool");
		cl.def("requireToken", (bool (easy3d::Tokenizer::*)(const char *)) &easy3d::Tokenizer::requireToken, "C++: easy3d::Tokenizer::requireToken(const char *) --> bool", pybind11::arg("find"));
		cl.def("requireTokenDelim", (bool (easy3d::Tokenizer::*)(const char *)) &easy3d::Tokenizer::requireTokenDelim, "C++: easy3d::Tokenizer::requireTokenDelim(const char *) --> bool", pybind11::arg("find"));
		cl.def("getTermChar", (char (easy3d::Tokenizer::*)() const) &easy3d::Tokenizer::getTermChar, "accessor to get character that caused 'stop' of last token read\n\nC++: easy3d::Tokenizer::getTermChar() const --> char");
		cl.def("getLastToken", (bool (easy3d::Tokenizer::*)(std::string &)) &easy3d::Tokenizer::getLastToken, "get last read token as a std::string\n\nC++: easy3d::Tokenizer::getLastToken(std::string &) --> bool", pybind11::arg("returnTok"));
		cl.def("getLastTokenPtr", (const char * (easy3d::Tokenizer::*)()) &easy3d::Tokenizer::getLastTokenPtr, "accessor to get last read token const char *\n\nC++: easy3d::Tokenizer::getLastTokenPtr() --> const char *", pybind11::return_value_policy::automatic);
		cl.def("getLastTokenLen", (unsigned int (easy3d::Tokenizer::*)() const) &easy3d::Tokenizer::getLastTokenLen, "accessor to get last read token length\n\nC++: easy3d::Tokenizer::getLastTokenLen() const --> unsigned int");
		cl.def("getTokenString", (bool (easy3d::Tokenizer::*)(std::string &)) &easy3d::Tokenizer::getTokenString, "get next token as a std::string\n\nC++: easy3d::Tokenizer::getTokenString(std::string &) --> bool", pybind11::arg("returnTok"));
		cl.def("getTokenFloat", (bool (easy3d::Tokenizer::*)(float &)) &easy3d::Tokenizer::getTokenFloat, "get next token as a floating-point number\n\nC++: easy3d::Tokenizer::getTokenFloat(float &) --> bool", pybind11::arg("out"));
		cl.def("getTokenInt", (bool (easy3d::Tokenizer::*)(int &)) &easy3d::Tokenizer::getTokenInt, "get next token as an integer\n\nC++: easy3d::Tokenizer::getTokenInt(int &) --> bool", pybind11::arg("out"));
		cl.def("getTokenUint", (bool (easy3d::Tokenizer::*)(unsigned int &)) &easy3d::Tokenizer::getTokenUint, "get next token as an unsigned integer\n\nC++: easy3d::Tokenizer::getTokenUint(unsigned int &) --> bool", pybind11::arg("out"));
		cl.def("getTokenBool", (bool (easy3d::Tokenizer::*)(bool &)) &easy3d::Tokenizer::getTokenBool, "get next token as some form of boolean value/string\n\nC++: easy3d::Tokenizer::getTokenBool(bool &) --> bool", pybind11::arg("out"));
	}
}
