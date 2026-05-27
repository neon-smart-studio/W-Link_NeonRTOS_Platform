from os.path import join, dirname, abspath, isfile
from SCons.Script import DefaultEnvironment, AlwaysBuild

env = DefaultEnvironment()
board = env.BoardConfig()

build_script = env.subst("$BUILD_SCRIPT")
platform_dir = abspath(join(dirname(build_script), ".."))

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
    SIZE="arm-none-eabi-size"
)

env.AppendUnique(
    LINKFLAGS=[
        "--specs=nano.specs",
        "--specs=nosys.specs"
    ]
)

env.Append(
    CPPDEFINES=[
        ("F_CPU", board.get("build.f_cpu")),
    ]
)

env.MergeFlags(env.get("BUILD_FLAGS", []))

ldscript = board.get("build.ldscript")
if ldscript:
    env.Replace(
        LDSCRIPT_PATH=join(platform_dir, "ldscripts", ldscript)
    )

env.Replace(
    GDB="arm-none-eabi-gdb",
    PROGNAME="firmware",
    PROGSUFFIX=".elf"
)

env.BuildFrameworks(env.get("PIOFRAMEWORK"))

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

upload_target = env.Alias(
    "upload",
    target_elf,
    env.VerboseAction(upload_cmd, "Uploading " + elf_path)
)

env.Replace(
    PROG_PATH=elf_path
)

env.Alias("__debug", target_elf)
env.Default(target_elf)