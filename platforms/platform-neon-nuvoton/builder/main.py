from os.path import join, dirname, abspath, isfile, isdir
from SCons.Script import (ARGUMENTS, COMMAND_LINE_TARGETS, AlwaysBuild, Builder, Default, DefaultEnvironment)

env = DefaultEnvironment()
board = env.BoardConfig()

build_script = env.subst("$BUILD_SCRIPT")
platform_dir = abspath(join(dirname(build_script), ".."))

project_dir = env.subst("$PROJECT_DIR")
project_include_dir = env.subst("$PROJECT_INCLUDE_DIR")
project_src_dir = env.subst("$PROJECT_SRC_DIR")

openocd_dir = env.PioPlatform().get_package_dir("tool-openocd-nuvoton")

openocd_exe = join(openocd_dir, "bin", "openocd.exe")
scripts_dir = join(openocd_dir, "scripts")

if not isfile(openocd_exe):
    raise Exception("openocd.exe not found: " + openocd_exe)

openocd_exe_u = openocd_exe.replace("\\", "/")
scripts_dir_u = scripts_dir.replace("\\", "/")

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

    PIODEBUGFLAGS=["-O0", "-g3", "-ggdb", "-gdwarf-2"],

    SIZEPROGREGEXP=r"^(?:\.text|\.data|\.rodata|\.text.align|\.ARM.exidx)\s+(\d+).*",
    SIZEDATAREGEXP=r"^(?:\.data|\.bss|\.noinit)\s+(\d+).*",
    SIZECHECKCMD="$SIZETOOL -A -d $SOURCES",
    SIZEPRINTCMD='$SIZETOOL -B -d $SOURCES',

    PROGSUFFIX=".elf"
)

env.Append(
    ASFLAGS=["-x", "assembler-with-cpp"],

    CCFLAGS=[
        "-Os",
        "-ffunction-sections",
        "-fdata-sections",
        "-mthumb",
        "-mabi=aapcs",
        "-march=armv7e-m",
        "-MMD",

        "-Wno-implicit-function-declaration",
        "-Wno-error=implicit-function-declaration",

        "-Wno-int-conversion",
        "-Wno-error=int-conversion",

        "-Wno-incompatible-pointer-types",
        "-Wno-error=incompatible-pointer-types",
    ],

    CXXFLAGS=[
        "-fno-exceptions",
        "-fno-threadsafe-statics",
        "-fno-rtti",
    ],

    CPPDEFINES=[
        ("F_CPU", "$BOARD_F_CPU"),
        "gcc",
    ],

    LINKFLAGS=[
        "-Os",
        "-ffunction-sections",
        "-fdata-sections",
        "-Wl,--gc-sections",
        "-Wl,--print-memory-usage",
        "-mthumb"
    ],

    LIBS=["m", "stdc++", "gcc", "nosys", "c"],

    BUILDERS=dict(
        ElfToHex=Builder(
            action=env.VerboseAction(" ".join([
                "$OBJCOPY",
                "-O",
                "ihex",
                "-R",
                ".eeprom",
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

    asflags = [
        "-mcpu=%s" % cpu
    ]

    ccflags = [
        "-mcpu=%s" % cpu
    ]

    linkflags = [
        "-mcpu=%s" % cpu
    ]

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

# Allow user to override via pre:script
if env.get("PROGNAME", "program") == "program":
    env.Replace(PROGNAME="firmware")

target_elf = env.BuildProgram()
elf_path = str(target_elf[0]).replace("\\", "/")

upload_cmd = (
    '"' + openocd_exe_u + '" '
    '-s "' + scripts_dir_u + '" '
    '-f interface/nulink.cfg '
    '-f target/numicroM4.cfg '
    '-c "gdb_port disabled" '
    '-c "tcl_port disabled" '
    '-c "telnet_port disabled" '
    '-c "program ' + elf_path + ' verify reset exit"'
)

env.Alias(
    "upload",
    target_elf,
    env.VerboseAction(upload_cmd, "Uploading " + elf_path)
)

env.Replace(
    PROG_PATH=elf_path
)

env.Alias("__debug", target_elf)
env.Default(target_elf)