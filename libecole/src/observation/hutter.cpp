#include <optional>

#include <nonstd/span.hpp>
#include <xtensor/xtensor.hpp>

#include "ecole/observation/hutter.hpp"
#include "ecole/scip/model.hpp"

namespace ecole::observation {

namespace {

using Features = HutterObs::Features;
using value_type = decltype(HutterObs::features)::value_type;

auto extract_features(scip::Model& model) {
	xt::xtensor<value_type, 1> observation({HutterObs::n_features}, std::nan(""));

	auto* const scip = model.get_scip_ptr();

	return observation;
}

}  // namespace

/*************************************
 *  Observation extracting function  *
 *************************************/

// void Hutter::before_reset(scip::Model& /* model */) {
// 	static_features = decltype(static_features){};
// }

auto Hutter::extract(scip::Model& model, bool /* done */) -> std::optional<HutterObs> {
	if (model.get_stage() >= SCIP_STAGE_SOLVING) {
		return {};
	}
	return {{extract_features(model)}};
}

}  // namespace ecole::observation
