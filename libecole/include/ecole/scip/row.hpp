#pragma once

#include <nonstd/optional.hpp>
#include <scip/scip.h>

#include "ecole/scip/type.hpp"
#include "ecole/scip/view.hpp"

namespace ecole {
namespace scip {

using nonstd::optional;

class RowProxy : public Proxy<SCIP_Row> {
public:
	using Proxy::Proxy;

	optional<real> lhs() const noexcept;
	optional<real> rhs() const noexcept;
};

using RowView = View<RowProxy>;

}  // namespace scip
}  // namespace ecole
