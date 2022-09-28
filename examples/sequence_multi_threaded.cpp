// C++ Standard Library
#include <iostream>

// Zen
#include <zen/parallel.hpp>

int main(int argc, char** argv)
{
  using namespace zen;

  exec::thread_pool tp{4};

  // clang-format off
  auto r = pass(argc, argc)
         | [](int a, float b) -> zen::result<float> { return 2 * b; }
         | any(
           tp,
           [](float a) -> zen::result<float> { return 2 * a; },
           [](float a) -> zen::result<float> { return 2 * a; })
         | all(
           tp,
           [](float a) -> zen::result<float> { return 2 * a; },
           [](float a) -> zen::result<float> { return 3 * a; },
           [](float a) -> zen::result<float> { return 4 * a; });
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
