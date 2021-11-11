import copy
import pickle

import ecole


def test_RandomEngine():
    """Test bindings of the RandomEngine."""
    assert ecole.RandomEngine.min_seed < ecole.RandomEngine.max_seed
    random_engine = ecole.RandomEngine(42)
    rand_val_1 = random_engine()
    assert isinstance(rand_val_1, int)

    random_engine.seed(42)
    rand_val_2 = random_engine()
    assert rand_val_1 == rand_val_2


def test_RandomEngine_copy():
    """Copy create a new RandomEngine with same state."""
    random_engine = ecole.RandomEngine(42)

    random_engine_copy = copy.copy(random_engine)
    assert random_engine_copy == random_engine
    assert random_engine_copy is not random_engine

    random_engine_deepcopy = copy.deepcopy(random_engine)
    assert random_engine_deepcopy == random_engine
    assert random_engine_deepcopy is not random_engine


def test_RandomEngine_pickle():
    """Pickle preserve the state of the RandomEngine."""
    random_engine = ecole.RandomEngine(42)
    assert random_engine == pickle.loads(pickle.dumps(random_engine))


def test_same_seed():
    """Same seed give same random engines."""
    ecole.seed(0)
    random_engine_1 = ecole.spawn_random_engine()
    ecole.seed(0)
    random_engine_2 = ecole.spawn_random_engine()
    assert random_engine_1 == random_engine_2


def test_differen_seed():
    """Different seeds give different random engines."""
    ecole.seed(0)
    random_engine_1 = ecole.spawn_random_engine()
    ecole.seed(2)
    random_engine_2 = ecole.spawn_random_engine()
    assert random_engine_1 != random_engine_2


def test_spawn_engine():
    """Successive random engines are different"""
    random_engine_1 = ecole.spawn_random_engine()
    random_engine_2 = ecole.spawn_random_engine()
    assert random_engine_1 != random_engine_2
