[![Unit Tests](https://github.com/briancairl/zen/actions/workflows/pr.yml/badge.svg)](https://github.com/briancairl/zen/actions/workflows/pr.yml)

# Zen

Zen provides facilities for railroad style programming in C++.

This library supports parallel execution which can be easily swapped into any single-threaded program written with Zen.

## Examples

### Simple sequence

```c++
#include <iostream>

#include <zen/zen.hpp>

int main(int argc, char** argv)
{
  using namespace zen;

  auto r = pass(argc, argc)
         | [](int a, float b) -> result<float> { return a + b; }
         | [](float b) -> result<float> { return 2.f * b; };

   if (r.valid())
   {
     std::cout << "r: " << *r << std::endl;
   }
   else
  {
    std::cout << r.status() << std::endl;
  }
};
```

### Merging with `any` and `all`

```c++
#include <iostream>

#include <zen/zen.hpp>

int main(int argc, char** argv)
{
  using namespace zen;
  auto r = pass(argc, argc)
         | [](int a, float b) -> result<float>
           {
             if (a > 2)
             {
               // Failure message
               return "invalid 1 "_msg;
             }
             return 2 * b;
           }
         | any(
           [](float a) -> result<float> { return 2 * a; },
           [](float a) -> result<float>
           {
             if (a > 2)
             {
               // Failure message
               return "invalid 2"_msg;
             }
             return 2 * a;
           })
         | all(
           [](float a) -> result<float> { return 2 * a; },
           [](float a) -> result<float>
           {
             if (a > 2)
             {
               // Failure message
               return "invalid 3"_msg;
             }
             return 2 * a;
           });
   
   if (r.valid())
   {
     const auto [a, b] = *r;
     std::cout << "a: " << a << std::endl;
     std::cout << "b: " << b << std::endl;
   }
   else
  {
    std::cout << r.status() << std::endl;
  }
};
```

### Easy multi-threading with `any` and `all`

```c++
#include <iostream>

#include <zen/zen.hpp>

int main(int argc, char** argv)
{
  using namespace zen;

  exec::thread_pool tp{4};

  auto r = pass(argc, argc)
         | [](int a, float b) -> result<float> { return 2 * b; }
         | any(
           tp,
           [](float a) -> result<float> { return 2 * a; },
           [](float a) -> result<float> { return 2 * a; })
         | all(
           tp,
           [](float a) -> result<float> { return 2 * a; },
           [](float a) -> result<float> { return 3 * a; },
           [](float a) -> result<float> { return 4 * a; });
   
   if (r.valid())
   {
     const auto [a, b, c] = *r;
     std::cout << "a: " << a << std::endl;
     std::cout << "b: " << b << std::endl;
     std::cout << "c: " << c << std::endl;
   }
   else
  {
    std::cout << r.status() << std::endl;
  }
};
```

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
