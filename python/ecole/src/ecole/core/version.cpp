#include <pybind11/pybind11.h>
#include <pybind11/stl/filesystem.h>

#include "ecole/version.hpp"

namespace ecole::version {

namespace py = pybind11;

void bind_submodule(py::module_ m) {
	m.doc() = "Ecole version utilities.";

	py::class_<VersionInfo>(m, "VersionInfo")  //
		.def_readwrite("major", &VersionInfo::major)
		.def_readwrite("minor", &VersionInfo::minor)
		.def_readwrite("patch", &VersionInfo::patch)
		.def_readwrite("revision", &VersionInfo::revision)
		.def_readwrite("build_type", &VersionInfo::build_type)
		.def_readwrite("build_os", &VersionInfo::build_os)
		.def_readwrite("build_time", &VersionInfo::build_time)
		.def_readwrite("build_compiler", &VersionInfo::build_compiler);

	m.def("get_ecole_lib_version", &get_ecole_lib_version);
	m.def("get_ecole_lib_path", &get_ecole_lib_path);
	m.def("get_scip_buildtime_version", &get_scip_buildtime_version);
	m.def("get_scip_lib_version", &get_scip_lib_version);
	m.def("get_scip_lib_path", &get_scip_lib_path);
}

}  // namespace ecole::version
