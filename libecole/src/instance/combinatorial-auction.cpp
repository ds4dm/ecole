#include <algorithm>
#include <map>

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

CombinatorialAuctionGenerator::CombinatorialAuctionGenerator(RandomEngine random_engine_, Parameters parameters_) :
	random_engine{random_engine_}, parameters{parameters_} {}
CombinatorialAuctionGenerator::CombinatorialAuctionGenerator(Parameters parameters_) :
	CombinatorialAuctionGenerator{ecole::spawn_random_engine(), parameters_} {}
CombinatorialAuctionGenerator::CombinatorialAuctionGenerator() : CombinatorialAuctionGenerator(Parameters{}) {}

scip::Model CombinatorialAuctionGenerator::next() {
	return generate_instance(random_engine, parameters);
}

void CombinatorialAuctionGenerator::seed(Seed seed) {
	random_engine.seed(seed);
}

namespace {

using vector = std::vector<std::size_t>;
template <typename T> using xvector = xt::xtensor<T, 1>;
template <typename T> using xmatric = xt::xtensor<T, 2>;

/**
 * Sample with replacement based on weights given by a probability or weight vector.
 *
 * Samples n_samples values from a weighted distribution defined by the weights.
 * The values are in the range of [1, weights.size()].
 */
auto arg_choice_without_replacement(std::size_t n_samples, xvector<double> weights, RandomEngine& random_engine)
	-> xvector<std::size_t> {
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
	xt::xtensor<int, 1> const& bundle_mask,
	xvector<double> const& interests,
	xmatric<double> const& compats,
	RandomEngine& random_engine) {
	auto const compats_masked = xt::index_view(compats, bundle_mask);
	auto const compats_masked_mean = xt::sum(compats_masked, 0);
	auto const probs = xt::eval((1 - bundle_mask) * interests * compats_masked_mean);
	return arg_choice_without_replacement(1, probs, random_engine)(0);
}

/** Adds a single variable with the coefficient price. */
auto add_var(SCIP* scip, std::size_t i, double price) {
	auto const name = fmt::format("x_{}", i);
	auto unique_var = scip::create_var_basic(scip, name.c_str(), 0., 1., price, SCIP_VARTYPE_BINARY);
	auto* var_ptr = unique_var.get();
	scip::call(SCIPaddVar, scip, var_ptr);
	return var_ptr;
}

/** Adds all constraints to the SCIP model.  */
auto add_constraints(SCIP* scip, xt::xtensor<SCIP_VAR*, 1> vars, std::vector<vector> bids_per_item) {

	for (std::size_t i = 0; i < bids_per_item.size(); ++i) {

		auto const item_bids = bids_per_item[i];

		if (!item_bids.empty()) {
			auto cons_vars = xt::xtensor<SCIP_VAR*, 1>{{item_bids.size()}};
			auto coefs = xt::xtensor<scip::real, 1>(cons_vars.shape(), 1.);
			for (std::size_t j = 0; j < item_bids.size(); ++j) {
				cons_vars(j) = vars(item_bids[j]);
			}
			auto name = fmt::format("c_{}", i);
			auto const inf = SCIPinfinity(scip);
			auto cons =
				scip::create_cons_basic_linear(scip, name.c_str(), cons_vars.size(), &cons_vars(0), coefs.data(), -inf, 1.0);
			scip::call(SCIPaddCons, scip, cons.get());
		}
	}
}

/**
 * Placeholder warning function.
 *
 * Ideally we should manage logging and warnings properly.
 */
template <typename String> void warning(String&& message, bool do_log) {
	if (do_log) {
		std::clog << "Warning: " << message << '\n';
	}
}

}  // namespace

/******************************************************
 *  CombinatorialAuctionGenerator::generate_instance  *
 ******************************************************/

scip::Model CombinatorialAuctionGenerator::generate_instance(RandomEngine& random_engine, Parameters parameters) {

	assert(parameters.min_value >= 0 && parameters.max_value >= parameters.min_value);
	assert(parameters.add_item_prob >= 0 && parameters.add_item_prob <= 1);

	auto const rand_val = xt::eval(xt::random::rand({parameters.n_items}, 0.0, 1.0, random_engine));
	auto const values = xt::eval(parameters.min_value + (parameters.max_value - parameters.min_value) * rand_val);

	// get compatibilities
	auto const compats_rand =
		xt::eval(xt::random::rand({parameters.n_items, parameters.n_items}, 0.0, 1.0, random_engine));
	auto compats = xt::eval(xt::triu(compats_rand, 1));
	compats += xt::transpose(compats);
	compats /= xt::sum(compats, 1);

	std::size_t n_dummy_items = 0;
	std::size_t bid_index = 0;

	std::vector<vector> bids_bundle{};
	std::vector<double> bids_price{};

	while (bid_index < parameters.n_bids) {

		// bidder item values (buy price) and interests
		auto const private_interests = xt::eval(xt::random::rand({parameters.n_items}, 0.0, 1.0, random_engine));
		auto const private_values =
			xt::eval(values + parameters.max_value * parameters.value_deviation * (2 * private_interests - 1));

		// substitutable bids of this bidder
		std::map<vector, double> bidder_bids = {};

		// generate initial bundle, choose first item according to bidder interests
		auto item = arg_choice_without_replacement(1, private_interests, random_engine)(0);

		xt::xtensor<int, 1> bundle_mask = xt::zeros<int>({parameters.n_items});
		bundle_mask(item) = 1;

		// add additional items, according to bidder interests and item compatibilities
		while (true) {
			double sampled_prob = xt::random::rand({1}, 0.0, 1.0, random_engine)[0];
			if (sampled_prob >= parameters.add_item_prob) {
				break;
			}

			if (static_cast<std::size_t>(xt::sum(bundle_mask)()) == parameters.n_items) {
				break;
			}

			item = choose_next_item(bundle_mask, private_interests, compats, random_engine);
			bundle_mask(item) = 1;
		}

		vector bundle = xt::nonzero(bundle_mask)[0];

		// get price of bundle
		auto const private_values_slice = xt::index_view(private_values, bundle);
		auto const bundle_power = std::pow(static_cast<double>(bundle.size()), 1.0 + parameters.additivity);
		auto price = xt::sum(private_values_slice)() + bundle_power;

		if (parameters.integers) {
			price = floor(price);
		}

		// restart bid if price < 0
		if (price < 0) {
			warning("Negatively priced bundle avoided", parameters.warnings);
			continue;
		}

		bidder_bids[bundle] = price;

		// get sub-bundles
		std::vector<vector> sub_candidates_bundle;
		std::vector<double> sub_candidates_price;

		for (size_t i = 0; i < bundle.size(); ++i) {

			std::size_t sub_item = bundle[i];

			// at least one item must be shared with initial bundle
			xt::xtensor<int, 1> sub_bundle_mask = xt::zeros<int>({parameters.n_items});
			sub_bundle_mask(sub_item) = 1;

			// add additional items, according to bidder interests and item compatibilities
			while (true) {
				auto const sub_mask_sum = static_cast<std::size_t>(xt::sum(sub_bundle_mask)());
				if (sub_mask_sum >= bundle.size()) {
					break;
				}

				sub_item = choose_next_item(sub_bundle_mask, private_interests, compats, random_engine);
				sub_bundle_mask(sub_item) = 1;
			}

			vector sub_bundle = xt::nonzero(sub_bundle_mask)[0];

			// get price of sub-bundle
			auto const sub_private_values_slice = xt::index_view(private_values, sub_bundle);
			auto const sub_bundle_power = std::pow(static_cast<double>(sub_bundle.size()), 1.0 + parameters.additivity);
			auto sub_price = xt::sum(sub_private_values_slice)() + sub_bundle_power;

			if (parameters.integers) {
				sub_price = floor(sub_price);
			}

			sub_candidates_bundle.push_back(sub_bundle);
			sub_candidates_price.push_back(sub_price);
		}

		auto const budget = parameters.budget_factor * price;

		auto const value_slice = xt::index_view(values, bundle);
		auto const min_resale_value = parameters.resale_factor * xt::sum(value_slice)();

		auto const price_tensor = xt::adapt(sub_candidates_price, {sub_candidates_price.size()});
		auto const sorted_indices = xt::argsort(price_tensor);

		// get XOR bids, higher priced candidates first
		for (std::size_t i = 0; i < sorted_indices.size(); ++i) {
			auto const idx = sorted_indices.size() - i - 1;
			auto bundle_i = sub_candidates_bundle[sorted_indices(idx)];
			auto price_i = sub_candidates_price[sorted_indices(idx)];

			if (bidder_bids.size() >= parameters.max_n_sub_bids + 1 || bid_index + bidder_bids.size() >= parameters.n_bids) {
				break;
			}

			if (price_i < 0) {
				warning("Negatively priced substitutable bundle avoided", parameters.warnings);
			}

			if (price_i > budget) {
				warning("Over priced substitutable bundle avoided", parameters.warnings);
				continue;
			}

			// FIXME unused  xvector value_slice_i = xt::index_view(values, bundle_i);
			double value_slice_sum_i = xt::sum(value_slice)();
			if (value_slice_sum_i < min_resale_value) {
				warning("Substitutable bundle below min resale value avoided", parameters.warnings);
				continue;
			}

			if (bidder_bids.count(bundle_i)) {  // REPL
				warning("Duplicated substitutable bundle avoided", parameters.warnings);
				continue;
			}

			bidder_bids[bundle_i] = price_i;
		}

		std::size_t dummy_item = 0;
		if (bidder_bids.size() > 2) {
			dummy_item = parameters.n_items + n_dummy_items;
			++n_dummy_items;
		}

		// add bids
		for (auto const& [b, p] : bidder_bids) {
			auto bund_copy = b;
			if (dummy_item) {
				bund_copy.push_back(dummy_item);
			}
			bids_bundle.push_back(bund_copy);
			bids_price.push_back(p);
			++bid_index;
		}

	}  // loop to get bids

	// create scip model
	auto model = scip::Model::prob_basic();
	auto* const scip = model.get_scip_ptr();
	scip::call(SCIPsetObjsense, scip, SCIP_OBJSENSE_MAXIMIZE);

	//  initialize bids_per_item vector
	std::vector<vector> bids_per_item;
	for (std::size_t i = 0; i < parameters.n_items + n_dummy_items; ++i) {
		vector bids_per_item_i;
		bids_per_item.push_back(bids_per_item_i);
	}

	// add variables
	auto vars = xt::xtensor<SCIP_VAR*, 1>{{bids_bundle.size()}};
	for (std::size_t i = 0; i < bids_bundle.size(); ++i) {
		vector bundle_i = bids_bundle[i];
		double price_i = bids_price[i];

		vars(i) = add_var(scip, i, price_i);

		for (auto const j : bundle_i) {
			bids_per_item[j].push_back(i);
		}
	}

	// add constraints
	add_constraints(scip, vars, bids_per_item);

	return model;
}

}  // namespace ecole::instance
