"""
CMSIS for Nuvoton NUC4x2
"""

import os
import re
from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()
platform = env.PioPlatform()
board = env.BoardConfig()

mcu = board.get("build.mcu", "")

platform_dir = platform.get_dir()

def get_cmsis_family(value):
    return "mspm0"

cmsis_family = get_cmsis_family(mcu)

CMSIS_ROOT = os.path.join(platform_dir, "packages", "cmsis_" + cmsis_family)
CMSIS_INC  = os.path.join(CMSIS_ROOT, "Include")
CMSIS_DEVICE_INC  = os.path.join(CMSIS_ROOT, "Device", "Include")
CMSIS_SRC  = os.path.join(CMSIS_ROOT, "Device", "Source")

env.Append(
    CPPPATH=[
        CMSIS_INC,
        CMSIS_DEVICE_INC,
    ],

    CCFLAGS=[
        "-I" + CMSIS_INC,
        "-I" + CMSIS_DEVICE_INC,
    ],

    CXXFLAGS=[
        "-I" + CMSIS_INC,
        "-I" + CMSIS_DEVICE_INC,
    ],

    ASFLAGS=[
        "-I" + CMSIS_INC,
        "-I" + CMSIS_DEVICE_INC,
    ]
)

if not env.get("MSPM0_CMSIS_BUILT"):
    env["MSPM0_CMSIS_BUILT"] = True

    env.BuildSources(
        os.path.join("$BUILD_DIR", "cmsis_" + cmsis_family),
        CMSIS_SRC,
        src_filter=[
            "+<*.c>",
            "+<*.S>",
            "+<*.s>"
        ]
    )