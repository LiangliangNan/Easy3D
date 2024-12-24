#include <easy3d/util/console_style.h>

#include <pybind11/pybind11.h>
#include <string>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_util_console_style(pybind11::module_& m)
{
	// easy3d::console::Style file:easy3d/util/console_style.h line:39
	pybind11::enum_<easy3d::console::Style>(m, "Style", pybind11::arithmetic(), "")
		.value("None", easy3d::console::None)
		.value("Black", easy3d::console::Black)
		.value("Red", easy3d::console::Red)
		.value("Green", easy3d::console::Green)
		.value("Yellow", easy3d::console::Yellow)
		.value("Blue", easy3d::console::Blue)
		.value("Purple", easy3d::console::Purple)
		.value("Cyan", easy3d::console::Cyan)
		.value("White", easy3d::console::White)
		.value("BBlack", easy3d::console::BBlack)
		.value("BRed", easy3d::console::BRed)
		.value("BGreen", easy3d::console::BGreen)
		.value("BYellow", easy3d::console::BYellow)
		.value("BBlue", easy3d::console::BBlue)
		.value("BPurple", easy3d::console::BPurple)
		.value("BCyan", easy3d::console::BCyan)
		.value("BWhite", easy3d::console::BWhite)
		.value("UBlack", easy3d::console::UBlack)
		.value("URed", easy3d::console::URed)
		.value("UGreen", easy3d::console::UGreen)
		.value("UYellow", easy3d::console::UYellow)
		.value("UBlue", easy3d::console::UBlue)
		.value("UPurple", easy3d::console::UPurple)
		.value("UCyan", easy3d::console::UCyan)
		.value("UWhite", easy3d::console::UWhite)
		.value("On_Black", easy3d::console::On_Black)
		.value("On_Red", easy3d::console::On_Red)
		.value("On_Green", easy3d::console::On_Green)
		.value("On_Yellow", easy3d::console::On_Yellow)
		.value("On_Blue", easy3d::console::On_Blue)
		.value("On_Purple", easy3d::console::On_Purple)
		.value("On_Cyan", easy3d::console::On_Cyan)
		.value("On_White", easy3d::console::On_White)
		.value("IBlack", easy3d::console::IBlack)
		.value("IRed", easy3d::console::IRed)
		.value("IGreen", easy3d::console::IGreen)
		.value("IYellow", easy3d::console::IYellow)
		.value("IBlue", easy3d::console::IBlue)
		.value("IPurple", easy3d::console::IPurple)
		.value("ICyan", easy3d::console::ICyan)
		.value("IWhite", easy3d::console::IWhite)
		.value("BIBlack", easy3d::console::BIBlack)
		.value("BIRed", easy3d::console::BIRed)
		.value("BIGreen", easy3d::console::BIGreen)
		.value("BIYellow", easy3d::console::BIYellow)
		.value("BIBlue", easy3d::console::BIBlue)
		.value("BIPurple", easy3d::console::BIPurple)
		.value("BICyan", easy3d::console::BICyan)
		.value("BIWhite", easy3d::console::BIWhite)
		.value("On_IBlack", easy3d::console::On_IBlack)
		.value("On_IRed", easy3d::console::On_IRed)
		.value("On_IGreen", easy3d::console::On_IGreen)
		.value("On_IYellow", easy3d::console::On_IYellow)
		.value("On_IBlue", easy3d::console::On_IBlue)
		.value("On_IPurple", easy3d::console::On_IPurple)
		.value("On_ICyan", easy3d::console::On_ICyan)
		.value("On_IWhite", easy3d::console::On_IWhite)
		.export_values();

	// easy3d::console::terminal_supports_styles() file:easy3d/util/console_style.h line:109
	m.def("terminal_supports_styles", (bool (*)()) &easy3d::console::terminal_supports_styles, "Returns if the terminal supports using styles in output.\n\nC++: easy3d::console::terminal_supports_styles() --> bool");

}
