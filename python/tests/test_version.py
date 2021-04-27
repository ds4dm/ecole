import pytest

import ecole.core


def test_version():
    """Extract version of library and git revision."""
    version = ecole.core.get_build_version()
    assert isinstance(version.major, int)
    assert isinstance(version.minor, int)
    assert isinstance(version.patch, int)
    assert isinstance(version.revision, str)
    assert isinstance(version.build_type, str)
    assert isinstance(version.build_os, str)
    assert isinstance(version.build_time, str)
    assert isinstance(version.build_compiler, str)


def test_scip_version():
    """Extract version of SCIP library used to compile Ecole."""
    version = ecole.core.get_build_scip_version()
    assert isinstance(version.major, int)
    assert isinstance(version.minor, int)
    assert isinstance(version.patch, int)


def test_match_importlib():
    """Package version match inner version."""
    try:
        import importlib.metadata

        try:
            version = importlib.metadata.version(__name__)
            assert version == ecole.__version__

        # Without packaging Ecole, we cannot use importlib to get the version
        except importlib.metadata.PackageNotFoundError:
            pytest.skip()

    # In Python <= 3.8 we cannot use importlib to get the version
    except ModuleNotFoundError:
        pytest.skip()
