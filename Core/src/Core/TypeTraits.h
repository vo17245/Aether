#pragma once
#include <tuple>
#include "TypeArray.h"
namespace Aether
{
template <typename T>
constexpr bool always_false_v = false;
#ifdef _MSC_VER
template <typename T>
void PrintType()
{
    static_assert(always_false_v<T>, __FUNCSIG__);
}
#else 
template <typename T>
void PrintType()
{
    static_assert(always_false_v<T>, __PRETTY_FUNCTION__);
}
#endif

template <typename T>
class IsLambda {
private:
    template <typename U>
    static auto Test(int) -> decltype(&U::operator(), std::true_type{});

    template <typename>
    static auto Test(...) -> std::false_type;

public:
    static constexpr bool Value = 
        std::is_class_v<T> && decltype(Test<T>(0))::value;
};

template <typename T>
inline constexpr bool is_lambda_v = IsLambda<T>::Value;

template <typename T,typename=void>
struct LambdaTraits
{
    static_assert(sizeof(T) == 0, "LambdaTraits is not specialized for this type. Please provide a specialization for your lambda type.");
};
namespace CoreDetail
{
    // 针对非泛型 lambda
    template <typename T>
    struct LambdaTraitsImpl;
    template <typename ClassType, typename Ret, typename... TArgs>
    struct LambdaTraitsImpl<Ret (ClassType::*)(TArgs...) const>
    {
        using RetType = Ret;
        using ArgTypes = std::tuple<TArgs...>;
    };
}
// 针对非泛型 lambda
template <typename T>
struct LambdaTraits<T,std::enable_if_t<is_lambda_v<T>>>
{
    using Impl= CoreDetail::LambdaTraitsImpl<decltype(&std::remove_reference_t<T>::operator())>;
    using RetType = Impl::RetType;
    using ArgTypes = Impl::ArgTypes;
};

} // namespace Aether