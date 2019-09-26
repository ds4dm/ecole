#pragma once

#include <memory>
#include <string>

#include "ecole/scip/variable.hpp"

// Avoid including SCIP header
typedef struct Scip Scip;

namespace ecole {
namespace scip {

struct ScipDeleter {
	void operator()(Scip* scip);
};

using ScipPtr = std::unique_ptr<Scip, ScipDeleter>;
ScipPtr create();

class Model {
private:
	ScipPtr scip;

public:
	Model();
	Model(ScipPtr&& scip);
	Model(Model const& model);
	Model& operator=(Model const&);
	Model(Model&&) noexcept = default;
	Model& operator=(Model&&) noexcept = default;
	virtual ~Model() = default;

	static Model from_file(std::string const& filename);

	void solve();

	std::size_t n_vars() const noexcept;
	VarView variables() const noexcept;
};

} // namespace scip
} // namespace ecole
