// C++ Standard Library
#include <iostream>
#include <tuple>

// Zen
#include <zen/core.hpp>

int main(int argc, char** argv)
{
  using namespace zen;

  // clang-format off
  auto r = pass(argc, argc)
         | [](int a, float b) -> zen::result<float> { return a + b; }
         | [](float b) { return make_result(2.f * b, 2.f); }
         | all([](float b, char c) { return make_result(2.f * b, 2.f); },
               [](float b, char c) { return make_result(2.f * b, 2.f); });
  // clang-format on

  if (r.valid())
  {
    std::cout << "r: " << std::get<0>(*r) << std::endl;
  }
  else
  {
    std::cout << r.status() << std::endl;
  }
};
