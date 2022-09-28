// C++ Standard Library
#include <iostream>

// Zen
#include <zen/core.hpp>

int main(int argc, char** argv)
{
  using namespace zen;
  // clang-format off
  auto r = pass(argc, argc)
         | [](int a, float b) -> zen::result<float>
           {
             if (a > 2)
             {
               return "invalid 1 "_msg;
             }
             return 2 * b;
           }
         | any(
           [](float a) -> zen::result<float> { return 2 * a; },
           [](float a) -> zen::result<float>
           {
             if (a > 2)
             {
               return "invalid 2"_msg;
             }
             return 2 * a;
           })
         | all(
           [](float a) { return make_result(2 * a, 1); },
           [](float a) -> zen::result<float>
           {
             if (a > 2)
             {
               return "invalid 3"_msg;
             }
             return 2 * a;
           });
  // clang-format on

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
