import ecole.version


if __name__ == "__main__":
    print("Ecole version info:")
    print(
        "    Version       : {v.major}.{v.minor}.{v.patch}".format(
            v=ecole.version.ecole_lib_version_info
        )
    )
    print("    Git Revision  : {v.revision}".format(v=ecole.version.ecole_lib_version_info))
    print("    Build Type    : {v.build_type}".format(v=ecole.version.ecole_lib_version_info))
    print("    Build OS      : {v.build_os}".format(v=ecole.version.ecole_lib_version_info))
    print("    Build Time    : {v.build_time}".format(v=ecole.version.ecole_lib_version_info))
    print("    Build Compiler: {v.build_compiler}".format(v=ecole.version.ecole_lib_version_info))

    print("Built against SCIP:")
    print(
        "    Version       : {v.major}.{v.minor}.{v.patch}".format(
            v=ecole.version.scip_buildtime_version_info
        )
    )

    print("Running SCIP:")
    print(
        "    Version       : {v.major}.{v.minor}.{v.patch}".format(
            v=ecole.version.scip_lib_version_info
        )
    )
