#!/usr/bin/python3

import module_db

import argparse
import os


def replace_if_different(output_path_str, new_content_path_str):
    import pathlib

    output_path = pathlib.Path(output_path_str)
    new_content_path = pathlib.Path(new_content_path_str)
    if not output_path.exists():
        new_content_path.replace(output_path)
        return
    if output_path.read_bytes() == new_content_path.read_bytes():
        new_content_path.unlink()
    else:
        new_content_path.replace(output_path)


def __make_modules_tests_header(module_db_file: dict, build_root: str, source_root: str, output: str):
    import glob

    mdb = module_db.load_db(module_db_file)
    ofilename = os.path.join(build_root, "modules", output)
    tmpfilename = ofilename + "~"
    with open(tmpfilename, "w") as f:
        for module in mdb.get_modules():
            if not module.build:
                continue
            path = os.path.join(os.path.join(source_root, module.path, "tests"))
            headers = glob.glob(os.path.join(path, "*.h"))
            for h in headers:
                f.write('#include "%s"\n' % (os.path.normpath(h)))
    replace_if_different(ofilename, tmpfilename)


def __make_modules_enabled_header(module_db_file: dict, build_root: str, output: str):
    mdb = module_db.load_db(module_db_file)
    modules_enabled: [str] = mdb.get_modules_enabled_names()

    ofilename = os.path.join(build_root, "modules", output)
    tmpfilename = ofilename + "~"
    with open(tmpfilename, "w") as f:
        f.write("#ifndef MODULE_GUARD_DEFINES\n")
        f.write("#define MODULE_GUARD_DEFINES\n\n")
        for module in modules_enabled:
            f.write("#define %s\n" % ("MODULE_" + module.upper() + "_ENABLED"))
        f.write("\n#endif\n")
    replace_if_different(ofilename, tmpfilename)


def __make_register_module_types_cpp(module_db_file: dict, build_root: str, output: str):
    includes_cpp = ""
    preregister_cpp = ""
    register_cpp = ""
    unregister_cpp = ""

    mdb = module_db.load_db(module_db_file)

    for module in mdb.get_modules():
        name = module.name
        path = module.path
        with open(os.path.join(build_root, path, "register_types.h")):
            includes_cpp += "#ifdef MODULE_" + name.upper() + "_ENABLED\n"
            includes_cpp += '#include "' + path + '/register_types.h"\n'
            includes_cpp += "#endif\n"
            preregister_cpp += "#ifdef MODULE_" + name.upper() + "_ENABLED\n"
            preregister_cpp += "#ifdef MODULE_" + name.upper() + "_HAS_PREREGISTER\n"
            preregister_cpp += "\tpreregister_" + name + "_types();\n"
            preregister_cpp += "#endif\n"
            preregister_cpp += "#endif\n"
            register_cpp += "#ifdef MODULE_" + name.upper() + "_ENABLED\n"
            register_cpp += "\tregister_" + name + "_types();\n"
            register_cpp += "#endif\n"
            unregister_cpp += "#ifdef MODULE_" + name.upper() + "_ENABLED\n"
            unregister_cpp += "\tunregister_" + name + "_types();\n"
            unregister_cpp += "#endif\n"

    modules_cpp = """// register_module_types.gen.cpp
/* THIS FILE IS GENERATED DO NOT EDIT */
#include "modules/register_module_types.h"

#include "modules/modules_enabled.gen.h"

%s

void preregister_module_types() {
%s
}

void register_module_types() {
%s
}

void unregister_module_types() {
%s
}
""" % (
        includes_cpp,
        preregister_cpp,
        register_cpp,
        unregister_cpp,
    )

    # NOTE: It is safe to generate this file here, since this is still executed serially
    tmpfilename = output + "~"
    with open(tmpfilename, "w") as f:
        f.write(modules_cpp)
    replace_if_different(output, tmpfilename)

    return


# TODO: I dont like this...
# We should register tests into the module db instead of globbing.
# def __make_modules_tests(module_db_file: str, output: str):

#     module_db_data = module_db.load_module_db(module_db_file)

#     with open(output, 'w') as f:
#         for module_data in module_db['modules'].values():
#             if

#         for name, path in env.module_list.items():
#             headers = glob.glob(os.path.join(path, "tests", "*.h"))
#             for h in headers:
#                 f.write('#include "%s"\n' % (os.path.normpath(h)))


if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="Module file generators")

    subparsers = parser.add_subparsers(help="sub-command help", dest="command")

    # module enabled
    module_enabled_parser = subparsers.add_parser("modules_enabled", help="Generate the modules_enabled file")
    module_enabled_parser.add_argument("module_db_file", type=str, help="The module db json file.")
    module_enabled_parser.add_argument("build_root", type=str, help="The project build root")
    module_enabled_parser.add_argument("output", type=str, help="The output header file.")
    module_enabled_parser.add_argument("-f", "--fake-stamp", default="")

    # module tests
    module_enabled_tests = subparsers.add_parser("modules_tests", help="Generate the modules_tests file")
    module_enabled_tests.add_argument("module_db_file", type=str, help="The module db json file.")
    module_enabled_tests.add_argument("build_root", type=str, help="The project build root")
    module_enabled_tests.add_argument("source_root", type=str, help="The project build root")
    module_enabled_tests.add_argument("output", type=str, help="The output header file.")
    module_enabled_tests.add_argument("-f", "--fake-stamp", default="")

    # register module type
    register_module_type_parser = subparsers.add_parser(
        "register_module_types", help="Generate the register_module_types file"
    )
    register_module_type_parser.add_argument("module_db_file", type=str, help="The module db json file.")
    register_module_type_parser.add_argument("build_root", type=str, help="The project source root")
    register_module_type_parser.add_argument("output", type=str, help="The output cpp file.")

    args = parser.parse_args()

    fake_stamp = None
    if args.command == "modules_enabled":
        __make_modules_enabled_header(args.module_db_file, args.build_root, args.output)
        fake_stamp = args.fake_stamp
    elif args.command == "register_module_types":
        __make_register_module_types_cpp(args.module_db_file, args.build_root, args.output)
    elif args.command == "modules_tests":
        __make_modules_tests_header(args.module_db_file, args.build_root, args.source_root, args.output)
        fake_stamp = args.fake_stamp
    else:
        sys.exit(255)

    # See scons_compat.py for explanation.
    if fake_stamp:
        with open(fake_stamp, "w") as fake_stamp:
            fake_stamp.write("#error This file should not be included, your include paths are wrong.\n")
