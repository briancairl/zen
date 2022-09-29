cc_library(
  name="fwd",
  hdrs=["include/zen/fwd.hpp"],
  strip_include_prefix="include",
  visibility=["//visibility:private"]
)

cc_library(
  name="meta",
  hdrs=["include/zen/meta.hpp"] + glob(["include/zen/meta/*.hpp"]),
  strip_include_prefix="include",
  visibility=["//:__subpackages__"]
)

cc_library(
  name="executor",
  hdrs=["include/zen/executor.hpp"] + glob(["include/zen/executor/*.hpp"]),
  strip_include_prefix="include",
  deps=[":core"],
  visibility=["//visibility:public"]
)

cc_library(
  name="result",
  hdrs=["include/zen/result.hpp"] + glob(["include/zen/result/*.hpp"]),
  strip_include_prefix="include",
  deps=[":fwd", ":meta"],
  visibility=["//visibility:public"]
)

cc_library(
  name="core",
  hdrs=["include/zen/core.hpp"],
  strip_include_prefix="include",
  deps=[":fwd", ":meta", ":result"],
  visibility=["//visibility:public"]
)

cc_library(
  name="parallel",
  hdrs=["include/zen/parallel.hpp"] + glob(["include/zen/parallel/*.hpp"]),
  strip_include_prefix="include",
  deps=[":core", ":executor"],
  visibility=["//visibility:public"]
)

cc_library(
  name="zen",
  hdrs=["include/zen/zen.hpp"],
  strip_include_prefix="include",
  deps=[":core", ":parallel"],
  visibility=["//visibility:public"]
)
