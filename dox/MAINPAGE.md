# Zen

Zen provides facilities for railroad style programming in C++.

This library supports parallel execution which can be easily swapped into any single-threaded program written with Zen.

# Running examples

```
bazel run examples:<target>
```

```
bazel run examples:zen
```

# Running tests

## bazel

```
bazel test test/... --test_output=all --cache_test_results=no --compilation_mode=dbg
```
