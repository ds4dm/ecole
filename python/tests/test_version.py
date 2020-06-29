import ecole.core


def test_version():
    """Extract version of library and git revision."""
    version = ecole.core.get_build_version()
    assert isinstance(version.major, int)
    assert isinstance(version.minor, int)
    assert isinstance(version.patch, int)
    assert isinstance(version.git_revision, str)
