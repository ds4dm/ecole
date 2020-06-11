import threading

import pytest

import ecole.environment
import ecole.observation


# FIXME set seed.
def run_environment(model):
    env = ecole.environment.Branching(observation_function=ecole.observation.Nothing())
    _, action_set, _, done = env.reset(model)
    while not done:
        _, action_set, _, done, _ = env.step(action_set[0])


@pytest.mark.parametrize("n_threads", (1, 2, 4, 8))
@pytest.mark.benchmark(group="Branching NodeBipartite")
@pytest.mark.slow
def test_environment_mulithread(benchmark, model, n_threads):
    def setup_threads():
        models = [model.copy_orig() for _ in range(n_threads)]
        threads = [threading.Thread(target=run_environment, args=(m,)) for m in models]
        return (threads,), {}

    def run_threads(threads):
        for t in threads:
            t.start()
        for t in threads:
            t.join()

    benchmark.pedantic(run_threads, setup=setup_threads, rounds=5)


# FIXME This is not comparable to the previous benchmark because it is not the same
# branching rule.
# FIXME set seed.
@pytest.mark.parametrize("n_threads", (1, 2, 4, 8))
@pytest.mark.benchmark(group="Solving Model")
@pytest.mark.slow
def test_solve_mulithread(benchmark, model, n_threads):
    def setup_threads():
        models = [model.copy_orig() for _ in range(n_threads)]
        threads = [threading.Thread(target=m.solve) for m in models]
        return (threads,), {}

    def run_threads(threads):
        for t in threads:
            t.start()
        for t in threads:
            t.join()

    benchmark.pedantic(run_threads, setup=setup_threads, rounds=5)
