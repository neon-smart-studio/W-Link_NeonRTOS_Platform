"""
    Builder for Texas Instruments
    SimpleLink CC3200 ARM Cortex-M4 Wi-Fi MCUs.
"""

import sys
from os.path import join

from SCons.Script import (ARGUMENTS, COMMAND_LINE_TARGETS, AlwaysBuild,
                          Builder, Default, DefaultEnvironment)

env = DefaultEnvironment()
platform = env.PioPlatform()
board = env.BoardConfig()

env.Replace(
    AR="arm-none-eabi-ar",
    AS="arm-none-eabi-as",
    CC="arm-none-eabi-gcc",
    CXX="arm-none-eabi-g++",
    GDB="arm-none-eabi-gdb",
    OBJCOPY="arm-none-eabi-objcopy",
    RANLIB="arm-none-eabi-ranlib",
    SIZETOOL="arm-none-eabi-size",

    ARFLAGS=["rc"],

    SIZEPROGREGEXP=r"^(?:\.text|\.data|\.rodata|\.text.align|\.ARM.exidx)\s+(\d+).*",
    SIZEDATAREGEXP=r"^(?:\.data|\.bss|\.noinit)\s+(\d+).*",
    SIZECHECKCMD="$SIZETOOL -A -d $SOURCES",
    SIZEPRINTCMD="$SIZETOOL -B -d $SOURCES",

    PROGSUFFIX=".elf"
)

if env.get("PROGNAME", "program") == "program":
    env.Replace(PROGNAME="firmware")

machine_flags = [
    "-mthumb"
]

env.Append(
    ASFLAGS=machine_flags,
    ASPPFLAGS=[
        "-x", "assembler-with-cpp"
    ],

    CCFLAGS=machine_flags + [
        "-Os",
        "-ffunction-sections",
        "-fdata-sections",
        "-Wall",
        "-nostdlib"
    ],

    CFLAGS=[
        "-std=gnu11"
    ],

    CXXFLAGS=[
        "-fno-rtti",
        "-fno-exceptions",
        "-fno-threadsafe-statics",
        "-std=gnu++11"
    ],

    CPPDEFINES=[
        ("F_CPU", "$BOARD_F_CPU"),
        "CC3200",
        "__CC3200__"
    ],

    LINKFLAGS=machine_flags + [
        "-Os",
        "-Wl,--gc-sections,--relax",
        "-Wl,--wrap=malloc",
        "-Wl,--wrap=free",
        "-Wl,--wrap=calloc",
        "-Wl,--wrap=realloc",
        "-Wl,--wrap=_malloc_r",
        "-Wl,--wrap=_free_r",
        "-Wl,--wrap=_calloc_r",
        "-Wl,--wrap=_realloc_r"
    ],

    LIBS=["c", "gcc", "m"],

    BUILDERS=dict(
        ElfToBin=Builder(
            action=env.VerboseAction(" ".join([
                "$OBJCOPY",
                "-O",
                "binary",
                "$SOURCES",
                "$TARGET"
            ]), "Building $TARGET"),
            suffix=".bin"
        ),
        ElfToHex=Builder(
            action=env.VerboseAction(" ".join([
                "$OBJCOPY",
                "-O",
                "ihex",
                "$SOURCES",
                "$TARGET"
            ]), "Building $TARGET"),
            suffix=".hex"
        )
    )
)

if "BOARD" in env:
    board_cfg = env.BoardConfig()

    cpu = board_cfg.get("build.cpu", "cortex-m4")
    fpu = board_cfg.get("build.fpu", None)
    float_abi = board_cfg.get("build.float-abi", None)

    asflags = ["-mcpu=%s" % cpu]
    ccflags = ["-mcpu=%s" % cpu]
    linkflags = ["-mcpu=%s" % cpu]

    if fpu and float_abi:
        asflags.extend([
            "-mfpu=%s" % fpu,
            "-mfloat-abi=%s" % float_abi
        ])

        ccflags.extend([
            "-mfpu=%s" % fpu,
            "-mfloat-abi=%s" % float_abi
        ])

        linkflags.extend([
            "-mfpu=%s" % fpu,
            "-mfloat-abi=%s" % float_abi
        ])

    env.Append(
        ASFLAGS=asflags,
        CCFLAGS=ccflags,
        LINKFLAGS=linkflags
    )

#
# Target: Build executable and linkable firmware
#

if "nobuild" in COMMAND_LINE_TARGETS:
    target_elf = join("$BUILD_DIR", "${PROGNAME}.elf")
    target_firm = join("$BUILD_DIR", "${PROGNAME}.bin")
else:
    target_elf = env.BuildProgram()
    target_firm = env.ElfToBin(join("$BUILD_DIR", "${PROGNAME}"), target_elf)
    env.Depends(target_firm, "checkprogsize")

AlwaysBuild(env.Alias("nobuild", target_firm))
target_buildprog = env.Alias("buildprog", target_firm, target_firm)

#
# Target: Print binary size
#

target_size = env.Alias(
    "size",
    target_elf,
    env.VerboseAction("$SIZEPRINTCMD", "Calculating size $SOURCE")
)
AlwaysBuild(target_size)

#
# Target: Upload firmware
#

openocd_args = [
    "-d%d" % (2 if int(ARGUMENTS.get("PIOVERBOSE", 0)) else 1)
]

openocd_args.extend(board.get("debug.tools.ti-icdi.server.arguments", []))

openocd_args.extend([
    "-c",
    "program {$SOURCE} %s verify reset; shutdown;" %
    board.get("upload.offset_address", "0x0")
])

openocd_args = [
    f.replace("$PACKAGE_DIR", platform.get_package_dir("tool-openocd") or "")
    for f in openocd_args
]

env.Replace(
    UPLOADER="openocd",
    UPLOADERFLAGS=openocd_args,
    UPLOADCMD="$UPLOADER $UPLOADERFLAGS"
)

target_upload = env.Alias(
    "upload",
    target_firm,
    env.VerboseAction("$UPLOADCMD", "Uploading $SOURCE")
)
AlwaysBuild(target_upload)

#
# Target: Default targets
#

Default([target_buildprog, target_size])