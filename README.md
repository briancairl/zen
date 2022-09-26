# Zen

Zen is a functional-ish programming library. I made it for fun, and nothing else.

Supports multi-threaded execution. Attempts to allow for the construction of execution-context generic sequences.

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

### Joins / Merges with `any` and `all`

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
               return status{"invalid 1 "};
             }
             return 2 * b;
           }
         | any(
           [](float a) -> result<float> { return 2 * a; },
           [](float a) -> result<float>
           {
             if (a > 2)
             {
               return status{"invalid 2"};
             }
             return 2 * a;
           })
         | all(
           [](float a) -> result<float> { return 2 * a; },
           [](float a) -> result<float>
           {
             if (a > 2)
             {
               return status{"invalid 3"};
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

# Running tests

## bazel

```
bazel test test/... --test_output=all --cache_test_results=no --compilation_mode=dbg
```
