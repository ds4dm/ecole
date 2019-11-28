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

template <typename T> struct Deleter { void operator()(T* ptr); };
template <typename T> using unique_ptr = std::unique_ptr<T, Deleter<T>>;

unique_ptr<Scip> create();

class Model {
public:
	using BranchFunc = std::function<VarProxy(Model&)>;

	Model();
	Model(unique_ptr<Scip>&& scip);
	Model(Model const& model);
	Model& operator=(Model const&);
	Model(Model&&) noexcept = default;
	Model& operator=(Model&&) noexcept = default;
	virtual ~Model() = default;

	static Model from_file(std::string const& filename);

	template <typename T> void set_param(const char* name, T value);
	template <typename T> T get_param(const char* name);

	void solve();
	void interrupt_solve();

	void disable_presolve();
	void disable_cuts();

	bool is_solved() const noexcept;

	VarView variables() const noexcept;
	VarView lp_branch_vars() const noexcept;

	void set_branch_rule(BranchFunc const& func);

private:
	class LambdaBranchRule;
	unique_ptr<Scip> scip;
};

}  // namespace scip
}  // namespace ecole
