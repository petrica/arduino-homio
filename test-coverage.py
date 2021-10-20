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
    os.system("lcov --remove lcov.info '*.pio*' '*v1*' -o filtered_lcov.info")
    os.system("genhtml -o coverage/ filtered_lcov.info")
    os.system("rm lcov.info filtered_lcov.info")

env.AddPostAction(".pio/build/test_device/program", generateCoverageInfo)