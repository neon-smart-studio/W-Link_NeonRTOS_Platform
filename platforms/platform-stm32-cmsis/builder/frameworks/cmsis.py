# Copyright 2014-present PlatformIO <contact@platformio.org>
#
# Licensed under the Apache License, Version 2.0

"""
CMSIS
"""

import glob
import os
import string

from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()
platform = env.PioPlatform()
board = env.BoardConfig()

mcu = board.get("build.mcu", "")
product_line = board.get("build.product_line", "")
assert product_line, "Missing MCU or Product Line field"


def get_cmsis_family(mcu):
    """
    PlatformIO old logic uses mcu[0:7].
    That breaks newer split families:
      stm32wba55cg -> stm32wb   wrong
      stm32h7r/s   -> stm32h7   wrong for H7RS package
    """
    mcu = mcu.lower()

    if mcu.startswith("stm32wba"):
        return "stm32wba"

    if mcu.startswith("stm32h7r") or mcu.startswith("stm32h7s"):
        return "stm32h7rs"

    return mcu[0:7]


def get_ld_family(mcu, cmsis_family):
    """
    Linker script package may or may not have separate folders.
    Keep default behavior unless special family is known.
    """
    return cmsis_family


cmsis_family = get_cmsis_family(mcu)
ld_family = get_ld_family(mcu, cmsis_family)

env.SConscript("_bare.py")

CMSIS_DIR = platform.get_package_dir("framework-cmsis")
CMSIS_DEVICE_DIR = platform.get_package_dir("framework-cmsis-" + cmsis_family)
LDSCRIPTS_DIR = platform.get_package_dir("tool-ldscripts-ststm32")

assert all(os.path.isdir(d) for d in (CMSIS_DIR, CMSIS_DEVICE_DIR, LDSCRIPTS_DIR))


def generate_ldscript(default_ldscript_path):
    ram = board.get("upload.maximum_ram_size", 0)
    flash = board.get("upload.maximum_size", 0)

    template_file = os.path.join(LDSCRIPTS_DIR, "tpl", "linker.tpl")
    content = ""

    with open(template_file) as fp:
        data = string.Template(fp.read())
        content = data.substitute(
            stack=hex(0x20000000 + ram),
            ram=str(int(ram / 1024)) + "K",
            flash=str(int(flash / 1024)) + "K"
        )

    default_dir = os.path.dirname(default_ldscript_path)
    if not os.path.isdir(default_dir):
        os.makedirs(default_dir)

    with open(default_ldscript_path, "w") as fp:
        fp.write(content)


def get_linker_script():
    ldscript_match = glob.glob(os.path.join(
        LDSCRIPTS_DIR,
        ld_family,
        mcu[0:11].upper() + "*_FLASH.ld"
    ))

    if ldscript_match and os.path.isfile(ldscript_match[0]):
        return ldscript_match[0]

    default_ldscript = os.path.join(
        LDSCRIPTS_DIR,
        ld_family,
        mcu[0:11].upper() + "_DEFAULT.ld"
    )

    print(
        "Warning! Cannot find a linker script for the required board! "
        "An auto-generated script will be used to link firmware!"
    )

    if not os.path.isfile(default_ldscript):
        generate_ldscript(default_ldscript)

    return default_ldscript


def prepare_startup_file(src_path):
    startup_file = os.path.join(
        src_path,
        "gcc",
        "startup_%s.S" % product_line.lower()
    )

    if not os.path.isfile(startup_file) and os.path.isfile(startup_file[:-2] + ".s"):
        try:
            os.replace(startup_file[:-2] + ".s", startup_file)
        except (FileNotFoundError, FileExistsError):
            print("Startup file was already renamed by another process.")

    if not os.path.isfile(startup_file):
        print(
            "Warning! Cannot find the default startup file for %s. "
            "Ignore this warning if the startup code is part of your project." % mcu
        )


if not board.get("build.ldscript", ""):
    env.Replace(LDSCRIPT_PATH=get_linker_script())


env.Append(
    CPPPATH=[
        os.path.join(CMSIS_DIR, "CMSIS", "Include"),
        os.path.join(CMSIS_DEVICE_DIR, "Include")
    ],

    LINKFLAGS=[
        "--specs=nano.specs",
        "--specs=nosys.specs"
    ]
)


sources_path = os.path.join(CMSIS_DEVICE_DIR, "Source", "Templates")
prepare_startup_file(sources_path)

env.BuildSources(
    os.path.join("$BUILD_DIR", "FrameworkCMSIS"),
    sources_path,
    src_filter=[
        "-<*>",
        "+<%s>" % board.get(
            "build.cmsis.system_file",
            "system_%sxx.c" % cmsis_family
        ),
        "+<gcc/%s>" % board.get(
            "build.cmsis.startup_file",
            "startup_%s.S" % product_line.lower()
        )
    ]
)