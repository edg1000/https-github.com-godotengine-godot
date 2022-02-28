#!/usr/bin/env python3

import fnmatch
import os
import re
import shutil
import subprocess
import sys


line_nb = False

for arg in sys.argv[1:]:
    if arg == "--with-line-nb":
        print("Enabling line numbers in the context locations.")
        line_nb = True
    else:
        os.sys.exit("Non supported argument '" + arg + "'. Aborting.")


if not os.path.exists("editor"):
    os.sys.exit("ERROR: This script should be started from the root of the git repo.")


matches = []
for root, dirnames, filenames in os.walk("."):
    dirnames[:] = [d for d in dirnames if d not in ["thirdparty"]]
    for filename in fnmatch.filter(filenames, "*.cpp"):
        matches.append(os.path.join(root, filename))
    for filename in fnmatch.filter(filenames, "*.h"):
        matches.append(os.path.join(root, filename))
matches.sort()


remaps = {}
remap_re = re.compile(r'capitalize_string_remaps\["(.+)"\] = "(.+)";')
with open("editor/editor_property_name_processor.cpp") as f:
    for line in f:
        m = remap_re.search(line)
        if m:
            remaps[m.group(1)] = m.group(2)


unique_str = []
unique_loc = {}
main_po = """
# LANGUAGE translation of the Godot Engine editor.
# Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.
# Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).
# This file is distributed under the same license as the Godot source code.
#
# FIRST AUTHOR <EMAIL@ADDRESS>, YEAR.
#
#, fuzzy
msgid ""
msgstr ""
"Project-Id-Version: Godot Engine editor\\n"
"Report-Msgid-Bugs-To: https://github.com/godotengine/godot\\n"
"MIME-Version: 1.0\\n"
"Content-Type: text/plain; charset=UTF-8\\n"
"Content-Transfer-Encoding: 8-bit\\n"\n
"""


message_patterns = {
    re.compile(r'RTR\("(([^"\\]|\\.)*)"\)'): False,
    re.compile(r'TTR\("(([^"\\]|\\.)*)"\)'): False,
    re.compile(r'TTRC\("(([^"\\]|\\.)*)"\)'): False,
    re.compile(r'_initial_set\("([^"]+?)",'): True,
    re.compile(r'GLOBAL_DEF(?:_RST)?\("([^".]+?)",'): True,
    re.compile(r'EDITOR_DEF(?:_RST)?\("([^"]+?)",'): True,
    re.compile(r'ADD_PROPERTY\(PropertyInfo\(Variant::[A-Z]+,\s*"([^"]+?)",'): True,
    re.compile(r'ADD_GROUP\("([^"]+?)",'): False,
}


# See String::camelcase_to_underscore().
capitalize_re = re.compile(r"(?<=\D)(?=\d)|(?<=\d)(?=\D([a-z]|\d))")


def _process_editor_string(name):
    # See String::capitalize().
    # fmt: off
    capitalized = " ".join(
        part.title()
        for part in capitalize_re.sub("_", name).replace("_", " ").split()
    )
    # fmt: on
    # See EditorStringProcessor::process_string().
    for key, value in remaps.items():
        capitalized = capitalized.replace(key, value)
    return capitalized


def _write_translator_comment(msg, translator_comment):
    if translator_comment == "":
        return

    global main_po
    msg_pos = main_po.find('\nmsgid "' + msg + '"')

    # If it's a new message, just append comment to the end of PO file.
    if msg_pos == -1:
        main_po += _format_translator_comment(translator_comment, True)
        return

    # Find position just before location. Translator comment will be added there.
    translator_comment_pos = main_po.rfind("\n\n#", 0, msg_pos) + 2
    if translator_comment_pos - 2 == -1:
        print("translator_comment_pos not found")
        return

    # Check if a previous translator comment already exists. If so, merge them together.
    if main_po.find("TRANSLATORS:", translator_comment_pos, msg_pos) != -1:
        translator_comment_pos = main_po.find("\n#:", translator_comment_pos, msg_pos) + 1
        if translator_comment_pos == 0:
            print('translator_comment_pos after "TRANSLATORS:" not found')
            return
        main_po = (
            main_po[:translator_comment_pos]
            + _format_translator_comment(translator_comment, False)
            + main_po[translator_comment_pos:]
        )
        return

    main_po = (
        main_po[:translator_comment_pos]
        + _format_translator_comment(translator_comment, True)
        + main_po[translator_comment_pos:]
    )


def _format_translator_comment(comment, new):
    if not comment:
        return ""

    comment_lines = comment.split("\n")

    formatted_comment = ""
    if not new:
        for comment in comment_lines:
            formatted_comment += "#. " + comment.strip() + "\n"
        return formatted_comment

    formatted_comment = "#. TRANSLATORS: "
    for i in range(len(comment_lines)):
        if i == 0:
            formatted_comment += comment_lines[i].strip() + "\n"
        else:
            formatted_comment += "#. " + comment_lines[i].strip() + "\n"
    return formatted_comment


def _is_block_translator_comment(translator_line):
    line = translator_line.strip()
    if line.find("//") == 0:
        return False
    else:
        return True


def _extract_translator_comment(line, is_block_translator_comment):
    line = line.strip()
    reached_end = False
    extracted_comment = ""

    start = line.find("TRANSLATORS:")
    if start == -1:
        start = 0
    else:
        start += len("TRANSLATORS:")

    if is_block_translator_comment:
        # If '*/' is found, then it's the end.
        if line.rfind("*/") != -1:
            extracted_comment = line[start : line.rfind("*/")]
            reached_end = True
        else:
            extracted_comment = line[start:]
    else:
        # If beginning is not '//', then it's the end.
        if line.find("//") != 0:
            reached_end = True
        else:
            start = 2 if start == 0 else start
            extracted_comment = line[start:]

    return (not reached_end, extracted_comment)


def process_file(f, fname):
    l = f.readline()
    lc = 1
    reading_translator_comment = False
    is_block_translator_comment = False
    translator_comment = ""

    while l:

        # Detect translator comments.
        if not reading_translator_comment and l.find("TRANSLATORS:") != -1:
            reading_translator_comment = True
            is_block_translator_comment = _is_block_translator_comment(l)
            translator_comment = ""

        # Gather translator comments. It will be gathered for the next translation function.
        if reading_translator_comment:
            reading_translator_comment, extracted_comment = _extract_translator_comment(l, is_block_translator_comment)
            if extracted_comment != "":
                translator_comment += extracted_comment + "\n"
            if not reading_translator_comment:
                translator_comment = translator_comment[:-1]  # Remove extra \n at the end.

        if not reading_translator_comment:
            for pattern, is_property_path in message_patterns.items():
                for m in pattern.finditer(l):
                    location = os.path.relpath(fname).replace("\\", "/")
                    if line_nb:
                        location += ":" + str(lc)

                    msg = m.group(1)

                    if is_property_path:
                        for part in msg.split("/"):
                            _add_message(_process_editor_string(part), location, translator_comment)
                    else:
                        _add_message(msg, location, translator_comment)

            translator_comment = ""

        l = f.readline()
        lc += 1


def _add_message(msg, location, translator_comment):
    global main_po, unique_str, unique_loc

    # Write translator comment.
    _write_translator_comment(msg, translator_comment)

    if not msg in unique_str:
        main_po += "#: " + location + "\n"
        main_po += 'msgid "' + msg + '"\n'
        main_po += 'msgstr ""\n\n'
        unique_str.append(msg)
        unique_loc[msg] = [location]
    elif not location in unique_loc[msg]:
        # Add additional location to previous occurrence too
        msg_pos = main_po.find('\nmsgid "' + msg + '"')
        if msg_pos == -1:
            print("Someone apparently thought writing Python was as easy as GDScript. Ping Akien.")
        main_po = main_po[:msg_pos] + " " + location + main_po[msg_pos:]
        unique_loc[msg].append(location)


print("Updating the editor.pot template...")

for fname in matches:
    with open(fname, "r", encoding="utf8") as f:
        process_file(f, fname)

with open("editor.pot", "w") as f:
    f.write(main_po)

if os.name == "posix":
    print("Wrapping template at 79 characters for compatibility with Weblate.")
    os.system("msgmerge -w79 editor.pot editor.pot > editor.pot.wrap")
    shutil.move("editor.pot.wrap", "editor.pot")

shutil.move("editor.pot", "editor/translations/editor.pot")

# TODO: Make that in a portable way, if we care; if not, kudos to Unix users
if os.name == "posix":
    added = subprocess.check_output(r"git diff editor/translations/editor.pot | grep \+msgid | wc -l", shell=True)
    removed = subprocess.check_output(r"git diff editor/translations/editor.pot | grep \\\-msgid | wc -l", shell=True)
    print("\n# Template changes compared to the staged status:")
    print("#   Additions: %s msgids.\n#   Deletions: %s msgids." % (int(added), int(removed)))
