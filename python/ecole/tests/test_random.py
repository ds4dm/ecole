import copy
import pickle

import ecole


def test_RandomGenerator():
    """Test bindings of the RandomGenerator."""
    assert ecole.RandomGenerator.min_seed < ecole.RandomGenerator.max_seed
    rng = ecole.RandomGenerator(42)
    rand_val_1 = rng()
    assert isinstance(rand_val_1, int)

    rng.seed(42)
    rand_val_2 = rng()
    assert rand_val_1 == rand_val_2


def test_RandomGenerator_copy():
    """Copy create a new RandomGenerator with same state."""
    rng = ecole.RandomGenerator(42)

    rng_copy = copy.copy(rng)
    assert rng_copy == rng
    assert rng_copy is not rng

    rng_deepcopy = copy.deepcopy(rng)
    assert rng_deepcopy == rng
    assert rng_deepcopy is not rng


def test_RandomGenerator_pickle():
    """Pickle preserve the state of the RandomGenerator."""
    rng = ecole.RandomGenerator(42)
    assert rng == pickle.loads(pickle.dumps(rng))


def test_same_seed():
    """Same seed give same random generators."""
    ecole.seed(0)
    rng_1 = ecole.spawn_random_generator()
    ecole.seed(0)
    rng_2 = ecole.spawn_random_generator()
    assert rng_1 == rng_2


def test_differen_seed():
    """Different seeds give different random generators."""
    ecole.seed(0)
    rng_1 = ecole.spawn_random_generator()
    ecole.seed(2)
    rng_2 = ecole.spawn_random_generator()
    assert rng_1 != rng_2


def test_spawn_generator():
    """Successive random generators are different"""
    rng_1 = ecole.spawn_random_generator()
    rng_2 = ecole.spawn_random_generator()
    assert rng_1 != rng_2
