#include "ecole/scip/model.hpp"

#include "benchmark.hpp"

namespace ecole::benchmark {

auto Instance::from_model(scip::Model const& model) -> Instance {
	return {model.name(), model.variables().size(), model.constraints().size(), model.nnz()};
}

}  // namespace ecole::benchmark
