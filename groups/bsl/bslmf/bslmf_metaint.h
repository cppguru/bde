// bslmf_metaint.h                                                    -*-C++-*-
#ifndef INCLUDED_BSLMF_METAINT
#define INCLUDED_BSLMF_METAINT

#include <bsls_ident.h>
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide a meta-function to map integral constants to unique types.
//
//@DEPRECATED: Use `bslmf_integralconstant` instead.
//
//@CLASSES:
//  bslmf::MetaInt: meta-function mapping integral constants to C++ types
//
//@DESCRIPTION: This component defines a simple template structure used to map
// an integral constant to a C++ type.  `bslmf::MetaInt<int>` defines a
// different type for each distinct compile-time constant integral parameter.
// That is, instantiations with different integer values form distinct types,
// so that `bslmf::MetaInt<0>` is a distinct type from `bslmf::MetaInt<1>`,
// which is also distinct from `bslmf::MetaInt<2>`, and so on.
//
///Usage
///-----
// This section illustrates intended usage of this component
//
///Example 1: Compile-Time Function Dispatching
/// - - - - - - - - - - - - - - - - - - - - - -
// The most common use of this structure is to perform static function
// dispatching based on a compile-time calculation.  Often the calculation is
// nothing more than a simple predicate, allowing us to select one of two
// functions.  The following function, `doSomething`, uses a fast
// implementation (e.g., `memcpy`) if the parameterized type allows for such
// operations, otherwise it will use a more generic and slower implementation
// (e.g., copy constructor).
// ```
// template <class T>
// void doSomethingImp(T *t, bslmf::MetaInt<0>)
// {
//     // slow generic implementation
//     (void) t;
//     // ...
// }
//
// template <class T>
// void doSomethingImp(T *t, bslmf::MetaInt<1>)
// {
//     // fast implementation (works only for some T's)
//     (void) t;
//     // ...
// }
//
// template <class T, bool IsFast>
// void doSomething(T *t)
// {
//     doSomethingImp(t, bslmf::MetaInt<IsFast>());
// }
// ```
// The power of this approach is that the compiler will compile only the
// implementation selected by the `MetaInt` argument.  For some parameter
// types, the fast version of `doSomethingImp` would be ill-formed.  This kind
// of compile-time dispatch prevents the ill-formed version from ever being
// instantiated.
// ```
// int main()
// {
//     int i;
//     doSomething<int, true>(&i); // fast version selected for int
//
//     double m;
//     doSomething<double, false>(&m); // slow version selected for double
//
//     return 0;
// }
// ```
//
///Example 2: Reading the `VALUE` Member
///- - - - - - - - - - - - - - - - - - -
// In addition to forming new types, the value of the integral parameter to
// `MetaInt` is "saved" in the enum member `VALUE`, and is accessible for use
// in compile-time or run-time operations.
// ```
// template <int V>
// unsigned g()
// {
//     bslmf::MetaInt<V> i;
//     assert(V == i.VALUE);
//     assert(V == bslmf::MetaInt<V>::VALUE);
//     return bslmf::MetaInt<V>::VALUE;
// }
//
// int main()
// {
//     int v = g<1>();
//     assert(1 == v);
//     return 0;
// }
// ```

#include <bslscm_version.h>

#include <bslmf_integralconstant.h>
#include <bslmf_tag.h>

#include <bsls_compilerfeatures.h>
#include <bsls_deprecatefeature.h>
#include <bsls_platform.h>

namespace BloombergLP {

namespace bslmf {

                           // ==============
                           // struct MetaInt
                           // ==============

/// Instantiating this template produces a distinct type for each
/// non-negative integer value.  This template has been deprecated in favor
/// of the standard `integral_constant` template.
template <int t_INT_VALUE>
struct
BSLS_DEPRECATE_FEATURE("bsl",
                       "bslmf_MetaInt",
                       "use 'integral_constant' instead")
MetaInt : public bsl::integral_constant<int, t_INT_VALUE> {

#if defined(BSLS_COMPILERFEATURES_SUPPORT_STATIC_ASSERT)
    static_assert(t_INT_VALUE >= 0, "t_INT_VALUE must be non-negative");
#endif

    // TYPES
    typedef MetaInt<t_INT_VALUE>    Type;

#ifdef BSLS_PLATFORM_CMP_IBM
// Suppress a compile warning on xlc for negative unsigned values.
#pragma report(disable, "1540-0724")
#endif

    typedef bslmf::Tag<t_INT_VALUE> Tag;

#ifdef BSLS_PLATFORM_CMP_IBM
#pragma report(pop)
#endif

    enum { VALUE = t_INT_VALUE };

    // CREATORS

    /// Does nothing (`MetaInt` is stateless).
    MetaInt();

    /// Convert from a `bsl::integral_constant<int, t_INT_VALUE>`.
    MetaInt(bsl::integral_constant<int, t_INT_VALUE>);              // IMPLICIT

    //! MetaInt(const MetaInt&) = default;
    //! MetaInt& operator=(const MetaInt&) = default;
    //! ~MetaInt() = default;

    // CLASS METHODS

    /// Declared but not defined.  Meta-function use only.  The tag can be
    /// used to recover meta-information from an expression.  Example:
    /// `sizeof(f(expr).tag())` returns a different compile-time value
    /// depending on the type of the result of calling the `f` function but
    /// does not actually call the `f` function or the `tag` method at
    /// run-time.  Note that `f(expr)::VALUE` or `sizeof(f(expr)::Type)`
    /// would be ill-formed and that `f(expr).value` is not a compile-time
    /// expression.
    static Tag& tag();
};

/// This specialization of `MetaInt` has a `VAL` of zero and is convertible
/// to and from `bsl::false_type`.
template <>
struct
BSLS_DEPRECATE_FEATURE("bsl",
                       "bslmf_MetaInt",
                       "use 'integral_constant' instead")
MetaInt<0> : public bsl::false_type {

    // TYPES
    typedef MetaInt<0>    Type;
    typedef bslmf::Tag<0> Tag;

    enum { VALUE = 0 };

    // CREATORS

    /// Does nothing (`MetaInt` is stateless).
    MetaInt();

    /// Convert from a `bsl::false_type`.
    MetaInt(bsl::false_type);                                       // IMPLICIT

    //! MetaInt(const MetaInt&) = default;
    //! MetaInt& operator=(const MetaInt&) = default;
    //! ~MetaInt() = default;

    // CLASS METHODS

    /// Declared but not defined.  Meta-function use only.  The tag can be
    /// used to recover meta-information from an expression.  Example:
    /// `sizeof(f(expr).tag())` returns a different compile-time value
    /// depending on the type of the result of calling the `f` function but
    /// does not actually call the `f` function or the `tag` method at
    /// run-time.  Note that `f(expr)::VALUE` or `sizeof(f(expr)::Type)`
    /// would be ill-formed and that `f(expr).value` is not a compile-time
    /// expression.
    static Tag& tag();

    // ACCESSORS

    /// Return `false`.  (This operator is conversion operator to `bool`.)
    operator bool() const;
};

/// This specialization of `MetaInt` has a `VAL` of one and is convertible
/// to and from `bsl::true_type`.
template <>
struct
BSLS_DEPRECATE_FEATURE("bsl",
                       "bslmf_MetaInt",
                       "use 'integral_constant' instead")
MetaInt<1> : public bsl::true_type {

    // TYPES
    typedef MetaInt<1>    Type;
    typedef bslmf::Tag<1> Tag;

    enum { VALUE = 1 };

    // CREATORS

    /// Does nothing (`MetaInt` is stateless).
    MetaInt();

    /// Convert from a `bsl::true_type`.
    MetaInt(bsl::true_type);                                        // IMPLICIT

    //! MetaInt(const MetaInt&) = default;
    //! MetaInt& operator=(const MetaInt&) = default;
    //! ~MetaInt() = default;

    // CLASS METHODS

    /// Declared but not defined.  Meta-function use only.  The tag can be
    /// used to recover meta-information from an expression.  Example:
    /// `sizeof(f(expr).tag())` returns a different compile-time value
    /// depending on the type of the result of calling the `f` function but
    /// does not actually call the `f` function or the `tag` method at
    /// run-time.  Note that `f(expr)::VALUE` or `sizeof(f(expr)::Type)`
    /// would be ill-formed and that `f(expr).value` is not a compile-time
    /// expression.
    static Tag& tag();

    // ACCESSORS

    /// Return `true`.  (This operator is conversion operator to `bool`.)
    operator bool() const;
};

/// Given an integral value, `V`, and an expression, `expr`, of type
/// `bslmf::MetaInt<V>`, this macro returns a compile-time constant with
/// value, `V`.  The expression, `expr`, is not evaluated at run-time.
#define BSLMF_METAINT_TO_INT(expr)  BSLMF_TAG_TO_INT((expr).tag())

/// Given an integral value, `V`, and an expression, `expr`, of type
/// `bslmf::MetaInt<V>`, this macro returns a compile-time constant with
/// value, `true` or `false`, according to the Boolean value of `V`.  The
/// expression, `expr`, is not evaluated at run-time.
#define BSLMF_METAINT_TO_BOOL(expr) BSLMF_TAG_TO_BOOL((expr).tag())

#ifndef BDE_OPENSOURCE_PUBLICATION  // BACKWARD_COMPATIBILITY
// ============================================================================
//                           BACKWARD COMPATIBILITY
// ============================================================================

#ifdef bslmf_MetaInt
#undef bslmf_MetaInt
#endif
/// This alias is defined for backward compatibility.
#define bslmf_MetaInt bslmf::MetaInt
#endif  // BDE_OPENSOURCE_PUBLICATION -- BACKWARD_COMPATIBILITY

// ============================================================================
//                            INLINE FUNCTIONS
// ============================================================================

// CREATORS
template <int t_INT_VALUE>
inline
MetaInt<t_INT_VALUE>::MetaInt()
{
}

template <int t_INT_VALUE>
inline
MetaInt<t_INT_VALUE>::MetaInt(bsl::integral_constant<int, t_INT_VALUE>)
{
}

inline
MetaInt<0>::MetaInt()
{
}

inline
MetaInt<0>::MetaInt(bsl::false_type)
{
}

inline
MetaInt<1>::MetaInt()
{
}

inline
MetaInt<1>::MetaInt(bsl::true_type)
{
}

// ACCESSORS
inline
MetaInt<0>::operator bool() const
{
    return false;
}

inline
MetaInt<1>::operator bool() const
{
    return true;
}

}  // close package namespace
}  // close enterprise namespace

#endif

// ----------------------------------------------------------------------------
// Copyright 2013 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
