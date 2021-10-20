import os
Import("env")

env.Append(
  LINKFLAGS=[
      "--coverage"
  ]
)

def generateCoverageInfo(source, target, env):
    build_dir = env['PIOENV']
    os.system("mkdir coverage")
    os.system(".pio/build/" + build_dir + "/program")
    os.system("lcov -d .pio/build/" + build_dir + "/src -c -o coverage/" + build_dir + ".info")
    os.system("lcov --extract coverage/" + build_dir + ".info '*src*' -o coverage/" + build_dir + ".info")
    os.system("lcov --remove coverage/" + build_dir + ".info '*.pio*' '*homio.h' '*main.cpp' '*ArduinoFake*' -o coverage/" + build_dir + ".info")
    merge = ""
    for file in os.listdir("coverage"):
      merge += "-a coverage/" + file + " "
    os.system("lcov " + merge + " -o coverage/lcov.info")
    os.system("rm coverage/" + build_dir + ".info")

env.AddPostAction(".pio/build/" + env['PIOENV'] + "/program", generateCoverageInfo)