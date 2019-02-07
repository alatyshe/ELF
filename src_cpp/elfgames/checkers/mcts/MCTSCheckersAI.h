#include "../mcts/CheckersMCTSActor.h"

namespace elf {
namespace ai {
namespace tree_search {

template <>
struct ActorTrait<CheckersMCTSActor> {
 public:
	static std::string to_string(const CheckersMCTSActor& a) {
		return a.info();
	}
};

} // namespace tree_search
} // namespace ai
} // namespace elf





class MCTSCheckersAI : public elf::ai::tree_search::MCTSAI_T<CheckersMCTSActor> {
 public:
	MCTSCheckersAI(
			const elf::ai::tree_search::TSOptions& options,
			std::function<CheckersMCTSActor*(int)> gen)
			: elf::ai::tree_search::MCTSAI_T<CheckersMCTSActor>(options, gen) {
	}

	float getValue() const {
		// Check if we need to resign.
		const auto& result = getLastResult();

		if (result.total_visits == 0)
			return result.root_value;
		else
			return result.best_edge_info.getQSA();
	}

	elf::ai::tree_search::MCTSPolicy<Coord> getMCTSPolicy() const {
		const auto& result = getLastResult();
		auto policy = result.mcts_policy;

		policy.normalize();
		return policy;
	}

	void setRequiredVersion(int64_t ver) {
		auto* engine = getEngine();
		assert(engine != nullptr);

		for (size_t i = 0; i < engine->getNumActors(); ++i) {
			engine->getActor(i).setRequiredVersion(ver);
		}
	}
};


