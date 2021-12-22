
namespace ecole::scip {
class Model;
}

namespace ecole::data {

/** Dummy data function to monitor what is happening. */
template <typename T> struct MockFunction {
	T val;

	MockFunction() = default;
	MockFunction(T val_) : val{val_} {}

	auto before_reset(scip::Model const& /* model */) -> void { ++val; };

	[[nodiscard]] auto extract(scip::Model const& /* model */, bool /* done */) const -> T { return val; }
};

using IntDataFunc = MockFunction<int>;
using DoubleDataFunc = MockFunction<double>;

}  // namespace ecole::data
