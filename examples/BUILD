load("@zen//bazel:test.bzl", "zen_cc_test_binary")

zen_cc_test_binary(
  name="sequence_multi_threaded",
  srcs=["sequence_multi_threaded.cpp"],
  deps=["//:parallel"]
)

zen_cc_test_binary(
  name="sequence_multi_threaded_early_stop",
  srcs=["sequence_multi_threaded_early_stop.cpp"],
  deps=["//:parallel"]
)

zen_cc_test_binary(
  name="sequence_with_all_any",
  srcs=["sequence_with_all_any.cpp"],
  deps=["//:core"]
)

zen_cc_test_binary(
  name="simple_sequence",
  srcs=["simple_sequence.cpp"],
  deps=["//:core"]
)

zen_cc_test_binary(
  name="zen",
  srcs=["zen.cpp"],
  deps=["//:zen"]
)
