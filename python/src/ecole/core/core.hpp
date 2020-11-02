#pragma once

#include <pybind11/pybind11.h>

#include "caster.hpp"

namespace ecole {

namespace scip {
void bind_submodule(pybind11::module_ const& m);
}

namespace observation {
void bind_submodule(pybind11::module_ const& m);
}

namespace reward {
void bind_submodule(pybind11::module_ const& m);
}

namespace dynamics {
void bind_submodule(pybind11::module_ const& m);
}

}  // namespace ecole
