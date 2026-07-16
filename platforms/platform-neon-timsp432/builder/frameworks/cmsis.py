"""
CMSIS for Nuvoton NUC4x2
"""

import os
from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()
platform = env.PioPlatform()
board = env.BoardConfig()

mcu = board.get("build.mcu", "")

platform_dir = platform.get_dir()

def get_cmsis_family(mcu):
    mcu = mcu.lower()

    if mcu.startswith("msp432p"):
        return "msp432p"

    if mcu.startswith("msp432e"):
        return "msp432e"

    return mcu

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

if not env.get("TIMSP432_CMSIS_BUILT"):
    env["TIMSP432_CMSIS_BUILT"] = True

    env.BuildSources(
        os.path.join("$BUILD_DIR", "cmsis_" + cmsis_family),
        CMSIS_SRC,
        src_filter=[
            "+<*.c>",
            "+<*.S>",
            "+<*.s>"
        ]
    )