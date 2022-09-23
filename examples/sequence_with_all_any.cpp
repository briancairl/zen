#include <iostream>

#include <zen/zen.hpp>

int main(int argc, char** argv)
{
  using namespace zen;
  // clang-format off
  auto r = begin(argc, argc)
         | [](int a, float b) -> zen::result<float>
           {
             if (a > 2)
             {
               return status{"invalid 1 "};
             }
             return 2 * b;
           }
         | any(
           [](float a) -> zen::result<float> { return 2 * a; },
           [](float a) -> zen::result<float>
           {
             if (a > 2)
             {
               return status{"invalid 2"};
             }
             return 2 * a;
           })
         | all(
           [](float a) -> zen::result<float> { return 2 * a; },
           [](float a) -> zen::result<float>
           {
             if (a > 2)
             {
               return status{"invalid 3"};
             }
             return 2 * a;
           });
  // clang-format on

  if (r.valid())
  {
    const auto [a, b] = *r;
    std::cout << "a: " << a << std::endl;
    std::cout << "b: " << b << std::endl;
  }
  else
  {
    std::cout << r.message() << std::endl;
  }
};
