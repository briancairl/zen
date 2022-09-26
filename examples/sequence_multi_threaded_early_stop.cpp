// C++ Standard Library
#include <chrono>
#include <iostream>
#include <thread>

// Zen
#include <zen/parallel.hpp>

int main(int argc, char** argv)
{
  using namespace zen;
  using namespace std::chrono_literals;

  exec::thread_pool tp{4};

  // clang-format off
  auto r = pass(argc)
         | all(
           tp,
           [](const auto& handle, int a) -> zen::result<float>
           {
            std::cerr << "start" << std::endl;
             for (int i = 0; i < a * 2; ++i)
             {
               std::this_thread::sleep_for(50ms);
             }
             return "fake failure"_msg; 
           },
           [](const auto& handle, int a) -> zen::result<float>
           {
            std::cerr << "start" << std::endl;
             for (int i = 0; i < a * 20; ++i)
             {
               std::this_thread::sleep_for(50ms);
               if (handle.is_cancelled())
               {
                 std::cerr << "cancelled" << std::endl;
                 return "was cancelled"_msg;
               }
             }
             return 3 * a; 
           },
           [](const auto& handle, int a) -> zen::result<float>
           {
             std::cerr << "start" << std::endl;
             for (int i = 0; i < a * 200; ++i)
             {
               std::this_thread::sleep_for(50ms);
               if (handle.is_cancelled())
               {
                 std::cerr << "cancelled" << std::endl;
                 return "was cancelled"_msg;
               }
             }
             return 4 * a;
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
