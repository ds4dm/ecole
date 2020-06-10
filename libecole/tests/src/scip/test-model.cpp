#include <future>
#include <limits>
#include <string>

#include <catch2/catch.hpp>
#include <scip/scip.h>

#include "ecole/scip/exception.hpp"
#include "ecole/scip/model.hpp"

#include "conftest.hpp"

using namespace ecole;

TEST_CASE("Creation of model") {
	scip::Model model{};
	SECTION("Move construct") { auto model_moved = std::move(model); }
}

TEST_CASE("Equality comparison") {
	auto model = scip::Model{};
	REQUIRE(model == model);
	REQUIRE(model != model.copy_orig());
}

TEST_CASE("Create model from file") {
	auto model = scip::Model::from_file(problem_file);
}

TEST_CASE("Raise if file does not exist") {
	REQUIRE_THROWS_AS(scip::Model::from_file("/does_not_exist.mps"), scip::Exception);
}

TEST_CASE("Model solving") {
	SECTION("Synchronously") {
		auto model = get_model();
		model.solve();
	}

	SECTION("Asynchronously") {
		auto load_solve = [] {
			get_model().solve();
			return true;
		};
		auto fut1 = std::async(std::launch::async, load_solve);
		auto fut2 = std::async(std::launch::async, load_solve);
		REQUIRE((fut1.get() && fut2.get()));
	}
}

TEST_CASE("Get and set parameters") {
	using scip::ParamType;

	auto model = scip::Model{};
	auto constexpr int_param = "conflict/minmaxvars";

	SECTION("Get parameters explicitly") { model.get_param_explicit<ParamType::Int>(int_param); }

	SECTION("Set parameters explicitly") {
		model.set_param_explicit<ParamType::Int>(int_param, false);
	}

	SECTION("Throw on wrong parameters type") {
		REQUIRE_THROWS_AS(model.get_param_explicit<ParamType::Real>(int_param), scip::Exception);
		REQUIRE_THROWS_AS(model.set_param_explicit<ParamType::Real>(int_param, 3.), scip::Exception);
	}

	SECTION("Get parameters with automatic casting") { model.get_param<double>(int_param); }

	SECTION("Set parameters with automatic casting") { model.set_param(int_param, 1.); }

	SECTION("String parameters can be converted to chars") {
		model.set_param("branching/scorefunc", "s");
		REQUIRE(model.get_param<char>("branching/scorefunc") == 's');
	}

	SECTION("Get parameters as variants") {
		auto val = model.get_param<scip::Param>(int_param);
		REQUIRE(nonstd::holds_alternative<int>(val));
		REQUIRE(nonstd::get<int>(val) == model.get_param<int>(int_param));
	}

	SECTION("Set parameters as variants") {
		scip::Param new_val = model.get_param<int>(int_param) + 1;
		model.set_param(int_param, new_val);
		REQUIRE(model.get_param<int>(int_param) == nonstd::get<int>(new_val));
	}

	SECTION("Extract map of parameters") {
		auto vals = model.get_params();
		REQUIRE(vals.size() > 0l);
		REQUIRE(vals[int_param] == scip::Param{model.get_param<int>(int_param)});
	}

	SECTION("Set map of parameters") {
		auto vals = model.get_params();
		vals[int_param] = nonstd::get<int>(vals[int_param]) + 1;
		model.set_params(vals);
		REQUIRE(vals[int_param] == scip::Param{model.get_param<int>(int_param)});
	}
}
