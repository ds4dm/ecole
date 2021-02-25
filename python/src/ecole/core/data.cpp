#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "ecole/data/abstract.hpp"
#include "ecole/data/constant.hpp"
#include "ecole/data/map.hpp"
#include "ecole/data/none.hpp"
#include "ecole/data/timed.hpp"
#include "ecole/data/vector.hpp"
#include "ecole/scip/model.hpp"

#include "core.hpp"

namespace ecole::data {

namespace py = pybind11;

/**
 * A C++ class to wrap any Python object as an data extraction function.
 *
 * This is used to bind templated types such as MapFunction and VectorFunction.
 */
class PyDataFunction final : DataFunction<py::object> {
public:
	PyDataFunction() noexcept = default;
	explicit PyDataFunction(py::object data_func) noexcept : data_function(std::move(data_func)) {}

	void before_reset(scip::Model& model) final { data_function.attr("before_reset")(&model); }

	py::object extract(scip::Model& model, bool done) final { return data_function.attr("extract")(&model, done); }

private:
	py::object data_function;
};

void bind_submodule(py::module_ const& m) {
	m.doc() = "Data extraction functions manipulation.";

	using PyConstantFunction = ConstantFunction<py::object>;
	py::class_<PyConstantFunction>(m, "ConstantFunction", "Always return the given value.")
		.def(py::init<py::object>())
		.def("before_reset", &PyConstantFunction::before_reset, py::arg("model"), "Do nothing.")
		.def("extract", &PyConstantFunction::extract, py::arg("model"), py::arg("done"), "Return the constant.");

	py::class_<NoneFunction>(m, "NoneFunction", "Always retrun None.")
		.def(py::init<>())
		.def("before_reset", &NoneFunction::before_reset, py::arg("model"), "Do nothing.")
		.def("extract", &NoneFunction::extract, py::arg("model"), py::arg("done"), "Return None.");

	using PyVectorFunction = VectorFunction<PyDataFunction>;
	py::class_<PyVectorFunction>(m, "VectorFunction", "Pack data extraction functions together and return data as list.")
		.def(py::init([](py::args const& objects) {
			auto functions = std::vector<PyDataFunction>{objects.size()};
			std::transform(objects.begin(), objects.end(), functions.begin(), [](py::handle obj) {
				return PyDataFunction{py::reinterpret_borrow<py::object>(obj)};
			});
			return std::make_unique<PyVectorFunction>(std::move(functions));
		}))
		.def(
			"before_reset",
			&PyVectorFunction::before_reset,
			py::arg("model"),
			"Call before_reset on all data extraction functions.")
		.def(
			"extract",
			&PyVectorFunction::extract,
			py::arg("model"),
			py::arg("done"),
			"Return data from all functions as a tuple.");

	using PyMapFunction = MapFunction<std::string, PyDataFunction>;
	py::class_<PyMapFunction>(m, "MapFunction", "Pack data extraction functions together and return data as a dict.")
		.def(py::init([](py::kwargs const& objects) {
			auto functions = std::map<std::string, PyDataFunction>{};
			for (auto [key, func] : objects) {
				functions.emplace(key.cast<std::string>(), py::reinterpret_borrow<py::object>(func));
			}
			return std::make_unique<PyMapFunction>(std::move(functions));
		}))
		.def(
			"before_reset",
			&PyMapFunction::before_reset,
			py::arg("model"),
			"Call before_reset on all data extraction functions.")
		.def(
			"extract",
			&PyMapFunction::extract,
			py::arg("model"),
			py::arg("done"),
			"Return data from all functions as a dict.");

	using PyTimedFunction = TimedFunction<PyDataFunction>;
	py::class_<PyTimedFunction>(m, "TimedFunction", "Time in seconds of any function.")
		.def(
			py::init([](py::object func, bool wall) {
				return std::make_unique<PyTimedFunction>(PyDataFunction{std::move(func)}, wall);
			}),
			py::arg("func"),
			py::arg("wall") = false)
		.def(py::init<bool>(), py::arg("wall") = false)
		.def(
			"before_reset",
			&PyTimedFunction::before_reset,
			py::arg("model"),
			"Call before_reset on the data extraction functions.")
		.def(
			"extract",
			&PyTimedFunction::extract,
			py::arg("model"),
			py::arg("done"),
			"Time the data extract function in seconds.");
}

}  // namespace ecole::data
