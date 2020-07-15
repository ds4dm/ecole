import ecole.core


if __name__ == "__main__":
    ecole_version = ecole.core.get_build_version()
    print("Ecole version info:")
    print("    Version: {v.major}.{v.minor}.{v.patch}".format(v=ecole_version))
    print("    Git Revision: {v.git_revision}".format(v=ecole_version))

    scip_version = ecole.core.get_build_scip_version()
    print("Built against SCIP:")
    print("    Version: {v.major}.{v.minor}.{v.patch}".format(v=scip_version))
