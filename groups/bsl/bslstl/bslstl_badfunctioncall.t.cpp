// bslstl_badfunctioncall.t.cpp                                       -*-C++-*-
#include <bslstl_badfunctioncall.h>

#include <bslmf_issame.h>

#include <bslma_default.h>
#include <bslma_testallocator.h>

#include <bsls_bsltestutil.h>
#include <bsls_buildtarget.h>
#include <bsls_compilerfeatures.h>
#include <bsls_exceptionutil.h>
#include <bsls_platform.h>
#include <bsls_review.h>

#include <algorithm>  // `swap` C++03
#include <exception>
#include <utility>    // `swap` C++11

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
//              DISABLE TESTING WHEN EXCEPTIONS ARE NOT ENABLED
// ----------------------------------------------------------------------------
// This component provides `bsl::bad_function_call` only when exceptions are
// enabled; otherwise the header defines nothing and there is nothing to test.

#ifndef BDE_BUILD_TARGET_EXC
int main(int argc, char *[])
{
    const bool verbose = argc > 2;
    if (verbose) puts("This component has no content when exceptions "
                      "are disabled.");
    return -1;
}
#else

using namespace BloombergLP;

//=============================================================================
//                       BUILD CONFIGURATION MACROS
//-----------------------------------------------------------------------------

/// Define this macro to enable compile-fail tests.
//#define BSLSTL_BADFUNCTIONCALL_SHOW_NO_ADL    1

// ============================================================================
//                             TEST PLAN
// ----------------------------------------------------------------------------
//                             Overview
//                             --------
// The type under test is `bsl::bad_function_call`, an exception type whose
// interface and contract is dictated by the C++ standard.  If
// `std::bad_function_call` exception is available, we need to check that
// `bsl::bad_function_call` is an alias of the standard's exception type.  If
// `std::bad_function_call` exception is not available, we need to check that
// `bsl::bad_function_call` satisfies the interface and contract of
// `std::bad_function_call`.
// ----------------------------------------------------------------------------
// [ 3] bad_function_call();
// [ 3] const char *what() const;
// ----------------------------------------------------------------------------
// [ 1] BREATHING TEST
// [ 2] CONCERN: `bsl::bad_function_call` is `std::bad_function_call`
// [ 4] CONCERN: `bsl::bad_function_call` can be thrown
// [ 5] USAGE EXAMPLE

// ============================================================================
//                     STANDARD BSL ASSERT TEST FUNCTION
// ----------------------------------------------------------------------------

namespace {

int testStatus = 0;

void aSsErT(bool condition, const char *message, int line)
{
    if (condition) {
        printf("Error " __FILE__ "(%d): %s    (failed)\n", line, message);
        fflush(stdout);

        if (0 <= testStatus && testStatus <= 100) {
            ++testStatus;
        }
    }
}

}  // close unnamed namespace

// ============================================================================
//               STANDARD BSL TEST DRIVER MACRO ABBREVIATIONS
// ----------------------------------------------------------------------------

#define ASSERT BSLS_BSLTESTUTIL_ASSERT
#define ASSERTV BSLS_BSLTESTUTIL_ASSERTV

#define LOOP_ASSERT BSLS_BSLTESTUTIL_LOOP_ASSERT
#define LOOP0_ASSERT BSLS_BSLTESTUTIL_LOOP0_ASSERT
#define LOOP1_ASSERT BSLS_BSLTESTUTIL_LOOP1_ASSERT
#define LOOP2_ASSERT BSLS_BSLTESTUTIL_LOOP2_ASSERT
#define LOOP3_ASSERT BSLS_BSLTESTUTIL_LOOP3_ASSERT
#define LOOP4_ASSERT BSLS_BSLTESTUTIL_LOOP4_ASSERT
#define LOOP5_ASSERT BSLS_BSLTESTUTIL_LOOP5_ASSERT
#define LOOP6_ASSERT BSLS_BSLTESTUTIL_LOOP6_ASSERT

#define Q BSLS_BSLTESTUTIL_Q    // Quote identifier literally.
#define P BSLS_BSLTESTUTIL_P    // Print identifier and value.
#define P_ BSLS_BSLTESTUTIL_P_  // P(X) without '\n'.
#define T_ BSLS_BSLTESTUTIL_T_  // Print a tab (w/o newline).
#define L_ BSLS_BSLTESTUTIL_L_  // current Line number

#define RUN_EACH_TYPE BSLTF_TEMPLATETESTFACILITY_RUN_EACH_TYPE

//=============================================================================
//                        DEFECT DETECTION MACROS
//-----------------------------------------------------------------------------

#if defined(BSLS_PLATFORM_CMP_MSVC)
// An implicit copy constructor is specified to perform a memberwise copy of
// direct bases and members, which does not involve name lookup.  MSVC's
// implicit definition of a copy constructor attempts to cast the passed
// `const Derived&` argument to `const Base&` to initialize each direct base,
// producing for any repeated base classes an ambiguous cast that results in
// compiler error C2594.
# define BUG_REPEATED_BASE_BAD_COPY_CTOR   1
#endif

//=============================================================================
//                         GLOBAL FUNCTIONS FOR TESTING
//-----------------------------------------------------------------------------

namespace bsl {
/// This function can be called without explicit qualification or `using` via
/// ADL, but only if the `bad_function_call` type is associated with the `bsl`
/// namespace.
void bslstl_badfunctioncall_show_ADL(bad_function_call &) {}

#if defined(BSLS_PLATFORM_HAS_PRAGMA_GCC_DIAGNOSTIC)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Winaccessible-base"
#elif defined(BSLS_PLATFORM_CMP_MSVC)
# pragma warning(push)
# pragma warning(disable: 4584)
#endif
/// This class derives ambiguously from `bsl::bad_function_call` and
/// `std::exception`, which means that a handler for `std::exception` will not
/// catch it, allowing a later handler for `bsl::bad_function_call` to catch it
/// instead.
struct ambiguous_exception : bad_function_call, std::exception {
#ifdef BUG_REPEATED_BASE_BAD_COPY_CTOR
    ambiguous_exception() {}
    ambiguous_exception(const ambiguous_exception& original)
    : bad_function_call(original)  // unambiguous: direct base
    , std::exception()             // value-initialize the `std::exception`
                                   // direct base sub-object.
    {}
#endif
};  // struct ambiguous_exception
#if defined(BSLS_PLATFORM_HAS_PRAGMA_GCC_DIAGNOSTIC)
# pragma GCC diagnostic pop
#elif defined(BSLS_PLATFORM_CMP_MSVC)
# pragma warning(pop)
#endif

}  // close namespace bsl

//=============================================================================
//                              MAIN PROGRAM
//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    const int                 test = argc > 1 ? atoi(argv[1]) : 0;
    const bool             verbose = argc > 2;
    const bool         veryVerbose = argc > 3;
    const bool     veryVeryVerbose = argc > 4;
    const bool veryVeryVeryVerbose = argc > 5;

    (void)    veryVerbose;
    (void)veryVeryVerbose;

    printf("TEST " __FILE__ " CASE %d\n", test);

    // CONCERN: `BSLS_REVIEW` failures should lead to test failures.
    bsls::ReviewFailureHandlerGuard reviewGuard(&bsls::Review::failByAbort);

    // CONCERN: In no case does memory come from the global allocator.
    static bslma::TestAllocator globalAllocator("global", veryVeryVeryVerbose);
    bslma::Default::setGlobalAllocator(&globalAllocator);

    // CONCERN: In no case does memory come from the default allocator.
    bslma::TestAllocator defaultAllocator("default", veryVeryVeryVerbose);
    ASSERT(0 == bslma::Default::setDefaultAllocator(&defaultAllocator));

    // Confirm no static initialization locked the default allocator
    ASSERT(&defaultAllocator == bslma::Default::defaultAllocator());

    switch (test) { case 0:
      case 5: {
        // --------------------------------------------------------------------
        // USAGE EXAMPLE
        //   Extracted from component header file.
        //
        // Concerns:
        // 1. The usage example provided in the component header file compiles,
        //    links, and runs as shown.
        //
        // Plan:
        // 1. Incorporate the usage example from the header into the test
        //    driver, remove leading comment characters, and replace `assert`
        //    with `ASSERT`.  (C-1)
        //
        // Testing:
        //   USAGE EXAMPLE
        // --------------------------------------------------------------------

        if (verbose) puts("\nUSAGE EXAMPLE\n=============");

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
    struct EmptyFunction {
        void operator()() const { BSLS_THROW(bsl::bad_function_call()); }
    };
// ```
// Then, we invoke the call operator inside a `try` block and confirm that the
// expected exception type is caught:
// ```
    bool caught = false;
    try {
        EmptyFunction()();
    }
    catch (const bsl::bad_function_call&) {
        caught = true;
    }
    ASSERT(caught);
// ```
      } break;
      case 4: {
        // --------------------------------------------------------------------
        // `bsl::bad_function_call` IS THROWABLE
        //  The whole purpose of `bsl::bad_function_call` is to serve as an
        //  exception, so we should confirm that we can throw and catch one.
        //
        // Concerns:
        // 1. `bad_function_call` can be used in a throw expression.
        // 2. `bad_function_call` does not have an ambiguous base class.
        //
        // Plan:
        // 1. Throw an object of `bsl::bad_function_call` and confirm that it
        //    is caught by the correct handler (C-1).
        //
        // 2. Throw an object of `bsl::bad_function_call` and confirm that it
        //    is caught by a handler for `std::exception` before a handler for
        //    `bsl::bad_function_call` (C-2).
        //
        // 3. Throw an object with an ambiguous base class of `std::exception`
        //    to demonstrate that it will instead be caught by a later handler
        //    for an unambiguous base.  Note that this is testing the test
        //    idiom to complete the proof.
        //
        // Testing:
        //   CONCERN: `bsl::bad_function_call` can be thrown
        // --------------------------------------------------------------------
        if (verbose) puts("\n`bsl::bad_function_call` IS THROWABLE"
                          "\n======================================");

        bool correctCatch = false;

        // Show that we can throw and catch a `bsl::bad_function_call` object.
        try
        {
            throw bsl::bad_function_call();
        }
        catch (const bsl::bad_function_call&)
        {
            correctCatch = true;
        }
        catch (...)
        {
            correctCatch = false;
        }

        ASSERT(correctCatch);

#if defined(BSLS_PLATFORM_HAS_PRAGMA_GCC_DIAGNOSTIC)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wpragmas"
# pragma GCC diagnostic ignored "-Wexceptions"
#elif defined(BSLS_PLATFORM_CMP_MSVC)
// Warning C4286 reports that a later `catch` clause is unreachable because
// an earlier one for a base type catches first; that arrangement is the
// property under test here.
# pragma warning(push)
# pragma warning(disable: 4286)
#endif
        // Show that `std::exception` is not an ambiguous base.
        correctCatch = false;
        try
        {
            throw bsl::bad_function_call();
        }
        catch (const std::exception&)
        {
            correctCatch = true;
        }
        catch (const bsl::bad_function_call&)
        {
            correctCatch = false;
        }
        catch (...)
        {
            correctCatch = false;
        }

        ASSERT(correctCatch);

        // Check the ambiguous case to confirm the test idiom.
        correctCatch = false;
        try
        {
            throw bsl::ambiguous_exception();
        }
        catch (const std::exception&)
        {
            correctCatch = false;
        }
        catch (const bsl::bad_function_call&)
        {
            correctCatch = true;
        }
        catch (...)
        {
            correctCatch = false;
        }
#if defined(BSLS_PLATFORM_HAS_PRAGMA_GCC_DIAGNOSTIC)
# pragma GCC diagnostic pop
#elif defined(BSLS_PLATFORM_CMP_MSVC)
# pragma warning(pop)
#endif

        ASSERT(correctCatch);
      } break;
      case 3: {
        // --------------------------------------------------------------------
        // DEFAULT CONSTRUCTION AND `what` METHOD
        //
        // Concerns:
        // 1. A `bad_function_call` object is default constructible.
        //
        // 2. Invoking the `what` method on a `bad_function_call` object
        //    returns a "bad_function_call" string in C++03, or the
        //    implementation-defined string from `std::bad_function_call` in
        //    C++11 or later.
        //
        // 3. The `what` method can be invoked on a const `bad_function_call`
        //    object.
        //
        // Plan:
        // 1. For concern 1, default construct a `bad_function_call` object.
        //
        // 2. For concern 2, invoke the `what` method on a `bad_function_call`
        //    object and check that the returned string is "bad_function_call"
        //    in C++03, or not an empty string in C++11 or later.
        //
        // 3. For concern 3, in step 2, use a const qualified
        //    `bad_function_call` object.
        //
        // Testing:
        //   bad_function_call();
        //   const char *what() const;
        // --------------------------------------------------------------------
        if (verbose) puts("\nDEFAULT CONSTRUCTION AND `what` METHOD"
                          "\n======================================");
        const bsl::bad_function_call b;
        const char *message = b.what();
#if !BSLS_COMPILERFEATURES_FULL_CPP11
        // The string returned by `what()` method is implementation specific
        // so we can only check our own implementation
        ASSERTV(message,
                0 != message && 0 == strcmp("bad_function_call", message));
#else
        ASSERTV(message, 0 != message && 0 != strlen(message));
#endif  // !BSLS_COMPILERFEATURES_FULL_CPP11

      } break;
      case 2: {
        // --------------------------------------------------------------------
        // CONCERN: `bsl::bad_function_call` is `std::bad_function_call`
        //
        // Concerns:
        // 1. If compiling as C++11 or later, `bsl::bad_function_call` is
        //    `std::bad_function_call`; there is no `std::bad_function_call` in
        //    C++03.
        //
        // 2. `bsl::bad_function_call` is associated with namespace `std`.
        //
        // 3. `bsl::bad_function_call` is associated with namespace `bsl` only
        //    if the class is defined there, i.e., in C++03.
        //
        // Plan:
        // 1. If compiling as C++11 or later, using the `bsl::is_same` type
        //    trait confirm that `bsl::bad_function_call` is the same type as
        //    `bsl::bad_function_call`.  In C++03 there is nothing to test.
        //
        // 2. Call the `std::swap` function with a `bsl::bad_function_call`
        //    object, where the name is not found by ordinary name lookup and
        //    only via ADL.
        //
        // 3. (Compile fail -- disable except for manual builds)
        //    To demonstrate that `swap` is found only using ADL, try to `swap`
        //    two `int` variables that have no namespace association.  This
        //    test driver should fail to compile.
        //
        // 4. Create a function in namespace `bsl` that does not match the name
        //    of anything in namespace `std`, and attempt to call it.  The call
        //    will compile only in C++03.
        //
        // 5. Hiding code behind the `BSLSTL_BADFUNCTIONCALL_SHOW_NO_ADL`
        //    macro, call the functions that are ADL tested in the other C++
        //    branch relying on only ADL to show that they fail to compile.
        //
        // Testing:
        //   CONCERN: `bsl::bad_function_call` is `std::bad_function_call`
        // --------------------------------------------------------------------

        if (verbose)
            puts(
            "\nCONCERN: `bsl::bad_function_call` is `std::bad_function_call`"
            "\n=============================================================");

#if BSLS_COMPILERFEATURES_FULL_CPP11
        ASSERT(
        (bsl::is_same<bsl::bad_function_call, std::bad_function_call>::value));

        // Do namespace association tests.
        bsl::bad_function_call obj{};
        swap(obj, obj);

# ifdef BSLSTL_BADFUNCTIONCALL_SHOW_NO_ADL
        int x = 0;
        swap(x, x);

        bslstl_badfunctioncall_show_ADL(obj);
# endif
#else
        // There is no `std::bad_function_call` in C++03.

        // Do namespace association tests.
        bsl::bad_function_call obj = bsl::bad_function_call();
        swap(obj, obj);

        bslstl_badfunctioncall_show_ADL(obj);

# ifdef BSLSTL_BADFUNCTIONCALL_SHOW_NO_ADL
        int x = 0;
        swap(x, x);
# endif
#endif  // BSLS_COMPILERFEATURES_FULL_CPP11
      } break;
      case 1: {
        // --------------------------------------------------------------------
        // BREATHING TEST
        //   This case exercises (but does not fully test) basic functionality.
        //
        // Concerns:
        // 1. The class is sufficiently functional to enable comprehensive
        //    testing in subsequent test cases.
        //
        // Plan:
        // 1. Perform an ad-hoc test of the primary modifiers and accessors.
        //
        // Testing:
        //   BREATHING TEST
        // --------------------------------------------------------------------

        if (verbose) puts("\nBREATHING TEST\n==============");
        bsl::bad_function_call b;
        const std::exception *ptr = &b;

        ASSERT(0 != b.what());
        ASSERT(0 != ptr->what());
      } break;
      default: {
        fprintf(stderr, "WARNING: CASE `%d' NOT FOUND.\n", test);
        testStatus = -1;
      }
    }

    // CONCERN: In no case does memory come from the default allocator.
    ASSERTV(defaultAllocator.numBlocksTotal(),
            0 == defaultAllocator.numBlocksTotal());

    // CONCERN: In no case does memory come from the global allocator.
    ASSERTV(globalAllocator.numBlocksTotal(),
            0 == globalAllocator.numBlocksTotal());

    if (testStatus > 0) {
        fprintf(stderr, "Error, non-zero test status = %d.\n", testStatus);
    }
    return testStatus;
}
#endif  // BDE_BUILD_TARGET_EXC

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
