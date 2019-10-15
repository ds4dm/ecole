import pytest

import ecole


@pytest.fixture
def branchEnv(problem_file):
    return ecole.BranchEnv.make_default(str(problem_file))


def test_BranchEnv(branchEnv):
    count = 0

    def f(obs):
        nonlocal count
        count += 1
        return 0

    branchEnv.run(f)
    assert count > 0
