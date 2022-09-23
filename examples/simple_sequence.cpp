#include <iostream>

#include <zen/zen.hpp>

int main(int argc, char** argv)
{
  using namespace zen;

  // clang-format off
  auto r = begin(argc, argc)
         | [](int a, float b) -> zen::result<float> { return a + b; }
         | [](float b) -> zen::result<float> { return 2.f * b; };
  // clang-format on

  if (r.valid())
  {
    std::cout << "r: " << *r << std::endl;
  }
  else
  {
    std::cout << r.message() << std::endl;
  }
};
