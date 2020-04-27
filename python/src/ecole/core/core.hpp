#pragma once

#include <pybind11/pybind11.h>

#include "ecole/none.hpp"

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

namespace pybind11 {
namespace detail {

/**
 * Custom caster for @ref ecole::NoneType.
 *
 * Cast to `None` in Python and does not cast to C++.
 */
template <> struct type_caster<ecole::NoneType> : void_caster<ecole::NoneType> {};

}  // namespace detail
}  // namespace pybind11
