#pragma once

#include <map>
#include <string>

#include "ecole/data/abstract.hpp"

namespace ecole::information {

/** The type of information dictionnaries. */
template <typename Information> using InformationMap = std::map<std::string, Information>;

/**
 * Abstract base class for all information functions.
 *
 * Information functions can be given to environments to parametrize what additional information is returned
 * in the information dictionnary at 3every transition.
 *
 * @tparam Information the type of the information extracted by this class.
 */
template <typename Information> using InformationFunction = data::DataFunction<InformationMap<Information>>;

}  // namespace ecole::information
