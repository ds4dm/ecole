import concurrent.futures as futures

import pytest

import ecole.environment
import ecole.observation


def run_environment(model) -> int:
    model.set_param("limits/totalnodes", 10)
    env = ecole.environment.Branching(
        observation_function=ecole.observation.NodeBipartite()
    )
    count = 0
    for _ in range(3):
        _, action_set, done = env.reset(model)
        while not done:
            _, action_set, _, done, _ = env.step(action_set[0])
            count += 1
    return count


@pytest.mark.parametrize("n_workers", (1, 2, 4, 8))
@pytest.mark.benchmark(group="Branching NodeBipartite")
@pytest.mark.slow
def test_mulithread(benchmark, model, n_workers):
    with futures.ThreadPoolExecutor(max_workers=n_workers) as executor:
        models = [model.clone() for _ in range(5)]

        @benchmark
        def run_asynch():
            submissions = [executor.submit(run_environment, model=m) for m in models]
            return [s.result() for s in submissions]
