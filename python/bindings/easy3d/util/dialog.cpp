#include <easy3d/util/dialog.h>

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


void bind_easy3d_util_dialog(pybind11::module_& m)
{
	// easy3d::dialog::open(const std::string &, const std::string &, const class std::vector<std::string> &, bool) file:easy3d/util/dialog.h line:52
	m.def("open", (class std::vector<std::string> (*)(const std::string &, const std::string &, const class std::vector<std::string> &, bool)) &easy3d::dialog::open, "Open a native file open dialog.\n \n\n The title of the dialog, e.g., \"Please choose the file(s) to open\".\n \n\n The default file path, e.g., resource::directory() + \"/data/\".\n \n\n The accepted file filters, e.g.,\n              const std::vector<std::string> filters = {\n                  \"Mesh Files (*.obj *.ply)\", \"*.obj *.ply\",\n                  \"Point Cloud Files (*.bin *.xyz *.ply)\", \"*.bin *.xyz *.ply\",\n                  \"All Files (*.*)\", \"*\"\n              };\n \n\n Allow choosing multiple files if it is true.\n \n\n The selected file names.\n\nC++: easy3d::dialog::open(const std::string &, const std::string &, const class std::vector<std::string> &, bool) --> class std::vector<std::string>", pybind11::arg("title"), pybind11::arg("default_directory"), pybind11::arg("filters"), pybind11::arg("multiple"));

	// easy3d::dialog::open(const std::string &, const std::string &, const class std::vector<std::string> &) file:easy3d/util/dialog.h line:73
	m.def("open", []() -> std::string { return easy3d::dialog::open(); }, "");
	m.def("open", [](const std::string & a0) -> std::string { return easy3d::dialog::open(a0); }, "", pybind11::arg("title"));
	m.def("open", [](const std::string & a0, const std::string & a1) -> std::string { return easy3d::dialog::open(a0, a1); }, "", pybind11::arg("title"), pybind11::arg("default_directory"));
	m.def("open", (std::string (*)(const std::string &, const std::string &, const class std::vector<std::string> &)) &easy3d::dialog::open, "Open a native file open dialog.\n \n\n The title of the dialog, e.g., \"Please choose a file to open\".\n \n\n The default file path, e.g., resource::directory() + \"/data/\".\n \n\n The accepted file filters. For each filter, the string you see in the file\n                dialog and the actual filter always come in pairs, e.g.,\n                      const std::vector<std::string> filters = {\n                          \"Mesh Files (*.obj *.ply)\", \"*.obj *.ply\",\n                          \"Point Cloud Files (*.bin *.xyz *.ply)\", \"*.bin *.xyz *.ply\",\n                          \"All Files (*.*)\", \"*\"\n                      };\n \n\n The selected file name (empty if no file was selected).\n \n\n   A filter needs only one string (the filter can be extracted from the string).\n\nC++: easy3d::dialog::open(const std::string &, const std::string &, const class std::vector<std::string> &) --> std::string", pybind11::arg("title"), pybind11::arg("default_directory"), pybind11::arg("filters"));

	// easy3d::dialog::save(const std::string &, const std::string &, const class std::vector<std::string> &, bool) file:easy3d/util/dialog.h line:95
	m.def("save", []() -> std::string { return easy3d::dialog::save(); }, "");
	m.def("save", [](const std::string & a0) -> std::string { return easy3d::dialog::save(a0); }, "", pybind11::arg("title"));
	m.def("save", [](const std::string & a0, const std::string & a1) -> std::string { return easy3d::dialog::save(a0, a1); }, "", pybind11::arg("title"), pybind11::arg("default_file_name"));
	m.def("save", [](const std::string & a0, const std::string & a1, const class std::vector<std::string> & a2) -> std::string { return easy3d::dialog::save(a0, a1, a2); }, "", pybind11::arg("title"), pybind11::arg("default_file_name"), pybind11::arg("filters"));
	m.def("save", (std::string (*)(const std::string &, const std::string &, const class std::vector<std::string> &, bool)) &easy3d::dialog::save, "Open a native file save dialog.\n \n\n The title of the dialog, e.g., \"Please choose a file name\".\n \n\n The suggested file name, e.g., /project/data/bunny.ply\"\n \n\n The accepted file filters, e.g.,\n              const std::vector<std::string> filters = {\n                  \"Mesh Files (*.obj *.ply)\", \"*.obj *.ply\",\n                  \"Point Cloud Files (*.bin *.xyz *.ply)\", \"*.bin *.xyz *.ply\",\n                  \"All Files (*.*)\", \"*.*\"\n              };\n \n\n If set true (default value), prompt a confirm dialog if a\n						    file with the same name already exists.\n \n\n The chosen file name (empty if no file name was chosen).\n\nC++: easy3d::dialog::save(const std::string &, const std::string &, const class std::vector<std::string> &, bool) --> std::string", pybind11::arg("title"), pybind11::arg("default_file_name"), pybind11::arg("filters"), pybind11::arg("confirm_overwrite"));

	// easy3d::dialog::open_folder(const std::string &, const std::string &) file:easy3d/util/dialog.h line:110
	m.def("open_folder", []() -> std::string { return easy3d::dialog::open_folder(); }, "");
	m.def("open_folder", [](const std::string & a0) -> std::string { return easy3d::dialog::open_folder(a0); }, "", pybind11::arg("title"));
	m.def("open_folder", (std::string (*)(const std::string &, const std::string &)) &easy3d::dialog::open_folder, "Open a folder dialog.\n \n\n The title of the dialog, e.g., \"Please choose a folder\".\n \n\n The suggested file path, e.g., /project/data/\"\n \n\n The chosen folder (empty if no folder was chosen).\n\nC++: easy3d::dialog::open_folder(const std::string &, const std::string &) --> std::string", pybind11::arg("title"), pybind11::arg("default_directory"));

	// easy3d::dialog::Type file:easy3d/util/dialog.h line:117
	pybind11::enum_<easy3d::dialog::Type>(m, "Type", "")
		.value("info", easy3d::dialog::Type::info)
		.value("warning", easy3d::dialog::Type::warning)
		.value("error", easy3d::dialog::Type::error)
		.value("question", easy3d::dialog::Type::question);

	// easy3d::dialog::Response file:easy3d/util/dialog.h line:124
	pybind11::enum_<easy3d::dialog::Response>(m, "Response", "")
		.value("cancel", easy3d::dialog::Response::cancel)
		.value("ok", easy3d::dialog::Response::ok)
		.value("yes", easy3d::dialog::Response::yes)
		.value("no", easy3d::dialog::Response::no)
		.value("abort", easy3d::dialog::Response::abort)
		.value("retry", easy3d::dialog::Response::retry)
		.value("ignore", easy3d::dialog::Response::ignore);

	// easy3d::dialog::Choice file:easy3d/util/dialog.h line:134
	pybind11::enum_<easy3d::dialog::Choice>(m, "Choice", "")
		.value("ok", easy3d::dialog::Choice::ok)
		.value("ok_cancel", easy3d::dialog::Choice::ok_cancel)
		.value("yes_no", easy3d::dialog::Choice::yes_no)
		.value("yes_no_cancel", easy3d::dialog::Choice::yes_no_cancel)
		.value("retry_cancel", easy3d::dialog::Choice::retry_cancel)
		.value("abort_retry_ignore", easy3d::dialog::Choice::abort_retry_ignore);

	// easy3d::dialog::notify(const std::string &, const std::string &, enum easy3d::dialog::Type) file:easy3d/util/dialog.h line:150
	m.def("notify", [](const std::string & a0, const std::string & a1) -> void { return easy3d::dialog::notify(a0, a1); }, "", pybind11::arg("title"), pybind11::arg("message"));
	m.def("notify", (void (*)(const std::string &, const std::string &, enum easy3d::dialog::Type)) &easy3d::dialog::notify, "Notification.\n \n\n The title of the notification dialog.\n \n\n The message string.\n \n\n The notification type. Default is 'info'. See 'Type'.\n\nC++: easy3d::dialog::notify(const std::string &, const std::string &, enum easy3d::dialog::Type) --> void", pybind11::arg("title"), pybind11::arg("message"), pybind11::arg("type"));

	// easy3d::dialog::message(const std::string &, const std::string &, enum easy3d::dialog::Choice, enum easy3d::dialog::Type) file:easy3d/util/dialog.h line:166
	m.def("message", [](const std::string & a0, const std::string & a1) -> easy3d::dialog::Response { return easy3d::dialog::message(a0, a1); }, "", pybind11::arg("title"), pybind11::arg("message"));
	m.def("message", [](const std::string & a0, const std::string & a1, enum easy3d::dialog::Choice const & a2) -> easy3d::dialog::Response { return easy3d::dialog::message(a0, a1, a2); }, "", pybind11::arg("title"), pybind11::arg("message"), pybind11::arg("choice"));
	m.def("message", (enum easy3d::dialog::Response (*)(const std::string &, const std::string &, enum easy3d::dialog::Choice, enum easy3d::dialog::Type)) &easy3d::dialog::message, "Message box.\n \n\n The title of the message box.\n \n\n The message string.\n \n\n The options for the user. Default is 'ok_cancel'. See 'Choice'.\n \n\n The message type. Default is 'info'. See 'Type'.\n \n\n The user's response. See \"Response'.\n\nC++: easy3d::dialog::message(const std::string &, const std::string &, enum easy3d::dialog::Choice, enum easy3d::dialog::Type) --> enum easy3d::dialog::Response", pybind11::arg("title"), pybind11::arg("message"), pybind11::arg("choice"), pybind11::arg("type"));

}
