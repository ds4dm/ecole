import pytest

import ecole


@pytest.fixture
def branchEnv(problem_file):
    env = ecole.BranchEnv.from_file(str(problem_file))
    env.disable_presolve()
    env.disable_cuts()
    return env


def test_BranchEnv(branchEnv):
    count = 0

    def f(obs):
        nonlocal count
        count += 1
        return 0

    branchEnv.run(f)
    assert count > 0
