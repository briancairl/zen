#pragma once

namespace zen::meta
{

template <typename PackT, template <typename> class TransformTmpl> struct transform;

template <template <typename...> class PackTmpl, typename... Ts, template <typename> class TransformTmpl>
struct transform<PackTmpl<Ts...>, TransformTmpl>
{
  using type = PackTmpl<typename TransformTmpl<Ts>::type...>;
};

template <typename PackT, template <typename> class TransformTmpl>
using transform_t = typename transform<PackT, TransformTmpl>::type;

}  // namespace zen::meta