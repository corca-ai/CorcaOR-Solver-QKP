import setuptools
from setuptools import find_packages
import cmake_build_extension

from pathlib import Path

PACKAGE_NAME = "CorcaOR_Solver_QKP"

setuptools.setup(
    ext_modules=[cmake_build_extension.CMakeExtension(
            name=PACKAGE_NAME,
            # Name of the resulting package name (import mymath_pybind11)
            install_prefix=PACKAGE_NAME,
            # Note: pybind11 is a build-system requirement specified in pyproject.toml,
            #       therefore pypa/pip or pypa/build will install it in the virtual
            #       environment created in /tmp during packaging.
            #       This cmake_depends_on option adds the pybind11 installation path
            #       to CMAKE_PREFIX_PATH so that the example finds the pybind11 targets
            #       even if it is not installed in the system.
            # Exposes the binary print_answer to the environment.
            # It requires also adding a new entry point in setup.cfg.
            source_dir=str(Path(__file__).parent.absolute()),
            cmake_configure_options=[
                "-DCALL_FROM_SETUP_PY:BOOL=ON",
            ]
        ),],
    cmdclass=dict(
        # Enable the CMakeExtension entries defined above
        build_ext=cmake_build_extension.BuildExtension,
        # If the setup.py or setup.cfg are in a subfolder wrt the main CMakeLists.txt,
        # you can use the following custom command to create the source distribution.
        # sdist=cmake_build_extension.GitSdistFolder
    )
)