cc_library(
  name="fwd",
  hdrs=["include/zen/fwd.hpp"],
  strip_include_prefix="include",
  visibility=["//visibility:public"]
)

cc_library(
  name="meta",
  hdrs=["include/zen/meta.hpp"] + glob(["include/zen/meta/*.hpp"]),
  strip_include_prefix="include",
  visibility=["//visibility:public"]
)

cc_library(
  name="executor",
  hdrs=["include/zen/executor.hpp"] + glob(["include/zen/executor/*.hpp"]),
  strip_include_prefix="include",
  deps=[":core"],
  visibility=["//visibility:public"]
)

cc_library(
  name="core",
  hdrs=["include/zen/core.hpp", "include/zen/result.hpp"],
  strip_include_prefix="include",
  deps=[":fwd", ":meta"],
  visibility=["//visibility:public"]
)

cc_library(
  name="zen",
  hdrs=["include/zen/zen.hpp"],
  strip_include_prefix="include",
  deps=[":core", ":executor"],
  visibility=["//visibility:public"]
)
