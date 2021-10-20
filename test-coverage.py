import os
Import("env")

env.Append(
  LINKFLAGS=[
      "--coverage"
  ]
)

def generateCoverageInfo(source, target, env):
    for file in os.listdir("test"):
        os.system(".pio/build/test_device/program test/"+file)
    os.system("lcov -d .pio/build/test_device/src -c -o lcov.info")
    os.system("lcov --extract lcov.info '*src*' -o extract_lcov.info")
    os.system("lcov --remove extract_lcov.info '*.pio*' '*homio.h' '*main.cpp' -o lcov.info")
    os.system("genhtml -o coverage/ lcov.info")
    os.system("rm extract_lcov.info")

env.AddPostAction(".pio/build/test_device/program", generateCoverageInfo)