#pragma once

#include <pybind11/pybind11.h>

namespace ecole {

namespace scip {
void bind_submodule(pybind11::module);
}

namespace observation {
void bind_submodule(pybind11::module);
}

namespace reward {
void bind_submodule(pybind11::module);
}

namespace termination {
void bind_submodule(pybind11::module);
}

namespace environment {
void bind_submodule(pybind11::module);
}

}  // namespace ecole
