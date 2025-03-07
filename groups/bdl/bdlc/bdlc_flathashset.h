// bdlc_flathashset.h                                                 -*-C++-*-
#ifndef INCLUDED_BDLC_FLATHASHSET
#define INCLUDED_BDLC_FLATHASHSET

#include <bsls_ident.h>
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide an open-addressed unordered set container.
//
//@CLASSES:
//   bdlc::FlatHashSet: open-addressed unordered set container
//
//@SEE_ALSO: bdlc_flathashtable, bdlc_flathashmap
//
//@DESCRIPTION: This component defines a single class template,
// `bdlc::FlatHashSet`, that implements an open-addressed unordered set of
// items with unique values.
//
// Unordered sets are useful in situations when there is no meaningful way to
// order key values, when the order of the values is irrelevant to the problem
// domain, or (even if there is a meaningful ordering) the value of ordering
// the results is outweighed by the higher performance provided by unordered
// sets (compared to ordered sets).  On platforms that support relevant SIMD
// instructions (e.g., SSE2), `bdlc::FlatHashSet` generally exhibits better
// performance than `bsl::unordered_set`.
//
// An instantiation of `bdlc::FlatHashSet` is an allocator-aware,
// value-semantic type whose salient attributes are the set of values
// contained, without regard to order.  An instantiation may be provided with
// custom hash and equality functors, but those are not salient attributes.  In
// particular, when comparing element values for equality between two different
// `bdlc::FlatHashSet` objects, the elements are compared using `operator==`.
//
// The implemented data structure is inspired by Google's `flat_hash_map`
// CppCon presentations (available on YouTube).  The implementation draws from
// Google's open source `raw_hash_set.h` file at:
// https://github.com/abseil/abseil-cpp/blob/master/absl/container/internal.
//
///Performance Caveats
///-------------------
// `bdlc::FlatHashSet` is recommended for Intel platforms *only* (i.e., Linux
// and Windows, and pre-ARM Macs); on platforms using other processors (i.e.,
// Sun and AIX), `bdlc::FlatHashSet` may have slower performance than
// `bsl::unordered_set`.  However, note that `bdlc::FlatHashSet` will use
// significantly less memory than `bsl::unordered_set` on *all* platforms.
// Given the Intel-only performance caveat, it is recommended to benchmark
// before using `bdlc::FlatHashSet` -- particularly on non-Intel production
// environments.
//
///Interface Differences with `bsl::unordered_set`
///-----------------------------------------------
// A `bdlc::FlatHashSet` meets most of the requirements of an unordered
// associative container with forward iterators in the C++11 Standard [23.2.5].
// It does not have the bucket interface, and locations of elements may change
// when the container is modified (and therefore iterators become invalid too).
// Allocator use follows BDE style, and the various allocator propagation
// attributes are not present (e.g., the allocator trait
// `propagate_on_container_copy_assignment`).  The maximum load factor of the
// container (the ratio of size to capacity) is maintained by the container
// itself and is not settable (the maximum load factor is implementation
// defined and fixed).
//
///Load Factor and Resizing
///------------------------
// An invariant of `bdlc::FlatHashSet` is that
// `0 <= load_factor() <= max_load_factor() <= 1.0`.  Any operation that would
// result in `load_factor() > max_load_factor()` for a `bdlc::FlatHashSet`
// causes the capacity to increase.  This resizing allocates new memory, copies
// or moves all elements to the new memory, and reclaims the original memory.
// The transfer of elements involves rehashing each element to determine its
// new location.  As such, all iterators, pointers, and references to elements
// of the `bdlc::FlatHashSet` are invalidated on a resize.
//
///Requirements on `KEY`, `HASH`, and `EQUAL`
///------------------------------------------
// The template parameter type `KEY` must be copy or move constructible.  The
// template parameter types `HASH` and `EQUAL` must be default and copy
// constructible function objects.
//
// `HASH` must support a function-call operator compatible with the following
// statements for an object `key` of type `KEY`:
// ```
// HASH        hash;
// bsl::size_t result = hash(key);
// ```
//
// `EQUAL` must support a function-call operator compatible with the
//  following statements for objects `key1` and `key2` of type `KEY`:
// ```
// EQUAL equal;
// bool  result = equal(key1, key2);
// ```
// where the definition of the called function defines an equivalence
// relationship on keys that is both reflexive and transitive.
//
// `HASH` and `EQUAL` function objects are further constrained: if the
// comparator determines that two values are equal, the hasher must produce the
// same hash value for each.
//
///Iterator, Pointer, and Reference Invalidation
///---------------------------------------------
// Any change in capacity of a `bdlc::FlatHashSet` invalidates all pointers,
// references, and iterators.  A `bdlc::FlatHashSet` manipulator that erases an
// element invalidates all pointers, references, and iterators to the erased
// element.
//
///Exception Safety
///----------------
// A `bdlc::FlatHashSet` is exception neutral, and all of the methods of
// `bdlc::FlatHashSet` provide the basic exception safety guarantee (see
// {`bsldoc_glossary`|Basic Guarantee}).
//
///Move Semantics in C++03
///-----------------------
// Move-only types are supported by `bdlc::FlatHashSet` on C++11, and later,
// platforms only (where `BSLMF_MOVABLEREF_USES_RVALUE_REFERENCES` is defined),
// and are not supported on C++03 platforms.  Unfortunately, in C++03, there
// are user-defined types where a `bslmf::MovableRef` will not safely degrade
// to an lvalue reference when a move constructor is not available (types
// providing a constructor template taking any type), so
// `bslmf::MovableRefUtil::move` cannot be used directly on a user-supplied
// template parameter type.
//
///Usage
///-----
// In this section we show intended use of this component.
//
///Example 1: Categorizing Data
/// - - - - - - - - - - - - - -
// Suppose one is analyzing data on a set of customers, and each customer is
// categorized by several attributes: customer type, geographic area, and
// (internal) project code; and that each attribute takes on one of a limited
// set of values.  This data can be handled by creating an enumeration for each
// of the attributes:
// ```
// typedef enum {
//     e_REPEAT
//   , e_DISCOUNT
//   , e_IMPULSE
//   , e_NEED_BASED
//   , e_BUSINESS
//   , e_NON_PROFIT
//   , e_INSTITUTE
//   // ...
// } CustomerCode;
//
// typedef enum {
//     e_USA_EAST
//   , e_USA_WEST
//   , e_CANADA
//   , e_MEXICO
//   , e_ENGLAND
//   , e_SCOTLAND
//   , e_FRANCE
//   , e_GERMANY
//   , e_RUSSIA
//   // ...
// } LocationCode;
//
// typedef enum {
//     e_TOAST
//   , e_GREEN
//   , e_FAST
//   , e_TIDY
//   , e_PEARL
//   , e_SMITH
//   // ...
// } ProjectCode;
// ```
// The data set (randomly generated for this example) is provided in a
// statically initialized array:
// ```
// static const struct CustomerProfile {
//     CustomerCode d_customer;
//     LocationCode d_location;
//     ProjectCode  d_project;
// } customerProfiles[] = {
//     { e_IMPULSE   , e_CANADA  , e_SMITH },
//     { e_NON_PROFIT, e_USA_EAST, e_GREEN },
//     { e_INSTITUTE , e_USA_EAST, e_TOAST },
//     { e_NON_PROFIT, e_CANADA  , e_PEARL },
//     { e_NEED_BASED, e_CANADA  , e_FAST  },
//     { e_BUSINESS  , e_ENGLAND , e_PEARL },
//     { e_REPEAT    , e_SCOTLAND, e_TIDY  },
//     { e_INSTITUTE , e_MEXICO  , e_PEARL },
//     { e_DISCOUNT  , e_USA_EAST, e_GREEN },
//     { e_BUSINESS  , e_USA_EAST, e_GREEN },
//     { e_IMPULSE   , e_MEXICO  , e_TOAST },
//     { e_DISCOUNT  , e_GERMANY , e_FAST  },
//     { e_INSTITUTE , e_FRANCE  , e_FAST  },
//     { e_NON_PROFIT, e_ENGLAND , e_PEARL },
//     { e_BUSINESS  , e_ENGLAND , e_TIDY  },
//     { e_BUSINESS  , e_CANADA  , e_GREEN },
//     { e_INSTITUTE , e_FRANCE  , e_FAST  },
//     { e_IMPULSE   , e_RUSSIA  , e_TOAST },
//     { e_REPEAT    , e_USA_WEST, e_TOAST },
//     { e_IMPULSE   , e_CANADA  , e_TIDY  },
//     { e_NON_PROFIT, e_GERMANY , e_GREEN },
//     { e_INSTITUTE , e_USA_EAST, e_TOAST },
//     { e_INSTITUTE , e_FRANCE  , e_FAST  },
//     { e_IMPULSE   , e_SCOTLAND, e_SMITH },
//     { e_INSTITUTE , e_USA_EAST, e_PEARL },
//     { e_INSTITUTE , e_USA_EAST, e_TOAST },
//     { e_NON_PROFIT, e_ENGLAND , e_PEARL },
//     { e_IMPULSE   , e_GERMANY , e_FAST  },
//     { e_REPEAT    , e_GERMANY , e_FAST  },
//     { e_REPEAT    , e_MEXICO  , e_PEARL },
//     { e_IMPULSE   , e_GERMANY , e_TIDY  },
//     { e_IMPULSE   , e_MEXICO  , e_TOAST },
//     { e_NON_PROFIT, e_SCOTLAND, e_SMITH },
//     { e_NEED_BASED, e_MEXICO  , e_TOAST },
//     { e_NON_PROFIT, e_FRANCE  , e_SMITH },
//     { e_INSTITUTE , e_MEXICO  , e_TIDY  },
//     { e_NON_PROFIT, e_FRANCE  , e_TIDY  },
//     { e_IMPULSE   , e_FRANCE  , e_FAST  },
//     { e_DISCOUNT  , e_RUSSIA  , e_TIDY  },
//     { e_IMPULSE   , e_USA_EAST, e_TIDY  },
//     { e_IMPULSE   , e_USA_WEST, e_FAST  },
//     { e_NON_PROFIT, e_FRANCE  , e_TIDY  },
//     { e_BUSINESS  , e_ENGLAND , e_GREEN },
//     { e_REPEAT    , e_FRANCE  , e_TOAST },
//     { e_REPEAT    , e_RUSSIA  , e_SMITH },
//     { e_REPEAT    , e_RUSSIA  , e_GREEN },
//     { e_IMPULSE   , e_CANADA  , e_FAST  },
//     { e_NON_PROFIT, e_USA_EAST, e_FAST  },
//     { e_NEED_BASED, e_USA_WEST, e_TOAST },
//     { e_NON_PROFIT, e_GERMANY , e_TIDY  },
//     { e_NON_PROFIT, e_ENGLAND , e_GREEN },
//     { e_REPEAT    , e_GERMANY , e_PEARL },
//     { e_NEED_BASED, e_USA_EAST, e_PEARL },
//     { e_NON_PROFIT, e_RUSSIA  , e_PEARL },
//     { e_NEED_BASED, e_ENGLAND , e_SMITH },
//     { e_INSTITUTE , e_CANADA  , e_SMITH },
//     { e_NEED_BASED, e_ENGLAND , e_TOAST },
//     { e_NON_PROFIT, e_MEXICO  , e_TIDY  },
//     { e_BUSINESS  , e_GERMANY , e_FAST  },
//     { e_NEED_BASED, e_SCOTLAND, e_PEARL },
//     { e_NON_PROFIT, e_USA_WEST, e_TIDY  },
//     { e_NON_PROFIT, e_USA_WEST, e_TOAST },
//     { e_IMPULSE   , e_FRANCE  , e_PEARL },
//     { e_IMPULSE   , e_ENGLAND , e_FAST  },
//     { e_IMPULSE   , e_USA_WEST, e_GREEN },
//     { e_DISCOUNT  , e_MEXICO  , e_SMITH },
//     { e_INSTITUTE , e_GERMANY , e_TOAST },
//     { e_NEED_BASED, e_CANADA  , e_PEARL },
//     { e_NON_PROFIT, e_USA_WEST, e_FAST  },
//     { e_DISCOUNT  , e_RUSSIA  , e_SMITH },
//     { e_INSTITUTE , e_USA_WEST, e_GREEN },
//     { e_INSTITUTE , e_RUSSIA  , e_TOAST },
//     { e_INSTITUTE , e_FRANCE  , e_SMITH },
//     { e_INSTITUTE , e_SCOTLAND, e_SMITH },
//     { e_NON_PROFIT, e_ENGLAND , e_PEARL },
//     { e_NON_PROFIT, e_CANADA  , e_SMITH },
//     { e_NON_PROFIT, e_USA_EAST, e_TOAST },
//     { e_REPEAT    , e_FRANCE  , e_TOAST },
//     { e_NEED_BASED, e_FRANCE  , e_FAST  },
//     { e_DISCOUNT  , e_MEXICO  , e_TOAST },
//     { e_DISCOUNT  , e_FRANCE  , e_GREEN },
//     { e_IMPULSE   , e_USA_EAST, e_FAST  },
//     { e_REPEAT    , e_USA_EAST, e_GREEN },
//     { e_NON_PROFIT, e_GERMANY , e_GREEN },
//     { e_INSTITUTE , e_CANADA  , e_SMITH },
//     { e_NEED_BASED, e_SCOTLAND, e_TOAST },
//     { e_NEED_BASED, e_GERMANY , e_FAST  },
//     { e_NON_PROFIT, e_RUSSIA  , e_TOAST },
//     { e_BUSINESS  , e_ENGLAND , e_PEARL },
//     { e_NEED_BASED, e_USA_EAST, e_TOAST },
//     { e_INSTITUTE , e_USA_EAST, e_SMITH },
//     { e_DISCOUNT  , e_USA_EAST, e_PEARL },
//     { e_REPEAT    , e_SCOTLAND, e_FAST  },
//     { e_IMPULSE   , e_GERMANY , e_TIDY  },
//     { e_DISCOUNT  , e_CANADA  , e_TIDY  },
//     { e_IMPULSE   , e_USA_EAST, e_TIDY  },
//     { e_IMPULSE   , e_GERMANY , e_TIDY  },
//     { e_NON_PROFIT, e_ENGLAND , e_FAST  },
//     { e_NON_PROFIT, e_USA_WEST, e_TIDY  },
//     { e_REPEAT    , e_MEXICO  , e_TOAST },
// };
// const bsl::size_t numCustomerProfiles = sizeof  customerProfiles
//                                       / sizeof *customerProfiles;
// ```
// Suppose, as the first step in our analysis, we wish to determine the number
// of unique combinations of customer attributes that exist in our data set.
// We can do that by inserting each data item into a flat hash set: the first
// insert of a combination will succeed, the others will fail, but at the end
// of the process, the set will contain one entry for every unique combination
// in our data.
//
// First, as there are no standard methods for hashing or comparing our
// user-defined types, we define `CustomerProfileHash` and
// `CustomerProfileEqual` classes, each a stateless functor.  Note that there
// is no meaningful ordering of the attribute values, they are merely arbitrary
// code numbers; nothing is lost by using an unordered set instead of an
// ordered set:
// ```
// class CustomerProfileHash {
//   public:
//     // CREATORS
//
//
//     /// Create a `CustomerProfileHash` object.
//     //! CustomerProfileHash() = default;
//
//     /// Create a `CustomerProfileHash` object.  Note that as
//     /// `CustomerProfileHash` is an empty (stateless) type, this
//     /// operation has no observable effect.
//     //! CustomerProfileHash(const CustomerProfileHash& original) = default;
//
//     /// Destroy this object.
//     //! ~CustomerProfileHash() = default;
//
//     // ACCESSORS
//
//     /// Return a hash value for the specified `x`.
//     bsl::size_t operator()(const CustomerProfile& x) const;
// };
// ```
// The hash function combines the several enumerated values from the class
// (each a small `int` value) into a single, unique `int` value, and then
// applies the default hash function for `int`.
// ```
// // ACCESSORS
// bsl::size_t CustomerProfileHash::operator()(const CustomerProfile& x) const
// {
//     return bsl::hash<int>()(  x.d_location * 100 * 100
//                             + x.d_customer * 100
//                             + x.d_project);
// }
//
// class CustomerProfileEqual {
//   public:
//     // CREATORS
//
//     /// Create a `CustomerProfileEqual` object.
//     //! CustomerProfileEqual() = default;
//
//     /// Create a `CustomerProfileEqual` object.  Note that as
//     /// `CustomerProfileEqual` is an empty (stateless) type, this
//     /// operation has no observable effect.
//     //! CustomerProfileEqual(const CustomerProfileEqual& original)
//     //!                                                          = default;
//
//     /// Destroy this object.
//     //! ~CustomerProfileEqual() = default;
//
//     // ACCESSORS
//
//     /// Return `true` if the specified `lhs` has the same value as the
//     /// specified `rhs`, and `false` otherwise.
//     bool operator()(const CustomerProfile& lhs,
//                     const CustomerProfile& rhs) const;
// };
//
// // ACCESSORS
// bool CustomerProfileEqual::operator()(const CustomerProfile& lhs,
//                                       const CustomerProfile& rhs) const
// {
//     return lhs.d_location == rhs.d_location
//         && lhs.d_customer == rhs.d_customer
//         && lhs.d_project  == rhs.d_project;
// }
// ```
// Notice that many of the required methods of the hash and comparator types
// are compiler generated.  (The declarations of those methods are commented
// out and suffixed by an `= default` comment.)
//
// Then, we define the type of the flat hash set:
// ```
// typedef bdlc::FlatHashSet<CustomerProfile,
//                           CustomerProfileHash,
//                           CustomerProfileEqual> ProfileCategories;
// ```
// Next, we create a flat hash set and insert each item of `customerProfiles`:
// ```
// bslma::TestAllocator oa("object", veryVeryVeryVerbose);
//
// ProfileCategories profileCategories(&oa);
//
// for (bsl::size_t idx = 0; idx < numCustomerProfiles; ++idx) {
//    profileCategories.insert(customerProfiles[idx]);
// }
//
// assert(numCustomerProfiles >= profileCategories.size());
// ```
// Notice that we ignore the status returned by the `insert` method.  We fully
// expect some operations to fail.
//
// Finally, the size of `profileCategories` matches the number of unique
// customer profiles in this data set:
// ```
// if (verbose) {
//     bsl::cout << numCustomerProfiles << ' ' << profileCategories.size()
//               << bsl::endl;
// }
// ```
// Standard output shows:
// ```
// 100 84
// ```

#include <bdlscm_version.h>

#include <bdlc_flathashtable.h>

#include <bslalg_hasstliterators.h>
#include <bslalg_swaputil.h>

#include <bslh_fibonaccibadhashwrapper.h>

#include <bslim_printer.h>

#include <bslma_allocator.h>
#include <bslma_constructionutil.h>
#include <bslma_usesbslmaallocator.h>

#include <bslmf_enableif.h>
#include <bslmf_isconvertible.h>
#include <bslmf_istransparentpredicate.h>
#include <bslmf_movableref.h>
#include <bslmf_util.h>    // 'forward(V)'

#include <bsls_assert.h>
#include <bsls_compilerfeatures.h>
#include <bsls_platform.h>
#include <bsls_util.h>     // 'forward<T>(V)'

#include <bslstl_equalto.h>
#include <bslstl_hash.h>

#if defined(BSLS_COMPILERFEATURES_SUPPORT_GENERALIZED_INITIALIZERS)
#include <bsl_initializer_list.h>
#endif
#include <bsl_cstddef.h>
#include <bsl_ostream.h>
#include <bsl_utility.h>

#if BSLS_COMPILERFEATURES_SIMULATE_CPP11_FEATURES
// clang-format off
// Include version that can be compiled with C++03
// Generated on Mon Jan 13 08:32:17 2025
// Command line: sim_cpp11_features.pl bdlc_flathashset.h

# define COMPILING_BDLC_FLATHASHSET_H
# include <bdlc_flathashset_cpp03.h>
# undef COMPILING_BDLC_FLATHASHSET_H

// clang-format on
#else

namespace BloombergLP {
namespace bdlc {

// FORWARD DECLARATIONS
template <class KEY,
          class HASH  = bslh::FibonacciBadHashWrapper<bsl::hash<KEY> >,
          class EQUAL = bsl::equal_to<KEY> >
class FlatHashSet;

template <class KEY, class HASH, class EQUAL>
bool operator==(const FlatHashSet<KEY, HASH, EQUAL> &a,
                const FlatHashSet<KEY, HASH, EQUAL> &b);

template <class KEY, class HASH, class EQUAL>
bool operator!=(const FlatHashSet<KEY, HASH, EQUAL> &a,
                const FlatHashSet<KEY, HASH, EQUAL> &b);

template <class KEY, class HASH, class EQUAL>
void swap(FlatHashSet<KEY, HASH, EQUAL>& a, FlatHashSet<KEY, HASH, EQUAL>& b);

                       // ============================
                       // struct FlatHashSet_EntryUtil
                       // ============================

/// This templated utility provides methods to construct an `ENTRY` and a
/// method to extract the key from an `ENTRY` (which is, identically, the
/// `ENTRY`).
template <class ENTRY>
struct FlatHashSet_EntryUtil
{
    // CLASS METHODS
#if !BSLS_COMPILERFEATURES_SIMULATE_CPP11_FEATURES
    /// Load into the specified `entry` the `ENTRY` value constructed from
    /// specified `args`, using the specified `allocator` to supply memory.
    ///  `allocator` is ignored if the (template parameter) type `ENTRY` is
    /// not allocator aware.
    template <class... ARGS>
    static void construct(
                        ENTRY                                       *entry,
                        bslma::Allocator                            *allocator,
                        ARGS&&...                                    args);
#endif

    /// Load into the specified `entry` the `ENTRY` value comprised of the
    /// specified `key`, using the specified `allocator` to supply memory.
    /// `allocator` is ignored if the (template parameter) type `ENTRY` is
    /// not allocator aware.
    template <class KEY_TYPE>
    static void constructFromKey(
                        ENTRY                                       *entry,
                        bslma::Allocator                            *allocator,
                        BSLS_COMPILERFEATURES_FORWARD_REF(KEY_TYPE)  key);

    /// Return the specified `entry`.
    static const ENTRY& key(const ENTRY& entry);
};

                            // =================
                            // class FlatHashSet
                            // =================

/// This class template implements a value-semantic container type holding
/// an unordered set of unique values of (template parameter) type `KEY`.
/// The (template parameter) type `HASH` is a functor providing the hash
/// value for `KEY`.  The (template parameter) type `EQUAL` is a functor
/// providing the equality function for two `KEY` values.  See {Requirements
/// on `KEY`, `HASH`, and `EQUAL`} for more information.
template <class KEY, class HASH, class EQUAL>
class FlatHashSet {

  private:
    // PRIVATE TYPES

    /// This is the underlying implementation class.
    typedef FlatHashTable<KEY,
                          KEY,
                          FlatHashSet_EntryUtil<KEY>,
                          HASH,
                          EQUAL> ImplType;

    // FRIENDS
    friend bool operator==<>(const FlatHashSet&, const FlatHashSet&);
    friend bool operator!=<>(const FlatHashSet&, const FlatHashSet&);

    // The following verbose declaration is required by the xlC 12.1 compiler.
    template <class K, class H, class E>
    friend void swap(FlatHashSet<K, H, E>&, FlatHashSet<K, H, E>&);

  public:
    // PUBLIC TYPES
    typedef KEY                                key_type;
    typedef KEY                                value_type;
    typedef bsl::size_t                        size_type;
    typedef bsl::ptrdiff_t                     difference_type;
    typedef EQUAL                              key_compare;
    typedef EQUAL                              value_compare;
    typedef HASH                               hasher;
    typedef value_type&                        reference;
    typedef const value_type&                  const_reference;
    typedef value_type*                        pointer;
    typedef const value_type*                  const_pointer;
    typedef typename ImplType::const_iterator  iterator;
    typedef typename ImplType::const_iterator  const_iterator;

  private:
    // DATA
    ImplType d_impl;  // underlying flat hash table used by this flat hash set

  public:
    // CREATORS

    /// Create an empty `FlatHashSet` object.  Optionally specify a
    /// `capacity` indicating the minimum initial size of the underlying
    /// array of entries of this container.  If `capacity` is not supplied
    /// or is 0, no memory is allocated.  Optionally specify a `hash`
    /// functor used to generate the hash values associated with the
    /// elements in this container.  If `hash` is not supplied, a
    /// default-constructed object of the (template parameter) type `HASH`
    /// is used.  Optionally specify an equality functor `equal` used to
    /// determine whether two elements are equivalent.  If `equal` is not
    /// supplied, a default-constructed object of the (template parameter)
    /// type `EQUAL` is used.  Optionally specify a `basicAllocator` used to
    /// supply memory.  If `basicAllocator` is not supplied or is 0, the
    /// currently installed default allocator is used.
    FlatHashSet();
    explicit FlatHashSet(bslma::Allocator *basicAllocator);
    explicit FlatHashSet(bsl::size_t capacity);
    FlatHashSet(bsl::size_t capacity, bslma::Allocator *basicAllocator);
    FlatHashSet(bsl::size_t       capacity,
                const HASH&       hash,
                bslma::Allocator *basicAllocator = 0);
    FlatHashSet(bsl::size_t       capacity,
                const HASH&       hash,
                const EQUAL&      equal,
                bslma::Allocator *basicAllocator = 0);

    /// Create a `FlatHashSet` object initialized by insertion of the values
    /// from the input iterator range specified by `first` through `last`
    /// (including `first`, excluding `last`).  Optionally specify a
    /// `capacity` indicating the minimum initial size of the underlying
    /// array of entries of this container.  If `capacity` is not supplied
    /// or is 0, no memory is allocated.  Optionally specify a `hash`
    /// functor used to generate hash values associated with the elements in
    /// this container.  If `hash` is not supplied, a default-constructed
    /// object of the (template parameter) type `HASH` is used.  Optionally
    /// specify an equality functor `equal` used to verify that two elements
    /// are equivalent.  If `equal` is not supplied, a default-constructed
    /// object of the (template parameter) type `EQUAL` is used.  Optionally
    /// specify a `basicAllocator` used to supply memory.  If
    /// `basicAllocator` is not supplied or is 0, the currently installed
    /// default allocator is used.  The behavior is undefined unless `first`
    /// and `last` refer to a sequence of valid values where `first` is at a
    /// position at or before `last`.  Note that if a member of the input
    /// sequence is equivalent to an earlier member, the later member will
    /// not be inserted.
    template <class INPUT_ITERATOR>
    FlatHashSet(INPUT_ITERATOR    first,
                INPUT_ITERATOR    last,
                bslma::Allocator *basicAllocator = 0);
    template <class INPUT_ITERATOR>
    FlatHashSet(INPUT_ITERATOR    first,
                INPUT_ITERATOR    last,
                bsl::size_t       capacity,
                bslma::Allocator *basicAllocator = 0);
    template <class INPUT_ITERATOR>
    FlatHashSet(INPUT_ITERATOR    first,
                INPUT_ITERATOR    last,
                bsl::size_t       capacity,
                const HASH&       hash,
                bslma::Allocator *basicAllocator = 0);
    template <class INPUT_ITERATOR>
    FlatHashSet(INPUT_ITERATOR    first,
                INPUT_ITERATOR    last,
                bsl::size_t       capacity,
                const HASH&       hash,
                const EQUAL&      equal,
                bslma::Allocator *basicAllocator = 0);

#if defined(BSLS_COMPILERFEATURES_SUPPORT_GENERALIZED_INITIALIZERS)
    /// Create a `FlatHashSet` object initialized by insertion of the
    /// specified `values`.  Optionally specify a `capacity` indicating the
    /// minimum initial size of the underlying array of entries of this
    /// container.  If `capacity` is not supplied or is 0, no memory is
    /// allocated.  Optionally specify a `hash` functor used to generate
    /// hash values associated with the elements in this container.  If
    /// `hash` is not supplied, a default-constructed object of the
    /// (template parameter) type `HASH` is used.  Optionally specify an
    /// equality functor `equal` used to verify that two elements are
    /// equivalent.  If `equal` is not supplied, a default-constructed
    /// object of the (template parameter) type `EQUAL` is used.  Optionally
    /// specify a `basicAllocator` used to supply memory.  If
    /// `basicAllocator` is not supplied or is 0, the currently installed
    /// default allocator is used.  Note that if a member of `values` has a
    /// key equivalent to an earlier member, the later member will not be
    /// inserted.
    FlatHashSet(bsl::initializer_list<KEY>  values,
                bslma::Allocator           *basicAllocator = 0);
    FlatHashSet(bsl::initializer_list<KEY>  values,
                bsl::size_t                 capacity,
                bslma::Allocator           *basicAllocator = 0);
    FlatHashSet(bsl::initializer_list<KEY>  values,
                bsl::size_t                 capacity,
                const HASH&                 hash,
                bslma::Allocator           *basicAllocator = 0);
    FlatHashSet(bsl::initializer_list<KEY>  values,
                bsl::size_t                 capacity,
                const HASH&                 hash,
                const EQUAL&                equal,
                bslma::Allocator           *basicAllocator = 0);
#endif

    /// Create a `FlatHashSet` object having the same value, hasher, and
    /// equality comparator as the specified `original` object.  Optionally
    /// specify a `basicAllocator` used to supply memory.  If
    /// `basicAllocator` is not specified or is 0, the currently installed
    /// default allocator is used.
    FlatHashSet(const FlatHashSet&  original,
                bslma::Allocator   *basicAllocator = 0);

    /// Create a `FlatHashSet` object having the same value, hasher,
    /// equality comparator, and allocator as the specified `original`
    /// object.  The contents of `original` are moved (in constant time) to
    /// this object, `original` is left in a (valid) unspecified state, and
    /// no exceptions will be thrown.
    FlatHashSet(bslmf::MovableRef<FlatHashSet> original);

    /// Create a `FlatHashSet` object having the same value, hasher, and
    /// equality comparator as the specified `original` object, using the
    /// specified `basicAllocator` to supply memory.  If `basicAllocator` is
    /// 0, the currently installed default allocator is used.  The allocator
    /// of `original` remains unchanged.  If `original` and the newly
    /// created object have the same allocator then the contents of
    /// `original` are moved (in constant time) to this object, `original`
    /// is left in a (valid) unspecified state, and no exceptions will be
    /// thrown; otherwise `original` is unchanged (and an exception may be
    /// thrown).
    FlatHashSet(bslmf::MovableRef<FlatHashSet>  original,
                bslma::Allocator               *basicAllocator);

    /// Destroy this object and each of its elements.
    ~FlatHashSet();

    // MANIPULATORS

    /// Assign to this object the value, hasher, and equality functor of the
    /// specified `rhs` object, and return a reference providing modifiable
    /// access to this object.
    FlatHashSet& operator=(const FlatHashSet& rhs);

    /// Assign to this object the value, hasher, and equality comparator of
    /// the specified `rhs` object, and return a reference providing
    /// modifiable access to this object.  If this object and `rhs` use the
    /// same allocator the contents of `rhs` are moved (in constant time) to
    /// this object.  `rhs` is left in a (valid) unspecified state.
    FlatHashSet& operator=(bslmf::MovableRef<FlatHashSet> rhs);

#if defined(BSLS_COMPILERFEATURES_SUPPORT_GENERALIZED_INITIALIZERS)
    /// Assign to this object the value resulting from first clearing this
    /// set and then inserting each object in the specified `values`
    /// initializer list, ignoring those objects having a value equivalent
    /// to that which appears earlier in the list; return a reference
    /// providing modifiable access to this object.  This method requires
    /// that the (template parameter) type `KEY` be `copy-insertable` into
    /// this set (see {Requirements on `KEY`, `HASH`, and `EQUAL`}).
    FlatHashSet& operator=(bsl::initializer_list<KEY> values);
#endif

    /// Remove all elements from this set.  Note that this set will be empty
    /// after calling this method, but allocated memory may be retained for
    /// future use.  See the `capacity` method.
    void clear();

#if !BSLS_COMPILERFEATURES_SIMULATE_CPP11_FEATURES
    /// Insert into this set a newly created `value_type` object,
    /// constructed by forwarding `get_allocator()` (if required) and the
    /// specified (variable number of) `args` to the corresponding
    /// constructor of `value_type`, if a key equivalent to such a value
    /// does not already exist in this set; otherwise, this method has no
    /// effect (other than possibly creating a temporary `value_type`
    /// object).  Return a pair whose `first` member is an iterator
    /// referring to the (possibly newly created and inserted) object in
    /// this set whose value is equivalent to that of an object constructed
    /// from `arguments`, and whose `second` member is `true` if a new value
    /// was inserted, and `false` if an equivalent key was already present.
    template <class... ARGS>
    bsl::pair<iterator, bool> emplace(ARGS&&... args);

    /// Insert into this set a newly created `value_type` object,
    /// constructed by forwarding `get_allocator()` (if required) and the
    /// specified (variable number of) `args` to the corresponding
    /// constructor of `value_type`, if a key equivalent to such a value
    /// does not already exists in this set; otherwise, this method has no
    /// effect (other than possibly creating a temporary `value_type`
    /// object).  Return an iterator referring to the (possibly newly
    /// created and inserted) object in this set whose value is equivalent
    /// to that of an object constructed from `arguments`.  The average and
    /// worst case complexity of this operation is not affected by the
    /// specified `hint`.  Note that `hint` is ignored (other than possibly
    /// asserting its validity in some build modes).
    template <class... ARGS>
    iterator emplace_hint(const_iterator hint, ARGS&&... args);
#endif

    /// Remove from this set the element whose key is equal to the specified
    /// `key`, if it exists, and return 1; otherwise (there is no element
    /// having `key` in this set), return 0 with no other effect.  This
    /// method invalidates all iterators and references to the removed
    /// element.
    bsl::size_t erase(const KEY& key);

#if !BSLS_COMPILERFEATURES_SIMULATE_CPP11_FEATURES
    /// Remove from this set the element whose key is equivalent to the
    /// specified `key`, if it exists, and return 1; otherwise (there is no
    /// element equivalent to `key` in this set), return 0 with no other effect.
    /// This method invalidates all iterators and references to the removed
    /// element.
    template <class LOOKUP_KEY>
    typename bsl::enable_if<
            BloombergLP::bslmf::IsTransparentPredicate<HASH, LOOKUP_KEY>::value
         && BloombergLP::bslmf::IsTransparentPredicate<EQUAL,LOOKUP_KEY>::value
          , bsl::size_t>::type
    erase(LOOKUP_KEY&& key)
    {
        return d_impl.erase(key);
    }
#endif

    /// Remove from this set the element at the specified `position`, and
    /// return a `const_iterator` referring to the element immediately
    /// following the removed element, or to the past-the-end position if
    /// the removed element was the last element in the sequence of elements
    /// maintained by this set.  This method invalidates all iterators and
    /// references to the removed element.  The behavior is undefined unless
    /// `position` refers to an element in this set.
    const_iterator erase(const_iterator position);

    /// Remove from this set the elements starting at the specified `first`
    /// position up to, but not including, the specified `last` position,
    /// and return `last`.  This method invalidates all iterators and
    /// references to the removed elements.  The behavior is undefined
    /// unless `first` and `last` are valid iterators on this set, and the
    /// `first` position is at or before the `last` position in the
    /// iteration sequence provided by this container.
    const_iterator erase(const_iterator first, const_iterator last);

    /// Insert the specified `value` into this set if the `value` does not
    /// already exist in this set; otherwise, this method has no effect.
    /// Return a `pair` whose `first` member is a `const_iterator` referring
    /// to the (possibly newly inserted) element in this set whose value is
    /// equivalent to that of the element to be inserted, and whose `second`
    /// member is `true` if a new element was inserted, and `false` if an
    /// equivalent value was already present.
    bsl::pair<const_iterator, bool> insert(const KEY& value)
    {
        // Note that some compilers require functions declared with 'enable_if'
        // to be defined inline.

        return d_impl.insert(value);
    }

    /// Insert the specified `value` into this set if the `value` does not
    /// already exist in this set; otherwise, this method has no effect.
    /// Return a `pair` whose `first` member is a `const_iterator` referring
    /// to the (possibly newly inserted) element in this set whose value is
    /// equivalent to that of the element to be inserted, and whose `second`
    /// member is `true` if a new element was inserted, and `false` if an
    /// equivalent value was already present.
    bsl::pair<const_iterator, bool> insert(bslmf::MovableRef<KEY> value)
    {
        // Note that some compilers require functions declared with 'enable_if'
        // to be defined inline.

        return d_impl.insert(BSLS_COMPILERFEATURES_FORWARD(KEY, value));
    }

#if !BSLS_COMPILERFEATURES_SIMULATE_CPP11_FEATURES
    /// Insert the specified `key` into this set if a key equivalent to `k`
    /// does not already exist in this set; otherwise, this method has no effect.
    /// Return a pair containing an `iterator` referring to the newly inserted
    /// element and `true` if a element was inserted.  Return a pair containing
    /// an `iterator` to an existing element and `false` if the value already
    /// existed in the set.
    template <class LOOKUP_KEY>
    typename bsl::enable_if<
            BloombergLP::bslmf::IsTransparentPredicate<HASH, LOOKUP_KEY>::value
         && BloombergLP::bslmf::IsTransparentPredicate<EQUAL,LOOKUP_KEY>::value
          , typename bsl::pair<iterator, bool> >::type
    insert(LOOKUP_KEY&& key)
    {
        return d_impl.insertTransparent(
                               BSLS_COMPILERFEATURES_FORWARD(LOOKUP_KEY, key));
    }
#endif

    /// Insert the specified `value` into this set if the `value` does not
    /// already exist in this set; otherwise, this method has no effect.
    /// Return a `const_iterator` referring to the (possibly newly inserted)
    /// element in this set whose value is equivalent to that of the
    /// element to be inserted.  The supplied `const_iterator` is ignored.
    const_iterator insert(const_iterator, const KEY& value)
    {
        // Note that some compilers require functions declared with 'enable_if'
        // to be defined inline.

        return d_impl.insert(value).first;
    }


    /// Insert the specified `value` into this set if the `value` does not
    /// already exist in this set; otherwise, this method has no effect.
    /// Return a `const_iterator` referring to the (possibly newly inserted)
    /// element in this set whose value is equivalent to that of the
    /// element to be inserted.  The supplied `const_iterator` is ignored.
    const_iterator insert(const_iterator, bslmf::MovableRef<KEY> value)
    {
        // Note that some compilers require functions declared with 'enable_if'
        // to be defined inline.

        return d_impl.insert(BSLS_COMPILERFEATURES_FORWARD(KEY,value)).first;
    }

#if !BSLS_COMPILERFEATURES_SIMULATE_CPP11_FEATURES
    /// Insert the specified `key` into this set if a key equivalent to `k`
    /// does not already exist in this set; otherwise, this method has no effect.
    /// Return a pair containing an `iterator` referring to the newly inserted
    /// element and `true` if a element was inserted.  Return a pair containing
    /// an `iterator` to an existing element and `false` if the value already
    /// existed in the set.  The supplied `const_iterator` is ignored.
    template <class LOOKUP_KEY>
    typename bsl::enable_if<
            BloombergLP::bslmf::IsTransparentPredicate<HASH, LOOKUP_KEY>::value
         && BloombergLP::bslmf::IsTransparentPredicate<EQUAL,LOOKUP_KEY>::value
         && bsl::is_convertible<LOOKUP_KEY&&, const_iterator>::value == false
         && bsl::is_convertible<const_iterator, LOOKUP_KEY&&>::value == false
          , iterator>::type
    insert(const_iterator, LOOKUP_KEY&& key)
    {
        return insert(BSLS_COMPILERFEATURES_FORWARD(LOOKUP_KEY, key)).first;
    }
#endif

    /// Insert into this set the value of each element in the input iterator
    /// range specified by `first` through `last` (including `first`,
    /// excluding `last`).  The behavior is undefined unless `first` and
    /// `last` refer to a sequence of valid values where `first` is at a
    /// position at or before `last`.  Note that if a member of the input
    /// sequence is equivalent to an earlier member, the later member will
    /// not be inserted.
    template <class INPUT_ITERATOR>
    void insert(INPUT_ITERATOR first, INPUT_ITERATOR last);

#if defined(BSLS_COMPILERFEATURES_SUPPORT_GENERALIZED_INITIALIZERS)
    /// Insert into this set an element having the value of each object in
    /// the specified `values` initializer list if an equivalent value is
    /// not already contained in this set.  This method requires that the
    /// (template parameter) type `KEY` be copy-insertable (see
    /// {Requirements on `KEY`, `HASH`, and `EQUAL`}).
    void insert(bsl::initializer_list<KEY> values);
#endif

    /// Change the capacity of this set to at least the specified
    /// `minimumCapacity`, and redistribute all the contained elements into
    /// a new sequence of entries according to their hash values.  If
    /// `0 == minimumCapacity` and `0 == size()`, the set is returned to the
    /// default constructed state.  After this call, `load_factor()` will be
    /// less than or equal to `max_load_factor()` and all iterators,
    /// pointers, and references to elements of this set are invalidated.
    void rehash(bsl::size_t minimumCapacity);

    /// Change the capacity of this set to at least a capacity that can
    /// accommodate the specified `numEntries` (accounting for the load
    /// factor invariant), and redistribute all the contained elements into
    /// a new sequence of entries according to their hash values.  If
    /// `0 == numEntries` and `0 == size()`, the set is returned to the
    /// default constructed state.  After this call, `load_factor()` will be
    /// less than or equal to `max_load_factor()` and all iterators,
    /// pointers, and references to elements of this set are invalidated.
    /// Note that this method is effectively equivalent to:
    /// ```
    ///    rehash(bsl::ceil(numEntries / max_load_factor()))
    /// ```
    void reserve(bsl::size_t numEntries);

    /// Remove all elements from this set and release all memory from this
    /// set, returning the set to the default constructed state.
    void reset();

                             // Aspects

    /// Exchange the value of this object as well as its hasher and equality
    /// functors with those of the specified `other` object.  The behavior
    /// is undefined unless this object was created with the same allocator
    /// as `other`.
    void swap(FlatHashSet& other);

    // ACCESSORS

    /// Return the number of elements this set could hold if the load factor
    /// were 1.
    bsl::size_t capacity() const;

    /// Return `true` if this set contains an element having the specified
    /// `key`, and `false` otherwise.
    bool contains(const KEY& key) const;

    /// Return `true` if this set contains an element whose key is equivalent
    /// to the specified `key`.
    ///
    /// Note: implemented inline due to Sun CC compilation error.
    template <class LOOKUP_KEY>
    typename bsl::enable_if<
            BloombergLP::bslmf::IsTransparentPredicate<HASH, LOOKUP_KEY>::value
        &&  BloombergLP::bslmf::IsTransparentPredicate<EQUAL,LOOKUP_KEY>::value
        , bool>::type
    contains(const LOOKUP_KEY& key) const
    {
        return find(key) != end();
    }

    /// Return the number of elements in this set having the specified
    /// `key`.  Note that since a flat hash set maintains unique keys, the
    /// returned value will be either 0 or 1.
    bsl::size_t count(const KEY& key) const;

    /// Return the number of `value_type` objects within this unordered set
    /// that are equivalent to the specified `key`.  The behavior is
    /// undefined unless `key` is equivalent to at most one element in this
    /// unordered set.  Note that since an unordered set maintains unique
    /// keys, the returned value will be either 0 or 1.
    ///
    /// Note: implemented inline due to Sun CC compilation error.
    template <class LOOKUP_KEY>
    typename bsl::enable_if<
            BloombergLP::bslmf::IsTransparentPredicate<HASH, LOOKUP_KEY>::value
         && BloombergLP::bslmf::IsTransparentPredicate<EQUAL,LOOKUP_KEY>::value
          , bsl::size_t >::type
    count(const LOOKUP_KEY& key) const
    {
        return find(key) != end() ? 1 : 0;
    }

    /// Return `true` if this set contains no elements, and `false`
    /// otherwise.
    bool empty() const;

    /// Return a pair of `const_iterator`s defining the sequence of elements
    /// in this set having the specified `key`, where the first iterator is
    /// positioned at the start of the sequence and the second iterator is
    /// positioned one past the end of the sequence.  If this set contains
    /// no `KEY` elements equivalent to `key`, then the two returned
    /// iterators will have the same value.  Note that since a set maintains
    /// unique keys, the range will contain at most one element.
    bsl::pair<const_iterator, const_iterator> equal_range(
                                                         const KEY& key) const;

    /// Return a pair of `const_iterator` providing non-modifiable access to
    /// the sequence of `value_type` objects in this unordered set that are
    /// equivalent to the specified `key`, where the first iterator is
    /// positioned at the start of the sequence and the second iterator is
    /// positioned one past the end of the sequence.  If this unordered set
    /// contains no `value_type` objects equivalent to `key`, then the two
    /// returned iterators will have the same value.  Note that since an
    /// unordered set maintains unique keys, the range will contain at most one
    /// element.
    ///
    /// Note: implemented inline due to Sun CC compilation error.
    template <class LOOKUP_KEY>
    typename bsl::enable_if<
            BloombergLP::bslmf::IsTransparentPredicate<HASH, LOOKUP_KEY>::value
         && BloombergLP::bslmf::IsTransparentPredicate<EQUAL,LOOKUP_KEY>::value
          , bsl::pair<const_iterator, const_iterator> >::type
    equal_range(const LOOKUP_KEY& key) const
    {
        return d_impl.equal_range(key);
    }

    /// Return a `const_iterator` referring to the element in this set
    /// having the specified `key`, or `end()` if no such entry exists in
    /// this set.
    const_iterator find(const KEY& key) const;

    /// Return a `const_iterator` referring to the element in this set
    /// having the specified `key`, or `end()` if no such entry exists in
    /// this set.
    ///
    /// Note: implemented inline due to Sun CC compilation error.
    template <class LOOKUP_KEY>
    typename bsl::enable_if<
            BloombergLP::bslmf::IsTransparentPredicate<HASH, LOOKUP_KEY>::value
         && BloombergLP::bslmf::IsTransparentPredicate<EQUAL,LOOKUP_KEY>::value
         , const_iterator>::type
    find(const LOOKUP_KEY& key) const
    {
        return iterator(d_impl.find(key));
    }


    /// Return (a copy of) the unary hash functor used by this set to
    /// generate a hash value (of type `bsl::size_t`) for a `KEY` object.
    HASH hash_function() const;

    /// Return (a copy of) the binary key-equality functor that returns
    /// `true` if the value of two `KEY` objects are equivalent, and `false`
    /// otherwise.
    EQUAL key_eq() const;

    /// Return the current ratio between the number of elements in this
    /// container and its capacity.
    float load_factor() const;

    /// Return the maximum load factor allowed for this set.  Note that if
    /// an insert operation would cause the load factor to exceed
    /// `max_load_factor()`, that same insert operation will increase the
    /// capacity and rehash the entries of the container (see {Load Factor
    /// and Resizing}).  Also note that the value returned by
    /// `max_load_factor` is implementation defined and cannot be changed by
    /// the user.
    float max_load_factor() const;

    /// Return the number of elements in this set.
    bsl::size_t size() const;

                          // Iterators

    /// Return a `const_iterator` to the first element in the sequence of
    /// elements maintained by this set, or the `end` iterator if this set
    /// is empty.
    const_iterator begin() const;

    /// Return a `const_iterator` to the first element in the sequence of
    /// elements maintained by this set, or the `end` iterator if this set
    /// is empty.
    const_iterator cbegin() const;

    /// Return a `const_iterator` to the past-the-end element in the
    /// sequence of `KEY` elements maintained by this set.
    const_iterator cend() const;

    /// Return a `const_iterator` to the past-the-end element in the
    /// sequence of `KEY` elements maintained by this set.
    const_iterator end() const;

                           // Aspects

    /// Return the allocator used by this flat hash set to supply memory.
    bslma::Allocator *allocator() const;

    /// Format this object to the specified output `stream` at the (absolute
    /// value of) the optionally specified indentation `level`, and return a
    /// reference to the modifiable `stream`.  If `level` is specified,
    /// optionally specify `spacesPerLevel`, the number of spaces per
    /// indentation level for this and all of its nested objects.  If
    /// `level` is negative, suppress indentation of the first line.  If
    /// `spacesPerLevel` is negative, format the entire output on one line,
    /// suppressing all but the initial indentation (as governed by
    /// `level`).  If `stream` is not valid on entry, this operation has no
    /// effect.
    bsl::ostream& print(bsl::ostream& stream,
                        int           level = 0,
                        int           spacesPerLevel = 4) const;
};

// FREE OPERATORS

/// Return `true` if the specified `lhs` and `rhs` objects have the same
/// value, and `false` otherwise.  Two `FlatHashSet` objects have the same
/// value if their sizes are the same and each element contained in one is
/// equal to an element of the other.  The hash and equality functors are
/// not involved in the comparison.
template <class KEY, class HASH, class EQUAL>
bool operator==(const FlatHashSet<KEY, HASH, EQUAL> &lhs,
                const FlatHashSet<KEY, HASH, EQUAL> &rhs);

/// Return `true` if the specified `lhs` and `rhs` objects do not have the
/// same value, and `false` otherwise.  Two `FlatHashSet` objects do not
/// have the same value if their sizes are different or one contains an
/// element equal to no element of the other.  The hash and equality
/// functors are not involved in the comparison.
template <class KEY, class HASH, class EQUAL>
bool operator!=(const FlatHashSet<KEY, HASH, EQUAL> &lhs,
                const FlatHashSet<KEY, HASH, EQUAL> &rhs);

/// Write the value of the specified `set` to the specified output `stream`
/// in a single-line format, and return a reference providing modifiable
/// access to `stream`.  If `stream` is not valid on entry, this operation
/// has no effect.  Note that this human-readable format is not fully
/// specified and can change without notice.
template <class KEY, class HASH, class EQUAL>
bsl::ostream& operator<<(bsl::ostream&                        stream,
                         const FlatHashSet<KEY, HASH, EQUAL>& set);

// FREE FUNCTIONS

/// Exchange the value, the hasher, and the key-equality functor of the
/// specified `a` and `b` objects.  This function provides the no-throw
/// exception-safety guarantee if the two objects were created with the same
/// allocator and the basic guarantee otherwise.
template <class KEY, class HASH, class EQUAL>
void swap(FlatHashSet<KEY, HASH, EQUAL>& a, FlatHashSet<KEY, HASH, EQUAL>& b);

// ============================================================================
//                  TEMPLATE AND INLINE FUNCTION DEFINITIONS
// ============================================================================

                       // ----------------------------
                       // struct FlatHashSet_EntryUtil
                       // ----------------------------

// CLASS METHODS
#if !BSLS_COMPILERFEATURES_SIMULATE_CPP11_FEATURES
template <class ENTRY>
template <class... ARGS>
inline
void FlatHashSet_EntryUtil<ENTRY>::construct(
                        ENTRY                                       *entry,
                        bslma::Allocator                            *allocator,
                        ARGS&&...                                    args)
{
    BSLS_ASSERT_SAFE(entry);
    bslma::ConstructionUtil::construct(
                                 entry,
                                 allocator,
                                 BSLS_COMPILERFEATURES_FORWARD(ARGS, args)...);
}
#endif

template <class ENTRY>
template <class KEY>
inline
void FlatHashSet_EntryUtil<ENTRY>::constructFromKey(
                             ENTRY                                  *entry,
                             bslma::Allocator                       *allocator,
                             BSLS_COMPILERFEATURES_FORWARD_REF(KEY)  key)
{
    BSLS_ASSERT_SAFE(entry);

    bslma::ConstructionUtil::construct(
                                 entry,
                                 allocator,
                                 BSLS_COMPILERFEATURES_FORWARD(KEY, key));
}

template <class ENTRY>
inline
const ENTRY& FlatHashSet_EntryUtil<ENTRY>::key(const ENTRY& entry)
{
    return entry;
}

                            // -----------------
                            // class FlatHashSet
                            // -----------------

// CREATORS
template <class KEY, class HASH, class EQUAL>
inline
FlatHashSet<KEY, HASH, EQUAL>::FlatHashSet()
: d_impl(0, HASH(), EQUAL())
{
}

template <class KEY, class HASH, class EQUAL>
inline
FlatHashSet<KEY, HASH, EQUAL>::FlatHashSet(bslma::Allocator *basicAllocator)
: d_impl(0, HASH(), EQUAL(), basicAllocator)
{
}

template <class KEY, class HASH, class EQUAL>
inline
FlatHashSet<KEY, HASH, EQUAL>::FlatHashSet(bsl::size_t capacity)
: d_impl(capacity, HASH(), EQUAL())
{
}

template <class KEY, class HASH, class EQUAL>
inline
FlatHashSet<KEY, HASH, EQUAL>::FlatHashSet(bsl::size_t       capacity,
                                           bslma::Allocator *basicAllocator)
: d_impl(capacity, HASH(), EQUAL(), basicAllocator)
{
}

template <class KEY, class HASH, class EQUAL>
inline
FlatHashSet<KEY, HASH, EQUAL>::FlatHashSet(bsl::size_t       capacity,
                                           const HASH&       hash,
                                           bslma::Allocator *basicAllocator)
: d_impl(capacity, hash, EQUAL(), basicAllocator)
{
}

template <class KEY, class HASH, class EQUAL>
inline
FlatHashSet<KEY, HASH, EQUAL>::FlatHashSet(bsl::size_t       capacity,
                                           const HASH&       hash,
                                           const EQUAL&      equal,
                                           bslma::Allocator *basicAllocator)
: d_impl(capacity, hash, equal, basicAllocator)
{
}

template <class KEY, class HASH, class EQUAL>
template <class INPUT_ITERATOR>
inline
FlatHashSet<KEY, HASH, EQUAL>::FlatHashSet(INPUT_ITERATOR    first,
                                           INPUT_ITERATOR    last,
                                           bslma::Allocator *basicAllocator)
: d_impl(0, HASH(), EQUAL(), basicAllocator)
{
    insert(first, last);
}

template <class KEY, class HASH, class EQUAL>
template <class INPUT_ITERATOR>
inline
FlatHashSet<KEY, HASH, EQUAL>::FlatHashSet(INPUT_ITERATOR    first,
                                           INPUT_ITERATOR    last,
                                           bsl::size_t       capacity,
                                           bslma::Allocator *basicAllocator)
: d_impl(capacity, HASH(), EQUAL(), basicAllocator)
{
    insert(first, last);
}

template <class KEY, class HASH, class EQUAL>
template <class INPUT_ITERATOR>
inline
FlatHashSet<KEY, HASH, EQUAL>::FlatHashSet(INPUT_ITERATOR    first,
                                           INPUT_ITERATOR    last,
                                           bsl::size_t       capacity,
                                           const HASH&       hash,
                                           bslma::Allocator *basicAllocator)
: d_impl(capacity, hash, EQUAL(), basicAllocator)
{
    insert(first, last);
}

template <class KEY, class HASH, class EQUAL>
template <class INPUT_ITERATOR>
inline
FlatHashSet<KEY, HASH, EQUAL>::FlatHashSet(INPUT_ITERATOR    first,
                                           INPUT_ITERATOR    last,
                                           bsl::size_t       capacity,
                                           const HASH&       hash,
                                           const EQUAL&      equal,
                                           bslma::Allocator *basicAllocator)
: d_impl(capacity, hash, equal, basicAllocator)
{
    insert(first, last);
}

#if defined(BSLS_COMPILERFEATURES_SUPPORT_GENERALIZED_INITIALIZERS)
template <class KEY, class HASH, class EQUAL>
inline
FlatHashSet<KEY, HASH, EQUAL>::FlatHashSet(
                                    bsl::initializer_list<KEY>  values,
                                    bslma::Allocator           *basicAllocator)
: FlatHashSet(values.begin(),
              values.end(),
              0,
              HASH(),
              EQUAL(),
              basicAllocator)
{
}

template <class KEY, class HASH, class EQUAL>
inline
FlatHashSet<KEY, HASH, EQUAL>::FlatHashSet(
                                    bsl::initializer_list<KEY>  values,
                                    bsl::size_t                 capacity,
                                    bslma::Allocator           *basicAllocator)
: FlatHashSet(values.begin(),
              values.end(),
              capacity,
              HASH(),
              EQUAL(),
              basicAllocator)
{
}

template <class KEY, class HASH, class EQUAL>
inline
FlatHashSet<KEY, HASH, EQUAL>::FlatHashSet(
                                    bsl::initializer_list<KEY>  values,
                                    bsl::size_t                 capacity,
                                    const HASH&                 hash,
                                    bslma::Allocator           *basicAllocator)
: FlatHashSet(values.begin(),
              values.end(),
              capacity,
              hash,
              EQUAL(),
              basicAllocator)
{
}

template <class KEY, class HASH, class EQUAL>
inline
FlatHashSet<KEY, HASH, EQUAL>::FlatHashSet(
                                    bsl::initializer_list<KEY>  values,
                                    bsl::size_t                 capacity,
                                    const HASH&                 hash,
                                    const EQUAL&                equal,
                                    bslma::Allocator           *basicAllocator)
: FlatHashSet(values.begin(),
              values.end(),
              capacity,
              hash,
              equal,
              basicAllocator)
{
}
#endif

template <class KEY, class HASH, class EQUAL>
inline
FlatHashSet<KEY, HASH, EQUAL>::FlatHashSet(const FlatHashSet&  original,
                                           bslma::Allocator   *basicAllocator)
: d_impl(original.d_impl, basicAllocator)
{
}

template <class KEY, class HASH, class EQUAL>
inline
FlatHashSet<KEY, HASH, EQUAL>::FlatHashSet(
                                       bslmf::MovableRef<FlatHashSet> original)
: d_impl(bslmf::MovableRefUtil::move(
                               bslmf::MovableRefUtil::access(original).d_impl))
{
}

template <class KEY, class HASH, class EQUAL>
inline
FlatHashSet<KEY, HASH, EQUAL>::FlatHashSet(
                                bslmf::MovableRef<FlatHashSet>  original,
                                bslma::Allocator               *basicAllocator)
: d_impl(bslmf::MovableRefUtil::move(
                               bslmf::MovableRefUtil::access(original).d_impl),
         basicAllocator)
{
}

template <class KEY, class HASH, class EQUAL>
inline
FlatHashSet<KEY, HASH, EQUAL>::~FlatHashSet()
{
}

// MANIPULATORS
template <class KEY, class HASH, class EQUAL>
inline
FlatHashSet<KEY, HASH, EQUAL>& FlatHashSet<KEY, HASH, EQUAL>::operator=(
                                                        const FlatHashSet& rhs)
{
    d_impl = rhs.d_impl;

    return *this;
}

template <class KEY, class HASH, class EQUAL>
inline
FlatHashSet<KEY, HASH, EQUAL>& FlatHashSet<KEY, HASH, EQUAL>::operator=(
                                            bslmf::MovableRef<FlatHashSet> rhs)
{
    FlatHashSet& lvalue = rhs;

    d_impl = bslmf::MovableRefUtil::move(lvalue.d_impl);

    return *this;
}

#if defined(BSLS_COMPILERFEATURES_SUPPORT_GENERALIZED_INITIALIZERS)
template <class KEY, class HASH, class EQUAL>
inline
FlatHashSet<KEY, HASH, EQUAL>& FlatHashSet<KEY, HASH, EQUAL>::operator=(
                                             bsl::initializer_list<KEY> values)
{
    FlatHashSet tmp(values.begin(),
                    values.end(),
                    0,
                    d_impl.hash_function(),
                    d_impl.key_eq(),
                    d_impl.allocator());

    this->swap(tmp);

    return *this;
}
#endif

template <class KEY, class HASH, class EQUAL>
inline
void FlatHashSet<KEY, HASH, EQUAL>::clear()
{
    d_impl.clear();
}

#if !BSLS_COMPILERFEATURES_SIMULATE_CPP11_FEATURES
template <class KEY, class HASH, class EQUAL>
template <class... ARGS>
inline bsl::pair<typename FlatHashSet<KEY, HASH, EQUAL>::iterator, bool>
FlatHashSet<KEY, HASH, EQUAL>::emplace(ARGS&&... args)
{
    return d_impl.emplace(BSLS_COMPILERFEATURES_FORWARD(ARGS, args)...);
}

template <class KEY, class HASH, class EQUAL>
template <class... ARGS>
inline typename FlatHashSet<KEY, HASH, EQUAL>::iterator
FlatHashSet<KEY, HASH, EQUAL>::emplace_hint(
                  typename FlatHashSet<KEY, HASH, EQUAL>::const_iterator,
                  ARGS&&...                                               args)
{
    return this->emplace(BSLS_COMPILERFEATURES_FORWARD(ARGS, args)...).first;
}

#endif


template <class KEY, class HASH, class EQUAL>
inline
bsl::size_t FlatHashSet<KEY, HASH, EQUAL>::erase(const KEY& key)
{
    return d_impl.erase(key);
}

template <class KEY, class HASH, class EQUAL>
inline
typename FlatHashSet<KEY, HASH, EQUAL>::const_iterator
                  FlatHashSet<KEY, HASH, EQUAL>::erase(const_iterator position)
{
    BSLS_ASSERT_SAFE(position != end());

    return d_impl.erase(position);
}

template <class KEY, class HASH, class EQUAL>
inline
typename FlatHashSet<KEY, HASH, EQUAL>::const_iterator
FlatHashSet<KEY, HASH, EQUAL>::erase(const_iterator first, const_iterator last)
{
    return d_impl.erase(first, last);
}

template <class KEY, class HASH, class EQUAL>
template <class INPUT_ITERATOR>
inline
void FlatHashSet<KEY, HASH, EQUAL>::insert(INPUT_ITERATOR first,
                                           INPUT_ITERATOR last)
{
    d_impl.insert(first, last);
}

#if defined(BSLS_COMPILERFEATURES_SUPPORT_GENERALIZED_INITIALIZERS)
template <class KEY, class HASH, class EQUAL>
inline
void FlatHashSet<KEY, HASH, EQUAL>::insert(bsl::initializer_list<KEY> values)
{
    insert(values.begin(), values.end());
}
#endif

template <class KEY, class HASH, class EQUAL>
inline
void FlatHashSet<KEY, HASH, EQUAL>::rehash(bsl::size_t minimumCapacity)
{
    d_impl.rehash(minimumCapacity);
}

template <class KEY, class HASH, class EQUAL>
inline
void FlatHashSet<KEY, HASH, EQUAL>::reserve(bsl::size_t numEntries)
{
    d_impl.reserve(numEntries);
}

template <class KEY, class HASH, class EQUAL>
inline
void FlatHashSet<KEY, HASH, EQUAL>::reset()
{
    d_impl.reset();
}

                             // Aspects

template <class KEY, class HASH, class EQUAL>
inline
void FlatHashSet<KEY, HASH, EQUAL>::swap(FlatHashSet& other)
{
    BSLS_ASSERT_SAFE(allocator() == other.allocator());

    d_impl.swap(other.d_impl);
}

// ACCESSORS
template <class KEY, class HASH, class EQUAL>
inline
bsl::size_t FlatHashSet<KEY, HASH, EQUAL>::capacity() const
{
    return d_impl.capacity();
}

template <class KEY, class HASH, class EQUAL>
inline
bool FlatHashSet<KEY, HASH, EQUAL>::contains(const KEY& key) const
{
    return d_impl.contains(key);
}

template <class KEY, class HASH, class EQUAL>
inline
bsl::size_t FlatHashSet<KEY, HASH, EQUAL>::count(const KEY& key) const
{
    return d_impl.count(key);
}

template <class KEY, class HASH, class EQUAL>
inline
bool FlatHashSet<KEY, HASH, EQUAL>::empty() const
{
    return d_impl.empty();
}

template <class KEY, class HASH, class EQUAL>
inline
bsl::pair<typename FlatHashSet<KEY, HASH, EQUAL>::const_iterator,
          typename FlatHashSet<KEY, HASH, EQUAL>::const_iterator>
               FlatHashSet<KEY, HASH, EQUAL>::equal_range(const KEY& key) const
{
    return d_impl.equal_range(key);
}

template <class KEY, class HASH, class EQUAL>
inline
typename FlatHashSet<KEY, HASH, EQUAL>::const_iterator
                      FlatHashSet<KEY, HASH, EQUAL>::find(const KEY& key) const
{
    return d_impl.find(key);
}

template <class KEY, class HASH, class EQUAL>
inline
HASH FlatHashSet<KEY, HASH, EQUAL>::hash_function() const
{
    return d_impl.hash_function();
}

template <class KEY, class HASH, class EQUAL>
inline
EQUAL FlatHashSet<KEY, HASH, EQUAL>::key_eq() const
{
    return d_impl.key_eq();
}

template <class KEY, class HASH, class EQUAL>
inline
float FlatHashSet<KEY, HASH, EQUAL>::load_factor() const
{
    return d_impl.load_factor();
}

template <class KEY, class HASH, class EQUAL>
inline
float FlatHashSet<KEY, HASH, EQUAL>::max_load_factor() const
{
    return d_impl.max_load_factor();
}

template <class KEY, class HASH, class EQUAL>
inline
bsl::size_t FlatHashSet<KEY, HASH, EQUAL>::size() const
{
    return d_impl.size();
}

                          // Iterators

template <class KEY, class HASH, class EQUAL>
inline
typename FlatHashSet<KEY, HASH, EQUAL>::const_iterator
                                   FlatHashSet<KEY, HASH, EQUAL>::begin() const
{
    return d_impl.begin();
}

template <class KEY, class HASH, class EQUAL>
inline
typename FlatHashSet<KEY, HASH, EQUAL>::const_iterator
                                 FlatHashSet<KEY, HASH, EQUAL>::cbegin() const
{
    return d_impl.cbegin();
}

template <class KEY, class HASH, class EQUAL>
inline
typename FlatHashSet<KEY, HASH, EQUAL>::const_iterator
                                   FlatHashSet<KEY, HASH, EQUAL>::cend() const
{
    return d_impl.cend();
}

template <class KEY, class HASH, class EQUAL>
inline
typename FlatHashSet<KEY, HASH, EQUAL>::const_iterator
                                     FlatHashSet<KEY, HASH, EQUAL>::end() const
{
    return d_impl.end();
}

                             // Aspects

template <class KEY, class HASH, class EQUAL>
inline
bslma::Allocator *FlatHashSet<KEY, HASH, EQUAL>::allocator() const
{
    return d_impl.allocator();
}

template <class KEY, class HASH, class EQUAL>
bsl::ostream& FlatHashSet<KEY, HASH, EQUAL>::print(
                                            bsl::ostream& stream,
                                            int           level,
                                            int           spacesPerLevel) const
{
    if (stream.bad()) {
        return stream;                                                // RETURN
    }

    bslim::Printer printer(&stream, level, spacesPerLevel);

    printer.start();

    const_iterator iter = begin();
    while (iter != end()) {
        printer.printValue(*iter);
        ++iter;
    }

    printer.end();

    return stream;
}

}  // close package namespace

// FREE OPERATORS
template <class KEY, class HASH, class EQUAL>
inline
bool bdlc::operator==(const FlatHashSet<KEY, HASH, EQUAL>& lhs,
                      const FlatHashSet<KEY, HASH, EQUAL>& rhs)
{
    return lhs.d_impl == rhs.d_impl;
}

template <class KEY, class HASH, class EQUAL>
inline
bool bdlc::operator!=(const FlatHashSet<KEY, HASH, EQUAL>& lhs,
                      const FlatHashSet<KEY, HASH, EQUAL>& rhs)
{
    return lhs.d_impl != rhs.d_impl;
}

template <class KEY, class HASH, class EQUAL>
inline
bsl::ostream& bdlc::operator<<(bsl::ostream&                        stream,
                               const FlatHashSet<KEY, HASH, EQUAL>& set)
{
    return set.print(stream, 0, -1);
}

// FREE FUNCTIONS
template <class KEY, class HASH, class EQUAL>
inline
void bdlc::swap(FlatHashSet<KEY, HASH, EQUAL>& a,
                FlatHashSet<KEY, HASH, EQUAL>& b)
{
    bslalg::SwapUtil::swap(&a.d_impl, &b.d_impl);
}

// ============================================================================
//                                TYPE TRAITS
// ============================================================================

namespace bslalg {

template <class KEY, class HASH, class EQUAL>
struct HasStlIterators<bdlc::FlatHashSet<KEY, HASH, EQUAL> >
     : bsl::true_type
{};

}  // close namespace bslalg

namespace bslma {

template <class KEY, class HASH, class EQUAL>
struct UsesBslmaAllocator<bdlc::FlatHashSet<KEY, HASH, EQUAL> >
     : bsl::true_type
{};

}  // close namespace bslma
}  // close enterprise namespace

#endif // End C++11 code

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
