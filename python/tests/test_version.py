import ecole.core


def test_version():
    """Extract version of library and git revision."""
    version = ecole.core.get_build_version()
    assert isinstance(version.major, int)
    assert isinstance(version.minor, int)
    assert isinstance(version.patch, int)
    assert isinstance(version.git_revision, str)


def test_scip_version():
    """Extract version of SCIP library used to compile Ecole."""
    version = ecole.core.get_build_scip_version()
    assert isinstance(version.major, int)
    assert isinstance(version.minor, int)
    assert isinstance(version.patch, int)
