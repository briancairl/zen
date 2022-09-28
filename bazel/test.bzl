def zen_cc_test(name, copts=[], linkopts=[], deps=[], **kwargs):
    '''
    A wrapper around cc_test for gtests
    Adds options to the compilation command.
    '''
    _GTEST_COPTS = [
        "-Iexternal/googletest/googletest/include",
        "-fsanitize=address",
        "-fsanitize-address-use-after-scope",
        "-DADDRESS_SANITIZER",
        "-g",
        "-fno-omit-frame-pointer",
        "-O0"
    ]

    _GTEST_LINKOPTS = [
        "-fsanitize=address",
        "-static-libasan"
    ]

    _GTEST_DEPS = [
        "@googletest//:gtest",
    ]

    native.cc_test(
        name=name,
        copts=_GTEST_COPTS + copts,
        deps=_GTEST_DEPS + deps,
        linkopts=_GTEST_LINKOPTS + linkopts,
        **kwargs
    )

def zen_cc_test_binary(name, copts=[], linkopts=[], **kwargs):
    '''
    A wrapper around cc_binary for sanitized
    Adds options to the compilation command.
    '''
    _TEST_BINARY_COPTS = [
        "-fsanitize=address",
        "-fsanitize-address-use-after-scope",
        "-DADDRESS_SANITIZER",
        "-g",
        "-fno-omit-frame-pointer",
        "-O0"
    ]

    _TEST_BINARY_LINKOPTS = [
        "-fsanitize=address",
        "-static-libasan"
    ]

    native.cc_binary(
        name=name,
        copts=_TEST_BINARY_COPTS + copts,
        linkopts=_TEST_BINARY_LINKOPTS + linkopts,
        **kwargs
    )
