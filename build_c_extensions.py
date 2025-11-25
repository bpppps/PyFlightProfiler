"""Build script."""

from distutils.errors import CCompilerError, DistutilsExecError, DistutilsPlatformError

from setuptools import Extension
from setuptools.command.build_ext import build_ext

extensions = [
    Extension(
        name="flight_profiler.ext.gilstat_C",
        include_dirs=["csrc"],
        sources=["csrc/symbol.cpp", "csrc/gilstat/gilstat.cpp"],
    ),
    Extension(
        name="flight_profiler.ext.stack_C",
        include_dirs=["csrc"],
        sources=["csrc/symbol.cpp", "csrc/stack/stack.cpp"],
    ),
    Extension(
        name="flight_profiler.ext.trace_profile_C",
        sources=["csrc/trace/trace_profile.c"],
    ),
]


class BuildFailed(Exception):
    pass


class ExtBuilder(build_ext):
    def run(self):
        try:
            build_ext.run(self)
        except (DistutilsPlatformError, FileNotFoundError):
            pass

    def build_extension(self, ext):
        try:
            build_ext.build_extension(self, ext)
        except (CCompilerError, DistutilsExecError, DistutilsPlatformError, ValueError):
            pass


def build(setup_kwargs):
    setup_kwargs.update(
        {"ext_modules": extensions, "cmdclass": {"build_ext": ExtBuilder}}
    )
