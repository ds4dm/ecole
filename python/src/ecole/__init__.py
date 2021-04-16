import sys


# Ask Python about our version (avoids templating this file)
if (sys.version_info.major == 3) and (sys.version_info.minor <= 7):
    import pkg_resources

    __version__ = pkg_resources.get_distribution(__name__).version
else:
    import importlib.metadata

    __version__ = importlib.metadata.version(__name__)


from ecole.core import RandomEngine, seed, spawn_random_engine, Exception

import ecole.data
import ecole.observation
import ecole.reward
import ecole.information
import ecole.scip
import ecole.instance
import ecole.dynamics
import ecole.environment
