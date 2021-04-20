import ecole.core

__version__ = "{v.minor}.{v.major}.{v.patch}".format(v=ecole.core.get_build_version())


if __name__ == "__main__":
    ecole_version = ecole.core.get_build_version()
    print("Ecole version info:")
    print("    Version       : {v.major}.{v.minor}.{v.patch}".format(v=ecole_version))
    print("    Git Revision  : {v.revision}".format(v=ecole_version))
    print("    Build Type    : {v.build_type}".format(v=ecole_version))
    print("    Build OS      : {v.build_os}".format(v=ecole_version))
    print("    Build Time    : {v.build_time}".format(v=ecole_version))
    print("    Build Compiler: {v.build_compiler}".format(v=ecole_version))

    scip_version = ecole.core.get_build_scip_version()
    print("Built against SCIP:")
    print("    Version       : {v.major}.{v.minor}.{v.patch}".format(v=scip_version))
