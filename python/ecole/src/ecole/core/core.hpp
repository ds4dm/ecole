#pragma once

#include <pybind11/pybind11.h>

#include "caster.hpp"

namespace ecole {

namespace version {
void bind_submodule(pybind11::module_ m);
}

namespace scip {
void bind_submodule(pybind11::module_ m);
}

namespace data {
void bind_submodule(pybind11::module_ const& m);
}

namespace instance {
void bind_submodule(pybind11::module const& m);
}

namespace observation {
void bind_submodule(pybind11::module_ const& m);
}

namespace reward {
void bind_submodule(pybind11::module_ const& m);
}

namespace information {
void bind_submodule(pybind11::module_ const& m);
}

namespace dynamics {
void bind_submodule(pybind11::module_ const& m);
}

}  // namespace ecole
