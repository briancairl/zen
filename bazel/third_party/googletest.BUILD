cc_library(
    name="gtest",
    srcs=glob(["googletest/src/*.cc",
               "googletest/src/*.h"],
              exclude=["googletest/src/gtest-all.cc"]
    ),
    hdrs=glob(["googletest/include/**/*.h",
               "googletest/**/*.h"]),
    copts=["-Iexternal/googletest/googletest/include",
           "-Iexternal/googletest/googletest"],
    linkopts=["-pthread"],
    visibility=["//visibility:public"],
)

# cc_library(
#     name="gmock",
#     srcs=glob(["googlemock/src/*.cc"],
#                 exclude=["googlemock/src/gmock-all.cc"]
#     ),
#     hdrs=glob(["googlemock/include/**/**/*.h",
#                "googlemock/include/**/*.h",
#                "googlemock/**/*.h"]),
#     copts=["-Iexternal/googletest/googletest/include",
#            "-Iexternal/googletest/googletest",
#            "-Iexternal/googletest/googlemock/include",
#            "-Iexternal/googletest/googlemock"],
#     linkopts=["-pthread"],
#     visibility=["//visibility:public"],
#     deps=[":gtest"]
# )
