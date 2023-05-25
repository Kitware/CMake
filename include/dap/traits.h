// Copyright 2021 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef dap_traits_h
#define dap_traits_h

#include <tuple>
#include <type_traits>

namespace dap {
namespace traits {

// NthTypeOf returns the `N`th type in `Types`
template <int N, typename... Types>
using NthTypeOf = typename std::tuple_element<N, std::tuple<Types...>>::type;

// `IsTypeOrDerived<BASE, T>::value` is true iff `T` is of type `BASE`, or
// derives from `BASE`.
template <typename BASE, typename T>
using IsTypeOrDerived = std::integral_constant<
    bool,
    std::is_base_of<BASE, typename std::decay<T>::type>::value ||
        std::is_same<BASE, typename std::decay<T>::type>::value>;

// `EachIsTypeOrDerived<N, BASES, TYPES>::value` is true iff all of the types in
// the std::tuple `TYPES` is of, or derives from the corresponding indexed type
// in the std::tuple `BASES`.
// `N` must be equal to the number of types in both the std::tuple `BASES` and
// `TYPES`.
template <int N, typename BASES, typename TYPES>
struct EachIsTypeOrDerived {
  using base = typename std::tuple_element<N - 1, BASES>::type;
  using type = typename std::tuple_element<N - 1, TYPES>::type;
  using last_matches = IsTypeOrDerived<base, type>;
  using others_match = EachIsTypeOrDerived<N - 1, BASES, TYPES>;
  static constexpr bool value = last_matches::value && others_match::value;
};

// EachIsTypeOrDerived specialization for N = 1
template <typename BASES, typename TYPES>
struct EachIsTypeOrDerived<1, BASES, TYPES> {
  using base = typename std::tuple_element<0, BASES>::type;
  using type = typename std::tuple_element<0, TYPES>::type;
  static constexpr bool value = IsTypeOrDerived<base, type>::value;
};

// EachIsTypeOrDerived specialization for N = 0
template <typename BASES, typename TYPES>
struct EachIsTypeOrDerived<0, BASES, TYPES> {
  static constexpr bool value = true;
};

// Signature describes the signature of a function.
template <typename RETURN, typename... PARAMETERS>
struct Signature {
  // The return type of the function signature
  using ret = RETURN;
  // The parameters of the function signature held in a std::tuple
  using parameters = std::tuple<PARAMETERS...>;
  // The type of the Nth parameter of function signature
  template <std::size_t N>
  using parameter = NthTypeOf<N, PARAMETERS...>;
  // The total number of parameters
  static constexpr std::size_t parameter_count = sizeof...(PARAMETERS);
};

// SignatureOf is a traits helper that infers the signature of the function,
// method, static method, lambda, or function-like object `F`.
template <typename F>
struct SignatureOf {
  // The signature of the function-like object `F`
  using type = typename SignatureOf<decltype(&F::operator())>::type;
};

// SignatureOf specialization for a regular function or static method.
template <typename R, typename... ARGS>
struct SignatureOf<R (*)(ARGS...)> {
  // The signature of the function-like object `F`
  using type = Signature<typename std::decay<R>::type,
                         typename std::decay<ARGS>::type...>;
};

// SignatureOf specialization for a non-static method.
template <typename R, typename C, typename... ARGS>
struct SignatureOf<R (C::*)(ARGS...)> {
  // The signature of the function-like object `F`
  using type = Signature<typename std::decay<R>::type,
                         typename std::decay<ARGS>::type...>;
};

// SignatureOf specialization for a non-static, const method.
template <typename R, typename C, typename... ARGS>
struct SignatureOf<R (C::*)(ARGS...) const> {
  // The signature of the function-like object `F`
  using type = Signature<typename std::decay<R>::type,
                         typename std::decay<ARGS>::type...>;
};

// SignatureOfT is an alias to `typename SignatureOf<F>::type`.
template <typename F>
using SignatureOfT = typename SignatureOf<F>::type;

// ParameterType is an alias to `typename SignatureOf<F>::type::parameter<N>`.
template <typename F, std::size_t N>
using ParameterType = typename SignatureOfT<F>::template parameter<N>;

// `HasSignature<F, S>::value` is true iff the function-like `F` has a matching
// signature to the function-like `S`.
template <typename F, typename S>
using HasSignature = std::integral_constant<
    bool,
    std::is_same<SignatureOfT<F>, SignatureOfT<S>>::value>;

// `Min<A, B>::value` resolves to the smaller value of A and B.
template <std::size_t A, std::size_t B>
using Min = std::integral_constant<std::size_t, (A < B ? A : B)>;

// `CompatibleWith<F, S>::value` is true iff the function-like `F`
// can be called with the argument types of the function-like `S`. Return type
// of the two functions are not considered.
template <typename F, typename S>
using CompatibleWith = std::integral_constant<
    bool,
    (SignatureOfT<S>::parameter_count == SignatureOfT<F>::parameter_count) &&
        EachIsTypeOrDerived<Min<SignatureOfT<S>::parameter_count,
                                SignatureOfT<F>::parameter_count>::value,
                            typename SignatureOfT<S>::parameters,
                            typename SignatureOfT<F>::parameters>::value>;

// If `CONDITION` is true then EnableIf resolves to type T, otherwise an
// invalid type.
template <bool CONDITION, typename T = void>
using EnableIf = typename std::enable_if<CONDITION, T>::type;

// If `BASE` is a base of `T` then EnableIfIsType resolves to type `TRUE_TY`,
// otherwise an invalid type.
template <typename BASE, typename T, typename TRUE_TY = void>
using EnableIfIsType = EnableIf<IsTypeOrDerived<BASE, T>::value, TRUE_TY>;

// If the function-like `F` has a matching signature to the function-like `S`
// then EnableIfHasSignature resolves to type `TRUE_TY`, otherwise an invalid type.
template <typename F, typename S, typename TRUE_TY = void>
using EnableIfHasSignature = EnableIf<HasSignature<F, S>::value, TRUE_TY>;

}  // namespace traits
}  // namespace dap

#endif  // dap_traits_h
