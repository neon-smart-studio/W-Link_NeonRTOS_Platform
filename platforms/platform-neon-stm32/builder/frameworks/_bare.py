# Copyright 2014-present PlatformIO <contact@platformio.org>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#
# Default flags for bare-metal programming (without any framework layers)
#

from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()

env.Append(
    ASFLAGS=[
        "-mthumb",
    ],
    ASPPFLAGS=[
        "-x", "assembler-with-cpp",
    ],

    CCFLAGS=[
        "-Os",  # optimize for size
        "-ffunction-sections",  # place each function in its own section
        "-fdata-sections",
        "-Wall",
        "-mthumb"
    ],

    CXXFLAGS=[
        "-fno-rtti",
        "-fno-exceptions"
    ],

    CPPDEFINES=[
        ("F_CPU", "$BOARD_F_CPU")
    ],

    LINKFLAGS=[
        "-Os",
        "-Wl,--gc-sections,--relax",
        "-mthumb"
    ],

    LIBS=["c", "gcc", "m", "stdc++"]
)

if "BOARD" in env:

    board_cfg = env.BoardConfig()

    cpu = board_cfg.get("build.cpu", "cortex-m3")
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