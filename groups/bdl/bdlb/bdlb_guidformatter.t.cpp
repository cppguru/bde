// bdlb_guidformatter.t.cpp                                           -*-C++-*-
#include <bdlb_guidformatter.h>

#include <bdlb_guid.h>

#include <bslfmt_format.h>              // Testing only
#include <bslfmt_formattertestutil.h>   // Testing only

#include <bslim_testutil.h>

#include <bslma_default.h>
#include <bslma_defaultallocatorguard.h>
#include <bslma_testallocator.h>

#include <bsls_review.h>

#include <bsl_cstdio.h>
#include <bsl_cstdlib.h>
#include <bsl_string.h>

using namespace BloombergLP;

// ============================================================================
//                             TEST PLAN
// ----------------------------------------------------------------------------
//                              Overview
//                              --------
// This component provides a `bsl::formatter` specialization for `bdlb::Guid`
// via the private implementation class template
// `bdlb::Guid_BslFmtFormatterImpl<t_CHAR>`.  The specialization supports
// only the `[fill][align][width]` portion of the standard format-spec
// grammar; every other spec character is rejected at parse time.
//
// The tests exercise the two publicly-observable operations of a formatter
// (`parse` and `format`) using `bslfmt::FormatterTestUtil` to drive them
// against both `bsl::format`, `bsl::vformat`, and (where available) the
// oracle standard library implementation.
// ----------------------------------------------------------------------------
// CLASS METHODS
// [ 2] BSLFMT FORMAT STRING PARSING
// [ 3] BSLFMT FORMATTING
// ----------------------------------------------------------------------------
// [ 1] BREATHING TEST
// [ 4] USAGE EXAMPLE

// ============================================================================
//                     STANDARD BDE ASSERT TEST FUNCTION
// ----------------------------------------------------------------------------

namespace {

int testStatus = 0;

void aSsErT(bool condition, const char *message, int line)
{
    if (condition) {
        bsl::printf("Error " __FILE__ "(%d): %s    (failed)\n", line, message);
        if (0 <= testStatus && testStatus <= 100) {
            ++testStatus;
        }
    }
}

}  // close unnamed namespace

// ============================================================================
//               STANDARD BDE TEST DRIVER MACRO ABBREVIATIONS
// ----------------------------------------------------------------------------

#define ASSERT       BSLIM_TESTUTIL_ASSERT
#define ASSERTV      BSLIM_TESTUTIL_ASSERTV

#define LOOP_ASSERT  BSLIM_TESTUTIL_LOOP_ASSERT
#define LOOP2_ASSERT BSLIM_TESTUTIL_LOOP2_ASSERT
#define LOOP3_ASSERT BSLIM_TESTUTIL_LOOP3_ASSERT

#define Q            BSLIM_TESTUTIL_Q   // Quote identifier literally.
#define P            BSLIM_TESTUTIL_P   // Print identifier and value.
#define P_           BSLIM_TESTUTIL_P_  // P(X) without '\n'.
#define T_           BSLIM_TESTUTIL_T_  // Print a tab (w/o newline).
#define L_           BSLIM_TESTUTIL_L_  // current Line number

// ============================================================================
//                              TEST GLOBALS
// ----------------------------------------------------------------------------

static bool             verbose;
static bool         veryVerbose;
static bool     veryVeryVerbose;
static bool veryVeryVeryVerbose;

// ============================================================================
//                                TEST DATA
// ----------------------------------------------------------------------------

namespace {

// A small selection of GUIDs and their canonical formatted representations.
static const struct {
    int                 d_line;
    const unsigned char d_bytes[bdlb::Guid::k_GUID_NUM_BYTES];
    const char         *d_expected;
} k_GUID_DATA[] = {
    { L_, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
       "00000000-0000-0000-0000-000000000000" },

    { L_, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
       "ffffffff-ffff-ffff-ffff-ffffffffffff" },

    { L_, { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
            0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10 },
       "01234567-89ab-cdef-fedc-ba9876543210" },

    { L_, { 0x5c, 0x9d, 0x4e, 0x53, 0x0d, 0xf1, 0x11, 0xe4,
            0x91, 0x91, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66 },
       "5c9d4e53-0df1-11e4-9191-0800200c9a66" },
};

const size_t k_NUM_GUID_DATA = sizeof k_GUID_DATA / sizeof *k_GUID_DATA;

}  // close unnamed namespace

// ============================================================================
//               HELPER FUNCTIONS FOR BSLFMT FORMATTER TESTING
// ----------------------------------------------------------------------------

namespace {

/// Verify that `bsl::formatter<bdlb::Guid, char>::parse` accepts the
/// specified `format` string (created at runtime).  Report failure with the
/// specified `line` for diagnostic purposes.  Return `true` on success.
bool testRuntimeCharParse(int line, const char *format)
{
    bsl::string message;

    const bool rv =
        bslfmt::FormatterTestUtil<char>::testParseVFormat<bdlb::Guid>(
                                                                     &message,
                                                                     false,
                                                                     format);
    ASSERTV(line, message.c_str(), rv);

    return rv;
}

/// Verify that `bsl::formatter<bdlb::Guid, wchar_t>::parse` accepts the
/// specified `format` string (created at runtime).  Report failure with the
/// specified `line` for diagnostic purposes.  Return `true` on success.
bool testRuntimeWcharParse(int line, const wchar_t *format)
{
    bsl::string message;

    const bool rv =
        bslfmt::FormatterTestUtil<wchar_t>::testParseVFormat<bdlb::Guid>(
                                                                     &message,
                                                                     false,
                                                                     format);
    ASSERTV(line, message.c_str(), rv);

    return rv;
}

/// Verify that formatting the specified `value` with the specified `format`
/// (using `bslfmt`) yields the specified `expected` result.  Report failure
/// with the specified `line` for diagnostic purposes.  Return `true` on
/// success.
template <class t_CHAR>
bool testRuntimeFormat(int               line,
                       const t_CHAR     *expected,
                       const t_CHAR     *format,
                       const bdlb::Guid& value)
{
    bsl::string message;
    int         dummyArg = 0;

    // Note that we pass extra `dummyArg` arguments so that the same
    // three-argument overload of `testEvaluateVFormat` is instantiated for
    // both plain and nested-width test rows.
    const bool rv = bslfmt::FormatterTestUtil<t_CHAR>::testEvaluateVFormat(
                                                                     &message,
                                                                     expected,
                                                                     false,
                                                                     format,
                                                                     value,
                                                                     dummyArg,
                                                                     dummyArg);
    ASSERTV(line, message.c_str(), rv);

    return rv;
}

/// Verify that formatting the specified `value` with the specified `format`
/// and the specified nested `arg` (using `bslfmt`) yields the specified
/// `expected` result.  Report failure with the specified `line` for
/// diagnostic purposes.  Return `true` on success.
template <class t_CHAR>
bool testRuntimeFormat(int               line,
                       const t_CHAR     *expected,
                       const t_CHAR     *format,
                       const bdlb::Guid& value,
                       int               arg)
{
    bsl::string message;
    int         dummyArg = 0;

    const bool rv = bslfmt::FormatterTestUtil<t_CHAR>::testEvaluateVFormat(
                                                                     &message,
                                                                     expected,
                                                                     false,
                                                                     format,
                                                                     value,
                                                                     arg,
                                                                     dummyArg);
    ASSERTV(line, message.c_str(), rv);

    return rv;
}

}  // close unnamed namespace

// ============================================================================
//                                USAGE EXAMPLE
// ----------------------------------------------------------------------------

namespace UsageExample {

///Example 1: Formatting a `bdlb::Guid`
/// - - - - - - - - - - - - - - - - - -
// Suppose we have a `bdlb::Guid` object and want to render it as a string:
// ```
    void formatOneGuid()
    {
        const unsigned char raw[bdlb::Guid::k_GUID_NUM_BYTES] = {
            0x5c, 0x9d, 0x4e, 0x53, 0x0d, 0xf1, 0x11, 0xe4,
            0x91, 0x91, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66 };
        bdlb::Guid guid(raw);

        bsl::string s = bsl::format("{}", guid);
        ASSERT(s == "5c9d4e53-0df1-11e4-9191-0800200c9a66");
// ```
// Width, alignment, and fill are supported as they would be for a string:
// ```
        bsl::string padded = bsl::format("{:*^40}", guid);
        ASSERT(padded == "**5c9d4e53-0df1-11e4-9191-0800200c9a66**");
    }
// ```

}  // close namespace UsageExample

// ============================================================================
//                                TEST DRIVER
// ----------------------------------------------------------------------------

namespace {

struct TestDriver {
    static void testCase4();
    static void testCase3();
    static void testCase2();
    static void testCase1();
};

}  // close unnamed namespace

//-----------------------------------------------------------------------------
//                              testCase4
//-----------------------------------------------------------------------------

void TestDriver::testCase4()
{
    // ------------------------------------------------------------------------
    // USAGE EXAMPLE
    //   Extracted from component header file.
    //
    // Concerns:
    // 1. The usage example provided in the component header file compiles,
    //    links, and runs as shown.
    //
    // Plan:
    // 1. Incorporate usage example from header into test driver, remove
    //    leading comment characters, and replace `assert` with `ASSERT`.
    //    (C-1)
    //
    // Testing:
    //   USAGE EXAMPLE
    // ------------------------------------------------------------------------

    if (verbose) bsl::puts("\nUSAGE EXAMPLE"
                           "\n=============");

    UsageExample::formatOneGuid();
}

//-----------------------------------------------------------------------------
//                              testCase3
//-----------------------------------------------------------------------------

void TestDriver::testCase3()
{
    // ------------------------------------------------------------------------
    // BSLFMT FORMATTING
    //
    // Concerns:
    // 1. Formatting produces the canonical
    //    `xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx` representation using lowercase
    //    hexadecimal digits.
    //
    // 2. When no width is specified (or width is not greater than the
    //    36-character content length), no padding is inserted.
    //
    // 3. Alignment, fill, and width (both literal and nested) work as they
    //    do for string-like types.  The default alignment is left.
    //
    // 4. The formatter behaves identically for `char` and `wchar_t` output.
    //
    // 5. A single formatter object may be reused across multiple `format`
    //    invocations with different nested-width argument values without
    //    the earlier postprocessed width leaking into the later call.
    //
    // Plan:
    // 1. For each GUID in `k_GUID_DATA` and each entry in a table of
    //    (format, expected) pairs, invoke `testRuntimeFormat` for both
    //    `char` and `wchar_t`.  (C-1..4)
    //
    // 2. Directly invoke `bsl::format` on a single formatter used twice with
    //    different width values and confirm both results are correct.  (C-5)
    //
    // Testing:
    //   CONCERN: BSLFMT FORMATTING
    // ------------------------------------------------------------------------

    if (verbose) bsl::puts("\nBSLFMT FORMATTING"
                           "\n=================");

    // Format-string / prefix-suffix pairs.
    static const struct {
        int            d_line;
        const char    *d_format;
        const wchar_t *d_wformat;
        const char    *d_prefix;
        const char    *d_suffix;
    } SPECS[] = {
        // line  format         wformat         prefix            suffix
        // ----  ------         -------         ------            ------
        { L_,     "{}",           L"{}",           "",                ""     },
        { L_,     "{:}",          L"{:}",          "",                ""     },

        // Width no larger than content length -> no padding.
        { L_,     "{:36}",        L"{:36}",        "",                ""     },
        { L_,     "{:30}",        L"{:30}",        "",                ""     },

        // Literal width with default alignment (left).
        { L_,     "{:40}",        L"{:40}",        "",                "    " },

        // Literal width with explicit alignment.
        { L_,     "{:<40}",       L"{:<40}",       "",                "    " },
        { L_,     "{:>40}",       L"{:>40}",       "    ",            ""     },
        { L_,     "{:^40}",       L"{:^40}",       "  ",              "  "   },

        // Fill character with each alignment.
        { L_,     "{:*<40}",      L"{:*<40}",      "",                "****" },
        { L_,     "{:*>40}",      L"{:*>40}",      "****",            ""     },
        { L_,     "{:*^40}",      L"{:*^40}",      "**",              "**"   },
        { L_,     "{:+<40}",      L"{:+<40}",      "",                "++++" },
        { L_,     "{:0>40}",      L"{:0>40}",      "0000",            ""     },

        // Nested widths.
        { L_,     "{:{}}",        L"{:{}}",        "",                "    " },
        { L_,     "{:<{}}",       L"{:<{}}",       "",                "    " },
        { L_,     "{:>{}}",       L"{:>{}}",       "    ",            ""     },
        { L_,     "{:^{}}",       L"{:^{}}",       "  ",              "  "   },
        { L_,     "{:*^{}}",      L"{:*^{}}",      "**",              "**"   },

        // Positional target with nested width.
        { L_,     "{0:>{1}}",     L"{0:>{1}}",     "    ",            ""     },
    };
    const size_t NUM_SPECS = sizeof SPECS / sizeof *SPECS;

    for (size_t gi = 0; gi < k_NUM_GUID_DATA; ++gi) {
        const int             GLINE      = k_GUID_DATA[gi].d_line;
        const bdlb::Guid      GUID(k_GUID_DATA[gi].d_bytes);
        const char *const     RENDER     = k_GUID_DATA[gi].d_expected;

        if (veryVerbose) { P_(GLINE) P(RENDER); }

        for (size_t si = 0; si < NUM_SPECS; ++si) {
            const int            SLINE   = SPECS[si].d_line;
            const char *const    FMT     = SPECS[si].d_format;
            const wchar_t *const WFMT    = SPECS[si].d_wformat;
            const char *const    PREFIX  = SPECS[si].d_prefix;
            const char *const    SUFFIX  = SPECS[si].d_suffix;

            bsl::string  expected  = bsl::string(PREFIX) + RENDER + SUFFIX;
            bsl::wstring wexpected(expected.begin(), expected.end());
                                          // safe: ascii + guid hex + dashes

            // Parse-first sanity check (runtime).
            testRuntimeCharParse(SLINE, FMT);
            testRuntimeWcharParse(SLINE, WFMT);

            testRuntimeFormat(SLINE, expected.c_str(),  FMT,  GUID, 40);
            testRuntimeFormat(SLINE, wexpected.c_str(), WFMT, GUID, 40);
        }
    }

    if (veryVerbose) bsl::puts("\tFormatter reuse across contexts.");
    {
        // Confirm that a single formatter object (created implicitly by
        // `bsl::format`'s cache) can be invoked with different nested-width
        // arguments in successive calls, and each call postprocesses the
        // width freshly.  This would fail if the impl were postprocessing
        // `d_spec` in place instead of a local copy.
        const bdlb::Guid GUID(k_GUID_DATA[3].d_bytes);

        bsl::string s40 = bsl::format("{:>{}}", GUID, 40);
        bsl::string s50 = bsl::format("{:>{}}", GUID, 50);

        ASSERTV(s40.size(), 40u == s40.size());
        ASSERTV(s50.size(), 50u == s50.size());
        ASSERTV(s40, s40 == bsl::string("    ") +
                                     "5c9d4e53-0df1-11e4-9191-0800200c9a66");
        ASSERTV(s50, s50 == bsl::string("              ") +
                                     "5c9d4e53-0df1-11e4-9191-0800200c9a66");
    }
}

//-----------------------------------------------------------------------------
//                              testCase2
//-----------------------------------------------------------------------------

void TestDriver::testCase2()
{
    // ------------------------------------------------------------------------
    // BSLFMT FORMAT STRING PARSING
    //
    // Concerns:
    // 1. Format specifications that consist only of the supported grammar
    //    `[[fill]align][width]` parse successfully for both character types.
    //
    // 2. Format specifications that include any unsupported spec character
    //    (sign, `#`, `0`, precision, locale, type, etc.) or that are
    //    otherwise malformed generate a parse error for both character types.
    //
    // 3. Both compile-time (`testParseFormat`) and runtime (`testParseVFormat`)
    //    parse paths report the same disposition.
    //
    // Plan:
    // 1. Construct format specs representing each supported combination of
    //    fill/align/width (literal and nested) and verify parse success.
    //    (C-1, C-3)
    //
    // 2. Construct format specs representing each unsupported spec character
    //    and other malformations, and verify parse failure.  (C-2, C-3)
    //
    // Testing:
    //   CONCERN: BSLFMT FORMAT STRING PARSING
    // ------------------------------------------------------------------------

    if (verbose) bsl::puts("\nBSLFMT FORMAT STRING PARSING"
                           "\n============================");

#define TEST_PARSE_FAIL(charType, fmtStr, useOracle)                          \
    do {                                                                      \
        bsl::string errorMsg;                                                 \
        const bool rv = bslfmt::FormatterTestUtil<charType>::                 \
                testParseFailure<bdlb::Guid>(&errorMsg, useOracle, fmtStr);   \
        ASSERTV(L_, errorMsg.c_str(), rv);                                    \
    } while (false)

#define TEST_PARSE_SUCCESS_F(charType, fmtStr, useOracle)                     \
    do {                                                                      \
        bsl::string errorMsg;                                                 \
        const bool rv = bslfmt::FormatterTestUtil<charType>::                 \
                testParseFormat<bdlb::Guid>(&errorMsg, useOracle, fmtStr);    \
        ASSERTV(L_, errorMsg.c_str(), rv);                                    \
    } while (false)

#define TEST_PARSE_SUCCESS_VF(charType, fmtStr, useOracle)                    \
    do {                                                                      \
        bsl::string errorMsg;                                                 \
        const bool rv = bslfmt::FormatterTestUtil<charType>::                 \
                testParseVFormat<bdlb::Guid>(&errorMsg, useOracle, fmtStr);   \
        ASSERTV(L_, errorMsg.c_str(), rv);                                    \
    } while (false)

    if (veryVerbose) bsl::puts("\tParse failures.");

    // Bad fill character (`{` immediately after `:`).  Note that `}` cannot
    // be tested as it would close the parse string.
    TEST_PARSE_FAIL(char,     "{:{<40}",     false);
    TEST_PARSE_FAIL(wchar_t, L"{:{<40}",     false);

    // Fill character present with no alignment character following.
    TEST_PARSE_FAIL(char,     "{:*40}",      false);
    TEST_PARSE_FAIL(wchar_t, L"{:*40}",      false);

    // Non-numeric width.
    TEST_PARSE_FAIL(char,     "{:X}",        false);
    TEST_PARSE_FAIL(char,     "{:*<X}",      false);
    TEST_PARSE_FAIL(wchar_t, L"{:X}",        false);
    TEST_PARSE_FAIL(wchar_t, L"{:*<X}",      false);

    // Sign flag not supported.
    TEST_PARSE_FAIL(char,     "{:+}",        false);
    TEST_PARSE_FAIL(char,     "{:-}",        false);
    TEST_PARSE_FAIL(char,     "{: }",        false);
    TEST_PARSE_FAIL(wchar_t, L"{:+}",        false);
    TEST_PARSE_FAIL(wchar_t, L"{:-}",        false);
    TEST_PARSE_FAIL(wchar_t, L"{: }",        false);

    // Alternate-form flag (`#`) not supported.
    TEST_PARSE_FAIL(char,     "{:#}",        false);
    TEST_PARSE_FAIL(char,     "{:*<#40}",    false);
    TEST_PARSE_FAIL(wchar_t, L"{:#}",        false);
    TEST_PARSE_FAIL(wchar_t, L"{:*<#40}",    false);

    // Precision not supported (both literal and nested forms).
    TEST_PARSE_FAIL(char,     "{:.5}",       false);
    TEST_PARSE_FAIL(char,     "{:40.5}",     false);
    TEST_PARSE_FAIL(char,     "{:.{}}",      false);
    TEST_PARSE_FAIL(char,     "{:40.{}}",    false);
    TEST_PARSE_FAIL(wchar_t, L"{:.5}",       false);
    TEST_PARSE_FAIL(wchar_t, L"{:40.5}",     false);
    TEST_PARSE_FAIL(wchar_t, L"{:.{}}",      false);
    TEST_PARSE_FAIL(wchar_t, L"{:40.{}}",    false);

    // Locale flag not supported.
    TEST_PARSE_FAIL(char,     "{:L}",        false);
    TEST_PARSE_FAIL(char,     "{:40L}",      false);
    TEST_PARSE_FAIL(wchar_t, L"{:L}",        false);
    TEST_PARSE_FAIL(wchar_t, L"{:40L}",      false);

    // Type specifier not supported (this formatter has no type chars).
    TEST_PARSE_FAIL(char,     "{:x}",        false);
    TEST_PARSE_FAIL(char,     "{:X}",        false);
    TEST_PARSE_FAIL(char,     "{:s}",        false);
    TEST_PARSE_FAIL(char,     "{:40s}",      false);
    TEST_PARSE_FAIL(wchar_t, L"{:x}",        false);
    TEST_PARSE_FAIL(wchar_t, L"{:X}",        false);
    TEST_PARSE_FAIL(wchar_t, L"{:s}",        false);
    TEST_PARSE_FAIL(wchar_t, L"{:40s}",      false);

    if (veryVerbose) bsl::puts("\tParse successes (literal specs).");

    TEST_PARSE_SUCCESS_F(char,     "{}",             true);
    TEST_PARSE_SUCCESS_F(wchar_t, L"{}",             true);
    TEST_PARSE_SUCCESS_F(char,     "{:}",            true);
    TEST_PARSE_SUCCESS_F(wchar_t, L"{:}",            true);

    TEST_PARSE_SUCCESS_F(char,     "{:5}",           true);
    TEST_PARSE_SUCCESS_F(wchar_t, L"{:5}",           true);
    TEST_PARSE_SUCCESS_F(char,     "{:40}",          true);
    TEST_PARSE_SUCCESS_F(wchar_t, L"{:40}",          true);

    TEST_PARSE_SUCCESS_F(char,     "{:<}",           true);
    TEST_PARSE_SUCCESS_F(wchar_t, L"{:<}",           true);
    TEST_PARSE_SUCCESS_F(char,     "{:>}",           true);
    TEST_PARSE_SUCCESS_F(wchar_t, L"{:>}",           true);
    TEST_PARSE_SUCCESS_F(char,     "{:^}",           true);
    TEST_PARSE_SUCCESS_F(wchar_t, L"{:^}",           true);

    TEST_PARSE_SUCCESS_F(char,     "{:<40}",         true);
    TEST_PARSE_SUCCESS_F(wchar_t, L"{:<40}",         true);
    TEST_PARSE_SUCCESS_F(char,     "{:>40}",         true);
    TEST_PARSE_SUCCESS_F(wchar_t, L"{:>40}",         true);
    TEST_PARSE_SUCCESS_F(char,     "{:^40}",         true);
    TEST_PARSE_SUCCESS_F(wchar_t, L"{:^40}",         true);

    TEST_PARSE_SUCCESS_F(char,     "{:*<40}",        true);
    TEST_PARSE_SUCCESS_F(wchar_t, L"{:*<40}",        true);
    TEST_PARSE_SUCCESS_F(char,     "{:*>40}",        true);
    TEST_PARSE_SUCCESS_F(wchar_t, L"{:*>40}",        true);
    TEST_PARSE_SUCCESS_F(char,     "{:*^40}",        true);
    TEST_PARSE_SUCCESS_F(wchar_t, L"{:*^40}",        true);

    if (veryVerbose) bsl::puts("\tParse successes (nested-width specs).");

    TEST_PARSE_SUCCESS_F(char,     "{:{}}",          true);
    TEST_PARSE_SUCCESS_F(wchar_t, L"{:{}}",          true);
    TEST_PARSE_SUCCESS_F(char,     "{:<{}}",         true);
    TEST_PARSE_SUCCESS_F(wchar_t, L"{:<{}}",         true);
    TEST_PARSE_SUCCESS_F(char,     "{:>{}}",         true);
    TEST_PARSE_SUCCESS_F(wchar_t, L"{:>{}}",         true);
    TEST_PARSE_SUCCESS_F(char,     "{:^{}}",         true);
    TEST_PARSE_SUCCESS_F(wchar_t, L"{:^{}}",         true);
    TEST_PARSE_SUCCESS_F(char,     "{:*<{}}",        true);
    TEST_PARSE_SUCCESS_F(wchar_t, L"{:*<{}}",        true);
    TEST_PARSE_SUCCESS_F(char,     "{:*>{}}",        true);
    TEST_PARSE_SUCCESS_F(wchar_t, L"{:*>{}}",        true);
    TEST_PARSE_SUCCESS_F(char,     "{:*^{}}",        true);
    TEST_PARSE_SUCCESS_F(wchar_t, L"{:*^{}}",        true);

    TEST_PARSE_SUCCESS_F(char,     "{0:*^{1}}",      true);
    TEST_PARSE_SUCCESS_F(wchar_t, L"{0:*^{1}}",      true);

    if (veryVerbose) bsl::puts("\tParse successes via vformat.");

    TEST_PARSE_SUCCESS_VF(char,     "{}",            true);
    TEST_PARSE_SUCCESS_VF(wchar_t, L"{}",            true);
    TEST_PARSE_SUCCESS_VF(char,     "{:*^40}",       true);
    TEST_PARSE_SUCCESS_VF(wchar_t, L"{:*^40}",       true);
    TEST_PARSE_SUCCESS_VF(char,     "{:*^{}}",       true);
    TEST_PARSE_SUCCESS_VF(wchar_t, L"{:*^{}}",       true);

#undef TEST_PARSE_FAIL
#undef TEST_PARSE_SUCCESS_F
#undef TEST_PARSE_SUCCESS_VF
}

//-----------------------------------------------------------------------------
//                              testCase1
//-----------------------------------------------------------------------------

void TestDriver::testCase1()
{
    // ------------------------------------------------------------------------
    // BREATHING TEST
    //
    // Concerns:
    // 1. The `bsl::formatter` specialization for `bdlb::Guid` is instantiable
    //    for both supported character types and can be used with `bsl::format`
    //    to produce the canonical GUID string.
    //
    // Plan:
    // 1. Instantiate `bsl::formatter<bdlb::Guid, char>` and
    //    `bsl::formatter<bdlb::Guid, wchar_t>` and verify default construction
    //    works. (C-1)
    //
    // 2. Format a single sample `bdlb::Guid` object using `bsl::format` with
    //    the trivial `"{}"` format string and verify the result.  (C-1)
    //
    // Testing:
    //   BREATHING TEST
    // ------------------------------------------------------------------------

    if (verbose) bsl::puts("\nBREATHING TEST"
                           "\n==============");

    {
        bsl::formatter<bdlb::Guid, char>    dummy;   (void)dummy;
        bsl::formatter<bdlb::Guid, wchar_t> wdummy;  (void)wdummy;
    }

    const bdlb::Guid guid(k_GUID_DATA[3].d_bytes);
    const bsl::string s = bsl::format("{}", guid);
    ASSERTV(s, s == "5c9d4e53-0df1-11e4-9191-0800200c9a66");
}

// ============================================================================
//                                MAIN PROGRAM
// ----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    const int  test    =  argc > 1 ? bsl::atoi(argv[1]) : 0;
    verbose            =  argc > 2;
    veryVerbose        =  argc > 3;
    veryVeryVerbose    =  argc > 4;
    veryVeryVeryVerbose = argc > 5;

    bsl::printf("TEST %s CASE %d\n", __FILE__, test);

    // CONCERN: `BSLS_REVIEW` failures should lead to test failures.
    bsls::ReviewFailureHandlerGuard reviewGuard(&bsls::Review::failByAbort);

    // CONCERN: In no case does memory come from the global allocator.
    bslma::TestAllocator globalAllocator("global", veryVeryVeryVerbose);
    bslma::Default::setGlobalAllocator(&globalAllocator);

    // CONCERN: No memory leaks from the default allocator.
    bslma::TestAllocator defaultAllocator("default", veryVeryVeryVerbose);
    ASSERT(0 == bslma::Default::setDefaultAllocator(&defaultAllocator));

    // Confirm no static initialization locked the default allocator.
    ASSERT(&defaultAllocator == bslma::Default::defaultAllocator());

    switch (test) { case 0:
      case 4: { TestDriver::testCase4(); } break;
      case 3: { TestDriver::testCase3(); } break;
      case 2: { TestDriver::testCase2(); } break;
      case 1: { TestDriver::testCase1(); } break;
      default: {
        bsl::fprintf(stderr, "WARNING: CASE `%d' NOT FOUND.\n", test);
        testStatus = -1;
      }
    }

    // CONCERN: No memory leaks from the default allocator.
    ASSERTV(defaultAllocator.numBlocksInUse(),
            0 == defaultAllocator.numBlocksInUse());

    // CONCERN: In no case does memory come from the global allocator.
    ASSERTV(globalAllocator.numBlocksTotal(),
            0 == globalAllocator.numBlocksTotal());

    if (testStatus > 0) {
        bsl::fprintf(stderr,
                     "Error, non-zero test status = %d.\n",
                     testStatus);
    }

    return testStatus;
}

// ----------------------------------------------------------------------------
// Copyright 2026 Bloomberg Finance L.P.
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
