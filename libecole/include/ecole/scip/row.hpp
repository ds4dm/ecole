#pragma once

#include <scip/scip.h>

#include "ecole/scip/type.hpp"
#include "ecole/scip/view.hpp"

namespace ecole {
namespace scip {

class RowProxy : public Proxy<SCIP_Row> {
public:
	using Proxy::Proxy;
};

using RowView = View<RowProxy>;

}  // namespace scip
}  // namespace ecole
