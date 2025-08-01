// ball_category.h                                                    -*-C++-*-
#ifndef INCLUDED_BALL_CATEGORY
#define INCLUDED_BALL_CATEGORY

#include <bsls_ident.h>
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide a container for a name and associated thresholds.
//
//@CLASSES:
//  ball::Category: container for a name and associated threshold levels
//  ball::CategoryHolder: *private* holder of a category and its maximum level
//  ball::CategoryManagerImpUtil: *private* used in creating a category manager
//
//@SEE_ALSO: ball_categorymanager
//
//@DESCRIPTION: This component primarily provides a class, `ball::Category`,
// used to describe the properties of a logging category.  A `ball::Category`
// provides access to the category name and the 4 logging threshold levels
// associated with a category (see {`ball_loggermanager`} for a description of
// the purpose of the various thresholds).
//
///`ball` "Private" Methods and Classes
///------------------------------------
// This component provides classes that are *not* intended for use by the users
// of the `ball` logging sub-system: `ball::CategoryHolder` and
// `ball::CategoryManagerImpUtil`.  These classes are defined in this component
// because they are either friends of `ball::Category` or have a circular
// definition with `ball::Category`.  They are used within the logging
// sub-system to efficiently process log records.
//
///`ball::CategoryHolder`
/// - - - - - - - - - - -
// A `ball::CategoryHolder` is a statically-initializable pointer to a log
// category.  It is designed to work with the logging macros provided by `ball`
// (see {`ball_log`}), and provide a static cache of the log category at the
// point where a log macro is invoked.
//
///`ball::CategoryManagerImpUtil`
/// - - - - - - - - - - - - - - -
// A `ball::CategoryManagerImpUtil` provides a suite of utility functions used
// in creating a manager for log categories (see {`ball_categorymanager`}).  A
// `ball::Category` object maintains private state that is accessed and
// manipulated via this utility.  Each `ball::Category` contains private data
// members that provide:
//
// * A linked list of associated `ball::CategoryHolder` objects that refer to
//   the category.
// * A cache of the logging rules that apply to the category.
// * A cache of the maximum threshold associated with any rule that applies
//   to the category (this is the threshold at which a more complicated
//   evaluation of the logging rules and current `ball::AttributeContext` must
//   be performed).
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: Basic Use of `ball::Category`
/// - - - - - - - - - - - - - - - - - - - -
// The following example demonstrates creating a category and accessing its
// threshold information.
//
// Note that other components in the logging subsystem provide more user
// focused examples of using categories (see {`ball_loggermanager`},
// {`ball_administration`}, and {`ball_categorymanager`}).
//
// First we create a simple category, `example`, that has the record-level,
// trigger-level, and trigger-all thresholds set to OFF and the pass-level set
// to WARN, and verify these values:
// ```
// ball::Category example("example",
//                        ball::Severity::e_OFF,
//                        ball::Severity::e_WARN,
//                        ball::Severity::e_OFF,
//                        ball::Severity::e_OFF);
//
// assert(0 == bsl::strcmp("example", example.categoryName());
// assert(ball::Severity::e_OFF  == example.recordLevel());
// assert(ball::Severity::e_WARN == example.passLevel());
// assert(ball::Severity::e_OFF  == example.triggerLevel());
// assert(ball::Severity::e_OFF  == example.triggerAllLevel());
// ```
// See {`ball_loggermanager`} for more information on the use of various
// thresholds levels.
//
// Finally, we test if a the category is enabled for log record recorded with
// `e_ERROR` severity:
// ```
// if (example.isEnabled(ball::Severity::e_ERROR)) {
//     // publish record
// }
// ```

#include <balscm_version.h>

#include <ball_ruleset.h>
#include <ball_thresholdaggregate.h>

#include <bdlb_bitutil.h>

#include <bslma_allocator.h>
#include <bslma_usesbslmaallocator.h>

#include <bslmf_nestedtraitdeclaration.h>

#include <bslmt_lockguard.h>
#include <bslmt_mutex.h>

#include <bsls_assert.h>
#include <bsls_atomic.h>
#include <bsls_atomicoperations.h>
#include <bsls_keyword.h>
#include <bsls_review.h>
#include <bsls_types.h>

#include <bsl_string_view.h>

namespace BloombergLP {
namespace ball {

class CategoryHolder;

                           // ==============
                           // class Category
                           // ==============

/// This class provides a container to hold the name and threshold levels of
/// a category.  Instances of `Category` are created and manipulated by
/// `CategoryManager`.  All threshold levels are integral values in the
/// range `[0 .. 255]`.
///
/// Implementation Note: The `d_ruleThreshold` and `d_relevantRuleMask`
/// serve as a cache for logging rule evaluation (see
/// `ball_attributecontext`).  They are not meant to be modified by users
/// of the logging system, and may be modified by `const` operations of the
/// logging system.
class Category {

    // DATA
    bsls::AtomicUint      d_thresholdLevels;   // record, pass, trigger, and
                                               // trigger-all levels

    bsls::AtomicInt       d_threshold;         // numerical maximum of the four
                                               // levels

    const bsl::string     d_categoryName;      // category name

    CategoryHolder       *d_categoryHolder_p;  // linked list of holders of
                                               // this category

    mutable bsls::AtomicUint
                          d_relevantRuleMask;  // the mask indicating which
                                               // rules are relevant (i.e.,
                                               // have been attached to this
                                               // category)

    mutable bsls::AtomicInt d_ruleThreshold;   // numerical maximum of all four
                                               // levels for all relevant rules

    mutable bslmt::Mutex  d_mutex;             // mutex providing mutually
                                               // exclusive access to all
                                               // non-atomic members

    // FRIENDS
    friend class CategoryManagerImpUtil;

    // NOT IMPLEMENTED
    Category(const Category&);
    Category& operator=(const Category&);

  private:
    // PRIVATE MANIPULATORS

    /// Load this category and its corresponding `maxLevel()` into the
    /// specified `categoryHolder`, and add `categoryHolder` to the linked
    /// list of category holders managed by this category.
    void linkCategoryHolder(CategoryHolder *categoryHolder);

    /// Reset the category holders to which this category is linked to their
    /// default value.  See the function-level documentation of
    /// `CategoryHolder::reset` for further information on the default value
    /// of category holders.
    void resetCategoryHolders();

    /// Update the threshold of all category holders that hold the address
    /// of this object to the maximum of `d_threshold` and
    /// `d_ruleThreshold`.  The behavior is undefined unless `d_mutex` is
    /// locked.
    void updateThresholdForHolders();

  public:
    // CLASS METHODS

    /// Return `true` if each of the specified `recordLevel`, `passLevel`,
    /// `triggerLevel` and `triggerAllLevel` threshold values are in the
    /// range `[0 .. 255]`, and `false` otherwise.
    static bool areValidThresholdLevels(int recordLevel,
                                        int passLevel,
                                        int triggerLevel,
                                        int triggerAllLevel);

    // TRAITS
    BSLMF_NESTED_TRAIT_DECLARATION(Category, bslma::UsesBslmaAllocator);

    // CREATORS

    /// Create a category having the specified `categoryName` and the
    /// specified `recordLevel`, `passLevel`, `triggerLevel`, and
    /// `triggerAllLevel` threshold values, respectively.  Optionally
    /// specify a `basicAllocator` used to supply memory.  If
    /// `basicAllocator` is 0, the currently installed default allocator is
    /// used.  The behavior is undefined unless each of the specified
    /// threshold levels is in the range `[0 .. 255]`.
    Category(const bsl::string_view&  categoryName,
             int                      recordLevel,
             int                      passLevel,
             int                      triggerLevel,
             int                      triggerAllLevel,
             bslma::Allocator        *basicAllocator = 0);

    /// Destroy this object.
    //! ~Category() = default;

    // MANIPULATORS

    /// Set the threshold levels of this category to the specified
    /// `recordLevel`, `passLevel`, `triggerLevel`, and `triggerAllLevel`
    /// values, respectively, if each of the specified values is in the
    /// range `[0 .. 255]`.  Return 0 on success, and a non-zero value
    /// otherwise (with no effect on the threshold levels of this category).
    int setLevels(int recordLevel,
                  int passLevel,
                  int triggerLevel,
                  int triggerAllLevel);

    // ACCESSORS

    // BDE_VERIFY pragma: push
    // BDE_VERIFY pragma: -FABC01: Functions not in alphanumeric order

    /// Return the name of this category.
    const char *categoryName() const;

    /// Return `true` if logging at the specified `level` is enabled for
    /// this category, and `false` otherwise.  Logging is enabled if `level`
    /// is numerically less than or equal to any of the four threshold
    /// levels of this category.
    bool isEnabled(int level) const;

    /// Return the numerical maximum of the four levels of this category.
    int maxLevel() const;

    /// Return the record level of this category.
    int recordLevel() const;

    /// Return the pass level of this category.
    int passLevel() const;

    /// Return the trigger level of this category.
    int triggerLevel() const;

    /// Return the trigger-all level of this category.
    int triggerAllLevel() const;

    /// Return the aggregate threshold levels of this category.
    ThresholdAggregate thresholdLevels() const;

    /// Return the current maximum threshold (i.e., the lowest severity)
    /// between the `recordLevel`, `passLevel`, `triggerLevel`, and
    /// `triggerAllLevel`.  Note that this is the threshold at which a log
    /// record having this severity will need to be acted upon.
    int threshold() const;

    /// Return the current maximum threshold (i.e., the lowest severity) for
    /// any logging rule associated with this category.  Note that the rule
    /// having this threshold may not be active given the current thread's
    /// logging context (see `ball::AttributeContext`); this value caches
    /// the lowest possible severity where the currently rules need to be
    /// evaluated (log records below this threshold do not need any rule
    /// evaluation).
    int ruleThreshold() const;

    /// Return a reference to the non-modifiable relevant rule mask for this
    /// category.  The returned `RuleSet::MaskType` value is a bit-mask,
    /// where each bit is a boolean value indicating whether the rule at the
    /// corresponding index (in the rule set of the category manager that
    /// owns this category) applies at this category.  Note that a rule
    /// applies to this category if the rule's pattern matches the name
    /// returned by `categoryName`.
    RuleSet::MaskType relevantRuleMask() const;
    // BDE_VERIFY pragma: pop
};

                        // ====================
                        // class CategoryHolder
                        // ====================

/// This class, informally referred to as a "category holder" (or simply
/// "holder"), holds a category, a threshold level, and a pointer to a
/// "next" holder.  Both the category and next pointer may be null.  The
/// intended use is as follows: (1) instances of this class are (only)
/// declared in contexts where logging occurs; (2) if the held category is
/// non-null, then the held threshold is the numerical maximum of the four
/// levels of that category; (3) if the next pointer is non-null, then the
/// holder pointed to holds the same category and threshold.  Instances of
/// this class must be *statically* initializable.  Hence, the data members
/// are `public`, and automatically generated constructors and destructor
/// are used.
///
/// This class should *not* be used directly by client code.  It is an
/// implementation detail of the `ball` logging system.
class CategoryHolder {

    // NOT IMPLEMENTED
    CategoryHolder& operator=(const CategoryHolder&) BSLS_KEYWORD_DELETED;

    // PRIVATE TYPES
    typedef bsls::AtomicOperations                       AtomicOps;
    typedef bsls::AtomicOperations::AtomicTypes::Int     AtomicInt;
    typedef bsls::AtomicOperations::AtomicTypes::Pointer AtomicPointer;

  public:
    // PUBLIC TYPES

    /// This enumeration defines distinguished values for category holder
    /// threshold levels.  Note that these values are intentionally outside
    /// the range `[0 .. 255]`.
    enum {
        e_UNINITIALIZED_CATEGORY = 256, // indicates no logger manager
        e_DYNAMIC_CATEGORY       = 257  // corresponding category is dynamic
    };

    // PUBLIC DATA

    // BDE_VERIFY pragma: push
    // BDE_VERIFY pragma: -MN01 // Class data members must be private
    AtomicInt     d_threshold;   // threshold level
    AtomicPointer d_category_p;  // held category (not owned)
    AtomicPointer d_next_p;      // next category holder in linked list
    // BDE_VERIFY pragma: pop

    // CREATORS

    // No constructors or destructors are declared in order to allow for static
    // initialization of instances of this class.

    // MANIPULATORS
    // BDE_VERIFY pragma: push
    // BDE_VERIFY pragma: -FABC01: Functions not in alphanumeric order

    /// Reset this object to its default value.  The default value is:
    /// ```
    /// { e_UNINITIALIZED_CATEGORY, 0, 0 }
    /// ```
    void reset();

    /// Set the address of the category held by this holder to the specified
    /// `category`.
    void setCategory(const Category *category);

    /// Set the threshold level held by this holder to the specified
    /// `threshold`.
    void setThreshold(int threshold);

    /// Set this holder to point to the specified `holder`.
    void setNext(CategoryHolder *holder);

    // ACCESSORS

    /// Return the address of the non-modifiable category held by this
    /// holder.
    const Category *category() const;

    /// Return the threshold level held by this holder.
    int threshold() const;

    /// Return the address of the modifiable holder held by this holder.
    CategoryHolder *next() const;
    // BDE_VERIFY pragma: pop
};

                    // ============================
                    // class CategoryManagerImpUtil
                    // ============================

/// This class provides a suite of free functions used to help implement a
/// manager of categories and category holders.
///
/// This class should *not* be used directly by client code.  It is an
/// implementation detail of the `ball` logging system.
class CategoryManagerImpUtil {

  public:
    // CLASS METHODS

    // BDE_VERIFY pragma: push
    // BDE_VERIFY pragma: -FABC01: Functions not in alphanumeric order

    /// Load the specified `category` and its corresponding `maxLevel()`
    /// into the specified `categoryHolder`, and add `categoryHolder` to
    /// the linked list of category holders maintained by `category`.
    static void linkCategoryHolder(Category       *category,
                                   CategoryHolder *categoryHolder);

    /// Reset the category holders to which the specified `category` is
    /// linked to their default value.  See the function-level documentation
    /// of `CategoryHolder::reset` for further information on the default
    /// value of category holders.
    static void resetCategoryHolders(Category *category);

    /// Update the threshold of all category holders that hold the address
    /// of the specified `category` object to the maximum of `d_threshold`
    /// and `d_ruleThreshold`.
    static void updateThresholdForHolders(Category *category);

    /// Set the cached rule threshold for the specified `category` to the
    /// specified `ruleThreshold`.
    static void setRuleThreshold(Category *category, int ruleThreshold);

    /// Set the bit in the relevant rule-mask at the specified `ruleIndex`
    /// in the specified `category` to `true`.
    static void enableRule(Category *category, int ruleIndex);

    /// Set the bit in the rule-mask at the specified `ruleIndex` in the
    /// specified `category` to `false`.
    static void disableRule(Category *category, int ruleIndex);

    /// Set the rule-mask for the specified `category` to the specified
    /// `mask`.
    static void setRelevantRuleMask(Category          *category,
                                    RuleSet::MaskType  mask);
    // BDE_VERIFY pragma: pop
};

// ============================================================================
//                              INLINE DEFINITIONS
// ============================================================================

                        // --------------
                        // class Category
                        // --------------

// BDE_VERIFY pragma: push
// BDE_VERIFY pragma: -FABC01: Functions not in alphanumeric order

// CLASS METHODS
inline
bool Category::areValidThresholdLevels(int recordLevel,
                                       int passLevel,
                                       int triggerLevel,
                                       int triggerAllLevel)
{
    enum { k_BITS_PER_CHAR = 8 };

    return !((recordLevel | passLevel | triggerLevel | triggerAllLevel)
             >> k_BITS_PER_CHAR);
}

// ACCESSORS
inline
const char *Category::categoryName() const
{
    return d_categoryName.c_str();
}

inline
bool Category::isEnabled(int level) const
{
    return d_threshold >= level;
}

inline
int Category::maxLevel() const
{
    return d_threshold;
}

inline
int Category::recordLevel() const
{
    return ThresholdAggregateUtil::unpack(d_thresholdLevels.loadAcquire())
            .recordLevel();
}

inline
int Category::passLevel() const
{
    return ThresholdAggregateUtil::unpack(d_thresholdLevels.loadAcquire())
            .passLevel();
}

inline
int Category::triggerLevel() const
{
    return ThresholdAggregateUtil::unpack(d_thresholdLevels.loadAcquire())
            .triggerLevel();
}

inline
int Category::triggerAllLevel() const
{
    return ThresholdAggregateUtil::unpack(d_thresholdLevels.loadAcquire())
            .triggerAllLevel();
}

inline
ThresholdAggregate Category::thresholdLevels() const
{
    return ThresholdAggregateUtil::unpack(d_thresholdLevels.loadAcquire());
}

inline
int Category::threshold() const
{
    return d_threshold;
}

inline
int Category::ruleThreshold() const
{
    return d_ruleThreshold.loadAcquire();
}

inline
RuleSet::MaskType Category::relevantRuleMask() const
{
    return d_relevantRuleMask.loadAcquire();
}

                        // --------------------
                        // class CategoryHolder
                        // --------------------

// MANIPULATORS
inline
void CategoryHolder::setCategory(const Category *category)
{
    AtomicOps::setPtrRelease(&d_category_p, const_cast<Category *>(category));
}

inline
void CategoryHolder::setThreshold(int threshold)
{
    AtomicOps::setIntRelaxed(&d_threshold, threshold);
}

inline
void CategoryHolder::setNext(CategoryHolder *holder)
{
    AtomicOps::setPtrRelease(&d_next_p, holder);
}

// ACCESSORS
inline
const Category *CategoryHolder::category() const
{
    return reinterpret_cast<const Category *>(
        AtomicOps::getPtrAcquire(&d_category_p));
}

inline
int CategoryHolder::threshold() const
{
    return AtomicOps::getIntRelaxed(&d_threshold);
}

inline
CategoryHolder *CategoryHolder::next() const
{
    return reinterpret_cast<CategoryHolder *>(
        AtomicOps::getPtrAcquire(&d_next_p));
}

                    // ----------------------------
                    // class CategoryManagerImpUtil
                    // ----------------------------

// CLASS METHODS
inline
void CategoryManagerImpUtil::linkCategoryHolder(Category       *category,
                                                CategoryHolder *categoryHolder)
{
    BSLS_ASSERT(category);
    BSLS_ASSERT(categoryHolder);

    category->linkCategoryHolder(categoryHolder);
}

inline
void CategoryManagerImpUtil::resetCategoryHolders(Category *category)
{
    BSLS_ASSERT(category);

    category->resetCategoryHolders();
}

inline
void CategoryManagerImpUtil::updateThresholdForHolders(Category *category)
{
    BSLS_ASSERT(category);

    bslmt::LockGuard<bslmt::Mutex> guard(&category->d_mutex);
    category->updateThresholdForHolders();
}

inline
void CategoryManagerImpUtil::setRuleThreshold(Category *category,
                                              int       ruleThreshold)
{
    bslmt::LockGuard<bslmt::Mutex> guard(&category->d_mutex);
    category->d_ruleThreshold.storeRelease(ruleThreshold);
}

inline
void CategoryManagerImpUtil::enableRule(Category *category, int ruleIndex)
{
    unsigned int currentMask = category->d_relevantRuleMask.loadRelaxed();
    unsigned int expectedMask;
    do {
        const unsigned int updatedMask = bdlb::BitUtil::withBitSet(currentMask,
                                                                   ruleIndex);
        expectedMask  = currentMask;
        currentMask   = category->d_relevantRuleMask.testAndSwapAcqRel(
                                                                  currentMask,
                                                                  updatedMask);
    } while (expectedMask != currentMask);
}

inline
void CategoryManagerImpUtil::disableRule(Category *category, int ruleIndex)
{
    unsigned int currentMask = category->d_relevantRuleMask.loadRelaxed();
    unsigned int expectedMask;
    do {
        const unsigned int updatedMask =
                         bdlb::BitUtil::withBitCleared(currentMask, ruleIndex);
        expectedMask = currentMask;
        currentMask  = category->d_relevantRuleMask.testAndSwapAcqRel(
                                                                  currentMask,
                                                                  updatedMask);
    } while (expectedMask != currentMask);
}

inline
void CategoryManagerImpUtil::setRelevantRuleMask(Category          *category,
                                                 RuleSet::MaskType  mask)
{
    category->d_relevantRuleMask.storeRelease(mask);
}

// BDE_VERIFY pragma: pop

}  // close package namespace
}  // close enterprise namespace

#endif

// ----------------------------------------------------------------------------
// Copyright 2015 Bloomberg Finance L.P.
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
