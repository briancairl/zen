// C++ Standard Library
#include <chrono>
#include <iostream>
#include <thread>

// Zen
#include <zen/zen.hpp>

int main(int argc, char** argv)
{
  using namespace zen;
  using namespace std::chrono_literals;

  exec::thread_pool tp{4};

  // clang-format off
  auto r = pass(argc)
         | all(
               tp,
               any(
                    tp,
                    any(
                        tp,
                        all(
                          [](int a) -> zen::result<float>  { return "(1)"_fail; }
                        ),
                        [](int a) -> zen::result<float>  { return "(1)"_fail; },
                        [](int a) -> zen::result<float>  { return "(2)"_fail; },
                        [](int a) -> zen::result<float>  { return "(3)"_fail; }
                    ),
                    [](const auto& handle, int a) -> zen::result<float>
                    {
                      if (a > 2 || handle.is_cancelled())
                      {
                        return "(a > 2)"_fail;
                      }
                      return (a * 3);
                    },
                    [](const auto& handle, int a) -> zen::result<float>
                    {
                      if (a > 4 || handle.is_cancelled())
                      {
                        return "(a > 4)"_fail;
                      }
                      return (a * 13);
                    }
              ),
              all(
                [](int a) { return std::make_tuple(a * 13, a + 5); },
                [](int a) { return std::make_tuple(a * 13, a + 6); }
              )
            )
        | [](const auto&... vs) -> result<int>
        {
          return (vs + ...);
        };
  // clang-format on 

  if (r.valid())
  {
    std::cout << "meaning of life: " << *r << std::endl;
  }
  else
  {
    std::cout << r.status() << std::endl;
  }
};
