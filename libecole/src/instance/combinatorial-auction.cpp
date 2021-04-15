#include <algorithm>
#include <map>
#include <stdexcept>
#include <tuple>
#include <utility>

#include <fmt/format.h>

#include <xtensor/xadapt.hpp>
#include <xtensor/xindex_view.hpp>
#include <xtensor/xrandom.hpp>
#include <xtensor/xsort.hpp>
#include <xtensor/xtensor.hpp>
#include <xtensor/xview.hpp>

#include "ecole/instance/combinatorial-auction.hpp"
#include "ecole/scip/cons.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/utils.hpp"
#include "ecole/scip/var.hpp"

namespace ecole::instance {

/*******************************************
 *  CombinatorialAuctionGenerator methods  *
 *******************************************/

CombinatorialAuctionGenerator::CombinatorialAuctionGenerator(Parameters parameters_, RandomEngine random_engine_) :
	random_engine{random_engine_}, parameters{parameters_} {}
CombinatorialAuctionGenerator::CombinatorialAuctionGenerator(Parameters parameters_) :
	CombinatorialAuctionGenerator{parameters_, ecole::spawn_random_engine()} {}
CombinatorialAuctionGenerator::CombinatorialAuctionGenerator() : CombinatorialAuctionGenerator(Parameters{}) {}

scip::Model CombinatorialAuctionGenerator::next() {
	return generate_instance(parameters, random_engine);
}

void CombinatorialAuctionGenerator::seed(Seed seed) {
	random_engine.seed(seed);
}

namespace {

template <typename T> using xvector = xt::xtensor<T, 1>;
template <typename T> using xmatrix = xt::xtensor<T, 2>;
using Bundle = std::vector<std::size_t>;
using Price = double;

/** Logs warnings for invalid bids.
 *
 * Logging warnings may be useful when using non-default parameters for the
 * generator as some sets of parameters will cause the generator to have
 * mostly invalid bids, making the compute time to generate an instance long.
 */
class Logger {
public:
	Logger(bool print_, std::string pattern_) : pattern{std::move(pattern_)}, print{print_} {}
	Logger(bool print_) : print{print_} {}

	template <typename T> void log(T&& message) {
		if (print) {
			fmt::print(pattern, std::forward<T>(message));
		}
	}

private:
	std::string pattern = "ecole::instance: {}\n";
	bool print = false;
};

/**
 * Sample with replacement based on weights given by a probability or weight vector.
 *
 * Samples n_samples values from a weighted distribution defined by the weights.
 * The values are in the range of [1, weights.size()].
 */
auto arg_choice_without_replacement(std::size_t n_samples, xvector<double> weights, RandomEngine& random_engine) {
	auto const wc = xt::eval(xt::cumsum(weights));
	auto weight_dist = std::uniform_real_distribution<double>{0, wc[wc.size() - 1]};

	xvector<std::size_t> indices({n_samples});
	for (auto& idx : indices) {
		const auto u = weight_dist(random_engine);
		idx = static_cast<std::size_t>(std::upper_bound(wc.cbegin(), wc.cend(), u) - wc.cbegin());
	}
	return indices;
}

/** Choose the next item to be added to the bundle/sub-bundle. */
auto choose_next_item(
	xvector<std::size_t> const& bundle_mask,
	xvector<double> const& interests,
	xmatrix<double> const& compats,
	RandomEngine& random_engine) {
	auto const compats_masked = xt::index_view(compats, bundle_mask);
	auto const compats_masked_mean = xt::sum(compats_masked, 0);
	auto const probs = xt::eval((1 - bundle_mask) * interests * compats_masked_mean);
	return arg_choice_without_replacement(1, probs, random_engine)(0);
}

/** Gets price of the bundle */
auto get_bundle_price(const Bundle& bundle, const xvector<double>& private_values, bool integers, double additivity) {

	auto bundle_sum = xt::sum(xt::index_view(private_values, bundle))();
	auto bundle_power = std::pow(static_cast<double>(bundle.size()), 1.0 + additivity);
	auto price = bundle_sum + bundle_power;

	if (integers) {
		price = floor(price);
	}

	return price;
}

/** Generate initial bundle, choose first item according to bidder interests */
auto get_bundle(
	const xmatrix<double>& compats,
	const xvector<double>& private_interests,
	const xvector<double>& private_values,
	std::size_t n_items,
	bool integers,
	double additivity,
	double add_item_prob,
	RandomEngine& random_engine) {

	auto item = arg_choice_without_replacement(1, private_interests, random_engine)(0);

	auto bundle_mask = xvector<std::size_t>({n_items}, 0);
	bundle_mask[item] = 1;

	// add additional items, according to bidder interests and item compatibilities
	while (true) {
		double sampled_prob = xt::random::rand({1}, 0.0, 1.0, random_engine)[0];
		if (sampled_prob >= add_item_prob) {
			break;
		}

		if (xt::sum(bundle_mask)() == n_items) {
			break;
		}

		item = choose_next_item(bundle_mask, private_interests, compats, random_engine);
		bundle_mask[item] = 1;
	}

	Bundle bundle = xt::nonzero(bundle_mask)[0];

	auto price = get_bundle_price(bundle, private_values, integers, additivity);

	return std::tuple{std::move(bundle), price};
}

/** Generate the set of subsitue bundles */
auto get_substitute_bundles(
	const Bundle& bundle,
	const xmatrix<double>& compats,
	const xvector<double>& private_interests,
	const xvector<double>& private_values,
	std::size_t n_items,
	bool integers,
	double additivity,
	RandomEngine& random_engine) {

	// get substitute bundles
	std::vector<std::tuple<Bundle, Price>> sub_bundles{};

	for (auto item : bundle) {

		// at least one item must be shared with initial bundle
		auto sub_bundle_mask = xvector<std::size_t>({n_items}, 0);
		sub_bundle_mask[item] = 1;

		// add additional items, according to bidder interests and item compatibilities
		while (true) {
			if (xt::sum(sub_bundle_mask)() >= bundle.size()) {
				break;
			}
			item = choose_next_item(sub_bundle_mask, private_interests, compats, random_engine);
			sub_bundle_mask[item] = 1;
		}

		Bundle sub_bundle = xt::nonzero(sub_bundle_mask)[0];

		auto sub_price = get_bundle_price(sub_bundle, private_values, integers, additivity);

		sub_bundles.emplace_back(sub_bundle, sub_price);
	}

	return sub_bundles;
}

/** Adds valid substitue bundles to bidder_bids.
 *
 * Bundles are added in descending price order until either
 * all valid bundles are added or the maximum number of bids
 * is reached.
 */
auto add_bundles(
	std::map<Bundle, Price>& bidder_bids,
	std::vector<std::tuple<Bundle, Price>> sub_bundles,
	const xvector<double>& values,
	const Bundle& bundle,
	Price price,
	std::size_t bid_index,
	Logger logger,
	double budget_factor,
	double resale_factor,
	std::size_t max_n_sub_bids,
	std::size_t n_bids) {

	std::vector<Bundle> bundles{};
	std::vector<double> prices{};

	auto budget = budget_factor * price;
	auto min_resale_value = resale_factor * xt::sum(xt::index_view(values, bundle))();

	// sort for highest price substitute bundles first
	std::sort(sub_bundles.begin(), sub_bundles.end(), [](auto const& a, auto const& b) {
		return (std::get<1>(a) > std::get<1>(b));
	});

	// add valid substitute bundles to bidder_bids
	for (auto [sub_bundle, sub_price] : sub_bundles) {

		if (bidder_bids.size() >= max_n_sub_bids + 1 || bid_index + bidder_bids.size() >= n_bids) {
			break;
		}

		if (sub_price < 0) {
			logger.log("warning, negatively priced substitutable bundle avoided");
			continue;
		}

		if (sub_price > budget) {
			logger.log("warning, over priced substitutable bundle avoided");
			continue;
		}

		if (xt::sum(xt::index_view(values, sub_bundle))() < min_resale_value) {
			logger.log("warning, substitutable bundle below min resale value avoided");
			continue;
		}

		if (bidder_bids.count(sub_bundle) == 1) {
			logger.log("warning, duplicated substitutable bundle avoided");
			continue;
		}

		bidder_bids[sub_bundle] = sub_price;
	}
}

/** Determines if a dummy item is required.  If so, n_dummy_items is incremented */
auto add_dummy_item(std::size_t& n_dummy_items, std::map<Bundle, Price> const& bidder_bids, std::size_t n_items) {

	std::size_t dummy_item = 0;
	if (bidder_bids.size() > 2) {
		dummy_item = n_items + n_dummy_items;
		++n_dummy_items;
	}

	return dummy_item;
}

/** Adds bids from bidder_bids to bids.  Adds dummy item to each bid. */
auto add_bids(
	std::vector<std::tuple<Bundle, Price>>& bids,
	std::map<Bundle, Price> const& bidder_bids,
	std::size_t& bid_index,
	std::size_t dummy_item) {

	for (auto const& [b, p] : bidder_bids) {
		auto bund_copy = b;
		if (dummy_item) {
			bund_copy.push_back(dummy_item);
		}
		bids[bid_index] = std::tuple{std::move(bund_copy), p};
		++bid_index;
	}
}

/** Gets all bids. */
auto get_bids(
	const xvector<double>& values,
	const xmatrix<double>& compats,
	unsigned int max_value,
	std::size_t n_items,
	std::size_t n_bids,
	std::size_t max_n_sub_bids,
	bool integers,
	double value_deviation,
	double additivity,
	double add_item_prob,
	double budget_factor,
	double resale_factor,
	Logger logger,
	RandomEngine& random_engine) {

	std::size_t n_dummy_items = 0;
	std::size_t bid_index = 0;
	std::vector<std::tuple<Bundle, Price>> bids{n_bids};

	while (bid_index < n_bids) {

		// bidder item values (buy price) and interests
		auto const private_interests = xt::eval(xt::random::rand({n_items}, 0.0, 1.0, random_engine));
		auto const private_values = xt::eval(values + max_value * value_deviation * (2 * private_interests - 1));

		// substitutable bids of this bidder
		std::map<Bundle, Price> bidder_bids = {};

		auto [bundle, price] = get_bundle(
			compats, private_interests, private_values, n_items, integers, additivity, add_item_prob, random_engine);

		// restart bid if price < 0
		if (price < 0) {
			logger.log("warning, negatively priced bundle avoided");
			continue;
		}

		// add bid to bidder_bids
		bidder_bids[bundle] = price;

		// get substitute bundles
		auto substitute_bundles = get_substitute_bundles(
			bundle, compats, private_interests, private_values, n_items, integers, additivity, random_engine);

		// add bundles to bidder_bids
		add_bundles(
			bidder_bids,
			substitute_bundles,
			values,
			bundle,
			price,
			bid_index,
			logger,
			budget_factor,
			resale_factor,
			max_n_sub_bids,
			n_bids);

		// get dummy item if required
		auto dummy_item = add_dummy_item(n_dummy_items, bidder_bids, n_items);

		// add all bids to bids
		add_bids(bids, bidder_bids, bid_index, dummy_item);

	}  // loop to get bids

	return std::tuple{bids, n_dummy_items};
}

/** Adds a single variable with the coefficient price. */
auto add_var(SCIP* scip, std::size_t i, Price price) {
	auto const name = fmt::format("x_{}", i);
	auto unique_var = scip::create_var_basic(scip, name.c_str(), 0., 1., price, SCIP_VARTYPE_BINARY);
	auto* var_ptr = unique_var.get();
	scip::call(SCIPaddVar, scip, var_ptr);
	return var_ptr;
}

/** Add all variables associated with the bundles. */
auto add_vars(SCIP* scip, std::vector<std::tuple<Bundle, Price>> const& bids) {
	auto vars = xvector<SCIP_VAR*>{{bids.size()}};
	std::size_t i = 0;
	for (auto [_, price] : bids) {
		vars[i] = add_var(scip, i, price);
		++i;
	}
	return vars;
}

/** Adds all constraints to the SCIP model.  */
auto add_constraints(SCIP* scip, xvector<SCIP_VAR*> vars, std::vector<Bundle> const& bids_per_item) {
	std::size_t index = 0;
	for (auto item_bids : bids_per_item) {
		if (!item_bids.empty()) {
			auto cons_vars = xvector<SCIP_VAR*>{{item_bids.size()}};
			auto coefs = xvector<SCIP_Real>(cons_vars.shape(), 1.);
			for (std::size_t j = 0; j < item_bids.size(); ++j) {
				cons_vars(j) = vars(item_bids[j]);
			}
			auto name = fmt::format("c_{}", index);
			auto const inf = SCIPinfinity(scip);
			auto cons =
				scip::create_cons_basic_linear(scip, name.c_str(), cons_vars.size(), &cons_vars(0), coefs.data(), -inf, 1.0);
			scip::call(SCIPaddCons, scip, cons.get());
		}
		++index;
	}
}

}  // namespace

/******************************************************
 *  CombinatorialAuctionGenerator::generate_instance  *
 ******************************************************/

scip::Model CombinatorialAuctionGenerator::generate_instance(Parameters parameters, RandomEngine& random_engine) {

	// check that parameters are valid
	if (!(parameters.max_value >= parameters.min_value)) {
		throw std::invalid_argument{
			"Parameters max_value and min_value must be defined such that: min_value <= max_value."};
	}

	if (!(parameters.add_item_prob >= 0 && parameters.add_item_prob <= 1)) {
		throw std::invalid_argument{"Parameter add_item_prob must be in range [0,1]."};
	}

	// initialize logger for warnings
	auto logger = Logger(parameters.warnings);

	// get values
	auto const rand_val = xt::eval(xt::random::rand({parameters.n_items}, 0.0, 1.0, random_engine));
	auto const values = xt::eval(parameters.min_value + (parameters.max_value - parameters.min_value) * rand_val);

	// get compatibilities
	auto const compats_rand =
		xt::eval(xt::random::rand({parameters.n_items, parameters.n_items}, 0.0, 1.0, random_engine));
	auto compats = xt::eval(xt::triu(compats_rand, 1));
	compats += xt::transpose(compats);
	compats /= xt::sum(compats, 1);

	// get all bids
	auto [bids, n_dummy_items] = get_bids(
		values,
		compats,
		parameters.max_value,
		parameters.n_items,
		parameters.n_bids,
		parameters.max_n_sub_bids,
		parameters.integers,
		parameters.value_deviation,
		parameters.additivity,
		parameters.add_item_prob,
		parameters.budget_factor,
		parameters.resale_factor,
		logger,
		random_engine);

	// create scip model
	auto model = scip::Model::prob_basic();
	model.set_name(fmt::format("CombinatorialAuction-{}-{}", parameters.n_items, parameters.n_bids));
	auto* const scip = model.get_scip_ptr();
	scip::call(SCIPsetObjsense, scip, SCIP_OBJSENSE_MAXIMIZE);

	// Compute bids_per_item
	std::vector<Bundle> bids_per_item{parameters.n_items + n_dummy_items};
	std::size_t i = 0;
	for (auto const& [bundle, _] : bids) {
		for (auto j : bundle) {
			bids_per_item[j].push_back(i);
		}
		++i;
	}

	auto const vars = add_vars(scip, bids);
	add_constraints(scip, vars, bids_per_item);

	return model;
}

}  // namespace ecole::instance
