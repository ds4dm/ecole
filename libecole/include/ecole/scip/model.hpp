#pragma once

#include <cstddef>
#include <functional>
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

using BranchFunc = std::function<std::size_t()>;

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

	void disable_presolve();
	void disable_cuts();

	std::size_t n_vars() const noexcept;
	VarView variables() const noexcept;

	void set_branch_rule(BranchFunc const& func);
};

} // namespace scip
} // namespace ecole
