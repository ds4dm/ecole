#pragma once

#include <map>
#include <string>

#include "ecole/data/abstract.hpp"

namespace ecole::information {

/** The type of information dictionnaries. */
template <typename Information> using InformationMap = std::map<std::string, Information>;
}  // namespace ecole::information
