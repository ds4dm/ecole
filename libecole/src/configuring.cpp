#include <scip/scip.h>

#include "ecole/configuring.hpp"

namespace ecole {
namespace configuring {

template <typename ParamType>
Configure<ParamType>::Configure(std::string param) noexcept : param(std::move(param)) {}

template <typename ParamType>
void Configure<ParamType>::set(scip::Model& model, action_t const& action) {
	model.set_param_explicit(param.c_str(), action);
}

// FIXME
template class Configure<SCIP_Bool>;
template class Configure<char>;
template class Configure<int>;
template class Configure<SCIP_Longint>;
template class Configure<SCIP_Real>;
template class Configure<char const*>;

}  // namespace configuring
}  // namespace ecole
