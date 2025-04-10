// bsls_fuzztestpreconditionexception.h                               -*-C++-*-
#ifndef INCLUDED_BSLS_FUZZTESTPRECONDITIONEXCEPTION
#define INCLUDED_BSLS_FUZZTESTPRECONDITIONEXCEPTION

#include <bsls_ident.h>
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide an exception type for handling failed preconditions.
//
//@CLASSES:
//  bsls::FuzzTestPreconditionException: type describing a failed precondition
//
//@SEE_ALSO: bsls_fuzztest
//
//@DESCRIPTION: This component implements an exception class,
// `bsls::FuzzTestPreconditionException`, that provides a mechanism to convey
// context information from a failing precondition to a test handler.  The
// context that is captured consists of the program source of the failing
// expression, the name of the file containing the assertion/review, the line
// number within that file where the asserted/reviewed expression may be found,
// and the level of the assertion/review that has failed.
//
///Usage
///-----
// First we write a macro to act as a precondition testing `assert` facility
// that will throw an exception of type `bsls::FuzzTestPreconditionException`
// if the asserted expression fails.  The thrown exception will capture the
// source-code of the expression, the filename and line number of the failing
// expression.
// ```
// #define TEST_PRECONDITION(EXPRESSION)                                    \$
//     if (!(EXPRESSION)) {                                                 \$
//         throw bsls::FuzzTestPreconditionException(#EXPRESSION, __FILE__, \$
//                                                   __LINE__, "LEVEL",     \$
//                                                   false);                \$
//     }
// ```
// Next we use the macro inside a try-block, so that we can catch the exception
// thrown if the tested expression fails.
// ```
//     try {
//         void *p = NULL;
//         TEST_PRECONDITION(0 != p);
//     }
// ```
// If the assertion fails, catch the exception and confirm that it correctly
// recorded the context of where the assertion failed.
// ```
//     catch (const bsls::FuzzTestPreconditionException& exception) {
//         assert(0     == strcmp("0 != p", exception.expression()));
//         assert(0     == strcmp(__FILE__, exception.filename()));
//         assert(9     == __LINE__ - exception.lineNumber());
//         assert(0     == strcmp("LEVEL",  exception.level()));
//         assert(false == exception.isReview());
//     }
// ```

#include <bsls_assert.h>
#include <bsls_compilerfeatures.h>
#include <bsls_keyword.h>
#include <bsls_review.h>

namespace BloombergLP {

namespace bsls {

                         // ===================================
                         // class FuzzTestPreconditionException
                         // ===================================

/// This class is an implementation detail of the `bsls` fuzz testing
/// framework and should not be used directly in user code.  It implements
/// an immutable mechanism to communicate to a test-case handler information
/// about the context of a precondition that fails.
class FuzzTestPreconditionException {

  private:
    // DATA
    const char *d_expression;  // expression that failed to assert as `true`
    const char *d_filename;    // name of file where the assert failed
    const bool  d_isReview;    // flag indicating if the failure is a review
    const char *d_level;       // level of failed assertion or review
    const int   d_lineNumber;  // line number in file where the assert failed

  private:
    // NOT IMPLEMENTED
    FuzzTestPreconditionException& operator=(
                    const FuzzTestPreconditionException&) BSLS_KEYWORD_DELETED;

  public:
    // CREATORS

    /// Create a `FuzzTestPreconditionException` object with the specified
    /// `expression`, `filename`, `lineNumber`, `level`, and `isReview`. The
    /// behavior is undefined unless `0 < line` and all of `expression`,
    /// `filename`, and `level` point to valid null-terminated character
    /// strings that will remain unmodified for the lifetime of this object
    /// (e.g., string literals).
    explicit BSLS_KEYWORD_CONSTEXPR
    FuzzTestPreconditionException(const char *expression,
                                  const char *filename,
                                  int         lineNumber,
                                  const char *level = "UNKNOWN",
                                  const bool  isReview = false);

#ifdef BSLS_COMPILERFEATURES_SUPPORT_DEFAULTED_FUNCTIONS
    // To avoid warnings about future incompatibility due to the deleted copy
    // assignment operator we declare the copy constructor as implicitly
    // generated.  For consistency the destructor was also placed here and
    // declared to be explicitly generated.

    /// Create a `FuzzTestPreconditionException` object that is a copy of
    /// the specified `original`, having the same values for the
    /// `expression`, `filename`, `lineNumber`, `level`, and `isReview`
    /// attributes.  Note that this trivial constructor's definition is
    /// compiler generated.
    FuzzTestPreconditionException(
                      const FuzzTestPreconditionException& original) = default;

    /// Destroy this object.  Note that this trivial destructor's definition
    /// is compiler generated.
    ~FuzzTestPreconditionException() = default;
#endif

    // ACCESSORS

    /// Return a string containing the program source of the assertion that has
    /// failed.
    const char *expression() const;

    /// Return a string containing the filename of the source file containing
    /// the assertion that has failed.
    const char *filename() const;

    /// Return a string containing a representation of the level of assertion
    /// or review macro that failed.
    const char *level() const;

    /// Return a flag indicating if the failure is a review.
    bool isReview() const;

    /// Return the number of the line within the file `filename` containing the
    /// assertion that failed.
    int lineNumber() const;
};

// ============================================================================
//                      INLINE FUNCTION DEFINITIONS
// ============================================================================

                    // ===================================
                    // class FuzzTestPreconditionException
                    // ===================================

// CREATORS
BSLS_KEYWORD_CONSTEXPR
inline
FuzzTestPreconditionException::FuzzTestPreconditionException(
                                        const char *expression,
                                        const char *filename,
                                        const int   lineNumber,
                                        const char *level,
                                        const bool  isReview)
: d_expression(expression)
, d_filename(filename)
, d_isReview(isReview)
, d_level(level)
, d_lineNumber(lineNumber)
{
}

// ACCESSORS
inline
const char *FuzzTestPreconditionException::expression() const
{
    return d_expression;
}

inline
const char *FuzzTestPreconditionException::filename() const
{
    return d_filename;
}

inline
const char *FuzzTestPreconditionException::level() const
{
    return d_level;
}

inline
bool  FuzzTestPreconditionException::isReview() const
{
    return d_isReview;
}

inline
int FuzzTestPreconditionException::lineNumber() const
{
    return d_lineNumber;
}

}  // close package namespace
}  // close enterprise namespace

#endif

// ----------------------------------------------------------------------------
// Copyright 2022 Bloomberg Finance L.P.
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
