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

def normalize_mcu_name(value):
    """
    Normalize MCU name.

    Examples:
        MSPM0G3507SPTR  -> mspm0g3507sptr
        mspm0l1306      -> mspm0l1306
        MSPM0G3507      -> mspm0g3507
    """
    return re.sub(r"[^a-zA-Z0-9]", "", value).lower()


def get_cmsis_family(value):
    """
    Convert an MSPM0 device name to its CMSIS package family.

    Examples:
        MSPM0G3507 -> mspm0g350x
        MSPM0G3506 -> mspm0g350x
        MSPM0G3107 -> mspm0g310x
        MSPM0L1306 -> mspm0l130x
        MSPM0L1105 -> mspm0l110x
        MSPM0C1104 -> mspm0c110x
        MSPM0H3216 -> mspm0h321x

    Board JSON may explicitly override this through:

        "build": {
            "cmsis_family": "mspm0g350x"
        }
    """

    normalized = normalize_mcu_name(value)

    if not normalized:
        raise RuntimeError(
            "Board manifest does not define build.mcu"
        )

    if not normalized.startswith("mspm0"):
        raise RuntimeError(
            "Unsupported MCU '%s': expected an MSPM0 device"
            % value
        )

    match = re.match(
        r"^(mspm0[a-z]+\d{3})\d",
        normalized
    )

    if match:
        return match.group(1) + "x"

    match = re.match(
        r"^(mspm0[a-z]+\d+x)",
        normalized
    )

    if match:
        return match.group(1)

    raise RuntimeError(
        "Unable to determine CMSIS family from MCU '%s'. "
        "Set build.cmsis_family explicitly in the board JSON."
        % value
    )


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