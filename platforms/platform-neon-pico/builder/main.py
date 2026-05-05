import os
import glob
import fnmatch
import subprocess
from SCons.Script import DefaultEnvironment, AlwaysBuild, Alias
import os
import sys
import urllib.request
import zipfile
import tarfile
import shutil

env = DefaultEnvironment()

board = env.BoardConfig()

mcu = str(board.get("build.mcu", "")).lower()
extra_flags = str(board.get("build.extra_flags", ""))

IS_RP2040 = (mcu == "rp2040") or ("RP2040" in extra_flags) or ("PICO_RP2040" in extra_flags)
IS_RP2350 = (mcu == "rp2350") or ("RP2350" in extra_flags) or ("PICO_RP2350" in extra_flags)

if IS_RP2350:
    cpu_flag = "cortex-m33"
elif IS_RP2040:
    cpu_flag = "cortex-m0plus"
else:
    raise RuntimeError(f"No RP chip defined, mcu={mcu}, extra_flags={extra_flags}")

PROJECT_DIR = env.subst("$PROJECT_DIR")
BUILD_DIR = env.subst("$BUILD_DIR")
PLATFORM_DIR = os.path.join(PROJECT_DIR, "platforms", "platform-neon-pico")

tools_dir = os.path.join(PROJECT_DIR, ".pio", "pico-tools")
os.makedirs(tools_dir, exist_ok=True)

# 判斷平台
if sys.platform.startswith("win"):
    url = "https://github.com/raspberrypi/pico-sdk-tools/releases/download/v2.2.0-3/picotool-2.2.0-a4-x64-win.zip"
    archive = os.path.join(tools_dir, "picotool.zip")
    picotool_exe = os.path.join(tools_dir, "picotool", "picotool.exe")

elif sys.platform.startswith("linux"):
    url = "https://github.com/raspberrypi/pico-sdk-tools/releases/download/v2.2.0-3/picotool-2.2.0-a4-x86_64-lin.tar.gz"
    archive = os.path.join(tools_dir, "picotool.tar.gz")
    picotool_exe = os.path.join(tools_dir, "picotool", "picotool")

elif sys.platform == "darwin":
    url = "https://github.com/raspberrypi/pico-sdk-tools/releases/download/v2.2.0-3/picotool-2.2.0-a4-mac.zip"
    archive = os.path.join(tools_dir, "picotool.zip")
    picotool_exe = os.path.join(tools_dir, "picotool", "picotool")

else:
    raise RuntimeError("Unsupported OS")

# 如果還沒下載
if not os.path.exists(picotool_exe):
    print(">>> Downloading picotool ...")
    urllib.request.urlretrieve(url, archive)

    print(">>> Extracting ...")

    if archive.endswith(".zip"):
        with zipfile.ZipFile(archive, "r") as z:
            z.extractall(tools_dir)

    elif archive.endswith(".tar.gz"):
        with tarfile.open(archive, "r:gz") as t:
            t.extractall(tools_dir)

    # Linux / mac 設執行權限
    if not sys.platform.startswith("win"):
        os.chmod(picotool_exe, 0o755)

# 給 builder 用
picotool = picotool_exe.replace("\\", "/")

toolchain = env.PioPlatform().get_package_dir("toolchain-gccarmnoneeabi")
toolbin = os.path.join(toolchain, "bin")

sdk_dir = os.path.join(PROJECT_DIR, ".pio", "framework-pico-sdk")

if not os.path.exists(sdk_dir):
    print(">>> Cloning Raspberry Pi Pico SDK ...")
    subprocess.check_call([
        "git", "clone",
        "https://github.com/raspberrypi/pico-sdk.git",
        sdk_dir
    ])
    subprocess.check_call([
        "git", "-C", sdk_dir,
        "submodule", "update", "--init", "--recursive"
    ])
    
version_h_src_path = os.path.join(PLATFORM_DIR, "builder", "files", "version.h")
version_h_dst_path = os.path.join(sdk_dir, "src", "common", "pico_base_headers", "include", "pico", "version.h")
shutil.copy(version_h_src_path, version_h_dst_path)

config_autogen_h_src_path = os.path.join(PLATFORM_DIR, "builder", "files", "config_autogen.h")
config_autogen_h_dst_path = os.path.join(sdk_dir, "src", "common", "pico_base_headers", "include", "pico", "config_autogen.h")
shutil.copy(config_autogen_h_src_path, config_autogen_h_dst_path)

env.Replace(
    CC=os.path.join(toolbin, "arm-none-eabi-gcc"),
    CXX=os.path.join(toolbin, "arm-none-eabi-g++"),
    AS=os.path.join(toolbin, "arm-none-eabi-as"),
    AR=os.path.join(toolbin, "arm-none-eabi-ar"),
    RANLIB=os.path.join(toolbin, "arm-none-eabi-ranlib"),
    OBJCOPY=os.path.join(toolbin, "arm-none-eabi-objcopy"),
    OBJDUMP=os.path.join(toolbin, "arm-none-eabi-objdump"),
    NM=os.path.join(toolbin, "arm-none-eabi-nm"),
    SIZE=os.path.join(toolbin, "arm-none-eabi-size")
)

env.PrependENVPath("PATH", toolbin)

def collect_sources(root, exts=(".c", ".cpp", ".S", ".s")):
    files = []
    if not os.path.exists(root):
        return files

    for ext in exts:
        files.extend(glob.glob(os.path.join(root, "**", "*" + ext), recursive=True))

    return files


def apply_ini_build_flags(envx):
    raw = envx.GetProjectOption("build_flags") or []

    if isinstance(raw, str):
        tokens = raw.split()
    elif isinstance(raw, list):
        tokens = []
        for item in raw:
            tokens += str(item).split()
    else:
        tokens = str(raw).split()

    flags = envx.ParseFlags(tokens)

    envx.AppendUnique(
        CPPDEFINES=flags.get("CPPDEFINES", []),
        CCFLAGS=flags.get("CCFLAGS", []),
        CFLAGS=flags.get("CFLAGS", []),
        CXXFLAGS=flags.get("CXXFLAGS", []),
        ASFLAGS=flags.get("ASFLAGS", []),
        LINKFLAGS=flags.get("LINKFLAGS", []),
        LIBPATH=flags.get("LIBPATH", []),
        LIBS=flags.get("LIBS", [])
    )

    includes = []
    for token in tokens:
        if token.startswith("-I"):
            path = token[2:]
            if not os.path.isabs(path):
                path = os.path.join(PROJECT_DIR, path)
            includes.append(path)

    envx.AppendUnique(CPPPATH=includes)


def parse_src_filter(envx):
    raw = envx.GetProjectOption("build_src_filter") or []

    if isinstance(raw, str):
        lines = raw.splitlines()
    else:
        lines = raw

    include_patterns = []
    exclude_patterns = []

    for line in lines:
        line = str(line).strip()
        if not line:
            continue

        if line.startswith("+<") and line.endswith(">"):
            include_patterns.append(line[2:-1])
        elif line.startswith("-<") and line.endswith(">"):
            exclude_patterns.append(line[2:-1])

    if not include_patterns:
        include_patterns = ["*"]

    def normalize(p):
        return p.replace("\\", "/").replace("**/", "").replace("**", "*")

    include_patterns = [normalize(p) for p in include_patterns]
    exclude_patterns = [normalize(p) for p in exclude_patterns]

    return include_patterns, exclude_patterns


def match_any(path, patterns):
    return any(fnmatch.fnmatch(path, p) for p in patterns)


apply_ini_build_flags(env)

board = env.BoardConfig()
f_cpu = board.get("build.f_cpu", "125000000L")

if IS_RP2040:
    pico_board = "pico"
if IS_RP2350:
    pico_board = "pico2"

chip_defines = []

if IS_RP2040:
    chip_defines += [
        "RP2040",
        "PICO_RP2040",
        ("PICO_CHIP", 2040),
    ]

elif IS_RP2350:
    chip_defines += [
        "RP2350",
        "PICO_RP2350",
        ("PICO_CHIP", 2350),
    ]

cpu_flags = [
    f"-mcpu={cpu_flag}",
    "-mthumb",
]

if IS_RP2350:
    cpu_flags += [
        "-mfpu=fpv5-sp-d16",
        "-mfloat-abi=hard",
    ]

env.AppendUnique(
    CPPDEFINES=chip_defines+[
        ("F_CPU", f_cpu),
        ("PICO_BOARD", f"\\\"{pico_board}\\\""),
        ("PICO_TARGET_NAME", "\\\"firmware\\\""),
        ("PICO_CMAKE_BUILD_TYPE", "\\\"Release\\\""),

        ("PICO_ON_DEVICE", 1),
        ("PICO_NO_HARDWARE", 0),
        ("PICO_BUILD", 1),
        ("PICO_USE_BLOCKED_RAM", 0),
    ],
    CCFLAGS=cpu_flags+[
        "-Os",
        "-ffunction-sections",
        "-fdata-sections",
        "-fno-common",
        "-Wall",
        "-Wextra",
        "-Wno-unused-parameter"
    ],
    CXXFLAGS=[
        "-fno-rtti",
        "-fno-exceptions"
    ],
    ASFLAGS=cpu_flags,
    LINKFLAGS=cpu_flags+[
        "-nostartfiles",
        "-Wl,--gc-sections",
        "--specs=nosys.specs",
        "--specs=nano.specs"
    ]
)

if IS_RP2040:
    boot2_src_rp2040 = os.path.join(sdk_dir, "src/rp2040/boot_stage2/boot2_w25q080.S")

    boot2_inc_rp2040 = [
        os.path.join(sdk_dir, "src/common/pico_base_headers/include"),
        os.path.join(sdk_dir, "src/rp2_common/cmsis/include"),
        os.path.join(sdk_dir, "src/rp2_common/pico_platform_compiler/include"),
        os.path.join(sdk_dir, "src/rp2_common/pico_platform_sections/include"),
        os.path.join(sdk_dir, "src/rp2_common/pico_platform_panic/include"),
        os.path.join(sdk_dir, "src/rp2_common/pico_platform_common/include"),
        os.path.join(sdk_dir, "src/rp2040/boot_stage2/asminclude"),
        os.path.join(sdk_dir, "src/rp2040/pico_platform/include"),
        os.path.join(sdk_dir, "src/rp2040/hardware_regs/include"),
    ]

    inc_flags = " ".join(["-I" + p.replace("\\", "/") for p in boot2_inc_rp2040])

    boot2_bin = os.path.join(BUILD_DIR, "boot2.bin")
    boot2_obj = os.path.join(BUILD_DIR, "boot2.o")
    boot2_elf = os.path.join(BUILD_DIR, "boot2_elf.o")

    patch_boot2_py = os.path.join(PLATFORM_DIR, "builder", "patch_boot2.py")

    boot2_stage2_ld = os.path.join(
        sdk_dir,
        "src",
        "rp2040",
        "boot_stage2",
        "boot_stage2.ld"
    ).replace("\\", "/")

    boot2_elf_tmp = os.path.join(BUILD_DIR, "boot2_tmp.elf").replace("\\", "/")

    boot2_bin_node = env.Command(
        boot2_bin,
        boot2_src_rp2040,
        f"$CC -c -mcpu=cortex-m0plus -mthumb -DPICO_BOOT_STAGE2 {inc_flags} $SOURCE -o {boot2_obj} && "
        f"$CC -nostdlib -Wl,--script={boot2_stage2_ld} -Wl,-Map={BUILD_DIR}/boot2.map "
        f"{boot2_obj} -o {boot2_elf_tmp} && "
        f"$OBJCOPY -O binary {boot2_elf_tmp} {boot2_bin} && "
        f"python {patch_boot2_py} {boot2_bin}"
    )

    make_boot2_s_py = os.path.join(PLATFORM_DIR, "builder", "make_boot2_s.py").replace("\\", "/")
    boot2_s = os.path.join(BUILD_DIR, "boot2_padded.S").replace("\\", "/")

    boot2_s_node = env.Command(
        boot2_s,
        boot2_bin_node,
        f"python {make_boot2_s_py} $SOURCE $TARGET"
    )

    boot2_elf_node = env.Object(
        target=boot2_elf,
        source=boot2_s_node
    )

pico_inc = [
    os.path.join(sdk_dir, "src/common/boot_picobin_headers/include"),
    os.path.join(sdk_dir, "src/common/boot_picoboot_headers/include"),
    os.path.join(sdk_dir, "src/common/boot_uf2_headers/include"),
    os.path.join(sdk_dir, "src/common/hardware_claim/include"),
    os.path.join(sdk_dir, "src/common/pico_base_headers/include"),
    os.path.join(sdk_dir, "src/common/pico_binary_info/include"),
    os.path.join(sdk_dir, "src/common/pico_bit_ops_headers/include"),
    os.path.join(sdk_dir, "src/common/pico_divider_headers/include"),
    os.path.join(sdk_dir, "src/common/pico_stdlib_headers/include"),
    os.path.join(sdk_dir, "src/common/pico_sync/include"),
    os.path.join(sdk_dir, "src/common/pico_time/include"),
    os.path.join(sdk_dir, "src/common/pico_usb_reset_interface_headers/include"),
    os.path.join(sdk_dir, "src/common/pico_util/include"),

    os.path.join(sdk_dir, "src/rp2_common/boot_bootrom_headers/include"),
    os.path.join(sdk_dir, "src/rp2_common/cmsis/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_adc/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_base/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_boot_lock/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_clocks/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_dcp/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_divider/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_dma/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_exception/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_flash/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_gpio/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_hazard3/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_i2c/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_interp/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_irq/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_pio/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_pll/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_powman/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_pwm/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_rcp/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_resets/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_riscv/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_riscv_platform_timer/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_rtc/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_sha256/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_spi/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_sync/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_sync_spin_lock/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_ticks/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_timer/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_uart/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_vreg/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_watchdog/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_xip_cache/include"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_xosc/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_aon_timer/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_async_context/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_atomic/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_bit_ops/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_bootrom/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_bootsel_via_double_reset/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_clib_interface/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_crt0/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_cxx_options/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_divider/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_double/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_fix/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_flash/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_float/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_i2c_slave/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_int64_ops/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_malloc/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_mem_ops/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_multicore/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_platform_common/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_platform_compiler/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_platform_panic/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_platform_sections/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_printf/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_rand/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_runtime/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_runtime_init/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_sha256/include"),
    #os.path.join(sdk_dir, "src/rp2_common/pico_standard_binary_info/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_status_led/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_stdio/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_stdio_semihosting/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_stdio_uart/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_stdio_usb/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_stdlib/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_time_adapter/include"),
    os.path.join(sdk_dir, "src/rp2_common/pico_unique_id/include"),
    
    os.path.join(sdk_dir, "src/boards/include"),
    
    os.path.join(sdk_dir, "src/rp2_common/cmsis/stub/CMSIS/Core/Include"),
]

pico_rp2040_inc = [
    os.path.join(sdk_dir, "src/rp2040/hardware_regs/include"),
    os.path.join(sdk_dir, "src/rp2040/hardware_structs/include"),
    os.path.join(sdk_dir, "src/rp2040/pico_platform/include"),
    os.path.join(sdk_dir, "src/rp2040/boot_stage2/asminclude"),
    os.path.join(sdk_dir, "src/rp2_common/cmsis/stub/CMSIS/Device/RP2040/Include"),
]

pico_rp2350_inc = [
    os.path.join(sdk_dir, "src/rp2350/boot_stage2/include"),
    os.path.join(sdk_dir, "src/rp2350/hardware_regs/include"),
    os.path.join(sdk_dir, "src/rp2350/hardware_structs/include"),
    os.path.join(sdk_dir, "src/rp2350/pico_platform/include"),
    os.path.join(sdk_dir, "src/rp2_common/cmsis/stub/CMSIS/Device/RP2350/Include"),
]

if IS_RP2040:
    env.AppendUnique(
        CPPPATH=pico_inc + pico_rp2040_inc + [
            os.path.join(PROJECT_DIR, "include"),
            os.path.join(PROJECT_DIR, "src")
        ]
    )
if IS_RP2350:
    env.AppendUnique(
        CPPPATH=pico_inc + pico_rp2350_inc + [
            os.path.join(PROJECT_DIR, "include"),
            os.path.join(PROJECT_DIR, "src")
        ]
    )

pico_src = [
    os.path.join(sdk_dir, "src/common/hardware_claim/claim.c"),
    os.path.join(sdk_dir, "src/common/pico_sync/critical_section.c"),
    os.path.join(sdk_dir, "src/common/pico_sync/lock_core.c"),
    os.path.join(sdk_dir, "src/common/pico_sync/mutex.c"),
    os.path.join(sdk_dir, "src/common/pico_sync/sem.c"),
    os.path.join(sdk_dir, "src/common/pico_time/time.c"),
    os.path.join(sdk_dir, "src/common/pico_util/datetime.c"),
    os.path.join(sdk_dir, "src/common/pico_util/pheap.c"),
    os.path.join(sdk_dir, "src/common/pico_util/queue.c"),

    os.path.join(sdk_dir, "src/rp2_common/hardware_adc/adc.c"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_boot_lock/boot_lock.c"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_clocks/clocks.c"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_divider/divider.c"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_dma/dma.c"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_exception/exception.c"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_flash/flash.c"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_gpio/gpio.c"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_i2c/i2c.c"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_interp/interp.c"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_irq/irq.c"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_irq/irq_handler_chain.S"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_pio/pio.c"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_pll/pll.c"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_spi/spi.c"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_sync/sync.c"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_sync_spin_lock/sync_spin_lock.c"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_ticks/ticks.c"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_timer/timer.c"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_uart/uart.c"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_vreg/vreg.c"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_watchdog/watchdog.c"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_xip_cache/xip_cache.c"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_xosc/xosc.c"),
    os.path.join(sdk_dir, "src/rp2_common/pico_aon_timer/aon_timer.c"),
    os.path.join(sdk_dir, "src/rp2_common/pico_atomic/atomic.c"),
    os.path.join(sdk_dir, "src/rp2_common/pico_bit_ops/bit_ops_aeabi.S"),
    os.path.join(sdk_dir, "src/rp2_common/pico_bootrom/bootrom.c"),
    os.path.join(sdk_dir, "src/rp2_common/pico_bootrom/bootrom_lock.c"),
    os.path.join(sdk_dir, "src/rp2_common/pico_bootsel_via_double_reset/pico_bootsel_via_double_reset.c"),
    os.path.join(sdk_dir, "src/rp2_common/pico_crt0/crt0.S"),
    os.path.join(sdk_dir, "src/rp2_common/pico_cxx_options/new_delete.cpp"),
    os.path.join(sdk_dir, "src/rp2_common/pico_divider/divider_compiler.c"),
    #os.path.join(sdk_dir, "src/rp2_common/pico_divider/divider_hardware.S"),
    os.path.join(sdk_dir, "src/rp2_common/pico_flash/flash.c"),
    os.path.join(sdk_dir, "src/rp2_common/pico_i2c_slave/i2c_slave.c"),
    os.path.join(sdk_dir, "src/rp2_common/pico_int64_ops/pico_int64_ops_aeabi.S"),
    os.path.join(sdk_dir, "src/rp2_common/pico_malloc/malloc.c"),
    os.path.join(sdk_dir, "src/rp2_common/pico_mem_ops/mem_ops.c"),
    os.path.join(sdk_dir, "src/rp2_common/pico_multicore/multicore.c"),
    os.path.join(sdk_dir, "src/rp2_common/pico_platform_common/common.c"),
    os.path.join(sdk_dir, "src/rp2_common/pico_platform_panic/panic.c"),
    os.path.join(sdk_dir, "src/rp2_common/pico_printf/printf.c"),
    os.path.join(sdk_dir, "src/rp2_common/pico_rand/rand.c"),
    os.path.join(sdk_dir, "src/rp2_common/pico_runtime/runtime.c"),
    os.path.join(sdk_dir, "src/rp2_common/pico_runtime_init/runtime_init.c"),
    os.path.join(sdk_dir, "src/rp2_common/pico_runtime_init/runtime_init_clocks.c"),
    os.path.join(sdk_dir, "src/rp2_common/pico_runtime_init/runtime_init_stack_guard.c"),
    os.path.join(sdk_dir, "src/rp2_common/pico_status_led/status_led.c"),
    os.path.join(sdk_dir, "src/rp2_common/pico_standard_binary_info/standard_binary_info.c"),
    os.path.join(sdk_dir, "src/rp2_common/pico_standard_link/binary_info.c"),
    os.path.join(sdk_dir, "src/rp2_common/pico_clib_interface/newlib_interface.c"),
    os.path.join(sdk_dir, "src/rp2_common/pico_stdio/stdio.c"),
    os.path.join(sdk_dir, "src/rp2_common/pico_stdio_semihosting/stdio_semihosting.c"),
    os.path.join(sdk_dir, "src/rp2_common/pico_stdio_uart/stdio_uart.c"),
    os.path.join(sdk_dir, "src/rp2_common/pico_stdlib/stdlib.c"),
    os.path.join(sdk_dir, "src/rp2_common/pico_unique_id/unique_id.c"),
]

pico_rp2040_src = [
    os.path.join(sdk_dir, "src/rp2_common/hardware_divider/divider.S"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_rtc/rtc.c"),
    os.path.join(sdk_dir, "src/rp2_common/cmsis/stub/CMSIS/Device/RP2040/Source/system_RP2040.c"),
]

pico_rp2350_src = [
    os.path.join(sdk_dir, "src/rp2_common/hardware_sha256/sha256.c"),
    os.path.join(sdk_dir, "src/rp2_common/hardware_powman/powman.c"),
    os.path.join(sdk_dir, "src/rp2_common/pico_sha256/sha256.c"),
    #os.path.join(sdk_dir, "src/rp2_common/pico_crt0/crt0_riscv.S"),
    os.path.join(sdk_dir, "src/rp2_common/cmsis/stub/CMSIS/Device/RP2350/Source/system_RP2350.c"),
]

sdk_objects = []
for src in pico_src:
    if os.path.exists(src):
        rel = os.path.relpath(src, PROJECT_DIR)
        obj = env.Object(
            target=os.path.join(BUILD_DIR, rel + ".o"),
            source=src
        )
        sdk_objects.append(obj)
        
if IS_RP2040:
    for src in pico_rp2040_src:
        if os.path.exists(src):
            rel = os.path.relpath(src, PROJECT_DIR)
            obj = env.Object(
                target=os.path.join(BUILD_DIR, rel + ".o"),
                source=src
            )
            sdk_objects.append(obj)
if IS_RP2350:
    for src in pico_rp2350_src:
        if os.path.exists(src):
            rel = os.path.relpath(src, PROJECT_DIR)
            obj = env.Object(
                target=os.path.join(BUILD_DIR, rel + ".o"),
                source=src
            )
            sdk_objects.append(obj)

if IS_RP2040:
    env.AppendUnique(
        LINKFLAGS=[
            "-Wl,--undefined=__boot2_start__",
            "-Wl,--undefined=_entry_point",
            "-Wl,--undefined=main",
            "-Wl,--section-start=.boot2=0x10000000"
        ]
    )

include_patterns, exclude_patterns = parse_src_filter(env)

project_src = []
all_project_src = collect_sources(os.path.join(PROJECT_DIR, "src"))

for src in all_project_src:
    rel = os.path.relpath(src, os.path.join(PROJECT_DIR, "src"))
    rel = rel.replace("\\", "/")

    if not match_any(rel, include_patterns):
        continue

    if match_any(rel, exclude_patterns):
        continue

    project_src.append(src)

project_objects = []
for src in project_src:
    rel = os.path.relpath(src, PROJECT_DIR)
    obj = env.Object(
        target=os.path.join(BUILD_DIR, rel + ".o"),
        source=src
    )
    project_objects.append(obj)
    
# CMake 原本會生成的 linker include
flash_region_ld = os.path.join(BUILD_DIR, "pico_flash_region.ld").replace("\\", "/")

with open(flash_region_ld, "w", encoding="utf-8") as f:
    f.write("""\
FLASH(rx) : ORIGIN = 0x10000000, LENGTH = 2M
""")
    
if IS_RP2040:
    ldscript = os.path.join(sdk_dir,"src","rp2_common","pico_crt0","rp2040","memmap_default.ld")
if IS_RP2350:
    ldscript = os.path.join(sdk_dir,"src","rp2_common","pico_crt0","rp2350","memmap_default.ld")

mapfile = os.path.join(BUILD_DIR, "firmware.map").replace("\\", "/")

env.AppendUnique(
    LINKFLAGS=[
        "-L" + BUILD_DIR.replace("\\", "/"),
        "-T", ldscript,
        "-Wl,-Map=" + mapfile
    ]
)

objects = sdk_objects + project_objects

if IS_RP2040:
    objects = boot2_elf_node + objects

elf = env.Program(
    target=os.path.join(BUILD_DIR, "firmware.elf"),
    source=objects
)

bin_file = env.Command(
    os.path.join(BUILD_DIR, "firmware.bin"),
    elf,
    "$OBJCOPY -O binary $SOURCE $TARGET"
)

AlwaysBuild(Alias("buildprog", elf))
AlwaysBuild(Alias("bin", bin_file))

env.Replace(
    PROGNAME="firmware",
    PROGSUFFIX=".elf"
)

uf2_file = env.Command(
    os.path.join(BUILD_DIR, "firmware.uf2"),
    elf,
    f"{picotool} uf2 convert $SOURCE $TARGET"
)

def upload_with_tool(source, target, env):
    elf_path = os.path.join(BUILD_DIR, "firmware.elf").replace("\\", "/")

    if IS_RP2040:
        rp2040tools = env.PioPlatform().get_package_dir("tool-rp2040tools")
        rp2040load = os.path.join(rp2040tools, "rp2040load").replace("\\", "/")

        print(">>> Uploading ELF with rp2040load ...")
        subprocess.check_call([rp2040load, "-v", "-D", elf_path])

    elif IS_RP2350:
        uf2_path = os.path.join(BUILD_DIR, "firmware.uf2").replace("\\", "/")
        print(">>> Uploading UF2 with picotool ...")
        subprocess.check_call([picotool, "load", "-x", uf2_path])

if IS_RP2350:
    upload_deps = [elf, bin_file, uf2_file]
else:
    upload_deps = [elf, bin_file]

upload_target = env.Command(
    "upload",
    upload_deps,
    upload_with_tool
)

AlwaysBuild(upload_target)
AlwaysBuild(Alias("upload", upload_target))