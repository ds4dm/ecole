#include <optional>

#include <nonstd/span.hpp>
#include <xtensor/xtensor.hpp>

#include "ecole/observation/hutter-2011.hpp"
#include "ecole/scip/model.hpp"

namespace ecole::observation {

namespace {

using Features = Hutter2011Obs::Features;
using value_type = decltype(Hutter2011Obs::features)::value_type;

auto extract_features(scip::Model& model) {
	xt::xtensor<value_type, 1> observation({Hutter2011Obs::n_features}, std::nan(""));

	auto* const scip = model.get_scip_ptr();

	return observation;
}

}  // namespace

/*************************************
 *  Observation extracting function  *
 *************************************/

// void Hutter2011::before_reset(scip::Model& /* model */) {
// 	static_features = decltype(static_features){};
// }

auto Hutter2011::extract(scip::Model& model, bool /* done */) -> std::optional<Hutter2011Obs> {
	if (model.get_stage() >= SCIP_STAGE_SOLVING) {
		return {};
	}
	return {{extract_features(model)}};
}

}  // namespace ecole::observation
