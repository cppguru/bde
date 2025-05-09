// bslstl_multimap.t.cpp                                              -*-C++-*-

#include <bsls_platform.h>

// the following suppresses warnings from `#include` inlined functions
#ifdef BSLS_PLATFORM_HAS_PRAGMA_GCC_DIAGNOSTIC
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif
#ifdef BSLS_PLATFORM_PRAGMA_GCC_DIAGNOSTIC_GCC
#pragma GCC diagnostic ignored "-Wstringop-overflow="
#endif

#include <bslstl_multimap.h>

#include <bslstl_iterator.h>
#include <bslstl_pair.h>

#include <bslalg_rangecompare.h>
#include <bslalg_scalarprimitives.h>

#include <bslma_allocator.h>
#include <bslma_default.h>
#include <bslma_defaultallocatorguard.h>
#include <bslma_destructorguard.h>
#include <bslma_mallocfreeallocator.h>
#include <bslma_testallocator.h>
#include <bslma_testallocatormonitor.h>
#include <bslma_usesbslmaallocator.h>

#include <bslmf_assert.h>
#include <bslmf_haspointersemantics.h>
#include <bslmf_issame.h>
#include <bslmf_nestedtraitdeclaration.h>

#include <bsls_alignmentutil.h>
#include <bsls_assert.h>
#include <bsls_asserttest.h>
#include <bsls_bsltestutil.h>
#include <bsls_compilerfeatures.h>
#include <bsls_libraryfeatures.h>
#include <bsls_nameof.h>
#include <bsls_objectbuffer.h>

#include <bsltf_allocargumenttype.h>
#include <bsltf_argumenttype.h>
#include <bsltf_convertiblevaluewrapper.h>
#include <bsltf_stdallocatoradaptor.h>
#include <bsltf_stdstatefulallocator.h>
#include <bsltf_stdtestallocator.h>
#include <bsltf_templatetestfacility.h>
#include <bsltf_testvaluesarray.h>

#include <algorithm>
#include <functional>

#if defined(BSLS_COMPILERFEATURES_SUPPORT_GENERALIZED_INITIALIZERS)
#include <initializer_list>
#endif

#include <limits.h>

#if defined(BSLS_LIBRARYFEATURES_HAS_CPP11_BASELINE_LIBRARY)
#include <random>
#endif

#include <stdio.h>       // stdout
#include <stdlib.h>      // atoi

// ============================================================================
//                             TEST PLAN
// ----------------------------------------------------------------------------
// NOTICE: To reduce the compilation time, this test driver has been broken
// into 2 parts, `bslstl_multimap.t.cpp` (cases 1-8, plus the usage example),
// and `bslstl_multimap_test.cpp` (cases 9 and higher).
//
//                             Overview
//                             --------
// The object under test is a container whose interface and contract is
// dictated by the C++ standard.  The general concerns are compliance,
// exception safety, and proper dispatching (for member function templates such
// as assign and insert).  This container is implemented in the form of a class
// template, and thus its proper instantiation for several types is a concern.
// Regarding the allocator template argument, we use mostly a `bsl::allocator`
// together with a `bslma::TestAllocator` mechanism, but we also verify the C++
// standard.
//
// Primary Manipulators:
//  - `insert`  (via the `primaryManipulator` helper function)
//  - `clear`
//
// Basic Accessors:
//  - `cbegin`
//  - `cend`
//  - `size`
//  - `get_allocator`
//
// This test plan follows the standard approach for components implementing
// value-semantic containers.  We have chosen as *primary* *manipulators* the
// `insert` and `clear` methods to be used by the generator functions `g` and
// `gg`.  Note that some manipulators must support aliasing, and those that
// perform memory allocation must be tested for exception neutrality via the
// `bslma_testallocator` component.  After the mandatory sequence of cases
// (1--10) for value-semantic types (cases 5 and 10 are not implemented, as
// there is not output or streaming below bslstl), we test each individual
// constructor, manipulator, and accessor in subsequent cases.
//
// ----------------------------------------------------------------------------
// 23.4.6.2, construct/copy/destroy:
// [19] multimap(const C& comparator, const A& allocator);
// [12] multimap(ITER first, ITER last, const C& comp, const A& alloc);
// [12] multimap(ITER first, ITER last, const A& alloc);
// [32] multimap(initializer_list<value_type>, const C& comp, const A& alloc);
// [32] multimap(initializer_list<value_type>, const A& alloc);
// [ 7] multimap(const multimap& original);
// [26] multimap(multimap&& original);
// [ 2] explicit multimap(const A& allocator);
// [ 7] multimap(const multimap& original, const A& allocator);
// [26] multimap(multimap&&, const ALLOCATOR&);
// [ 2] ~multimap();
// [ 9] multimap& operator=(const multimap& rhs);
// [27] multimap& operator=(multimap&& rhs);
// [32] multimap& operator=(initializer_list<value_type>);
// [ 4] allocator_type get_allocator() const;
//
// iterators:
// [14] iterator begin();
// [14] const_iterator begin() const;
// [14] iterator end();
// [14] const_iterator end() const;
// [14] reverse_iterator rbegin();
// [14] const_reverse_iterator rbegin() const;
// [14] reverse_iterator rend();
// [14] const_reverse_iterator rend() const;
// [ 4] const_iterator cbegin() const;
// [ 4] const_iterator cend() const;
// [14] const_reverse_iterator crbegin() const;
// [14] const_reverse_iterator crend() const;
//
// capacity:
// [20] bool empty() const;
// [ 4] size_type size() const;
// [20] size_type max_size() const;
//
// modifiers:
// [15] iterator insert(const value_type& value);
// [28] iterator insert(value_type&& value);
// [16] iterator insert(const_iterator position, const value_type& value);
// [29] iterator insert(const_iterator position, value_type&& value);
// [17] void insert(INPUT_ITERATOR first, INPUT_ITERATOR last);
// [32] void insert(initializer_list<value_type>);
//
// [30] iterator emplace(Args&&... args);
// [31] iterator emplace_hint(const_iterator hint, Args&&... args);
//
// [18] iterator erase(const_iterator position);
// [18] iterator erase(const position);
// [18] size_type erase(const key_type& key);
// [18] iterator erase(const_iterator first, const_iterator last);
// [ 8] void swap(multimap& other);
// [ 2] void clear();
//
// observers:
// [21] key_compare key_comp() const;
// [21] value_compare value_comp() const;
//
// multimap operations:
// [13] bool contains(const key_type& key);
// [13] bool contains(const LOOKUP_KEY& key);
// [13] iterator find(const key_type& key);
// [13] const_iterator find(const key_type& key) const;
// [13] size_type count(const key_type& key) const;
// [13] iterator lower_bound(const key_type& key);
// [13] const_iterator lower_bound(const key_type& key) const;
// [13] iterator upper_bound(const key_type& key);
// [13] const_iterator upper_bound(const key_type& key) const;
// [13] bsl::pair<iterator, iterator> equal_range(const key_type& key);
// [13] bsl::pair<const_iter, const_iter> equal_range(const key_type&) const;
//
// [ 6] bool operator==(const multimap& lhs, const multimap& rhs);
// [19] bool operator< (const multimap& lhs, const multimap& rhs);
// [ 6] bool operator!=(const multimap& lhs, const multimap& rhs);
// [19] bool operator> (const multimap& lhs, const multimap& rhs);
// [19] bool operator>=(const multimap& lhs, const multimap& rhs);
// [19] bool operator<=(const multimap& lhs, const multimap& rhs);
// [19] auto operator<=>(const multimap& lhs, const multimap& rhs);
//
//// specialized algorithms:
// [ 8] void swap(multimap& a, multimap& b);
//
// ----------------------------------------------------------------------------
// [ 1] BREATHING TEST
// [40] USAGE EXAMPLE
//
// TEST APPARATUS: GENERATOR FUNCTIONS
// [ 3] int ggg(multimap *object, const char *spec, int verbose = 1);
// [ 3] multimap& gg(multimap *object, const char *spec);
//
// [22] CONCERN: `multimap` is compatible with standard allocators.
// [23] CONCERN: `multimap` has the necessary type traits.
// [24] CONCERN: Constructor of a template wrapper class compiles.
// [25] CONCERN: The type provides the full interface defined by the standard.
// [33] CONCERN: `multimap` supports incomplete types.
// [34] CONCERN: Methods qualifed `noexcept` in standard are so implemented.
// [35] CONCERN: `erase` overload is deduced correctly.
// [36] CONCERN: `find`        properly handles transparent comparators.
// [36] CONCERN: `count`       properly handles transparent comparators.
// [36] CONCERN: `lower_bound` properly handles transparent comparators.
// [36] CONCERN: `upper_bound` properly handles transparent comparators.
// [36] CONCERN: `equal_range` properly handles transparent comparators.
// [38] CONCERN: `multimap` IS A C++20 RANGE

// ============================================================================
//                      STANDARD BDE ASSERT TEST MACROS
// ----------------------------------------------------------------------------
// NOTE: THIS IS A LOW-LEVEL COMPONENT AND MAY NOT USE ANY C++ LIBRARY
// FUNCTIONS, INCLUDING IOSTREAMS.

namespace {

int testStatus = 0;

void aSsErT(bool b, const char *s, int i)
{
    if (b) {
        printf("Error " __FILE__ "(%d): %s    (failed)\n", i, s);
        fflush(stdout);
        if (testStatus >= 0 && testStatus <= 100) ++testStatus;
    }
}

}  // close unnamed namespace

//=============================================================================
//                       STANDARD BDE TEST DRIVER MACROS
//-----------------------------------------------------------------------------

#define ASSERT       BSLS_BSLTESTUTIL_ASSERT
#define LOOP_ASSERT  BSLS_BSLTESTUTIL_LOOP_ASSERT
#define LOOP0_ASSERT BSLS_BSLTESTUTIL_LOOP0_ASSERT
#define LOOP1_ASSERT BSLS_BSLTESTUTIL_LOOP1_ASSERT
#define LOOP2_ASSERT BSLS_BSLTESTUTIL_LOOP2_ASSERT
#define LOOP3_ASSERT BSLS_BSLTESTUTIL_LOOP3_ASSERT
#define LOOP4_ASSERT BSLS_BSLTESTUTIL_LOOP4_ASSERT
#define LOOP5_ASSERT BSLS_BSLTESTUTIL_LOOP5_ASSERT
#define LOOP6_ASSERT BSLS_BSLTESTUTIL_LOOP6_ASSERT
#define ASSERTV      BSLS_BSLTESTUTIL_ASSERTV

#define Q   BSLS_BSLTESTUTIL_Q   // Quote identifier literally.
#define P   BSLS_BSLTESTUTIL_P   // Print identifier and value.
#define P_  BSLS_BSLTESTUTIL_P_  // P(X) without '\n'.
#define T_  BSLS_BSLTESTUTIL_T_  // Print a tab (w/o newline).
#define L_  BSLS_BSLTESTUTIL_L_  // current Line number

#define RUN_EACH_TYPE BSLTF_TEMPLATETESTFACILITY_RUN_EACH_TYPE

// ============================================================================
//                  NEGATIVE-TEST MACRO ABBREVIATIONS
// ----------------------------------------------------------------------------

#define ASSERT_SAFE_PASS(EXPR) BSLS_ASSERTTEST_ASSERT_SAFE_PASS(EXPR)
#define ASSERT_SAFE_FAIL(EXPR) BSLS_ASSERTTEST_ASSERT_SAFE_FAIL(EXPR)
#define ASSERT_PASS(EXPR)      BSLS_ASSERTTEST_ASSERT_PASS(EXPR)
#define ASSERT_FAIL(EXPR)      BSLS_ASSERTTEST_ASSERT_FAIL(EXPR)
#define ASSERT_OPT_PASS(EXPR)  BSLS_ASSERTTEST_ASSERT_OPT_PASS(EXPR)
#define ASSERT_OPT_FAIL(EXPR)  BSLS_ASSERTTEST_ASSERT_OPT_FAIL(EXPR)

// ============================================================================
//                  PRINTF FORMAT MACRO ABBREVIATIONS
// ----------------------------------------------------------------------------

#define ZU BSLS_BSLTESTUTIL_FORMAT_ZU

// ============================================================================
//                      TEST CONFIGURATION MACROS
// ----------------------------------------------------------------------------

#if defined(BDE_BUILD_TARGET_EXC)
// The following enum is set to `1` when exceptions are enabled and to `0`
// otherwise.  It's here to avoid having preprocessor macros throughout.
enum { PLAT_EXC = 1 };
#else
enum { PLAT_EXC = 0 };
#endif

// ============================================================================
//                             SWAP TEST HELPERS
// ----------------------------------------------------------------------------

namespace incorrect {

/// Fail.  In a successful test, this `swap` should never be called.  It is
/// set up to be called (and fail) in the case where ADL fails to choose the
/// right `swap` in `invokeAdlSwap` below.
template <class TYPE>
void swap(TYPE&, TYPE&)
{
    ASSERT(0 && "incorrect swap called");
}

}  // close namespace incorrect

/// Exchange the values of the specified `*a` and `*b` objects using the
/// `swap` method found by ADL (Argument Dependent Lookup).
template <class TYPE>
void invokeAdlSwap(TYPE *a, TYPE *b)
{
    using incorrect::swap;

    // A correct ADL will key off the types of `*a` and `*b`, which will be of
    // our `bsl` container type, to find the right `bsl::swap` and not
    // `incorrect::swap`.

    swap(*a, *b);
}

/// Exchange the values of the specified `*a` and `*b` objects using the
/// `swap` method found by the recommended pattern for calling `swap`.
template <class TYPE>
void invokePatternSwap(TYPE *a, TYPE *b)
{
    // Invoke `swap` using the recommended pattern for `bsl` clients.

    using bsl::swap;

    swap(*a, *b);
}

// The following `using` directives must come *after* the definition of
// `invokeAdlSwap` and `invokePatternSwap` (above).

using namespace BloombergLP;
using bsl::pair;
using bsl::multimap;
using bsls::NameOf;

// ============================================================================
//                       GLOBAL TEST VALUES
// ----------------------------------------------------------------------------

static bool             verbose;
static bool         veryVerbose;
static bool     veryVeryVerbose;
static bool veryVeryVeryVerbose;

//=============================================================================
//                  GLOBAL TYPEDEFS/CONSTANTS FOR TESTING
//-----------------------------------------------------------------------------

// Define DEFAULT DATA used in multiple test cases.

struct DefaultDataRow {
    int         d_line;     // source line number
    int         d_index;    // lexical order
    const char *d_spec;     // specification string, for input to `gg` function
    const char *d_results;  // expected element values
};

static
const DefaultDataRow DEFAULT_DATA[] = {
    //line idx  spec                 results
    //---- ---  --------             -------
    { L_,    0, "",                  "" },
    { L_,    1, "A",                 "A" },
    { L_,    2, "AA",                "AA" },
    { L_,    4, "ABCA",              "AABC" },
    { L_,    5, "AB",                "AB" },
    { L_,    5, "BA",                "AB" },
    { L_,    7, "ABC",               "ABC" },
    { L_,    7, "ACB",               "ABC" },
    { L_,    7, "BAC",               "ABC" },
    { L_,    7, "BCA",               "ABC" },
    { L_,    7, "CAB",               "ABC" },
    { L_,    7, "CBA",               "ABC" },
    { L_,    6, "ABCB",              "ABBC" },
    { L_,    8, "ABCC",              "ABCC" },
    { L_,    3, "ABCABC",            "AABBCC" },
    { L_,    3, "AABBCC",            "AABBCC" },
    { L_,    9, "ABCD",              "ABCD" },
    { L_,    9, "ACBD",              "ABCD" },
    { L_,    9, "BDCA",              "ABCD" },
    { L_,    9, "DCBA",              "ABCD" },
    { L_,   10, "ABCDE",             "ABCDE" },
    { L_,   10, "ACBDE",             "ABCDE" },
    { L_,   10, "CEBDA",             "ABCDE" },
    { L_,   10, "EDCBA",             "ABCDE" },
    { L_,   11, "FEDCBA",            "ABCDEF" },
    { L_,   12, "ABCDEFG",           "ABCDEFG" },
    { L_,   13, "ABCDEFGH",          "ABCDEFGH" },
    { L_,   14, "ABCDEFGHI",         "ABCDEFGHI" },
    { L_,   15, "ABCDEFGHIJKLMNOP",  "ABCDEFGHIJKLMNOP" },
    { L_,   15, "PONMLKJIGHFEDCBA",  "ABCDEFGHIJKLMNOP" },
    { L_,   16, "ABCDEFGHIJKLMNOPQ", "ABCDEFGHIJKLMNOPQ" },
    { L_,   16, "DHBIMACOPELGFKNJQ", "ABCDEFGHIJKLMNOPQ" },
    { L_,   17, "BAD",               "ABD" },
    { L_,   18, "BEAD",              "ABDE" },
    { L_,   19, "AC",                "AC" },
    { L_,   20, "B",                 "B" },
    { L_,   21, "BCDE",              "BCDE" },
    { L_,   22, "FEDCB",             "BCDEF" },
    { L_,   23, "CD",                "CD" },
};
static const int DEFAULT_NUM_DATA = static_cast<int>(
                                   sizeof DEFAULT_DATA / sizeof *DEFAULT_DATA);

typedef bsltf::NonDefaultConstructibleTestType TestKeyType;
typedef bsltf::NonTypicalOverloadsTestType     TestValueType;

//=============================================================================
//                  GLOBAL HELPER FUNCTIONS FOR TESTING
//-----------------------------------------------------------------------------

// Fundamental-type-specific print functions.

namespace bsl {

template <class FIRST, class SECOND>
inline
void debugprint(const bsl::pair<FIRST, SECOND>& p)
{
    bsls::BslTestUtil::callDebugprint(p.first);
    bsls::BslTestUtil::callDebugprint(p.second);
}

// multimap-specific print function.
template <class KEY, class VALUE, class COMP, class ALLOC>
void debugprint(const bsl::multimap<KEY, VALUE, COMP, ALLOC>& s)
{
    if (s.empty()) {
        printf("<empty>");
    }
    else {
        typedef typename bsl::multimap<KEY, VALUE, COMP, ALLOC>::const_iterator
                                                                         CIter;
        putchar('"');
        for (CIter it = s.begin(); it != s.end(); ++it) {
            putchar(static_cast<char>(
                       bsltf::TemplateTestFacility::getIdentifier(it->first)));
        }
        putchar('"');
    }
    fflush(stdout);
}

}  // close namespace bsl


namespace {

bslma::TestAllocator *scratchSingleton()
{
    static bslma::TestAllocator scratch("scratch singleton",
                                        veryVeryVeryVerbose);

    return &scratch;
}

/// Verify the specified `container` has the specified `expectedSize` and
/// contains the same values as the array in the specified `expectedValues`.
/// Return 0 if `container` has the expected values, and a non-zero value
/// otherwise.
template<class CONTAINER, class VALUES>
size_t verifyContainer(const CONTAINER&  container,
                       const VALUES&     expectedValues,
                       size_t            expectedSize)
{
    ASSERTV(expectedSize, container.size(), expectedSize == container.size());

    if (expectedSize != container.size()) {
        return static_cast<size_t>(-1);                               // RETURN
    }

    typename CONTAINER::const_iterator it = container.cbegin();
    for (size_t i = 0; i < expectedSize; ++i) {
        ASSERTV(it != container.cend());
        ASSERTV(i, expectedValues[i], *it, expectedValues[i] == *it);

        if (bsltf::TemplateTestFacility::getIdentifier(expectedValues[i].first)
            != bsltf::TemplateTestFacility::getIdentifier(it->first)) {
            return i + 1;                                             // RETURN
        }
        ++it;
    }
    return 0;
}

                            // ==========================
                            // class StatefulStlAllocator
                            // ==========================

/// This class implements a standard compliant allocator that has an
/// attribute, `id`.
template <class VALUE>
class StatefulStlAllocator : public bsltf::StdTestAllocator<VALUE>
{
    // DATA
    int d_id;  // identifier

  private:
    // TYPES

    /// Alias for the base class.
    typedef bsltf::StdTestAllocator<VALUE> StlAlloc;

  public:
    /// This nested `struct` template, parameterized by some
    /// `BDE_OTHER_TYPE`, provides a namespace for an `other` type alias,
    /// which is an allocator type following the same template as this one
    /// but that allocates elements of `BDE_OTHER_TYPE`.  Note that this
    /// allocator type is convertible to and from `other` for any
    /// `BDE_OTHER_TYPE` including `void`.
    template <class BDE_OTHER_TYPE>
    struct rebind
    {

        typedef StatefulStlAllocator<BDE_OTHER_TYPE> other;
    };

    // CREATORS

    /// Create a `StatefulStlAllocator` object.
    StatefulStlAllocator()
    : StlAlloc()
    {
    }

    // StatefulStlAllocator(const StatefulStlAllocator& original) = default;
        // Create a `StatefulStlAllocator` object having the same id as the
        // specified `original`.

    /// Create a `StatefulStlAllocator` object having the same id as the
    /// specified `original` with a different template type.
    template <class BDE_OTHER_TYPE>
    StatefulStlAllocator(const StatefulStlAllocator<BDE_OTHER_TYPE>& original)
    : StlAlloc(original)
    , d_id(original.id())
    {
    }

    // MANIPULATORS

    /// Set the `id` attribute of this object to the specified `value`.
    void setId(int value)
    {
        d_id = value;
    }

    // ACCESSORS

    /// Return the value of the `id` attribute of this object.
    int id() const
    {
        return d_id;
    }
};

                            // ======================
                            // class ExceptionProctor
                            // ======================

/// This class provides a mechanism to verify the strong exception guarantee
/// in exception-throwing code.  On construction, this class stores a copy
/// of an object of the (template parameter) type `OBJECT` and the address
/// of that object.  On destruction, if `release` was not invoked, it will
/// verify the value of the object is the same as the value of the copy
/// created on construction.  This class requires that the copy constructor
/// and `operator ==` be tested before use.
template <class OBJECT>
struct ExceptionProctor {

    // DATA
    int           d_line;      // line number at construction
    OBJECT        d_control;   // copy of the object being proctored
    const OBJECT *d_object_p;  // address of the original object

  private:
    // NOT IMPLEMENTED
    ExceptionProctor(const ExceptionProctor&);
    ExceptionProctor& operator=(const ExceptionProctor&);

  public:
    // CREATORS

    /// Create an exception proctor for the specified `object` at the
    /// specified `line` number.  Optionally specify a `basicAllocator` used
    /// to supply memory.  If `basicAllocator` is 0, the currently installed
    /// default allocator is used.
    ExceptionProctor(const OBJECT     *object,
                     int               line,
                     bslma::Allocator *basicAllocator = 0)
    : d_line(line)
    , d_control(*object, basicAllocator)
    , d_object_p(object)
    {
    }

    /// Create an exception proctor for the specified `object` at the
    /// specified `line` number using the specified `control` object.
    ExceptionProctor(const OBJECT              *object,
                     int                        line,
                     bslmf::MovableRef<OBJECT>  control)
    : d_line(line)
    , d_control(bslmf::MovableRefUtil::move(control))
    , d_object_p(object)
    {
    }

    /// Destroy this exception proctor.  If the proctor was not released,
    /// verify that the state of the object supplied at construction has not
    /// changed.
    ~ExceptionProctor()
    {
        if (d_object_p) {
            const int LINE = d_line;
            ASSERTV(LINE, d_control, *d_object_p, d_control == *d_object_p);
        }
    }

    // MANIPULATORS

    /// Release this proctor from verifying the state of the object
    /// supplied at construction.
    void release()
    {
        d_object_p = 0;
    }
};

bool g_enableLessThanFunctorFlag = true;

                       // ====================
                       // class TestComparator
                       // ====================

/// This test class provides a mechanism that defines a function-call
/// operator that compares two objects of the parameterized `TYPE`.  The
/// function-call operator is implemented with integer comparison using
/// integers converted from objects of `TYPE` by the class method
/// `TemplateTestFacility::getIdentifier`.  The function-call operator also
/// increments a global counter used to keep track the method call count.
/// Object of this class can be identified by an id passed on construction.
template <class TYPE>
class TestComparator {

    // DATA
    int         d_id;           // identifier for the functor
    bool        d_compareLess;  // indicate whether this object use '<' or '>'
    mutable int d_count;        // number of times `operator()` is called

  public:
    // CLASS METHOD

    /// Disable all objects of `TestComparator` such that an `ASSERT` will
    /// be triggered if `operator()` is invoked
    static void disableFunctor()
    {
        g_enableLessThanFunctorFlag = false;
    }

    /// Enable all objects of `TestComparator` such that `operator()` may
    /// be invoked
    static void enableFunctor()
    {
        g_enableLessThanFunctorFlag = true;
    }

    // CREATORS
    //! TestComparator(const TestComparator& original) = default;
        // Create a copy of the specified `original`.

    /// Create a `TestComparator`.  Optionally, specify `id` that can be
    /// used to identify the object.
    explicit TestComparator(int id = 0, bool compareLess = true)
    : d_id(id)
    , d_compareLess(compareLess)
    , d_count(0)
    {
    }

    // ACCESSORS

    /// Increment a counter that records the number of times this method is
    /// called.   Return `true` if the integer representation of the
    /// specified `lhs` is less than integer representation of the specified
    /// `rhs`.
    bool operator() (const TYPE& lhs, const TYPE& rhs) const
    {
        if (!g_enableLessThanFunctorFlag) {
            ASSERTV("`TestComparator` was invoked when disabled", false);
        }

        ++d_count;

        if (d_compareLess) {
            return bsltf::TemplateTestFacility::getIdentifier(lhs)
                 < bsltf::TemplateTestFacility::getIdentifier(rhs);   // RETURN
        }
        else {
            return bsltf::TemplateTestFacility::getIdentifier(lhs)
                 > bsltf::TemplateTestFacility::getIdentifier(rhs);   // RETURN
        }
    }

    bool operator== (const TestComparator& rhs) const
    {
        return (id() == rhs.id() && d_compareLess == rhs.d_compareLess);
    }

    /// Return the `id` of this object.
    int id() const
    {
        return d_id;
    }

    /// Return the number of times `operator()` is called.
    size_t count() const
    {
        return d_count;
    }
};

                       // ============================
                       // class TestComparatorNonConst
                       // ============================

/// This test class provides a mechanism that defines a non-`const`
/// function-call operator that compares two objects of the parameterized
/// `TYPE`.  The function-call operator is implemented with integer
/// comparison using integers converted from objects of `TYPE` by the class
/// method `TemplateTestFacility::getIdentifier`.  The function-call
/// operator also increments a counter used to keep track the method call
/// count.  Object of this class can be identified by an id passed on
/// construction.
template <class TYPE>
class TestComparatorNonConst {

    // DATA
    int         d_id;           // identifier for the functor
    bool        d_compareLess;  // indicate whether this object use '<' or '>'
    mutable int d_count;        // number of times `operator()` is called

  public:
    // CREATORS

    explicit TestComparatorNonConst(int id = 0, bool compareLess = true)
    : d_id(id)
    , d_compareLess(compareLess)
    , d_count(0)
    {
    }

    // ACCESSORS

    /// Increment a counter that records the number of times this method is
    /// called.   Return `true` if the integer representation of the
    /// specified `lhs` is less than integer representation of the specified
    /// `rhs`.
    bool operator() (const TYPE& lhs, const TYPE& rhs)
    {
        ++d_count;

        if (d_compareLess) {
            return bsltf::TemplateTestFacility::getIdentifier(lhs)
                 < bsltf::TemplateTestFacility::getIdentifier(rhs);   // RETURN
        }
        else {
            return bsltf::TemplateTestFacility::getIdentifier(lhs)
                 > bsltf::TemplateTestFacility::getIdentifier(rhs);   // RETURN
        }
    }

    bool operator== (const TestComparatorNonConst& rhs) const
    {
        return id() == rhs.id();
    }

    /// Return the `id` of this object.
    int id() const
    {
        return d_id;
    }

    /// Return the number of times `operator()` is called.
    size_t count() const
    {
        return d_count;
    }
};

                            // =============================
                            // struct ThrowingSwapComparator
                            // =============================

/// Comparator with throwing `swap`.
template <class TYPE>
struct ThrowingSwapComparator : public std::less<TYPE> {

    // MANIPULATORS

    /// Exchange the value of this object with that of the specified `other`
    /// object.
    void swap(
      ThrowingSwapComparator& other) BSLS_KEYWORD_NOEXCEPT_SPECIFICATION(false)
    {
        (void)other;
    }

    // FREE FUNCTIONS

    /// Exchange the values of the specified `a` and `b` objects.
    friend void swap(
          ThrowingSwapComparator& a,
          ThrowingSwapComparator& b) BSLS_KEYWORD_NOEXCEPT_SPECIFICATION(false)
    {
        (void)a;
        (void)b;
    }
};

                       // =====================
                       // class TemplateWrapper
                       // =====================

/// This class inherits from the container, but do nothing otherwise.  A
/// compiler bug in AIX prevents the compiler from finding the definition of
/// the default arguments for the constructor.  This class is created to
/// test this scenario.
template <class KEY, class VALUE, class COMPARATOR, class ALLOCATOR>
class TemplateWrapper {

    // DATA
    bsl::multimap<KEY, VALUE, COMPARATOR, ALLOCATOR> d_member;

  public:
    // CREATORS
    TemplateWrapper()
    : d_member()
    {
    }

    //! TemplateWrapper(const TemplateWrapper&) = default;

    template <class INPUT_ITERATOR>
    TemplateWrapper(INPUT_ITERATOR begin, INPUT_ITERATOR end)
    : d_member(begin, end)
    {
    }
};

                       // =====================
                       // class TemplateWrapper
                       // =====================

/// A dummy comparator class.  Must be defined after `TemplateWrapper` to
/// reproduce the AIX bug.
class DummyComparator {

  public:
    bool operator() (int, int)
    {
        return true;
    }
};

                       // ====================
                       // class DummyAllocator
                       // ====================

/// A dummy allocator class.  Must be defined after `TemplateWrapper` to
/// reproduce the AIX bug.  Every method is a noop.
template <class TYPE>
class DummyAllocator {

  public:
    // PUBLIC TYPES
    typedef std::size_t     size_type;
    typedef std::ptrdiff_t  difference_type;
    typedef TYPE           *pointer;
    typedef const TYPE     *const_pointer;
    typedef TYPE&           reference;
    typedef const TYPE&     const_reference;
    typedef TYPE            value_type;

    template <class BDE_OTHER_TYPE>
    struct rebind
    {
        typedef DummyAllocator<BDE_OTHER_TYPE> other;
    };

    // CREATORS
    DummyAllocator() {}

    // DummyAllocator(const DummyAllocator& original) = default;

    template <class BDE_OTHER_TYPE>
    DummyAllocator(const DummyAllocator<BDE_OTHER_TYPE>& original)
    {
        (void) original;
    }

    // ~DummyAllocator() = default;
        // Destroy this object.

    // MANIPULATORS
    // DummyAllocator& operator=(const DummyAllocator& rhs) = default;

    pointer allocate(size_type    /* numElements */,
                     const void * /* hint */ = 0)
    {
        return 0;
    }

    void deallocate(pointer /* address */, size_type /* numElements */ = 1) {}

    template <class ELEMENT_TYPE>
    void construct(ELEMENT_TYPE * /* address */) {}
    template <class ELEMENT_TYPE>
    void construct(ELEMENT_TYPE * /* address */,
                   const ELEMENT_TYPE& /* value */) { }

    template <class ELEMENT_TYPE>
    void destroy(ELEMENT_TYPE * /* address */) {}

    // ACCESSORS
    pointer address(reference /* object */) const { return 0; }

    const_pointer address(const_reference /* object */) const { return 0; }

    size_type max_size() const { return 0; }
};

                       // =========================
                       // class CharToPairConverter
                       // =========================

/// Convert a `int` value to a `bsl::pair` of the parameterized `KEY` and
/// `VALUE` type.
template <class KEY, class VALUE, class ALLOC>
struct CharToPairConverter {

    // CLASS METHODS

    /// Create a new `pair<KEY, VALUE>` object at the specified `address`,
    /// passing values derived from the specified `id` to the `KEY` and
    /// `VALUE` constructors and using the specified `allocator` to supply
    /// memory.  The behavior is undefined unless `0 < id < 128`.
    static
    void createInplace(pair<KEY, VALUE> *address, int id, ALLOC allocator)
    {
        BSLS_ASSERT(address);
        BSLS_ASSERT( 0 < id);
        BSLS_ASSERT(id < 128);

        typedef typename bsl::remove_const<KEY>::type VarKey;

        // Support generation of pairs `(K, V1)`, `(K, V2)` where
        // `V1 != V2`.  E.g., `A` and `a` map to the same `KEY` but
        // distinct `VALUE`s.

        const int key = id, value = id - 'A' + '0';

        // If the `VALUE` type is a move-enabled allocating type, we want to
        // pass `allocator` to the `emplace` methods, since with some of the
        // strange allocators this function gets called with, no allocator
        // gets passed to the move c'tors, in which case the object in the
        // container will ultimately be using the allocator we used which
        // creating `tempKey` and `tempValue`.  Otherwise, the move will call
        // a copy c'tor, in which case the allocator used in the original case
        // won't be propagated regardless.  However, if we wound up using the
        // container allocator for `tempKey` and `tempValue` in the cases
        // where they are copied and not moved, it would throw off some cases
        // which are very closely monitoring the number of allocations from
        // that allocator.

        bslma::TestAllocator *pss = scratchSingleton();
        const bool useSingleton =
                     !bsl::is_same<VALUE, bsltf::MovableAllocTestType>::value
                  && !bsl::is_same<VALUE, bsltf::MoveOnlyAllocTestType>::value
                  && !bsl::is_same<VALUE,
                               bsltf::WellBehavedMoveOnlyAllocTestType>::value;

        // Note that `allocator` and `pss` are of different types, and
        // sometimes this function is called with `ALLOC` being a type that has
        // no c'tor that takes an `bslma::Allocator *` arg, so we can't use a
        // ternary on `useSingleton` to choose which allocator to pass to the
        // `emplace` methods.

        bsls::ObjectBuffer<VarKey> tempKey;
        if (useSingleton) {
            bsltf::TemplateTestFacility::emplace(tempKey.address(), key, pss);
        }
        else {
            bsltf::TemplateTestFacility::emplace(
                                            tempKey.address(), key, allocator);
        }
        bslma::DestructorGuard<VarKey> keyGuard(tempKey.address());

        bsls::ObjectBuffer<VALUE>  tempValue;
        if (useSingleton) {
            bsltf::TemplateTestFacility::emplace(
                                              tempValue.address(), value, pss);
        }
        else {
            bsltf::TemplateTestFacility::emplace(
                                        tempValue.address(), value, allocator);
        }
        bslma::DestructorGuard<VALUE>  valueGuard(tempValue.address());

        bsl::allocator_traits<ALLOC>::construct(
                          allocator,
                          address,
                          bslmf::MovableRefUtil::move(tempKey.object()),
                          bslmf::MovableRefUtil::move(tempValue.object()));
    }
};

/// This test class provides a mechanism that defines a function-call
/// operator that compares two objects of the parameterized `TYPE`.  The
/// function-call operator is implemented with integer comparison using
/// integers converted from objects of `TYPE` by the class method
/// `TemplateTestFacility::getIdentifier`.
template <class TYPE>
class GreaterThanFunctor {

  public:
    // ACCESSORS

    /// Return `true` if the integer representation of the specified `lhs`
    /// is less than integer representation of the specified `rhs`.
    bool operator() (const TYPE& lhs, const TYPE& rhs) const
    {
        return bsltf::TemplateTestFacility::getIdentifier(lhs)
             > bsltf::TemplateTestFacility::getIdentifier(rhs);
    }
};

// FREE OPERATORS

/// Return `true` if the integer representation of the specified `lhs` is
/// less than integer representation of the specified `rhs`.
template <class TYPE>
bool lessThanFunction(const TYPE& lhs, const TYPE& rhs)
{
    return bsltf::TemplateTestFacility::getIdentifier(lhs)
         < bsltf::TemplateTestFacility::getIdentifier(rhs);
}

}  // close unnamed namespace

// ============================================================================
//                     GLOBAL TYPEDEFS FOR TESTING
// ----------------------------------------------------------------------------

template <class ITER, class KEY, class VALUE>
class TestMovableTypeUtil
{
  public:
    static ITER findFirstNotMovedInto(ITER, ITER end)
    {
        return end;
    }
};

template <class ITER>
class TestMovableTypeUtil<ITER,
                          bsltf::MovableAllocTestType,
                          bsltf::MovableAllocTestType>
{
  public:
    static ITER findFirstNotMovedInto(ITER begin, ITER end)
    {
        for (; begin != end; ++begin) {
            if (!begin->second.movedInto())
            {
                break;
            }
        }
        return begin;
    }
};

class TestAllocatorUtil
{
  public:
    template <class TYPE>
    static void test(const TYPE&, const bslma::Allocator&)
    {
    }

    static void test(const bsltf::AllocEmplacableTestType& value,
                     const bslma::Allocator&               oa)
    {
        ASSERTV(&oa == value.arg01().allocator());
        ASSERTV(&oa == value.arg02().allocator());
        ASSERTV(&oa == value.arg03().allocator());
        ASSERTV(&oa == value.arg04().allocator());
        ASSERTV(&oa == value.arg05().allocator());
        ASSERTV(&oa == value.arg06().allocator());
        ASSERTV(&oa == value.arg07().allocator());
        ASSERTV(&oa == value.arg08().allocator());
        ASSERTV(&oa == value.arg09().allocator());
        ASSERTV(&oa == value.arg10().allocator());
    }
};

namespace {

                       // =========================
                       // struct TestIncompleteType
                       // =========================

struct IncompleteType;

/// This `struct` provides a simple compile-time test to verify that
/// incomplete types can be used in container definitions.  Currently,
/// definitions of `bsl::multimap` can contain incomplete types on all
/// supported platforms.
///
/// See `TestIncompleteType` in bslstl_map.t.cpp for the rationale behind
/// this test type.
struct TestIncompleteType {

    // PUBLIC TYPES
    typedef bsl::multimap<int, IncompleteType>::iterator            Iter1;
    typedef bsl::multimap<IncompleteType, int>::iterator            Iter2;
    typedef bsl::multimap<IncompleteType, IncompleteType>::iterator Iter3;

    // PUBLIC DATA
    bsl::multimap<int, IncompleteType>            d_data1;
    bsl::multimap<IncompleteType, int>            d_data2;
    bsl::multimap<IncompleteType, IncompleteType> d_data3;
};

struct IncompleteType {
    int d_data;
};

}  // close unnamed namespace

// ============================================================================
//                          TEST DRIVER TEMPLATE
// ----------------------------------------------------------------------------

/// This class template provides a namespace for testing the `multimap`
/// container.  The template parameter types `KEY`/`VALUE`, `COMP`, and
/// `ALLOC` specify the value type, comparator type, and allocator type,
/// respectively.  Each "testCase*" method tests a specific aspect of
/// `multimap<KEY, VALUE, COMP, ALLOC>`.  Every test cases should be invoked
/// with various type arguments to fully test the container.  Note that the
/// (template parameter) `VALUE` type must be defaulted (to `KEY`) for the
/// benefit of `RUN_EACH_TYPE`-style testing.
template <class KEY,
          class VALUE = KEY,
          class COMP  = TestComparator<KEY>,
          class ALLOC = bsl::allocator<bsl::pair<const KEY, VALUE> > >
class TestDriver {

  private:
    // TYPES

    // Shorthands

    typedef bsl::multimap<KEY, VALUE, COMP, ALLOC> Obj;
    typedef typename Obj::iterator                 Iter;
    typedef typename Obj::const_iterator           CIter;
    typedef typename Obj::reverse_iterator         RIter;
    typedef typename Obj::const_reverse_iterator   CRIter;
    typedef typename Obj::size_type                SizeType;
    typedef typename Obj::value_type               ValueType;

    typedef bslma::ConstructionUtil                ConsUtil;
    typedef bslmf::MovableRefUtil                  MoveUtil;
    typedef bsltf::MoveState                       MoveState;
    typedef bsltf::TemplateTestFacility            TstFacility;
    typedef TestMovableTypeUtil<CIter, KEY, VALUE> TstMoveUtil;

    typedef bsl::allocator_traits<ALLOC>          AllocatorTraits;

    typedef bsltf::TestValuesArray<typename Obj::value_type, ALLOC,
                     CharToPairConverter<const KEY, VALUE, ALLOC> > TestValues;

    /// Comparator functor with a non-`const` function call operator.
    typedef TestComparatorNonConst<KEY>           NonConstComp;

    enum AllocCategory { e_BSLMA, e_ADAPTOR, e_STATEFUL };

  public:
    typedef bsltf::StdTestAllocator<ValueType> StlAlloc;

  private:
    // TEST APPARATUS
    //-------------------------------------------------------------------------
    // The generating functions interpret the given `spec` in order from left
    // to right to configure the object according to a custom language.
    // Uppercase letters [A..Z] correspond to arbitrary (but unique) char
    // values to be appended to the `multimap<KEY, VALUE, COMP, ALLOC>` object.
    // ```
    // LANGUAGE SPECIFICATION:
    // -----------------------
    //
    // <SPEC>       ::= <EMPTY>   | <LIST>
    //
    // <EMPTY>      ::=
    //
    // <LIST>       ::= <ITEM>    | <ITEM><LIST>
    //
    // <ITEM>       ::= <ELEMENT> | <CLEAR>
    //
    // <ELEMENT>    ::= 'A' | 'B' | 'C' | 'D' | 'E' | ... | 'Z'
    //                                      // unique but otherwise arbitrary
    // Spec String  Description
    // -----------  -----------------------------------------------------------
    // ""           Has no effect; leaves the object empty.
    // "A"          Insert the value corresponding to A.
    // "AA"         Insert two values both corresponding to A.
    // "ABC"        Insert three values corresponding to A, B and C.
    //-------------------------------------------------------------------------
    // ```

    // CLASS DATA
    static
    const AllocCategory s_allocCategory =
                        bsl::is_same<ALLOC, bsl::allocator<ValueType> >::value
                        ? e_BSLMA
                        : bsl::is_same<ALLOC,
                                       bsltf::StdAllocatorAdaptor<
                                           bsl::allocator<ValueType> > >::value
                        ? e_ADAPTOR
                        : e_STATEFUL;

    static
    const bool s_valueIsMoveEnabled =
                    bsl::is_same<VALUE, bsltf::MovableTestType>::value ||
                    bsl::is_same<VALUE, bsltf::MovableAllocTestType>::value ||
                    bsl::is_same<VALUE, bsltf::MoveOnlyAllocTestType>::value ||
                    bsl::is_same<VALUE,
                               bsltf::WellBehavedMoveOnlyAllocTestType>::value;

    // CLASS METHODS
    static
    const char *allocCategoryAsStr()
    {
        return e_BSLMA == s_allocCategory ? "bslma"
                                          : e_ADAPTOR == s_allocCategory
                                          ? "adaptor"
                                          : e_STATEFUL == s_allocCategory
                                          ? "stateful"
                                          : "<INVALID>";
    }

    /// Configure the specified `object` according to the specified `spec`,
    /// using only the primary manipulator function `insert` and white-box
    /// manipulator `clear`.  Optionally specify a zero `verbose` to
    /// suppress `spec` syntax error messages.  Return the index of the
    /// first invalid character, and a negative value otherwise.  Note that
    /// this function is used to implement `gg` as well as allow for
    /// verification of syntax error detection.
    static int ggg(Obj *object, const char *spec, int verbose = 1);

    /// Return, by reference, the specified object with its value adjusted
    /// according to the specified `spec`.
    static Obj& gg(Obj *object, const char *spec);

    static void storeFirstNElemAddr(typename Obj::const_pointer *pointers,
                                    const Obj&                   object,
                                    size_t                       n)
    {
        size_t i = 0;

        for (CIter b = object.cbegin(); b != object.cend() && i < n; ++b)
        {
            pointers[i++] = bsls::Util::addressOf(*b);
        }
    }

    static
    int checkFirstNElemAddr(typename Obj::const_pointer *pointers,
                            const Obj&                   object,
                            size_t                       n)
    {
        int    count = 0;
        size_t i     = 0;

        for (CIter b = object.cbegin(); b != object.end() && i < n; ++b)
        {
            if (pointers[i++] != bsls::Util::addressOf(*b)) {
                ++count;
            }
        }
        return count;
    }

    /// Insert into the specified `container` the value object indicated by
    /// the specified `identifier`, ensuring that the overload of the
    /// primary manipulator taking a modifiable rvalue is invoked (rather
    /// than the one taking an lvalue).  Return the result of invoking the
    /// primary manipulator.
    static Iter primaryManipulator(Obj   *container,
                                   int    identifier,
                                   ALLOC  allocator)
    {
        typedef bsl::pair<KEY, VALUE> TValueType;

        bsls::ObjectBuffer<TValueType> buffer;
        CharToPairConverter<KEY, VALUE, ALLOC>::createInplace(
                                      buffer.address(), identifier, allocator);
        bslma::DestructorGuard<TValueType> guard(buffer.address());

        return container->insert(MoveUtil::move(buffer.object()));
    }

  public:
    // TEST CASES

    /// Test generator functions `g`.
    static void testCase11();

    // static void testCase10();
        // Reserved for BSLX.

    /// Test `swap` member.
    static void testCase8_dispatch();

    /// Test `swap` noexcept.
    static void testCase8_noexcept();

    template <bool SELECT_ON_CONTAINER_COPY_CONSTRUCTION_FLAG,
              bool OTHER_FLAGS>
    static void testCase7_select_on_container_copy_construction_dispatch();

    /// Test `select_on_container_copy_construction`.
    static void testCase7_select_on_container_copy_construction();

    /// Test copy constructor.
    static void testCase7();

    /// Test equality operator (`operator==`).
    static void testCase6();

    // static void testCase5();
        // Reserved for (<<) operator.

    /// Test basic accessors (`size`, `cbegin`, `cend` and `get_allocator`).
    static void testCase4();

    /// Test generator functions `ggg`, and `gg`.
    static void testCase3();

    /// Test primary manipulators (`insert` and `clear`).
    static void testCase2();

    /// Breathing test.  This test *exercises* basic functionality but
    /// *test* nothing.
    static void testCase1(const COMP&  comparator,
                          KEY         *testKeys,
                          VALUE       *testValues,
                          size_t       numValues);
};

template <class KEY, class VALUE = KEY>
class StdAllocTestDriver : public TestDriver<KEY,
                                             VALUE,
                                             TestComparator<KEY>,
                      bsltf::StdTestAllocator<bsl::pair<const KEY, VALUE> > > {
};

                               // --------------
                               // TEST APPARATUS
                               // --------------

template <class KEY, class VALUE, class COMP, class ALLOC>
int TestDriver<KEY, VALUE, COMP, ALLOC>::ggg(Obj        *object,
                                             const char *spec,
                                             int         verbose)
{
    enum { SUCCESS = -1 };

    for (int i = 0; spec[i]; ++i) {
        if ('A' <= spec[i] && spec[i] <= 'Z') {
            primaryManipulator(object, spec[i], object->get_allocator());
        }
        else {
            if (verbose) {
                printf("Error, bad character ('%c') "
                       "in spec \"%s\" at position %d.\n", spec[i], spec, i);
            }
            // Discontinue processing this spec.
            return i;                                                 // RETURN
        }
   }
   return SUCCESS;
}

template <class KEY, class VALUE, class COMP, class ALLOC>
bsl::multimap<KEY, VALUE, COMP, ALLOC>&
TestDriver<KEY, VALUE, COMP, ALLOC>::gg(Obj        *object,
                                        const char *spec)
{
    ASSERTV(ggg(object, spec) < 0);
    return *object;
}

template <class KEY, class VALUE, class COMP, class ALLOC>
void TestDriver<KEY, VALUE, COMP, ALLOC>::testCase8_dispatch()
{
    // ------------------------------------------------------------------------
    // SWAP MEMBER AND FREE FUNCTIONS
    //   Ensure that, when member and free `swap` are implemented, we can
    //   exchange the values of any two objects that use the same
    //   allocator.
    //
    // Concerns:
    // 1. Both functions exchange the values of the (two) supplied objects.
    //
    // 2. Both functions have standard signatures and return types.
    //
    // 3. Using either function to swap an object with itself does not
    //    affect the value of the object (alias-safety).
    //
    // 4. If the two objects being swapped uses the same allocator, neither
    //    function allocates memory from any allocator and the allocator
    //    address held by both objects is unchanged.
    //
    // 5. If the two objects being swapped uses different allocators and
    //    `AllocatorTraits::propagate_on_container_swap` is an alias to
    //    `false_type`, then both function may allocate memory and the
    //    allocator address held by both object is unchanged.
    //
    // 6. If the two objects being swapped uses different allocators and
    //    `AllocatorTraits::propagate_on_container_swap` is an alias to
    //    `true_type`, then no memory will be allocated and the allocators will
    //    also be swapped.
    //
    // 7. The free `swap` function is discoverable through ADL (Argument
    //    Dependent Lookup).
    //
    // Plan:
    // 1. Use the addresses of the `swap` member and free functions defined
    //    in this component to initialize, respectively, member-function
    //    and free-function pointers having the appropriate signatures and
    //    return types.  (C-2)
    //
    // 2. Create a `bslma::TestAllocator` object, and install it as the
    //    default allocator (note that a ubiquitous test allocator is
    //    already installed as the global allocator).
    //
    // 3. Using the table-driven technique:
    //
    //   1. Specify a set of (unique) valid object values (one per row) in
    //      terms of their individual attributes, including (a) first, the
    //      default value, (b) boundary values corresponding to every range
    //      of values that each individual attribute can independently
    //      attain, and (c) values that should require allocation from each
    //      individual attribute that can independently allocate memory.
    //
    //   2. Additionally, provide a (tri-valued) column, `MEM`, indicating
    //      the expectation of memory allocation for all typical
    //      implementations of individual attribute types: ('Y') "Yes",
    //      ('N') "No", or ('?') "implementation-dependent".
    //
    // 4. For each row `R1` in the table of P-3:  (C-1, 3..6)
    //
    //   1. Create a `bslma::TestAllocator` object, `oa`.
    //
    //   2. Use the value constructor and `oa` to create a modifiable
    //      `Obj`, `mW`, having the value described by `R1`; also use the
    //      copy constructor and a "scratch" allocator to create a `const`
    //      `Obj` `XX` from `mW`.
    //
    //   3. Use the member and free `swap` functions to swap the value of
    //      `mW` with itself; verify, after each swap, that:  (C-3..4)
    //
    //     1. The value is unchanged.  (C-3)
    //
    //     2. The allocator address held by the object is unchanged.  (C-4)
    //
    //     3. There was no additional object memory allocation.  (C-4)
    //
    //   4. For each row `R2` in the table of P-3:  (C-1, 4)
    //
    //     1. Use the copy constructor and `oa` to create a modifiable
    //        `Obj`, `mX`, from `XX` (P-4.2).
    //
    //     2. Use the value constructor and `oa` to create a modifiable
    //        `Obj`, `mY`, and having the value described by `R2`; also use
    //        the copy constructor to create, using a "scratch" allocator,
    //        a `const` `Obj`, `YY`, from `Y`.
    //
    //     3. Use, in turn, the member and free `swap` functions to swap
    //        the values of `mX` and `mY`; verify, after each swap, that:
    //        (C-1..2)
    //
    //       1. The values have been exchanged.  (C-1)
    //
    //       2. The common object allocator address held by `mX` and `mY`
    //          is unchanged in both objects.  (C-4)
    //
    //       3. There was no additional object memory allocation.  (C-4)
    //
    //     5. Use the value constructor and `oaz` to a create a modifiable
    //        `Obj` `mZ`, having the value described by `R2`; also use the copy
    //        constructor to create, using a "scratch" allocator, a const
    //        `Obj`, `ZZ`, from `Z`.
    //
    //     6. Use the member and free `swap` functions to swap the values of
    //        `mX` and `mZ` respectively (when
    //        AllocatorTraits::propagate_on_container_swap is an alias to
    //        false_type) verify, after each swap, that: (C-1, 5)
    //
    //       1. The values have been exchanged.  (C-1)
    //
    //       2. The common object allocator address held by `mX` and `mZ` is
    //          unchanged in both objects.  (C-5)
    //
    //       3. Temporary memory were allocated from `oa` if `mZ` is not empty,
    //          and temporary memory were allocated from `oaz` if `mX` is not
    //          empty.  (C-5)
    //
    //     7. Create a new object allocator, `oap`.
    //
    //     8. Use the value constructor and `oap` to create a modifiable `Obj`
    //        `mP`, having the value described by `R2`; also use the copy
    //        constructor to create, using a "scratch" allocator, a const
    //        `Obj`, `PP`, from 'P.
    //
    //     9. Manually change `AllocatorTraits::propagate_on_container_swap` to
    //        be an alias to `true_type` (Instead of this manual step, use an
    //        allocator that enables propagate_on_container_swap when
    //        AllocatorTraits supports it) and use the member and free `swap`
    //        functions to swap the values `mX` and `mZ` respectively; verify,
    //        after each swap, that: (C-1, 6)
    //
    //       1. The values have been exchanged.  (C-1)
    //
    //       2. The allocators addresses have been exchanged.  (C-6)
    //
    //       3. There was no additional object memory allocation.  (C-6)
    //
    // 5. Verify that the free `swap` function is discoverable through ADL:
    //    (C-7)
    //
    //   1. Create a set of attribute values, `A`, distinct from the values
    //      corresponding to the default-constructed object, choosing
    //      values that allocate memory if possible.
    //
    //   2. Create a `bslma::TestAllocator` object, `oa`.
    //
    //   3. Use the default constructor and `oa` to create a modifiable
    //      `Obj` `mX` (having default attribute values); also use the copy
    //      constructor and a "scratch" allocator to create a `const` `Obj`
    //      `XX` from `mX`.
    //
    //   4. Use the value constructor and `oa` to create a modifiable `Obj`
    //      `mY` having the value described by the `Ai` attributes; also
    //      use the copy constructor and a "scratch" allocator to create a
    //      `const` `Obj` `YY` from `mY`.
    //
    //   5. Use the `invokeAdlSwap` helper function template to swap the
    //      values of `mX` and `mY`, using the free `swap` function defined
    //      in this component, then verify that:  (C-7)
    //
    //     1. The values have been exchanged.  (C-1)
    //
    //     2. There was no additional object memory allocation.  (C-4)
    //
    // Testing:
    //   void swap(multimap& other);
    //   void swap(multimap<& a, multimap& b);
    // ------------------------------------------------------------------------

    const bool isPropagate =
                           AllocatorTraits::propagate_on_container_swap::value;
    const bool otherTraitsSet =
                AllocatorTraits::propagate_on_container_copy_assignment::value;

    // We can print the banner now:

    if (verbose) printf("%sTESTING SWAP `%s` OTHER:%c PROP:%c ALLOC: %s\n",
                        veryVerbose ? "\n" : "",
                        NameOf<VALUE>().name(), otherTraitsSet ? 'T' : 'F',
                        isPropagate ? 'T' : 'F',
                        allocCategoryAsStr());

    if (veryVerbose) printf(
                     "\nAssign the address of each function to a variable.\n");
    {
        typedef void (Obj::*funcPtr)(Obj&);
        typedef void (*freeFuncPtr)(Obj&, Obj&);

        // Verify that the signatures and return types are standard.

        funcPtr     memberSwap = &Obj::swap;
        freeFuncPtr freeSwap   = bsl::swap;

        (void) memberSwap;  // quash potential compiler warnings
        (void) freeSwap;
    }

    if (veryVerbose) printf(
                 "\nCreate a test allocator and install it as the default.\n");

    bslma::TestAllocator         doa("default",  veryVeryVeryVerbose);
    bslma::DefaultAllocatorGuard dag(&doa);
    ALLOC                        da(&doa);

    bslma::TestAllocator         ooa("object",   veryVeryVeryVerbose);
    bslma::TestAllocator         soa("scratch",  veryVeryVeryVerbose);
    bslma::TestAllocator         zoa("z_object", veryVeryVeryVerbose);

    ALLOC                        oa(&ooa);
    ALLOC                        sa(&soa);
    ALLOC                        za(&zoa);

    if (veryVerbose) printf(
       "\nUse a table of distinct object values and expected memory usage.\n");

    const int NUM_DATA                     = DEFAULT_NUM_DATA;
    const DefaultDataRow (&DATA)[NUM_DATA] = DEFAULT_DATA;

    for (int ti = 0; ti < NUM_DATA; ++ti) {
        const char *const SPEC1   = DATA[ti].d_spec;

        if (ti && DATA[ti-1].d_index == DATA[ti].d_index) {
            // redundant, skip

            continue;
        }

        Obj mW(oa);        const Obj& W  = gg(&mW,  SPEC1);
        Obj mXX(sa);       const Obj& XX = gg(&mXX, SPEC1);

        if (veryVerbose) { T_ P_(SPEC1) P_(W) P(XX) }

        // Ensure the first row of the table contains the
        // default-constructed value.

        static bool firstFlag = true;
        if (firstFlag) {
            ASSERTV(SPEC1, Obj(), W, Obj() == W);
            firstFlag = false;
        }

        for (int member = 0; member < 2; ++member) {
            bslma::TestAllocatorMonitor oam(&ooa);

            if (member) {
                // member `swap`

                mW.swap(mW);
            }
            else {
                // free function `swap`

                swap(mW, mW);
            }

            ASSERTV(SPEC1, XX, W, XX == W);
            ASSERTV(SPEC1, oa == W.get_allocator());
            ASSERTV(SPEC1, oam.isTotalSame());
        }

        for (int tj = 0; tj < NUM_DATA; ++tj) {
            const char *const SPEC2   = DATA[tj].d_spec;

            if (tj && DATA[tj-1].d_index == DATA[tj].d_index) {
                // redundant, skip

                continue;
            }

            Obj mYY(sa);      const Obj& YY = gg(&mYY, SPEC2);

            for (int member = 0; member < 2; ++member) {
                Obj mX(oa);      const Obj& X = gg(&mX, SPEC1);
                Obj mY(oa);      const Obj& Y = gg(&mY, SPEC2);

                if (veryVerbose) { T_ P_(SPEC2) P_(X) P_(Y) P(YY) }

                {
                    bslma::TestAllocatorMonitor oam(&ooa);

                    if (member) {
                        // member `swap`

                        mX.swap(mY);
                    }
                    else {
                        // free function `swap`

                        swap(mX, mY);
                    }

                    ASSERTV(SPEC1, SPEC2, YY, X, YY == X);
                    ASSERTV(SPEC1, SPEC2, XX, Y, XX == Y);
                    ASSERTV(SPEC1, SPEC2, oa == X.get_allocator());
                    ASSERTV(SPEC1, SPEC2, oa == Y.get_allocator());
                    ASSERTV(SPEC1, SPEC2, oam.isTotalSame());
                }

                Obj mZ(za);       const Obj& Z  = gg(&mZ,  SPEC1);

                ASSERT(XX == Z);
                ASSERT(YY == X);

                if (veryVerbose) { T_ P_(SPEC2) P_(X) P_(Y) P(YY) }

                bslma::TestAllocatorMonitor oam(&ooa);
                bslma::TestAllocatorMonitor oazm(&zoa);

                if (member) {
                    // member `swap`

                    mX.swap(mZ);
                }
                else {
                    // free function `swap`

                    swap(mX, mZ);
                }

                ASSERTV(SPEC1, SPEC2, XX, X, XX == X);
                ASSERTV(SPEC1, SPEC2, YY, Z, YY == Z);
                ASSERTV(SPEC1, SPEC2, (isPropagate ? za : oa) ==
                                                            X.get_allocator());
                ASSERTV(SPEC1, SPEC2, (isPropagate ? oa : za) ==
                                                            Z.get_allocator());

                if (isPropagate || 0 == X.size()) {
                    ASSERTV(SPEC1, SPEC2, oam.isTotalSame());
                }
                else {
                    ASSERTV(SPEC1, SPEC2, oam.isTotalUp());
                }

                if (isPropagate || 0 == Z.size()) {
                    ASSERTV(SPEC1, SPEC2, oazm.isTotalSame());
                }
                else {
                    ASSERTV(SPEC1, SPEC2, oazm.isTotalUp());
                }
            }
        }
    }

    {
        // `A` values: Should cause memory allocation if possible.

        Obj mX(oa);       const Obj& X  = gg(&mX,  "DD");
        Obj mXX(sa);      const Obj& XX = gg(&mXX, "DD");

        Obj mY(oa);       const Obj& Y  = gg(&mY,  "ABC");
        Obj mYY(sa);      const Obj& YY = gg(&mYY, "ABC");

        if (veryVerbose) { T_ P_(X) P(Y) }

        bslma::TestAllocatorMonitor oam(&ooa);

        if (veryVerbose) printf(
            "\nInvoke free `swap` function in a context where ADL is used.\n");

        invokeAdlSwap(&mX, &mY);

        ASSERTV(YY, X, YY == X);
        ASSERTV(XX, Y, XX == Y);
        ASSERT(oam.isTotalSame());

        if (veryVerbose) { T_ P_(X) P(Y) }

        if (veryVerbose) printf(
            "\nInvoke free `swap` function using the standard BDE pattern.\n");

        invokePatternSwap(&mX, &mY);

        ASSERTV(YY, X, XX == X);
        ASSERTV(XX, Y, YY == Y);
        ASSERT(oam.isTotalSame());

        if (veryVerbose) { T_ P_(X) P(Y) }
    }
}

/// Verify that noexcept specification of the member `swap` function is
/// correct.
template <class KEY, class VALUE, class COMP, class ALLOC>
void TestDriver<KEY, VALUE, COMP, ALLOC>::testCase8_noexcept()
{
    Obj a;
    Obj b;

#if BSLS_KEYWORD_NOEXCEPT_AVAILABLE
    const bool isNoexcept =
                        bsl::allocator_traits<ALLOC>::is_always_equal::value &&
                        bsl::is_nothrow_swappable<COMP>::value;
    ASSERT(isNoexcept == BSLS_KEYWORD_NOEXCEPT_OPERATOR(a.swap(b)));
#endif
}

template <class KEY, class VALUE, class COMP, class ALLOC>
template <bool SELECT_ON_CONTAINER_COPY_CONSTRUCTION_FLAG,
          bool OTHER_FLAGS>
void TestDriver<KEY, VALUE, COMP, ALLOC>::
                     testCase7_select_on_container_copy_construction_dispatch()
{
    const int TYPE_ALLOC = bslma::UsesBslmaAllocator<KEY>::value +
                           bslma::UsesBslmaAllocator<VALUE>::value;

    // Set the three properties of `bsltf::StdStatefulAllocator` that are not
    // under test in this test case to `false`.

    typedef bsltf::StdStatefulAllocator<
                                    KEY,
                                    SELECT_ON_CONTAINER_COPY_CONSTRUCTION_FLAG,
                                    OTHER_FLAGS,
                                    OTHER_FLAGS,
                                    OTHER_FLAGS>      StdAlloc;

    typedef bsl::multimap<KEY, VALUE, COMP, StdAlloc> Obj;

    const bool PROPAGATE = SELECT_ON_CONTAINER_COPY_CONSTRUCTION_FLAG;

    static const char *SPECS[] = {
        "",
        "A",
        "BC",
        "CDE",
    };
    const int NUM_SPECS = static_cast<int>(sizeof SPECS / sizeof *SPECS);

    for (int ti = 0; ti < NUM_SPECS; ++ti) {
        const char *const SPEC   = SPECS[ti];
        const size_t      LENGTH = strlen(SPEC);

        TestValues VALUES(SPEC);

        bslma::TestAllocator da("default", veryVeryVeryVerbose);
        bslma::TestAllocator oa("object",  veryVeryVeryVerbose);

        bslma::DefaultAllocatorGuard dag(&da);

        StdAlloc ma(&oa);

        {
            const Obj W(VALUES.begin(), VALUES.end(), COMP(), ma);  // control

            ASSERTV(ti, LENGTH == W.size());  // same lengths
            if (veryVerbose) { printf("\tControl Obj: "); P(W); }

            VALUES.resetIterators();

            Obj mX(VALUES.begin(), VALUES.end(), COMP(), ma);
            const Obj& X = mX;

            if (veryVerbose) { printf("\t\tDynamic Obj: "); P(X); }

            bslma::TestAllocatorMonitor dam(&da);
            bslma::TestAllocatorMonitor oam(&oa);

            const Obj Y(X);

            ASSERTV(SPEC, W == Y);
            ASSERTV(SPEC, W == X);
            ASSERTV(SPEC, PROPAGATE, PROPAGATE == (ma == Y.get_allocator()));
            ASSERTV(SPEC, PROPAGATE,               ma == X.get_allocator());

            if (PROPAGATE) {
                ASSERTV(SPEC, 0 != TYPE_ALLOC || dam.isInUseSame());
                ASSERTV(SPEC, 0 ==     LENGTH || oam.isInUseUp());
            }
            else {
                ASSERTV(SPEC, 0 ==     LENGTH || dam.isInUseUp());
                ASSERTV(SPEC, oam.isTotalSame());
            }
        }
        ASSERTV(SPEC, 0 == da.numBlocksInUse());
        ASSERTV(SPEC, 0 == oa.numBlocksInUse());
    }
}

template <class KEY, class VALUE, class COMP, class ALLOC>
void TestDriver<KEY, VALUE, COMP, ALLOC>::
                              testCase7_select_on_container_copy_construction()
{
    // ------------------------------------------------------------------------
    // COPY CONSTRUCTOR: ALLOCATOR PROPAGATION
    //
    // Concerns:
    // 1. The allocator of a source object using a standard allocator is
    //    propagated to the newly constructed object according to the
    //    `select_on_container_copy_construction` method of the allocator.
    //
    // 2. In the absence of a `select_on_container_copy_construction` method,
    //    the allocator of a source object using a standard allocator is always
    //    propagated to the newly constructed object (C++03 semantics).
    //
    // 3. The effect of the `select_on_container_copy_construction` trait is
    //    independent of the other three allocator propagation traits.
    //
    // Plan:
    // 1. Specify a set S of object values with varied differences, ordered by
    //    increasing length, to be used in the following tests.
    //
    // 2. Create a `bsltf::StdStatefulAllocator` with its
    //    `select_on_container_copy_construction` property configured to
    //    `false`.  In two successive iterations of P-3..5, first configure the
    //    three properties not under test to be `false`, then confgiure them
    //    all to be `true`.
    //
    // 3. For each value in S, initialize objects `W` (a control) and `X` using
    //    the allocator from P-2.
    //
    // 4. Copy construct `Y` from `X` and use `operator==` to verify that both
    //    `X` and `Y` subsequently have the same value as `W`.
    //
    // 5. Use the `get_allocator` method to verify that the allocator of `X`
    //    is *not* propagated to `Y`.
    //
    // 6. Repeat P-2..5 except that this time configure the allocator property
    //    under test to `true` and verify that the allocator of `X` *is*
    //    propagated to `Y`.  (C-1)
    //
    // 7. Repeat P-2..5 except that this time use a `StatefulStlAllocator`,
    //    which does not define a `select_on_container_copy_construction`
    //    method, and verify that the allocator of `X` is *always* propagated
    //    to `Y`.  (C-2..3)
    //
    // Testing:
    //   select_on_container_copy_construction
    // ------------------------------------------------------------------------

    if (verbose) printf("\n'select_on_container_copy_construction' "
                        "propagates *default* allocator.\n");

    testCase7_select_on_container_copy_construction_dispatch<false, false>();
    testCase7_select_on_container_copy_construction_dispatch<false, true>();

    if (verbose) printf("\n'select_on_container_copy_construction' "
                        "propagates allocator of source object.\n");

    testCase7_select_on_container_copy_construction_dispatch<true, false>();
    testCase7_select_on_container_copy_construction_dispatch<true, true>();

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    if (verbose) printf("\nVerify C++03 semantics (allocator has no "
                        "`select_on_container_copy_construction` method).\n");

    typedef StatefulStlAllocator<KEY>                  Allocator;
    typedef bsl::multimap<KEY, VALUE, COMP, Allocator> Obj;

    {
        static const char *SPECS[] = {
            "",
            "A",
            "BC",
            "CDE",
        };
        const int NUM_SPECS = static_cast<int>(sizeof SPECS / sizeof *SPECS);

        for (int ti = 0; ti < NUM_SPECS; ++ti) {
            const char *const SPEC   = SPECS[ti];
            const size_t      LENGTH = strlen(SPEC);
            TestValues VALUES(SPEC);

            const int ALLOC_ID = ti + 73;

            Allocator a;  a.setId(ALLOC_ID);

            const Obj W(VALUES.begin(), VALUES.end(), COMP(), a);  // control

            ASSERTV(ti, LENGTH == W.size());  // same lengths
            if (veryVerbose) { printf("\tControl Obj: "); P(W); }

            VALUES.resetIterators();

            Obj mX(VALUES.begin(), VALUES.end(), COMP(), a); const Obj& X = mX;

            if (veryVerbose) { printf("\t\tDynamic Obj: "); P(X); }

            const Obj Y(X);

            ASSERTV(SPEC,        W == Y);
            ASSERTV(SPEC,        W == X);
            ASSERTV(SPEC, ALLOC_ID == Y.get_allocator().id());
        }
    }
}

template <class KEY, class VALUE, class COMP, class ALLOC>
void TestDriver<KEY, VALUE, COMP, ALLOC>::testCase7()
{
    // ------------------------------------------------------------------------
    // TESTING COPY CONSTRUCTOR
    //
    // Concerns:
    // 1. The new object's value is the same as that of the original object
    //    (relying on the equality operator) and created with the correct
    //    capacity.
    //
    // 2. All internal representations of a given value can be used to create a
    //    new object of equivalent value.
    //
    // 3. The value of the original object is left unaffected.
    //
    // 4. Subsequent changes in or destruction of the source object have no
    //    effect on the copy-constructed object.
    //
    // 5. Subsequent changes (`insert`s) on the created object have no
    //    effect on the original and change the capacity of the new object
    //    correctly.
    //
    // 6. The object has its internal memory management system hooked up
    //    properly so that *all* internally allocated memory draws from a
    //    user-supplied allocator whenever one is specified.
    //
    // 7. The function is exception neutral w.r.t. memory allocation.
    //
    // Plan:
    // 1. Specify a set S of object values with substantial and varied
    //    differences, ordered by increasing length, to be used in the
    //    following tests.
    //
    // 2. For each value in S, initialize objects w and x, copy construct y
    //    from x and use `operator==` to verify that both x and y subsequently
    //    have the same value as w.  Let x go out of scope and again verify
    //    that w == y.  (C-1..4)
    //
    // 3. For each value in S initialize objects w and x, and copy construct y
    //    from x.  Change the state of y, by using the *primary* *manipulator*
    //    `push_back`.  Using the `operator!=` verify that y differs from x and
    //    w, and verify that the capacity of y changes correctly.  (C-5)
    //
    // 4. Perform tests performed as P-2:  (C-6)
    //   1. While passing a testAllocator as a parameter to the new object and
    //      ascertaining that the new object gets its memory from the provided
    //      testAllocator.
    //   2. Verify neither of global and default allocator is used to supply
    //      memory.  (C-6)
    //
    // 5. Perform tests as P-2 in the presence of exceptions during memory
    //    allocations using a `bslma::TestAllocator` and varying its
    //    *allocation* *limit*.  (C-7)
    //
    // Testing:
    //   multimap(const multimap& original);
    //   multimap(const multimap& original, const A& allocator);
    // ------------------------------------------------------------------------

    bslma::TestAllocator oa(veryVeryVerbose);

    const TestValues VALUES;

    const int TYPE_ALLOC = (bslma::UsesBslmaAllocator<KEY>::value +
                            bslma::UsesBslmaAllocator<VALUE>::value);

    if (verbose)
        printf("\nTesting parameters: TYPE_ALLOC = %d.\n", TYPE_ALLOC);
    {
        static const char *SPECS[] = {
            "",
            "A",
            "BC",
            "CDE",
            "DEAB",
            "EABCD",
            "ABCDEFG",
            "HFGEDCBA",
            "CFHEBIDGA",
            "BENCKHGMALJDFOI",
            "IDMLNEFHOPKGBCJA",
            "OIQGDNPMLKBACHFEJ"
        };

        const int NUM_SPECS = static_cast<int>(sizeof SPECS / sizeof *SPECS);

        for (int ti = 0; ti < NUM_SPECS; ++ti) {
            const char *const SPEC   = SPECS[ti];
            const size_t      LENGTH = strlen(SPEC);

            if (verbose) {
                printf("\nFor an object of length " ZU ":\n", LENGTH);
                P(SPEC);
            }

            // Create control object w.
            Obj mW; const Obj& W = gg(&mW, SPEC);

            ASSERTV(ti, LENGTH == W.size()); // same lengths
            if (veryVerbose) { printf("\tControl Obj: "); P(W); }

            Obj mX(&oa);  const Obj& X = gg(&mX, SPEC);

            if (veryVerbose) { printf("\t\tDynamic Obj: "); P(X); }

            {   // Testing concern 1..4.

                if (veryVerbose) { printf("\t\t\tRegular Case :"); }

                Obj *pX = new Obj(&oa);
                gg(pX, SPEC);

                const Obj Y0(*pX);

                ASSERTV(SPEC, W == Y0);
                ASSERTV(SPEC, W == X);
                ASSERTV(SPEC, Y0.get_allocator() ==
                                           bslma::Default::defaultAllocator());

                delete pX;
                ASSERTV(SPEC, W == Y0);
            }
            {   // Testing concern 5.

                if (veryVerbose) printf("\t\t\tInsert into created obj, "
                                        "without test allocator:\n");

                Obj Y1(X);

                if (veryVerbose) {
                    printf("\t\t\t\tBefore Insert: "); P(Y1);
                }

                Iter RESULT = Y1.insert(VALUES['Z' - 'A']);
                ASSERTV(VALUES['Z' - 'A'] == *RESULT);

                if (veryVerbose) {
                    printf("\t\t\t\tAfter Insert : ");
                    P(Y1);
                }

                ASSERTV(SPEC, Y1.size() == LENGTH + 1);
                ASSERTV(SPEC, W != Y1);
                ASSERTV(SPEC, X != Y1);
            }
            {   // Testing concern 5 with test allocator.

                if (veryVerbose)
                    printf("\t\t\tInsert into created obj, "
                           "with test allocator:\n");

                const bsls::Types::Int64 BB = oa.numBlocksTotal();
                const bsls::Types::Int64  B = oa.numBlocksInUse();

                if (veryVerbose) {
                    printf("\t\t\t\tBefore Creation: "); P_(BB); P(B);
                }

                Obj Y11(X, &oa);

                const bsls::Types::Int64 AA = oa.numBlocksTotal();
                const bsls::Types::Int64  A = oa.numBlocksInUse();

                if (veryVerbose) {
                    printf("\t\t\t\tAfter Creation: "); P_(AA); P(A);
                    printf("\t\t\t\tBefore Append: "); P(Y11);
                }

                if (LENGTH == 0) {
                    ASSERTV(SPEC, BB + 0 == AA);
                    ASSERTV(SPEC,  B + 0 ==  A);
                }
                else {
                    const int TYPE_ALLOCS = TYPE_ALLOC
                                           * static_cast<int>(X.size());
                    ASSERTV(SPEC, BB + 1 + TYPE_ALLOCS == AA);
                    ASSERTV(SPEC,  B + 1 + TYPE_ALLOCS ==  A);
                }

                const bsls::Types::Int64 CC = oa.numBlocksTotal();
                const bsls::Types::Int64  C = oa.numBlocksInUse();

                Iter RESULT = Y11.insert(VALUES['Z' - 'A']);
                ASSERTV(VALUES['Z' - 'A'] == *RESULT);

                const bsls::Types::Int64 DD = oa.numBlocksTotal();
                const bsls::Types::Int64  D = oa.numBlocksInUse();

                if (veryVerbose) {
                    printf("\t\t\t\tAfter Append : ");
                    P(Y11);
                }

                ASSERTV(SPEC, CC + 1 + TYPE_ALLOC == DD);
                ASSERTV(SPEC, C  + 1 + TYPE_ALLOC ==  D);

                ASSERTV(SPEC, Y11.size() == LENGTH + 1);
                ASSERTV(SPEC, W != Y11);
                ASSERTV(SPEC, X != Y11);
                ASSERTV(SPEC, Y11.get_allocator() == X.get_allocator());
            }
            {   // Exception checking.

                const bsls::Types::Int64 BB = oa.numBlocksTotal();
                const bsls::Types::Int64  B = oa.numBlocksInUse();

                if (veryVerbose) {
                    printf("\t\t\t\tBefore Creation: "); P_(BB); P(B);
                }

                BSLMA_TESTALLOCATOR_EXCEPTION_TEST_BEGIN(oa) {
                    const Obj Y2(X, &oa);
                    if (veryVerbose) {
                        printf("\t\t\tException Case  :\n");
                        printf("\t\t\t\tObj : "); P(Y2);
                    }
                    ASSERTV(SPEC, W == Y2);
                    ASSERTV(SPEC, W == X);
                    ASSERTV(SPEC, Y2.get_allocator() == X.get_allocator());
                } BSLMA_TESTALLOCATOR_EXCEPTION_TEST_END

                const bsls::Types::Int64 AA = oa.numBlocksTotal();
                const bsls::Types::Int64  A = oa.numBlocksInUse();

                if (veryVerbose) {
                    printf("\t\t\t\tAfter Creation: "); P_(AA); P(A);
                }

                if (LENGTH == 0) {
                    ASSERTV(SPEC, BB + 0 == AA);
                    ASSERTV(SPEC,  B + 0 ==  A);
                }
                else {
                    ASSERTV(SPEC, B + 0 == A);
                }
            }
        }
    }
}

template <class KEY, class VALUE, class COMP, class ALLOC>
void TestDriver<KEY, VALUE, COMP, ALLOC>::testCase6()
{
    // ---------------------------------------------------------------------
    // TESTING EQUALITY OPERATORS
    //
    // Concerns:
    // 1. Two objects, `X` and `Y`, compare equal if and only if they contain
    //    the same values.
    //
    // 2. No non-salient attributes (i.e., `allocator`) participate.
    //
    // 3. `true  == (X == X)` (i.e., identity)
    //
    // 4. `false == (X != X)` (i.e., identity)
    //
    // 5. `X == Y` if and only if `Y == X` (i.e., commutativity)
    //
    // 6. `X != Y` if and only if `Y != X` (i.e., commutativity)
    //
    // 7. `X != Y` if and only if `!(X == Y)`
    //
    // 8. Comparison is symmetric with respect to user-defined conversion
    //    (i.e., both comparison operators are free functions).
    //
    // 9. Non-modifiable objects can be compared (i.e., objects or references
    //    providing only non-modifiable access).
    //
    // 10. `operator==` is defined in terms of `operator==(KEY)` instead of the
    //    supplied comparator function.
    //
    // 11. No memory allocation occurs as a result of comparison (e.g., the
    //    arguments are not passed by value).
    //
    // 12. The equality operator's signature and return type are standard.
    //
    // 13. The inequality operator's signature and return type are standard.
    //
    // Plan:
    // 1. Use the respective addresses of `operator==` and `operator!=` to
    //    initialize function pointers having the appropriate signatures and
    //    return types for the two homogeneous, free equality- comparison
    //    operators defined in this component.  (C-8..9, 12..13)
    //
    // 2. Create a `bslma::TestAllocator` object, and install it as the default
    //    allocator (note that a ubiquitous test allocator is already installed
    //    as the global allocator).
    //
    // 3. Using the table-driven technique, specify a set of distinct
    //    specifications for the `gg` function.
    //
    // 4. For each row `R1` in the table of P-3: (C-1..7)
    //
    //   1. Create a single object, using a comparator that can be disabled and
    //      a"scratch" allocator, and use it to verify the reflexive
    //      (anti-reflexive) property of equality (inequality) in the presence
    //      of aliasing.  (C-3..4)
    //
    //   2. For each row `R2` in the table of P-3: (C-1..2, 5..7)
    //
    //     1. Record, in `EXP`, whether or not distinct objects created from
    //        `R1` and `R2`, respectively, are expected to have the same value.
    //
    //     2. For each of two configurations, `a` and `b`: (C-1..2, 5..7)
    //
    //       1. Create two (object) allocators, `oax` and `oay`.
    //
    //       2. Create an object `X`, using `oax`, having the value `R1`.
    //
    //       3. Create an object `Y`, using `oax` in configuration `a` and
    //          `oay` in configuration `b`, having the value `R2`.
    //
    //       4. Disable the comparator so that it will cause an error if it's
    //          used.
    //
    //       5. Verify the commutativity property and expected return value for
    //          both `==` and `!=`, while monitoring both `oax` and `oay` to
    //          ensure that no object memory is ever allocated by either
    //          operator.  (C-1..2, 5..7, 10)
    //
    // 5. Use the test allocator from P-2 to verify that no memory is ever
    //    allocated from the default allocator.  (C-11)
    //
    // Testing:
    //   bool operator==(const multimap& lhs, const multimap& rhs);
    //   bool operator!=(const multimap& lhs, const multimap& rhs);
    // ------------------------------------------------------------------------

    if (verbose) printf("\nEQUALITY-COMPARISON OPERATORS"
                        "\n=============================\n");

    if (verbose)
              printf("\nAssign the address of each operator to a variable.\n");
    {
        using namespace bsl;

        typedef bool (*operatorPtr)(const Obj&, const Obj&);

        // Verify that the signatures and return types are standard.

        operatorPtr operatorEq = operator==;
        (void) operatorEq;  // quash potential compiler warnings

#ifdef BSLS_COMPILERFEATURES_SUPPORT_THREE_WAY_COMPARISON
        (void) [](const Obj& lhs, const Obj& rhs) -> bool {
            return lhs != rhs;
        };
#else
        operatorPtr operatorNe = operator!=;
        (void) operatorNe;
#endif
    }

    const int NUM_DATA                     = DEFAULT_NUM_DATA;
    const DefaultDataRow (&DATA)[NUM_DATA] = DEFAULT_DATA;

    if (verbose) printf("\nCompare every value with every value.\n");
    {
        // Create first object
        for (int ti = 0; ti < NUM_DATA; ++ti) {
            const int         LINE1   = DATA[ti].d_line;
            const int         INDEX1  = DATA[ti].d_index;
            const char *const SPEC1   = DATA[ti].d_spec;
            const size_t      LENGTH1 = strlen(DATA[ti].d_results);

           if (veryVerbose) { T_ P_(LINE1) P_(INDEX1) P_(LENGTH1) P(SPEC1) }

            // Ensure an object compares correctly with itself (alias test).
            {
                bslma::TestAllocator scratch("scratch", veryVeryVeryVerbose);

                Obj mX(&scratch); const Obj& X = gg(&mX, SPEC1);

                ASSERTV(LINE1, X,   X == X);
                ASSERTV(LINE1, X, !(X != X));
            }

            for (int tj = 0; tj < NUM_DATA; ++tj) {
                const int         LINE2   = DATA[tj].d_line;
                const int         INDEX2  = DATA[tj].d_index;
                const char *const SPEC2   = DATA[tj].d_spec;
                const size_t      LENGTH2 = strlen(DATA[tj].d_results);

                if (veryVerbose) {
                              T_ T_ P_(LINE2) P_(INDEX2) P_(LENGTH2) P(SPEC2) }

                const bool EXP = INDEX1 == INDEX2;  // expected result

                for (char cfg = 'a'; cfg <= 'b'; ++cfg) {

                    const char CONFIG = cfg;  // Determines `Y`s allocator.

                    // Create two distinct test allocators, `oax` and `oay`.

                    bslma::TestAllocator oax("objectx", veryVeryVeryVerbose);
                    bslma::TestAllocator oay("objecty", veryVeryVeryVerbose);

                    // Map allocators above to objects `X` and `Y` below.

                    bslma::TestAllocator& xa = oax;
                    bslma::TestAllocator& ya = 'a' == CONFIG ? oax : oay;

                    Obj mX(&xa); const Obj& X = gg(&mX, SPEC1);
                    Obj mY(&ya); const Obj& Y = gg(&mY, SPEC2);

                    ASSERTV(LINE1, LINE2, CONFIG, LENGTH1 == X.size());
                    ASSERTV(LINE1, LINE2, CONFIG, LENGTH2 == Y.size());

                    if (veryVerbose) { T_ T_ P_(X) P(Y); }

                    // Verify value, commutativity, and no memory allocation.

                    bslma::TestAllocatorMonitor oaxm(&xa);
                    bslma::TestAllocatorMonitor oaym(&ya);

                    TestComparator<KEY>::disableFunctor();

                    ASSERTV(LINE1, LINE2, CONFIG,  EXP == (X == Y));
                    ASSERTV(LINE1, LINE2, CONFIG,  EXP == (Y == X));

                    ASSERTV(LINE1, LINE2, CONFIG, !EXP == (X != Y));
                    ASSERTV(LINE1, LINE2, CONFIG, !EXP == (Y != X));

                    TestComparator<KEY>::enableFunctor();

                    ASSERTV(LINE1, LINE2, CONFIG, oaxm.isTotalSame());
                    ASSERTV(LINE1, LINE2, CONFIG, oaym.isTotalSame());
                }
            }
        }
    }
}

template <class KEY, class VALUE, class COMP, class ALLOC>
void TestDriver<KEY, VALUE, COMP, ALLOC>::testCase4()
{
    // ------------------------------------------------------------------------
    // BASIC ACCESSORS
    //   Ensure each basic accessor:
    //     - cbegin
    //     - cend
    //     - size
    //     - get_allocator
    //   properly interprets object state.
    //
    // Concerns:
    // 1. Each accessor returns the value of the correct property of the
    //    object.
    //
    // 2. Each accessor method is declared `const`.
    //
    // 3. No accessor allocates any memory.
    //
    // 4. The range `[cbegin(), cend())` contains inserted elements the sorted
    //    order.
    //
    // Plan:
    // 1. For each set of `SPEC` of different length:
    //
    //   1. Default construct the object with various configuration:
    //
    //     1. Use the `gg` function to populate the object based on the SPEC.
    //
    //     2. Verify the correct allocator is installed with the
    //        `get_allocator` method.
    //
    //     3. Verify the object contains the expected number of elements.
    //
    //     4. Use `cbegin` and `cend` to iterate through all elements and
    //        verify the values are as expected.  (C-1..2, 4)
    //
    //     5. Monitor the memory allocated from both the default and object
    //        allocators before and after calling the accessor; verify that
    //        there is no change in total memory allocation.  (C-3)
    //
    // Testing:
    //   const_iterator cbegin();
    //   const_iterator cend();
    //   size_type size() const;
    //   allocator_type get_allocator() const;
    // ------------------------------------------------------------------------

    static const struct {
        int         d_line;                     // source line number
        const char *d_spec;                     // specification string
        const char *d_results;                  // expected results
    } DATA[] = {
        //line  spec      result
        //----  --------  ------
        { L_,   "",       ""      },
        { L_,   "A",      "A"     },
        { L_,   "AB",     "AB"    },
        { L_,   "ABC",    "ABC"   },
        { L_,   "ABCD",   "ABCD"  },
        { L_,   "ABCDE",  "ABCDE" },
        { L_,   "AAAAA",  "AAAAA" }
    };
    const int NUM_DATA = static_cast<int>(sizeof DATA / sizeof *DATA);

    if (verbose) { printf(
                "\nCreate objects with various allocator configurations.\n"); }
    {
        for (int ti = 0; ti < NUM_DATA; ++ti) {
            const int         LINE   = DATA[ti].d_line;
            const char *const SPEC   = DATA[ti].d_spec;
            const size_t      LENGTH = strlen(DATA[ti].d_results);
            const TestValues  EXP(DATA[ti].d_results);

            if (verbose) { P_(LINE) P_(LENGTH) P(SPEC); }

            for (char cfg = 'a'; cfg <= 'd'; ++cfg) {
                const char CONFIG = cfg;

                bslma::TestAllocator  da("default",   veryVeryVeryVerbose);
                bslma::TestAllocator  fa("footprint", veryVeryVeryVerbose);
                bslma::TestAllocator sa1("supplied1", veryVeryVeryVerbose);
                bslma::TestAllocator sa2("supplied2", veryVeryVeryVerbose);

                bslma::DefaultAllocatorGuard dag(&da);

                Obj                 *objPtr;
                bslma::TestAllocator *objAllocatorPtr;

                switch (CONFIG) {
                  case 'a': {
                      objPtr = new (fa) Obj();
                      objAllocatorPtr = &da;
                  } break;
                  case 'b': {
                      objPtr = new (fa) Obj(0);
                      objAllocatorPtr = &da;
                  } break;
                  case 'c': {
                      objPtr = new (fa) Obj(&sa1);
                      objAllocatorPtr = &sa1;
                  } break;
                  case 'd': {
                      objPtr = new (fa) Obj(&sa2);
                      objAllocatorPtr = &sa2;
                  } break;
                  default: {
                      ASSERTV(CONFIG, "Bad allocator config.", false);
                  } return;                                           // RETURN
                }

                Obj& mX = *objPtr; const Obj& X = gg(&mX, SPEC);

                bslma::TestAllocator&  oa = *objAllocatorPtr;
                bslma::TestAllocator& noa = ('c' == CONFIG || 'd' == CONFIG)
                                         ? da
                                         : sa1;

                // --------------------------------------------------------

                // Verify basic accessor

                bslma::TestAllocatorMonitor oam(&oa);

                ASSERTV(LINE, SPEC, CONFIG, &oa == X.get_allocator());
                ASSERTV(LINE, SPEC, CONFIG, LENGTH == X.size());

                size_t i = 0;
                for (CIter iter = X.cbegin(); iter != X.cend(); ++iter, ++i) {
                    ASSERTV(LINE, SPEC, CONFIG, EXP[i] == *iter);
                }

                ASSERTV(LINE, SPEC, CONFIG, LENGTH == i);

                ASSERT(oam.isTotalSame());

                // --------------------------------------------------------

                // Reclaim dynamically allocated object under test.

                fa.deleteObject(objPtr);

                // Verify no allocation from the non-object allocator.

                ASSERTV(LINE, CONFIG, noa.numBlocksTotal(),
                        0 == noa.numBlocksTotal());

                // Verify all memory is released on object destruction.

                ASSERTV(LINE, CONFIG, da.numBlocksInUse(),
                        0 == da.numBlocksInUse());
                ASSERTV(LINE, CONFIG, fa.numBlocksInUse(),
                        0 == fa.numBlocksInUse());
                ASSERTV(LINE, CONFIG, sa1.numBlocksInUse(),
                        0 == sa1.numBlocksInUse());
                ASSERTV(LINE, CONFIG, sa2.numBlocksInUse(),
                        0 == sa2.numBlocksInUse());
            }
        }
    }
}

template <class KEY, class VALUE, class COMP, class ALLOC>
void TestDriver<KEY, VALUE, COMP, ALLOC>::testCase3()
{
    // ------------------------------------------------------------------------
    // TESTING PRIMITIVE GENERATOR FUNCTIONS gg AND ggg
    //   Having demonstrated that our primary manipulators work as expected
    //   under normal conditions
    //
    // Concerns:
    // 1. Valid generator syntax produces expected results
    //
    // 2. Invalid syntax is detected and reported.
    //
    // Plan:
    // 1. For each of an enumerated sequence of `spec` values, ordered by
    //    increasing `spec` length:
    //
    //   1. Use the primitive generator function `gg` to set the state of a
    //      newly created object.
    //
    //   2. Verify that `gg` returns a valid reference to the modified argument
    //      object.
    //
    //   3. Use the basic accessors to verify that the value of the object is
    //      as expected.  (C-1)
    //
    // 2. For each of an enumerated sequence of `spec` values, ordered by
    //    increasing `spec` length, use the primitive generator function `ggg`
    //    to set the state of a newly created object.
    //
    //   1. Verify that `ggg` returns the expected value corresponding to the
    //      location of the first invalid value of the `spec`.  (C-2)
    //
    // Testing:
    //   multimap& gg(multimap *object, const char *spec);
    //   int ggg(multimap *object, const char *spec, int verbose = 1);
    // ------------------------------------------------------------------------

    bslma::TestAllocator oa(veryVeryVerbose);

    if (verbose) printf("\nTesting generator on valid specs.\n");
    {
        static const struct {
            int         d_line;                 // source line number
            const char *d_spec;                 // specification string
            const char *d_results;  // expected element values
        } DATA[] = {
            //line  spec      results
            //----  --------  -------
            { L_,   "",       ""      },
            { L_,   "A",      "A"     },
            { L_,   "B",      "B"     },
            { L_,   "AB",     "AB"    },
            { L_,   "CD",     "CD"    },
            { L_,   "ABC",    "ABC"   },
            { L_,   "ABCD",   "ABCD"  },
            { L_,   "ABCDE",  "ABCDE" },

        };
        const int NUM_DATA = static_cast<int>(sizeof DATA / sizeof *DATA);

        int oldLen = -1;
        for (int ti = 0; ti < NUM_DATA ; ++ti) {
            const int         LINE   = DATA[ti].d_line;
            const char *const SPEC   = DATA[ti].d_spec;
            const size_t      LENGTH = strlen(DATA[ti].d_results);
            const TestValues  EXP(DATA[ti].d_results);
            const int         curLen = (int)strlen(SPEC);

            Obj mX(&oa);
            const Obj& X = gg(&mX, SPEC);   // original spec

            Obj mY(&oa);
            const Obj& Y = gg(&mY, SPEC);    // extended spec

            if (curLen != oldLen) {
                if (verbose) printf("\tof length %d:\n", curLen);
                ASSERTV(LINE, oldLen <= curLen);  // non-decreasing
                oldLen = curLen;
            }

            if (veryVerbose) {
                printf("\t\tSpec = \"%s\"\n", SPEC);
                T_ T_ T_ P(X);
                T_ T_ T_ P(Y);
            }

            ASSERTV(LINE, LENGTH == X.size());
            ASSERTV(LINE, LENGTH == Y.size());
            ASSERTV(0 == verifyContainer(X, EXP, LENGTH));
            ASSERTV(0 == verifyContainer(Y, EXP, LENGTH));
        }
    }

    if (verbose) printf("\nTesting generator on invalid specs.\n");
    {
        static const struct {
            int         d_line;     // source line number
            const char *d_spec;   // specification string
            int         d_index;    // offending character index
        } DATA[] = {
            //line  spec      index
            //----  --------  -----
            { L_,   "",       -1,     }, // control

            { L_,   "A",      -1,     }, // control
            { L_,   " ",       0,     },
            { L_,   ".",       0,     },
            { L_,   "E",       -1,    }, // control
            { L_,   "a",       0,     },
            { L_,   "z",       0,     },

            { L_,   "AE",     -1,     }, // control
            { L_,   "aE",      0,     },
            { L_,   "Ae",      1,     },
            { L_,   ".~",      0,     },
            { L_,   "~!",      0,     },
            { L_,   "  ",      0,     },

            { L_,   "ABC",    -1,     }, // control
            { L_,   " BC",     0,     },
            { L_,   "A C",     1,     },
            { L_,   "AB ",     2,     },
            { L_,   "?#:",     0,     },
            { L_,   "   ",     0,     },

            { L_,   "ABCDE",  -1,     }, // control
            { L_,   "aBCDE",   0,     },
            { L_,   "ABcDE",   2,     },
            { L_,   "ABCDe",   4,     },
            { L_,   "AbCdE",   1,     }
        };
        const int NUM_DATA =
                            static_cast<int>(sizeof DATA / sizeof *DATA);

        int oldLen = -1;
        for (int ti = 0; ti < NUM_DATA ; ++ti) {
            const int         LINE   = DATA[ti].d_line;
            const char *const SPEC   = DATA[ti].d_spec;
            const int         INDEX  = DATA[ti].d_index;
            const size_t      LENGTH = strlen(SPEC);

            Obj mX(&oa);

            if ((int)LENGTH != oldLen) {
                if (verbose) printf("\tof length " ZU ":\n", LENGTH);
                 ASSERTV(LINE, oldLen <= (int)LENGTH);  // non-decreasing
                oldLen = static_cast<int>(LENGTH);
            }

            if (veryVerbose) printf("\t\tSpec = \"%s\"\n", SPEC);

            int RESULT = ggg(&mX, SPEC, veryVerbose);

            ASSERTV(LINE, INDEX == RESULT);
        }
    }
}

template <class KEY, class VALUE, class COMP, class ALLOC>
void TestDriver<KEY, VALUE, COMP, ALLOC>::testCase2()
{
    // ------------------------------------------------------------------------
    // TESTING PRIMARY MANIPULATORS (BOOTSTRAP)
    //   The basic concern is that the default constructor, the destructor,
    //   and, under normal conditions (i.e., no aliasing), the primary
    //   manipulators
    //      - insert
    //      - clear
    //
    // Concerns:
    // 1. An object created with the default constructor (with or without a
    //    supplied allocator) has the contractually specified default value.
    //
    // 2. If an allocator is NOT supplied to the default constructor, the
    //    default allocator in effect at the time of construction becomes the
    //    object allocator for the resulting object.
    //
    // 3. If an allocator IS supplied to the default constructor, that
    //    allocator becomes the object allocator for the resulting object.
    //
    // 4. Supplying a null allocator address has the same effect as not
    //    supplying an allocator.
    //
    // 5. Supplying an allocator to the default constructor has no effect on
    //    subsequent object values.
    //
    // 6. Any memory allocation is from the object allocator.
    //
    // 7. There is no temporary allocation from any allocator.
    //
    // 8. Every object releases any allocated memory at destruction.
    //
    // 9. QoI: The default constructor allocates no memory.
    //
    // 10. `insert` adds an additional element to the object and return the
    //    iterator to the newly added element.
    //
    // 11. Duplicated values are inserted at the end of its range.
    //
    // 12. `clear` properly destroys each contained element value.
    //
    // 13. `clear` does not allocate memory.
    //
    // 14. Any argument can be `const`.
    //
    // 15. Any memory allocation is exception neutral.
    //
    // Plan:
    // 1. For each value of increasing length, `L`:
    //
    //   2. Using a loop-based approach, default-construct three distinct
    //      objects, in turn, but configured differently: (a) without passing
    //      an allocator, (b) passing a null allocator address explicitly,
    //      and (c) passing the address of a test allocator distinct from the
    //      default.  For each of these three iterations:  (C-1..14)
    //
    //     1. Create three `bslma::TestAllocator` objects, and install one as
    //        the current default allocator (note that a ubiquitous test
    //        allocator is already installed as the global allocator).
    //
    //     2. Use the default constructor to dynamically create an object
    //        `X`, with its object allocator configured appropriately (see
    //        P-2); use a distinct test allocator for the object's footprint.
    //
    //     3. Use the (as yet unproven) `get_allocator` to ensure that its
    //        object allocator is properly installed.  (C-2..4)
    //
    //     4. Use the appropriate test allocators to verify that no memory is
    //        allocated by the default constructor.  (C-9)
    //
    //     5. Use the individual (as yet unproven) salient attribute accessors
    //        to verify the default-constructed value.  (C-1)
    //
    //     6. Insert `L - 1` elements in order of increasing value into the
    //        container.
    //
    //     7. Insert the `L`th value in the presense of exception and use the
    //        (as yet unproven) basic accessors to verify the container has the
    //        expected values.  Verify the number of allocation is as expected.
    //        (C-5..6, 14..15)
    //
    //     8. Verify that no temporary memory is allocated from the object
    //        allocator.  (C-7)
    //
    //     9. Invoke `clear` and verify that the container is empty.  Verify
    //        that no memory is allocated.  (C-12..13)
    //
    //    10. Verify that all object memory is released when the object is
    //        destroyed.  (C-8)
    //
    //    11. Insert `L` distinct elements and record the iterators returned.
    //
    //    12. Insert the same `L` elements again and verify that incrementing
    //        the iterators returned gives the iterator to the next smallest
    //        value.
    //
    //    13. Perform P-1.2.12 again.  (C-11)
    //
    // Testing:
    //   multimap(const C& comparator, const A& allocator);
    //   ~multimap();
    //   iterator insert(const value_type& value);
    //   void clear();
    // ------------------------------------------------------------------------

    const int TYPE_ALLOC = (bslma::UsesBslmaAllocator<KEY>::value +
                            bslma::UsesBslmaAllocator<VALUE>::value);

    if (verbose) { P(TYPE_ALLOC); }

    const TestValues VALUES;  // contains 52 distinct increasing values

    const size_t MAX_LENGTH = 9;

    for (size_t ti = 0; ti < MAX_LENGTH; ++ti) {
        const size_t LENGTH = ti;

        if (verbose) {
            printf("\nTesting with various allocator configurations.\n");
        }
        for (char cfg = 'a'; cfg <= 'c'; ++cfg) {
            const char CONFIG = cfg;  // how we specify the allocator

            bslma::TestAllocator da("default",   veryVeryVeryVerbose);
            bslma::TestAllocator fa("footprint", veryVeryVeryVerbose);
            bslma::TestAllocator sa("supplied",  veryVeryVeryVerbose);

            bslma::DefaultAllocatorGuard dag(&da);

            // ----------------------------------------------------------------

            if (veryVerbose) {
                printf("\n\tTesting default constructor.\n");
            }

            Obj                  *objPtr;
            bslma::TestAllocator *objAllocatorPtr;

            switch (CONFIG) {
              case 'a': {
                  objPtr = new (fa) Obj();
                  objAllocatorPtr = &da;
              } break;
              case 'b': {
                  objPtr = new (fa) Obj(0);
                  objAllocatorPtr = &da;
              } break;
              case 'c': {
                  objPtr = new (fa) Obj(&sa);
                  objAllocatorPtr = &sa;
              } break;
              default: {
                  ASSERTV(CONFIG, "Bad allocator config.", false);
              } return;                                               // RETURN
            }

            Obj&                   mX = *objPtr; const Obj& X = mX;
            bslma::TestAllocator&  oa = *objAllocatorPtr;
            bslma::TestAllocator& noa = 'c' != CONFIG ? sa : da;

            // Verify any attribute allocators are installed properly.

            ASSERTV(LENGTH, CONFIG, &oa == X.get_allocator());

            // Verify no allocation from the object/non-object allocators.

            ASSERTV(LENGTH, CONFIG, oa.numBlocksTotal(),
                    0 ==  oa.numBlocksTotal());
            ASSERTV(LENGTH, CONFIG, noa.numBlocksTotal(),
                    0 == noa.numBlocksTotal());

            ASSERTV(LENGTH, CONFIG, 0 == X.size());
            ASSERTV(LENGTH, CONFIG, X.cbegin() == X.cend());

            // ----------------------------------------------------------------

            if (veryVerbose) { printf("\n\tTesting `insert` (bootstrap).\n"); }

            bslma::TestAllocator scratch("scratch", veryVeryVeryVerbose);

            if (0 < LENGTH) {
                if (verbose) {
                    printf("\t\tOn an object of initial length " ZU ".\n",
                           LENGTH);
                }

                for (size_t tj = 0; tj < LENGTH; ++tj) {
                    int id      = TstFacility::getIdentifier(VALUES[tj].first);
                    Iter RESULT = primaryManipulator(&mX, id, &scratch);

                    ASSERTV(LENGTH, tj, CONFIG, VALUES[tj] == *RESULT);
                }

                ASSERTV(LENGTH, CONFIG, LENGTH == X.size());
                if (veryVerbose) {
                    printf("\t\t\tMultimap: ");
                    P(X);
                }
            }

            // ----------------------------------------------------------------

            if (veryVerbose) { printf("\n\tTesting `clear`.\n"); }
            {
                const bsls::Types::Int64 BB = oa.numBlocksTotal();
                const bsls::Types::Int64 B  = oa.numBlocksInUse();

                mX.clear();

                ASSERTV(LENGTH, CONFIG, 0 == X.size());
                ASSERTV(LENGTH, CONFIG, X.cbegin() == X.cend());

                const bsls::Types::Int64 AA = oa.numBlocksTotal();
                const bsls::Types::Int64 A  = oa.numBlocksInUse();

                ASSERTV(LENGTH, CONFIG, BB == AA);
                ASSERTV(LENGTH, CONFIG, B, A,
                        B - (int)LENGTH * TYPE_ALLOC == A);


                ASSERTV(LENGTH, CONFIG, 0 == X.size());
                ASSERTV(LENGTH, CONFIG, X.cbegin() == X.cend());
            }

            // ----------------------------------------------------------------

            if (veryVerbose) { printf(
                                "\n\tTesting `insert` duplicated values.\n"); }
            {
                CIter ITER[MAX_LENGTH + 1];

                for (size_t tj = 0; tj < LENGTH; ++tj) {
                    int id = TstFacility::getIdentifier(VALUES[tj].first);
                    ITER[tj] = primaryManipulator(&mX, id, &scratch);
                    ASSERTV(LENGTH, tj, CONFIG, VALUES[tj] == *ITER[tj]);
                }
                ITER[LENGTH] = X.end();

                ASSERTV(LENGTH, CONFIG, LENGTH == X.size());

                for (size_t tj = 0; tj < LENGTH; ++tj) {
                    int id = TstFacility::getIdentifier(VALUES[tj].first);
                    Iter RESULT = primaryManipulator(&mX, id, &scratch);
                    ASSERTV(LENGTH, tj, CONFIG, VALUES[tj] == *RESULT);
                    ++RESULT;
                    ASSERTV(LENGTH, tj, CONFIG, ITER[tj + 1] == RESULT);
                }

                ASSERTV(LENGTH, CONFIG, 2 * LENGTH == X.size());

                for (size_t tj = 0; tj < LENGTH; ++tj) {
                    int id = TstFacility::getIdentifier(VALUES[tj].first);
                    Iter RESULT = primaryManipulator(&mX, id, &scratch);
                    ASSERTV(LENGTH, tj, CONFIG, VALUES[tj] == *RESULT);
                    ++RESULT;
                    ASSERTV(LENGTH, tj, CONFIG, ITER[tj + 1] == RESULT);
                }

                ASSERTV(LENGTH, CONFIG, 3 * LENGTH == X.size());
            }

            // ----------------------------------------------------------------

            // Reclaim dynamically allocated object under test.

            fa.deleteObject(objPtr);

            // Verify all memory is released on object destruction.

            ASSERTV(LENGTH, CONFIG, da.numBlocksInUse(),
                    0 == da.numBlocksInUse());
            ASSERTV(LENGTH, CONFIG, fa.numBlocksInUse(),
                    0 == fa.numBlocksInUse());
            ASSERTV(LENGTH, CONFIG, sa.numBlocksInUse(),
                    0 == sa.numBlocksInUse());
        }
    }
}

template <class KEY, class VALUE, class COMP, class ALLOC>
void TestDriver<KEY, VALUE, COMP, ALLOC>::testCase1(const COMP&  comparator,
                                             KEY         *testKeys,
                                             VALUE       *testValues,
                                             size_t       numValues)
{
    // ------------------------------------------------------------------------
    // BREATHING TEST
    //   This case exercises (but does not fully test) basic functionality.
    //
    // Concerns:
    // 1. The class is sufficiently functional to enable comprehensive
    //    testing in subsequent test cases.
    //
    // Plan:
    // 1. Execute each methods to verify functionality for simple case.
    //
    // Testing:
    //   BREATHING TEST
    // ------------------------------------------------------------------------

    typedef bsl::multimap<KEY, VALUE, COMP>  Obj;
    typedef typename Obj::iterator               iterator;
    typedef typename Obj::const_iterator         const_iterator;
    typedef typename Obj::reverse_iterator       reverse_iterator;
    typedef typename Obj::const_reverse_iterator const_reverse_iterator;

    typedef typename Obj::value_type Value;

    bslma::TestAllocator defaultAllocator("defaultAllocator");
    bslma::DefaultAllocatorGuard defaultGuard(&defaultAllocator);

    bslma::TestAllocator objectAllocator("objectAllocator");

    // Sanity check.

    ASSERTV(0 < numValues);
    ASSERTV(8 > numValues);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    if (veryVerbose) {
        printf("Default construct an empty multimap.\n");
    }
    {
        Obj x(&objectAllocator); const Obj& X = x;
        ASSERTV(0    == X.size());
        ASSERTV(true == X.empty());
        ASSERTV(0    <  X.max_size());
        ASSERTV(0    == defaultAllocator.numBytesInUse());
        ASSERTV(0    == objectAllocator.numBytesInUse());
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    if (veryVerbose) {
        printf("Test use of allocators.\n");
    }
    {
        bslma::TestAllocator objectAllocator1("objectAllocator1");
        bslma::TestAllocator objectAllocator2("objectAllocator2");

        Obj o1(comparator, &objectAllocator1); const Obj& O1 = o1;
        ASSERTV(&objectAllocator1 == O1.get_allocator().mechanism());

        for (size_t i = 0; i < numValues; ++i) {
            o1.insert(Value(testKeys[i], testValues[i]));
        }
        ASSERTV(numValues == O1.size());
        ASSERTV(0 <  objectAllocator1.numBytesInUse());
        ASSERTV(0 == objectAllocator2.numBytesInUse());

        bslma::TestAllocatorMonitor monitor1(&objectAllocator1);
        Obj o2(O1, &objectAllocator2); const Obj& O2 = o2;
        ASSERTV(&objectAllocator2 == O2.get_allocator().mechanism());

        ASSERTV(numValues == O1.size());
        ASSERTV(numValues == O2.size());
        ASSERTV(monitor1.isInUseSame());
        ASSERTV(monitor1.isTotalSame());
        ASSERTV(0 <  objectAllocator1.numBytesInUse());
        ASSERTV(0 <  objectAllocator2.numBytesInUse());

        Obj o3(comparator, &objectAllocator1); const Obj& O3 = o3;
        ASSERTV(&objectAllocator1 == O3.get_allocator().mechanism());

        ASSERTV(numValues == O1.size());
        ASSERTV(numValues == O2.size());
        ASSERTV(0         == O3.size());
        ASSERTV(monitor1.isInUseSame());
        ASSERTV(monitor1.isTotalSame());
        ASSERTV(0 <  objectAllocator1.numBytesInUse());
        ASSERTV(0 <  objectAllocator2.numBytesInUse());

        o1.swap(o3);
        ASSERTV(0         == O1.size());
        ASSERTV(numValues == O2.size());
        ASSERTV(numValues == O3.size());
        ASSERTV(monitor1.isInUseSame());
        ASSERTV(monitor1.isTotalSame());
        ASSERTV(0 <  objectAllocator1.numBytesInUse());
        ASSERTV(0 <  objectAllocator2.numBytesInUse());

        o3.swap(o2);
        ASSERTV(0         == O1.size());
        ASSERTV(numValues == O2.size());
        ASSERTV(numValues == O3.size());
        ASSERTV(!monitor1.isInUseUp());  // Memory usage may go down depending
                                         // on implementation
        ASSERTV(monitor1.isTotalUp());
        ASSERTV(0 <  objectAllocator1.numBytesInUse());
        ASSERTV(0 <  objectAllocator2.numBytesInUse());

        ASSERTV(&objectAllocator1 == O1.get_allocator().mechanism());
        ASSERTV(&objectAllocator2 == O2.get_allocator().mechanism());
        ASSERTV(&objectAllocator1 == O3.get_allocator().mechanism());
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    if (veryVerbose) {
        printf("Test primary manipulators/accessors on every permutation.\n");
    }

    std::sort(testKeys, testKeys + numValues, comparator);
    do {
        // For each possible permutation of values, insert values, iterate over
        // the resulting container, find values, and then erase values.

        Obj x(comparator, &objectAllocator); const Obj& X = x;
        for (size_t i = 0; i < numValues; ++i) {
            Obj y(X, &objectAllocator); const Obj& Y = y;

            ASSERTV(X == Y);
            ASSERTV(!(X != Y));

            // Test `insert`.
            Value value(testKeys[i], testValues[i]);
            iterator result = x.insert(value);
            ASSERTV(X.end()       != result);
            ASSERTV(testKeys[i]   == result->first);
            ASSERTV(testValues[i] == result->second);


            // Test size, empty.
            ASSERTV(i + 1 == X.size());
            ASSERTV(false == X.empty());

            // Test lower_bound, upper_bound, and find
            const_iterator start = X.lower_bound(testKeys[i]);
            const_iterator end   = X.upper_bound(testKeys[i]);
            ASSERTV(result->first == X.find(testKeys[i])->first);
            ASSERTV(X.end() == end || end->first != result->first);
            if (X.begin() != start) {
                const_iterator prev = start; --prev;
                ASSERTV(prev->first != result->first);
            }
            bool found = false;
            for (; start != end; ++start) {
                ASSERTV(result->first == start->first);
                if (start == result) {
                    found = true;
                }
            }
            ASSERTV(found);

            ASSERTV(X != Y);
            ASSERTV(!(X == Y));

            y = x;
            ASSERTV(X == Y);
            ASSERTV(!(X != Y));
        }

        ASSERTV(0 != objectAllocator.numBytesInUse());
        ASSERTV(0 == defaultAllocator.numBytesInUse());
        // Verify sorted order of elements.

        {
            const_iterator last = X.begin();
            const_iterator it   = ++(X.begin());
            while (it != X.end()) {
                ASSERTV(!comparator(it->first, last->first));
                ASSERTV(!comparator((*it).first, (*last).first));

                last = it;
                ++it;
            }
        }

        // Test iterators.
        {
            const_iterator cbi  = X.begin();
            const_iterator ccbi = X.cbegin();
            iterator       bi   = x.begin();

            const_iterator last = X.begin();
            while (cbi != X.end()) {
                ASSERTV(cbi == ccbi);
                ASSERTV(cbi == bi);

                if (cbi != X.begin()) {
                    ASSERTV(!comparator(cbi->first, last->first));
                }
                last = cbi;
                ++bi; ++ccbi; ++cbi;
            }

            ASSERTV(cbi  == X.end());
            ASSERTV(ccbi == X.end());
            ASSERTV(bi   == X.end());
            --bi; --ccbi; --cbi;

            reverse_iterator       ri   = x.rbegin();
            const_reverse_iterator rci  = X.rbegin();
            const_reverse_iterator rcci = X.crbegin();

            while  (rci != X.rend()) {
                ASSERTV(cbi == ccbi);
                ASSERTV(cbi == bi);
                ASSERTV(rci == rcci);
                ASSERTV(ri->first == rcci->first);

                if (rci !=  X.rbegin()) {
                    // Verify that cbi & rci are <= last.
                    ASSERTV(!comparator(last->first, cbi->first));
                    ASSERTV(!comparator(last->first, rci->first));
                }

                last = cbi;
                if (cbi != X.begin()) {
                    --bi; --ccbi; --cbi;
                }
                ++ri; ++rcci; ++rci;
            }
            ASSERTV(cbi  == X.begin());
            ASSERTV(ccbi == X.begin());
            ASSERTV(bi   == X.begin());

            ASSERTV(rci  == X.rend());
            ASSERTV(rcci == X.rend());
            ASSERTV(ri   == x.rend());
        }

        // Use erase(iterator) on all the elements.
        for (size_t i = 0; i < numValues; ++i) {
            const_iterator it     = x.find(testKeys[i]);
            const_iterator nextIt = it;
            ++nextIt;

            ASSERTV(X.end()       != it);
            ASSERTV(testKeys[i]   == it->first);
            ASSERTV(testValues[i] == it->second);

            const_iterator resIt     = x.erase(it);
            ASSERTV(resIt             == nextIt);
            ASSERTV(numValues - i - 1 == X.size());
            if (resIt != X.end()) {
                ASSERTV(!comparator(resIt->first, testKeys[i]));
            }
        }
    } while (std::next_permutation(testKeys,
                                   testKeys + numValues,
                                   comparator));

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    if (veryVerbose) {
        printf("Test `erase(const_iterator, const_iterator )`.\n");
    }

    {
        // Iterate over the starting point and end point for a range of values
        // to remove from the multimap, then verify the values have been
        // removed.  Note that this test requires the `testKeys` array to be in
        // the same (sorted) order as the eventual container.

        std::sort(testKeys, testKeys + numValues, comparator);
        for (size_t i = 0; i < numValues; ++i) {
            for (size_t j = i; j < numValues; ++j) {
                Obj x(comparator, &objectAllocator); const Obj& X = x;
                for (size_t k = 0; k < numValues; ++k) {
                    x.insert(Value(testKeys[k], static_cast<VALUE>(k)));
                }

                const_iterator a = X.find(testKeys[i]);
                const_iterator b = X.find(testKeys[j]);

                if (!comparator(testKeys[i], testKeys[j])) {
                    std::swap(a, b);
                }

                KEY min = a->first;
                KEY max = b->first;
                ASSERTV(!comparator(max, min)); // min <= max

                size_t numElements = bsl::distance(a, b);
                iterator endPoint = x.erase(a, b);

                ASSERTV(numValues - numElements == X.size());
                ASSERTV(endPoint                == b);

                for (size_t k = 0; k < numValues; ++k) {

                    Value value(testKeys[k], static_cast<VALUE>(k));
                    const_iterator it = std::find(X.begin(),
                                                  X.end(),
                                                  value);
                    bool found = it != X.end();
                    ASSERTV(found == (k < i || k >=j));
                }
            }
        }
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if defined(BSLS_LIBRARYFEATURES_HAS_CPP11_BASELINE_LIBRARY)
    std::shuffle(testKeys,
                 testKeys + numValues,
                 std::default_random_engine());
#else  // fall-back for C++03, potentially unsupported in C++17
    std::random_shuffle(testKeys,  testKeys + numValues);
#endif

    if (veryVerbose) {
        printf("Test `erase(const key_type&)`.\n");
    }
    {
        Obj x(comparator, &objectAllocator); const Obj& X = x;
        for (size_t i = 0; i < numValues; ++i) {
            Value value(testKeys[i], testValues[i]);
            x.insert(value);
        }

        size_t totalRemoved = 0;
        for (size_t i = 0; i < numValues; ++i) {
            size_t count = bsl::distance(X.lower_bound(testKeys[i]),
                                         X.upper_bound(testKeys[i]));
            ASSERTV(count == x.erase(testKeys[i]));
            ASSERTV(0     == x.erase(testKeys[i]));

            totalRemoved += count;
            ASSERTV(numValues - totalRemoved == X.size());
        }
        ASSERTV(0 == X.size());
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    if (veryVerbose) {
        printf("Test insert & multimap for iterator ranges.\n");
    }
    {
        // Create a temporary array of multimap::value_type objects, and
        // insert various ranges from that array into a multimap.

        typedef bsl::pair<KEY, VALUE> NonConstValue;
        NonConstValue *myValues = new NonConstValue[numValues];
        for (size_t i = 0; i < numValues; ++i) {
            myValues[i].first  = testKeys[i];
            myValues[i].second = testValues[i];
        }

        for (size_t i = 0; i < numValues; ++i) {
            for (size_t length = 0; length <= numValues - i; ++length) {
                Obj x(comparator, &objectAllocator); const Obj& X = x;
                for (size_t k = 0; k < length; ++k) {
                    size_t index = i + k;
                    iterator result = x.insert(myValues[index]);
                    ASSERTV(myValues[index].first  == result->first);
                    ASSERTV(myValues[index].second == result->second);
                }
                Obj y(comparator, &objectAllocator); const Obj& Y = y;
                y.insert(myValues + i, myValues + (i + length));

                Obj z(myValues + i,
                      myValues + (i + length),
                      comparator,
                      &objectAllocator);
                const Obj& Z = z;
                ASSERTV(X == Y);
                ASSERTV(X == Z);
            }
        }
        delete [] myValues;
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    if (veryVerbose) {
        printf("Test `equal_range`\n");
    }
    {
        Obj x(comparator, &objectAllocator); const Obj& X = x;
        for (size_t i = 0; i < numValues; ++i) {
            Value value(testKeys[i], testValues[i]);
            x.insert(value);
        }

        for (size_t i = 0; i < numValues; ++i) {
            bsl::pair<iterator, iterator> result = x.equal_range(testKeys[i]);
            bsl::pair<const_iterator, const_iterator> cresult =
                                                    X.equal_range(testKeys[i]);

            ASSERTV(cresult.first  == result.first);
            ASSERTV(cresult.second == result.second);
            const_iterator it  = cresult.first;
            const_iterator end = cresult.second;

            if (it != X.begin()) {
                const_iterator prev = it; --prev;
                ASSERTV(testKeys[i] != prev->first);
            }
            for (; it != end; ++it) {
                ASSERTV(testKeys[i] == it->first);
            }
            ASSERTV(X.end() == end || testKeys[i] != end->first);
        }
        for (size_t i = 0; i < numValues; ++i) {
            x.erase(testKeys[i]);
            bsl::pair<iterator, iterator> result = x.equal_range(testKeys[i]);
            bsl::pair<const_iterator, const_iterator> cresult =
                                                  x.equal_range(testKeys[i]);

            iterator       li = x.lower_bound(testKeys[i]);
            const_iterator LI = X.lower_bound(testKeys[i]);
            iterator       ui = x.upper_bound(testKeys[i]);
            const_iterator UI = X.upper_bound(testKeys[i]);

            ASSERTV(result.first   == li);
            ASSERTV(result.second  == ui);
            ASSERTV(cresult.first  == LI);
            ASSERTV(cresult.second == UI);
        }
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    if (veryVerbose) {
        printf("Test `operator<`, `operator>`, `operator<=`, `operator>=`.\n");
    }
    {
        // Iterate over possible selections of elements to add to two
        // containers, `X` and `Y` then compare the results of the comparison
        // operators to an "oracle" result from
        // `bslalg::RangeCompare::lexicographical` over the same range.

        for (size_t i = 0; i < numValues; ++i) {
            for (size_t j = 0; j < numValues; ++j) {
                for (size_t length = 0; length < numValues; ++length) {
                    Obj x(comparator, &objectAllocator); const Obj& X = x;
                    Obj y(comparator, &objectAllocator); const Obj& Y = y;
                    for (size_t k = 0; k < j; ++k) {
                        size_t xIndex = (i + length) % numValues;
                        size_t yIndex = (j + length) % numValues;

                        Value xValue(testKeys[xIndex], testValues[xIndex]);
                        x.insert(xValue);
                        Value yValue(testKeys[yIndex], testValues[yIndex]);
                        y.insert(yValue);
                    }

                    int comp = bslalg::RangeCompare::lexicographical(X.begin(),
                                                                    X.end(),
                                                                    Y.begin(),
                                                                    Y.end());
                    ASSERTV((comp < 0)  == (X < Y));
                    ASSERTV((comp > 0)  == (X > Y));
                    ASSERTV((comp <= 0) == (X <= Y));
                    ASSERTV((comp >= 0) == (X >= Y));
                }
            }
        }
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    std::sort(testKeys, testKeys + numValues, comparator);
    if (veryVerbose) {
        printf("Test `key_comp` and `value_comp`.\n");
    }
    {
        Obj x(comparator, &objectAllocator); const Obj& X = x;
        typename Obj::key_compare   keyComp   = X.key_comp();
        typename Obj::value_compare valueComp = X.value_comp();
        for (size_t i = 0; i < numValues - 1; ++i) {
            ASSERTV(!keyComp(testKeys[i+1], testKeys[i]));
            ASSERTV(!valueComp(Value(testKeys[i+1], testValues[i+1]),
                               Value(testKeys[i],   testValues[i])));
        }
    }
}

                       // =====================
                       // struct MetaTestDriver
                       // =====================

/// This `struct` is to be call by the `RUN_EACH_TYPE` macro, and the
/// functions within it dispatch to functions in `TestDriver` instantiated
/// with different types of allocator.
template <class KEY,
          class VALUE = KEY,
          class COMP  = TestComparator<KEY> >
struct MetaTestDriver {

    typedef bsl::pair<const KEY, VALUE>     Pair;
    typedef bsl::allocator<Pair>            BAP;
    typedef bsltf::StdAllocatorAdaptor<BAP> SAA;

    /// Test member and free `swap`.
    static void testCase8();
};

template <class KEY, class VALUE, class COMP>
void MetaTestDriver<KEY, VALUE, COMP>::testCase8()
{
    // The low-order bit of the identifier specifies whether the third boolean
    // argument of the stateful allocator, which indicates propagate on
    // container swap, is set.

    typedef bsltf::StdStatefulAllocator<Pair, false, false, false, false> S00;
    typedef bsltf::StdStatefulAllocator<Pair, false, false,  true, false> S01;
    typedef bsltf::StdStatefulAllocator<Pair,  true,  true, false,  true> S10;
    typedef bsltf::StdStatefulAllocator<Pair,  true,  true,  true,  true> S11;

    if (verbose) printf("\n");

    TestDriver<KEY, VALUE, COMP, BAP>::testCase8_dispatch();

    TestDriver<KEY, VALUE, COMP, SAA>::testCase8_dispatch();

    TestDriver<KEY, VALUE, COMP, S00>::testCase8_dispatch();
    TestDriver<KEY, VALUE, COMP, S01>::testCase8_dispatch();
    TestDriver<KEY, VALUE, COMP, S10>::testCase8_dispatch();
    TestDriver<KEY, VALUE, COMP, S11>::testCase8_dispatch();
}

//=============================================================================
//                                USAGE EXAMPLE
//-----------------------------------------------------------------------------

namespace UsageExample {

class string {

    // DATA
    char             *d_value_p;      // 0 terminated character array
    size_t            d_size;         // length of d_value_p
    bslma::Allocator *d_allocator_p;  // allocator (held, not owned)

     // PRIVATE CLASS DATA
    static const char *EMPTY_STRING;

  public:
    // PUBLIC TYPE TRAITS
    BSLMF_NESTED_TRAIT_DECLARATION(string, bslma::UsesBslmaAllocator);

    // CREATORS
    explicit string(bslma::Allocator *basicAllocator = 0)
    : d_value_p(const_cast<char *>(EMPTY_STRING))
    , d_size(0)
    , d_allocator_p(bslma::Default::allocator(basicAllocator))
    {
    }

    string(const char      *value,
           bslma::Allocator *basicAllocator = 0)                    // IMPLICIT
    : d_value_p(const_cast<char *>(EMPTY_STRING))
    , d_size(std::strlen(value))
    , d_allocator_p(bslma::Default::allocator(basicAllocator))
    {
        if (d_size > 0) {
            d_value_p = static_cast<char *>(
                                          d_allocator_p->allocate(d_size + 1));
            std::memcpy(d_value_p, value, d_size + 1);
        }
    }

    string(const string&    original,
           bslma::Allocator *basicAllocator = 0)
    : d_value_p(const_cast<char *>(EMPTY_STRING))
    , d_size(original.d_size)
    , d_allocator_p(bslma::Default::allocator(basicAllocator))
    {
        if (d_size > 0) {
            d_value_p = static_cast<char *>(
                                          d_allocator_p->allocate(d_size + 1));
            std::memcpy(d_value_p, original.d_value_p, d_size + 1);
        }
    }

    ~string()
    {
        if (d_size > 0) {
            d_allocator_p->deallocate(d_value_p);
        }
    }

    // MANIPULATORS
    string& operator=(const string& rhs)
    {
        string temp(rhs);
        temp.swap(*this);
        return *this;
    }

    char &operator[](int index)
    {
        return d_value_p[index];
    }

    void swap(string& other)
    {
        BSLS_ASSERT(d_allocator_p == other.d_allocator_p);

        std::swap(d_value_p, other.d_value_p);
        std::swap(d_size, other.d_size);
    }

    // ACCESSORS
    size_t size() const
    {
        return d_size;
    }

    bool empty() const
    {
        return 0 == d_size;
    }

    const char *c_str() const
    {
        return d_value_p;
    }
};

inline
bool operator==(const string& lhs, const string& rhs)
{
    return 0 == std::strcmp(lhs.c_str(), rhs.c_str());
}

inline
bool operator!=(const string& lhs, const string& rhs)
{
    return !(lhs == rhs);
}

inline
bool operator<(const string& lhs, const string& rhs)
{
    return std::strcmp(lhs.c_str(), rhs.c_str()) < 0;
}

inline
bool operator>(const string& lhs, const string& rhs)
{
    return rhs < lhs;
}


const char *string::EMPTY_STRING = "";

///Usage
///-----
// In this section we show intended use of this component.
//
///Example 1: Creating a Phone Book
/// - - - - - - - - - - - - - - - -
// In this example, we will define a class `PhoneBook`, that provides a mapping
// of names to phone numbers.  The `PhoneBook` class will be implemented using
// a `bsl::multimap`, and will supply manipulators, allowing a client to add or
// remove entries from the phone book, as well as accessors, allowing clients
// to efficiently lookup entries by name, and to iterate over the entries in
// the phone book in sorted order.
//
// Note that this example uses a type `string` that is based on the standard
// type `string` (see `bslstl_string`).  For the sake of brevity, the
// implementation of `string` is not explored here.
//
// First, we define an alias for a pair of `string` objects that we will use
// to represent names in the phone book:
// ```

    /// This `typedef` provides an alias for a pair of `string` objects,
    /// whose `first` and `second` elements refer to the first and last
    /// names of a person, respectively.
    typedef bsl::pair<string, string> FirstAndLastName;
// ```
// Then, we define a comparison functor for `FirstAndLastName` objects (note
// that this comparator is required because we intend for the last name to
// take precedence over the first name in the ordering of entries maintained
// by the phone book, which differs from the behavior supplied by `operator<`
// for `bsl::pair`):
// ```

    /// This `struct` defines an ordering on `FirstAndLastName` values,
    /// allowing them to be included in sorted containers such as
    /// `bsl::multimap`.  Note that last name (the `second` member of a
    /// `FirstAndLastName` value) takes precedence over first name in the
    /// ordering defined by this functor.
    struct FirstAndLastNameLess {

        /// Return `true` if the value of the specified `lhs` is less than
        /// (ordered before) the value of the specified `rhs`, and `false`
        /// otherwise.  The `lhs` value is considered less than the `rhs`
        /// value if the second value in the `lhs` pair (the last name) is
        /// less than the second value in the `rhs` pair or, if the second
        /// values are equal, if the first value in the `lhs` pair (the
        /// first name) is less than the first value in the `rhs` pair.
        bool operator()(const FirstAndLastName& lhs,
                        const FirstAndLastName& rhs) const
        {
            int cmp = std::strcmp(lhs.second.c_str(), rhs.second.c_str());
            if (0 == cmp) {
                cmp = std::strcmp(lhs.first.c_str(), rhs.first.c_str());
            }
            return cmp < 0;
        }
    };
// ```
// Next, we define the public interface for `PhoneBook`:
// ```

    /// This class provides a mapping of a person's name to their phone
    /// number.  Names within a `Phonebook` are represented using a using
    /// `FirstAndLastName` object, and phone numbers are represented using a
    /// `bsls::Types::Uint64` value.
    class PhoneBook {
// ```
// Here, we create a type alias, `NameToNumberMap`, for a `bsl::multimap` that
// will serve as the data member for a `PhoneBook`.  A `NameToNumberMap` has
// keys of type `FirstAndLastName`, mapped-values of type
// `bsls::Types::Uint64`, and a comparator of type `FirstAndLastNameLess`.  We
// use the default `ALLOCATOR` template parameter as we intend to use
// `PhoneBook` with `bslma` style allocators:
// ```
        // PRIVATE TYPES

        /// This `typedef` is an alias for a mapping between names and phone
        /// numbers.
        typedef bsl::multimap<FirstAndLastName,
                              bsls::Types::Uint64,
                              FirstAndLastNameLess> NameToNumberMap;

        // DATA
        NameToNumberMap d_nameToNumber;  // mapping of names to phone numbers

        // FRIENDS
        friend bool operator==(const PhoneBook&, const PhoneBook&);

      public:
        // PUBLIC TYPES

        /// This `typedef` provides an alias for the type of an unsigned
        /// integers used to represent phone-numbers in a `PhoneBook`.
        typedef bsls::Types::Uint64 PhoneNumber;

        /// This `typedef` provides an alias for the type of an iterator
        /// providing non-modifiable access to the entries in a `PhoneBook`.
        typedef NameToNumberMap::const_iterator ConstIterator;

        // CREATORS

        /// Create an empty `PhoneBook` object.  Optionally specify a
        /// `basicAllocator` used to supply memory.  If `basicAllocator` is
        /// 0, the currently installed default allocator is used.
        explicit
        PhoneBook(bslma::Allocator *basicAllocator = 0);

        /// Create a `PhoneBook` object having the same value as the
        /// specified `original` object.  Optionally specify a
        /// `basicAllocator` used to supply memory.  If `basicAllocator` is
        /// 0, the currently installed default allocator is used.
        PhoneBook(const PhoneBook&  original,
                  bslma::Allocator  *basicAllocator = 0);

        //! ~PhoneBook() = default;
            // Destroy this object.

        // MANIPULATORS

        /// Assign to this object the value of the specified `rhs` object,
        /// and return a reference providing modifiable access to this
        /// object.
        PhoneBook& operator=(const PhoneBook& rhs);

        /// Add an entry to this phone book having the specified `name` and
        /// `number`.  The behavior is undefined unless `name.first` and
        /// `name.end` are non-empty strings.
        void addEntry(const FirstAndLastName& name, PhoneNumber number);

        /// Remove the entries from this phone book having the specified
        /// `name` and `number`, if they exists, and return the number of
        /// removed entries; otherwise, return 0 with no other effects.
        int removeEntry(const FirstAndLastName& name, PhoneNumber number);

        // ACCESSORS

        /// Return a pair of iterators to the ordered sequence of entries
        /// held in this phone book having the specified `name`, where the
        /// first iterator is position at the start of the sequence, and the
        /// second is positioned one past the last entry in the sequence.
        /// If `name` does not exist in this phone book, then the two
        /// returned iterators will have the same value.
        bsl::pair<ConstIterator, ConstIterator> lookupByName(
                                           const FirstAndLastName& name) const;

        /// Return an iterator providing non-modifiable access to the first
        /// entry in the ordered sequence of entries held in this phone
        /// book, or the past-the-end iterator if this phone book is empty.
        ConstIterator begin() const;

        /// Return an iterator providing non-modifiable access to the
        /// past-the-end entry in the ordered sequence of entries maintained
        /// by this phone book.
        ConstIterator end() const;

        /// Return the number of entries contained in this phone book.
        size_t numEntries() const;
    };
// ```
// Then, we declare the free operators for `PhoneBook`:
// ```

    /// Return `true` if the specified `lhs` and `rhs` objects have the same
    /// value, and `false` otherwise.  Two `PhoneBook` objects have the same
    /// value if they have the same number of entries, and each
    /// corresponding entry, in their respective ordered sequence of
    /// entries, is the same.
    inline
    bool operator==(const PhoneBook& lhs, const PhoneBook& rhs);

    /// Return `true` if the specified `lhs` and `rhs` objects do not have
    /// the same value, and `false` otherwise.  Two `PhoneBook` objects do
    /// not have the same value if they either differ in their number of
    /// contained entries, or if any of the corresponding entries, in their
    /// respective ordered sequences of entries, is not the same.
    inline
    bool operator!=(const PhoneBook& lhs, const PhoneBook& rhs);
// ```
// Now, we define the implementations methods of the `PhoneBook` class:
// ```
    // CREATORS
    inline
    PhoneBook::PhoneBook(bslma::Allocator *basicAllocator)
    : d_nameToNumber(FirstAndLastNameLess(), basicAllocator)
    {
    }
// ```
// Notice that, on construction, we pass the contained `bsl::multimap`
// (`d_nameToNumber`), a default constructed `FirstAndLastNameLess` object that
// it will use to perform comparisons, and the allocator supplied to
// `PhoneBook` at construction'.
// ```
    inline
    PhoneBook::PhoneBook(const PhoneBook&   original,
                         bslma::Allocator  *basicAllocator)
    : d_nameToNumber(original.d_nameToNumber, basicAllocator)
    {
    }

    // MANIPULATORS
    inline
    PhoneBook& PhoneBook::operator=(const PhoneBook& rhs)
    {
        d_nameToNumber = rhs.d_nameToNumber;
        return *this;
    }

    inline
    void PhoneBook::addEntry(const FirstAndLastName& name, PhoneNumber number)
    {
        BSLS_ASSERT(!name.first.empty());
        BSLS_ASSERT(!name.second.empty());

        d_nameToNumber.insert(NameToNumberMap::value_type(name, number));
    }

    inline
    int PhoneBook::removeEntry(const FirstAndLastName& name,
                               PhoneNumber             number)
    {
        bsl::pair<NameToNumberMap::iterator, NameToNumberMap::iterator> range =
                                              d_nameToNumber.equal_range(name);

        NameToNumberMap::iterator it = range.first;

        int numRemovedEntries = 0;

        while (it != range.second) {
            if (it->second == number) {
                it = d_nameToNumber.erase(it);
                ++numRemovedEntries;
            }
            else {
                ++it;
            }
        }

        return numRemovedEntries;
    }

    // ACCESSORS
    inline
    bsl::pair<PhoneBook::ConstIterator, PhoneBook::ConstIterator>
    PhoneBook::lookupByName(const FirstAndLastName&  name) const
    {
        return d_nameToNumber.equal_range(name);
    }

    inline
    PhoneBook::ConstIterator PhoneBook::begin() const
    {
        return d_nameToNumber.begin();
    }

    inline
    PhoneBook::ConstIterator PhoneBook::end() const
    {
        return d_nameToNumber.end();
    }

    inline
    size_t PhoneBook::numEntries() const
    {
        return d_nameToNumber.size();
    }
    // ```
    // Finally, we implement the free operators for `PhoneBook`:
    // ```
    inline
    bool operator==(const PhoneBook& lhs, const PhoneBook& rhs)
    {
        return lhs.d_nameToNumber == rhs.d_nameToNumber;
    }

    inline
    bool operator!=(const PhoneBook& lhs, const PhoneBook& rhs)
    {
        return !(lhs == rhs);
    }
// ```

}  // close namespace UsageExample

// ============================================================================
//                            MAIN PROGRAM
// ----------------------------------------------------------------------------

bool intLessThan(int a, int b)
{
    return a < b;
}

int main(int argc, char *argv[])
{
    int test = argc > 1 ? atoi(argv[1]) : 0;

                verbose = argc > 2;
            veryVerbose = argc > 3;
        veryVeryVerbose = argc > 4;
    veryVeryVeryVerbose = argc > 5;

    printf("TEST " __FILE__ " CASE %d\n", test);

    bslma::TestAllocator globalAllocator("global", veryVeryVeryVerbose);
    bslma::Default::setGlobalAllocator(&globalAllocator);

    bslma::TestAllocator defaultAllocator("default", veryVeryVeryVerbose);
    ASSERT(0 == bslma::Default::setDefaultAllocator(&defaultAllocator));

    // check integrity of `DATA` table

    {
        const int NUM_DATA                     = DEFAULT_NUM_DATA;
        const DefaultDataRow (&DATA)[NUM_DATA] = DEFAULT_DATA;

        for (int ii = 0; ii < NUM_DATA; ++ii) {
            for (int jj = 0; jj < NUM_DATA; ++jj) {
                ASSERT((DATA[ii].d_index == DATA[jj].d_index) ==
                         !std::strcmp(DATA[ii].d_results, DATA[jj].d_results));
            }
        }
    }

    switch (test) { case 0:
      case 40: {
        // --------------------------------------------------------------------
        // USAGE EXAMPLE
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
        // --------------------------------------------------------------------

        if (verbose) printf("\nUSAGE EXAMPLE"
                            "\n=============\n");

        {
            using namespace UsageExample;
            typedef FirstAndLastName Name;

            bslma::TestAllocator defaultAllocator("defaultAllocator");
            bslma::DefaultAllocatorGuard defaultGuard(&defaultAllocator);

            bslma::TestAllocator objectAllocator("objectAllocator");


            PhoneBook phoneBook(&objectAllocator);

            phoneBook.addEntry(Name("John",  "Smith"),  8005551000ULL);
            ASSERT(1 == phoneBook.numEntries());

            phoneBook.addEntry(Name("Bill",  "Smith"),  8005551001ULL);
            ASSERT(2 == phoneBook.numEntries());

            phoneBook.addEntry(Name("Bill",  "Smithy"), 8005551002ULL);
            ASSERT(3 == phoneBook.numEntries());

            phoneBook.addEntry(Name("Bill",  "Smj"),    8005551003ULL);
            ASSERT(4 == phoneBook.numEntries());

            phoneBook.addEntry(Name("Bill",  "Smj"),    8005551004ULL);
            ASSERT(5 == phoneBook.numEntries());

            bsl::pair<PhoneBook::ConstIterator, PhoneBook::ConstIterator>
                           range = phoneBook.lookupByName(Name("Bill", "Smj"));


            int count = 0;
            for (PhoneBook::ConstIterator itr = range.first;
                 itr != range.second;
                 ++itr) {
                ++count;
                ASSERT(Name("Bill", "Smj") == itr->first);
            }
            ASSERT(2 == count);

            ASSERT(1 ==
                   phoneBook.removeEntry(Name("Bill",  "Smj"), 8005551003ULL));

            ASSERT(4 == phoneBook.numEntries());

            ASSERT(0 ==
                   phoneBook.removeEntry(Name("Bill",  "Smj"), 8005551003ULL));

            ASSERT(4 == phoneBook.numEntries());

            ASSERT(0 == defaultAllocator.numBytesInUse());
            ASSERT(0 < objectAllocator.numBytesInUse());
        }

      } break;
      case 38: // falls through
      case 39: // falls through
      case 37: // falls through
      case 36: // falls through
      case 35: // falls through
      case 34: // falls through
      case 33: // falls through
      case 32: // falls through
      case 31: // falls through
      case 30: // falls through
      case 29: // falls through
      case 28: // falls through
      case 27: // falls through
      case 26: // falls through
      case 25: // falls through
      case 24: // falls through
      case 23: // falls through
      case 22: // falls through
      case 21: // falls through
      case 20: // falls through
      case 19: // falls through
      case 18: // falls through
      case 17: // falls through
      case 16: // falls through
      case 15: // falls through
      case 14: // falls through
      case 13: // falls through
      case 12: // falls through
      case 11: // falls through
      case 10: // falls through
      case  9: {
        if (verbose) printf(
               "\nTEST CASE %d IS DELEGATED TO `bslstl_multimap_test.t.cpp`"
               "\n=========================================================\n",
               test);
      } break;
      case 8: {
        // --------------------------------------------------------------------
        // MANIPULATOR AND FREE FUNCTION `swap`
        // --------------------------------------------------------------------

        if (verbose) printf("\nMANIPULATOR AND FREE FUNCTION `swap`"
                            "\n====================================\n");

        RUN_EACH_TYPE(MetaTestDriver,
                      testCase8,
                      BSLTF_TEMPLATETESTFACILITY_TEST_TYPES_REGULAR);

        MetaTestDriver<int,
                    bsltf::MoveOnlyAllocTestType>::testCase8();
        MetaTestDriver<int,
                       bsltf::WellBehavedMoveOnlyAllocTestType>::testCase8();
        MetaTestDriver<TestKeyType, TestValueType>::testCase8();

#if BSLS_KEYWORD_NOEXCEPT_AVAILABLE
        // Test noexcept
#ifndef BSLMF_ISNOTHROWSWAPPABLE_ALWAYS_FALSE
        {
            typedef bsltf::StdStatefulAllocator<int,
                                                false,
                                                false,
                                                false,
                                                false> Alloc;
            typedef TestComparator<int> Comp;

            ASSERT(!bsl::allocator_traits<Alloc>::is_always_equal::value);
            ASSERT( bsl::is_nothrow_swappable<Comp>::value);
            TestDriver<int, int, Comp, Alloc>::testCase8_noexcept();
        }
        {
            typedef bsltf::StdStatefulAllocator<int,
                                                false,
                                                false,
                                                false,
                                                false,
                                                true> Alloc;
            typedef TestComparator<int> Comp;

            ASSERT( bsl::allocator_traits<Alloc>::is_always_equal::value);
            ASSERT( bsl::is_nothrow_swappable<Comp>::value);
            TestDriver<int, int, Comp, Alloc>::testCase8_noexcept();
        }
#endif
        {
            typedef bsltf::StdStatefulAllocator<int,
                                                false,
                                                false,
                                                false,
                                                false> Alloc;
            typedef ThrowingSwapComparator<int> Comp;

            ASSERT(!bsl::allocator_traits<Alloc>::is_always_equal::value);
            ASSERT(!bsl::is_nothrow_swappable<Comp>::value);
            TestDriver<int, int, Comp, Alloc>::testCase8_noexcept();
        }
        {
            typedef bsltf::StdStatefulAllocator<int,
                                                false,
                                                false,
                                                false,
                                                false,
                                                true> Alloc;
            typedef ThrowingSwapComparator<int> Comp;

            ASSERT( bsl::allocator_traits<Alloc>::is_always_equal::value);
            ASSERT(!bsl::is_nothrow_swappable<Comp>::value);
            TestDriver<int, int, Comp, Alloc>::testCase8_noexcept();
        }
#endif
      } break;
      case 7: {
        // --------------------------------------------------------------------
        // COPY CONSTRUCTOR
        // --------------------------------------------------------------------

        if (verbose) printf("\nTesting Copy Constructors"
                            "\n=========================\n");

        RUN_EACH_TYPE(TestDriver,
                      testCase7,
                      BSLTF_TEMPLATETESTFACILITY_TEST_TYPES_REGULAR,
                      bsltf::NonOptionalAllocTestType);

        TestDriver<TestKeyType, TestValueType>::testCase7();

        // `select_on_container_copy_construction` testing

        if (verbose) printf("\nCOPY CONSTRUCTOR: ALLOCATOR PROPAGATION"
                            "\n=======================================\n");

        RUN_EACH_TYPE(TestDriver,
                      testCase7_select_on_container_copy_construction,
                      BSLTF_TEMPLATETESTFACILITY_TEST_TYPES_REGULAR,
                      bsltf::NonOptionalAllocTestType);

        TestDriver<TestKeyType, TestValueType>::
                             testCase7_select_on_container_copy_construction();
      } break;
      case 6: {
        // --------------------------------------------------------------------
        // EQUALITY OPERATORS
        // --------------------------------------------------------------------

        if (verbose) printf("\nTesting Equality Operators"
                            "\n==========================\n");

        RUN_EACH_TYPE(TestDriver,
                      testCase6,
                      BSLTF_TEMPLATETESTFACILITY_TEST_TYPES_REGULAR,
                      bsltf::NonOptionalAllocTestType,
                      bsltf::MoveOnlyAllocTestType,
                      bsltf::WellBehavedMoveOnlyAllocTestType);

        TestDriver<TestKeyType, TestValueType>::testCase6();
      } break;
      case 5: {
        // --------------------------------------------------------------------
        // TESTING OUTPUT (<<) OPERATOR
        // --------------------------------------------------------------------

        if (verbose) printf("\nTesting Output (<<) Operator"
                            "\n============================\n");

        if (verbose)
                   printf("There is no output operator for this component.\n");
      } break;
      case 4: {
        // --------------------------------------------------------------------
        // BASIC ACCESSORS
        // --------------------------------------------------------------------

        if (verbose) printf("\nTesting Basic Accessors"
                            "\n=======================\n");

        RUN_EACH_TYPE(TestDriver,
                      testCase4,
                      BSLTF_TEMPLATETESTFACILITY_TEST_TYPES_REGULAR,
                      bsltf::NonOptionalAllocTestType,
                      bsltf::MoveOnlyAllocTestType,
                      bsltf::WellBehavedMoveOnlyAllocTestType);
        TestDriver<TestKeyType, TestValueType>::testCase4();
      } break;
      case 3: {
        // --------------------------------------------------------------------
        // GENERATOR FUNCTIONS `gg` and `ggg`
        // --------------------------------------------------------------------

        if (verbose) printf("\nTesting `gg`"
                            "\n============\n");

        RUN_EACH_TYPE(TestDriver,
                      testCase3,
                      BSLTF_TEMPLATETESTFACILITY_TEST_TYPES_REGULAR,
                      bsltf::NonOptionalAllocTestType,
                      bsltf::MoveOnlyAllocTestType,
                      bsltf::WellBehavedMoveOnlyAllocTestType);

        TestDriver<TestKeyType, TestValueType>::testCase3();
      } break;
      case 2: {
        // --------------------------------------------------------------------
        // PRIMARY MANIPULATORS
        // --------------------------------------------------------------------

        if (verbose) printf("\nTesting Primary Manipulators"
                            "\n============================\n");

        RUN_EACH_TYPE(TestDriver,
                      testCase2,
                      BSLTF_TEMPLATETESTFACILITY_TEST_TYPES_REGULAR,
                      bsltf::NonOptionalAllocTestType,
                      bsltf::MoveOnlyAllocTestType,
                      bsltf::WellBehavedMoveOnlyAllocTestType);

        TestDriver<TestKeyType, TestValueType>::testCase2();
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
        // 1. Run each method with arbitrary inputs and verify the behavior is
        //    as expected.
        //
        // Testing:
        //   BREATHING TEST
        // --------------------------------------------------------------------

        if (verbose) printf("\nBREATHING TEST"
                            "\n==============\n");
        {
            int INT_VALUES[]   = { INT_MIN, -2, -1, 0, 1, 2, INT_MAX };
            int NUM_INT_VALUES = static_cast<int>(
                                     sizeof(INT_VALUES) / sizeof(*INT_VALUES));

            typedef bool (*Comparator)(int, int);
            TestDriver<int, int, Comparator>::testCase1(&intLessThan,
                                                        INT_VALUES,
                                                        INT_VALUES,
                                                        NUM_INT_VALUES);
            TestDriver<int, int, std::less<int> >::testCase1(std::less<int>(),
                                                             INT_VALUES,
                                                             INT_VALUES,
                                                             NUM_INT_VALUES);
        }
#ifdef BSLS_COMPILERFEATURES_SUPPORT_GENERALIZED_INITIALIZERS
        if (verbose) {
             printf("Test initializer lists.\n");
        }
        {
            ASSERT((0 == []() -> bsl::multimap<char, int> {
                return {};
            }().size()));
            ASSERT((1 == []() -> bsl::multimap<char, int> {
                return {{'a', 1}};
            }().size()));
            ASSERT((3 == []() -> bsl::multimap<char, int> {
                return {{'a', 1}, {'b', 2}, {'a', 3}};
            }().size()));
        }
#endif
      } break;
      default: {
        fprintf(stderr, "WARNING: CASE `%d' NOT FOUND.\n", test);
        testStatus = -1;
      }
    }

    // CONCERN: In no case does memory come from the global allocator.
    ASSERTV(globalAllocator.numBlocksTotal(),
            0 == globalAllocator.numBlocksTotal());

    if (testStatus > 0) {
        fprintf(stderr, "Error, non-zero test status = %d.\n", testStatus);
    }
    return testStatus;
}

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
