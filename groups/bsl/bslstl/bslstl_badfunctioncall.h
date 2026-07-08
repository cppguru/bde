// bslstl_badfunctioncall.h                                           -*-C++-*-
#ifndef INCLUDED_BSLSTL_BADFUNCTIONCALL
#define INCLUDED_BSLSTL_BADFUNCTIONCALL

#include <bsls_ident.h>
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide an exception class thrown by `bsl::function`.
//
//@CLASSES:
//  bsl::bad_function_call: exception type thrown by `bsl::function`
//
//@CANONICAL_HEADER: bsl_functional.h
//
//@SEE_ALSO: bslstl_function, bslstl_stdexceptionutil
//
//@DESCRIPTION: This component provides a `bsl::bad_function_call` exception
// class when exceptions are enabled, and nothing otherwise.  This exception is
// thrown by `bsl::function::operator()` when the function wrapper object has
// no target.  If compiling as C++11 or later, `bsl::bad_function_call` is an
// alias for `std::bad_function_call`.
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: A Degenerate Function Wrapper
/// - - - - - - - - - - - - - - - - - - - -
// Suppose we want to write a null functor, the functional equivalent of a
// null pointer, which throws an exception when invoked.  We can use the
// `bsl::bad_function_call` exception type as our common vocabulary for
// reporting the invocation of functors when they are not in a callable state.
//
// First, we define the class and give it a call operator that unconditionally
// throws:
// ```
// struct EmptyFunction {
//     void operator()() const { BSLS_THROW(bsl::bad_function_call()); }
// };
// ```
// Then, we invoke the call operator inside a `try` block and confirm that the
// expected exception type is caught:
// ```
// bool caught = false;
// try {
//     EmptyFunction()();
// }
// catch (const bsl::bad_function_call&) {
//     caught = true;
// }
// assert(caught);
// ```

#include <bslscm_version.h>

#include <bsls_buildtarget.h>
#include <bsls_compilerfeatures.h>
#include <bsls_exceptionutil.h>

#ifdef BDE_BUILD_TARGET_EXC

#include <exception>
#include <functional>  // for `std::bad_function_call` in C++11

#if BSLS_COMPILERFEATURES_FULL_CPP11
namespace bsl {
    using std::bad_function_call;
}  // close namespace bsl
#else
namespace bsl {
                        // =======================
                        // class bad_function_call
                        // =======================

class bad_function_call : public std::exception {
  public:
    // CREATORS

    /// Create a `bad_function_call` object.  Note that this function is
    /// explicitly user-declared, to make it simple to declare `const`
    /// objects of this type.
    bad_function_call();

    // ACCESSORS

    /// Return a pointer to the string literal "bad_function_call", with a
    /// storage duration of the lifetime of the program.  Note that the
    /// caller should *not* attempt to free this memory.
    const char *what() const BSLS_EXCEPTION_VIRTUAL_NOTHROW;
};
}  // close namespace bsl
#endif  // BSLS_COMPILERFEATURES_FULL_CPP11
#endif  // BDE_BUILD_TARGET_EXC
#endif

// ----------------------------------------------------------------------------
// Copyright 2020 Bloomberg Finance L.P.
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
