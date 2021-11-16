import ecole.version


if __name__ == "__main__":
    ecole_lib_version_info = ecole.version.get_ecole_lib_version()
    scip_lib_version_info = ecole.version.get_scip_lib_version()
    scip_buildtime_version_info = ecole.version.get_scip_buildtime_version()
    print(
        (
            "Ecole version info:\n"
            "    Version       : {v.major}.{v.minor}.{v.patch}\n"
            "    Git Revision  : {v.revision}\n"
            "    Build Type    : {v.build_type}\n"
            "    Build OS      : {v.build_os}\n"
            "    Build Time    : {v.build_time}\n"
            "    Build Compiler: {v.build_compiler}\n"
            "    Path:         : {path}"
        ).format(v=ecole_lib_version_info, path=ecole.version.get_ecole_lib_path())
    )
    print(
        (
            "Built against SCIP:\n"
            "    Version       : {v.major}.{v.minor}.{v.patch}\n"
            "Running SCIP:\n"
            "    Version       : {v.major}.{v.minor}.{v.patch}\n"
            "    Path:         : {path}"
        ).format(v=scip_lib_version_info, path=ecole.version.get_scip_lib_path())
    )
