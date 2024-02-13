"""Functions used to generate source files during build time

All such functions are invoked in a subprocess on Windows to prevent build flakiness.

"""
import os
from detect import get_mingw_tool
from platform_methods import subprocess_main


def make_debug_mingw(target, source, env):
    objcopy = get_mingw_tool("objcopy", env["mingw_prefix"], env["arch"])
    strip = get_mingw_tool("strip", env["mingw_prefix"], env["arch"])

    if not objcopy or not strip:
        print('`separate_debug_symbols` requires both "objcopy" and "strip" to function.')
        return

    os.system("{0} --only-keep-debug {1} {1}.debugsymbols".format(objcopy, target[0]))
    os.system("{0} --strip-debug --strip-unneeded {1}".format(strip, target[0]))
    os.system("{0} --add-gnu-debuglink={1}.debugsymbols {1}".format(objcopy, target[0]))


if __name__ == "__main__":
    subprocess_main(globals())
