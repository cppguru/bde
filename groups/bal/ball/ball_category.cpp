// ball_category.cpp                                                  -*-C++-*-
#include <ball_category.h>

#include <bsls_ident.h>
BSLS_IDENT_RCSID(ball_category_cpp,"$Id$ $CSID$")

#include <ball_severity.h>    // for testing only

#include <bslim_printer.h>

#include <bslmf_assert.h>
#include <bslmf_issame.h>

#include <bslmt_mutexassert.h>

#include <bsls_assert.h>

#include <bsl_algorithm.h>
#include <bsl_ostream.h>

namespace BloombergLP {
namespace ball {

                            // --------------
                            // class Category
                            // --------------

// `d_relevantRuleMask` is semantically of type `RuleSet::MaskType`, but needs
// to be atomic.  Assert that the type of `RuleSet::MaskType` hasn't changed.
BSLMF_ASSERT((bsl::is_same<RuleSet::MaskType, unsigned int>::value));

// PRIVATE CREATORS

/// Note that this constructor is private, so the validation of the
/// threshold level values does not need to be repeated here.  They are
/// validated in `CategoryManager::addCategory`, prior to creating an
/// instance of this class.
Category::Category(const bsl::string_view&  categoryName,
                   int                      recordLevel,
                   int                      passLevel,
                   int                      triggerLevel,
                   int                      triggerAllLevel,
                   bslma::Allocator        *basicAllocator)
: d_thresholdLevels(ThresholdAggregateUtil::pack(
    ThresholdAggregate(recordLevel, passLevel, triggerLevel, triggerAllLevel)))
, d_threshold(ThresholdAggregate::maxLevel(recordLevel,
                                           passLevel,
                                           triggerLevel,
                                           triggerAllLevel))
, d_categoryName(categoryName, basicAllocator)
, d_categoryHolder_p(0)
, d_relevantRuleMask(0)
, d_ruleThreshold(0)
, d_mutex()
{
}

// PRIVATE MANIPULATORS
void
Category::linkCategoryHolder(CategoryHolder *categoryHolder)
{
    BSLS_ASSERT(categoryHolder);

    bslmt::LockGuard<bslmt::Mutex> guard(&d_mutex);
    if (!categoryHolder->category()) {
        categoryHolder->setThreshold(bsl::max(d_threshold.loadRelaxed(),
                                              d_ruleThreshold.loadRelaxed()));
        categoryHolder->setCategory(this);
        categoryHolder->setNext(d_categoryHolder_p);
        d_categoryHolder_p = categoryHolder;
    }
}

void Category::resetCategoryHolders()
{
    bslmt::LockGuard<bslmt::Mutex>  guard(&d_mutex);

    CategoryHolder *holder = d_categoryHolder_p;

    while (holder) {
        CategoryHolder *nextHolder = holder->next();
        holder->reset();
        holder = nextHolder;
    }
    d_categoryHolder_p = 0;
}

// CLASS METHODS
void Category::updateThresholdForHolders()
{
    BSLMT_MUTEXASSERT_IS_LOCKED(&d_mutex);

    if (d_categoryHolder_p) {
        CategoryHolder *holder = d_categoryHolder_p;
        const int       threshold = bsl::max(d_threshold.loadRelaxed(),
                                             d_ruleThreshold.loadRelaxed());
        if (threshold != holder->threshold()) {
            do {
                holder->setThreshold(threshold);
                holder = holder->next();
            } while (holder);
        }
    }
}

// MANIPULATORS
int Category::setLevels(int recordLevel,
                        int passLevel,
                        int triggerLevel,
                        int triggerAllLevel)
{
    if (Category::areValidThresholdLevels(recordLevel,
                                          passLevel,
                                          triggerLevel,
                                          triggerAllLevel)) {
        bslmt::LockGuard<bslmt::Mutex> guard(&d_mutex);

        d_thresholdLevels = ThresholdAggregateUtil::pack(
                ThresholdAggregate(recordLevel,
                                   passLevel,
                                   triggerLevel,
                                   triggerAllLevel));

        d_threshold = ThresholdAggregate::maxLevel(recordLevel,
                                                   passLevel,
                                                   triggerLevel,
                                                   triggerAllLevel);

        updateThresholdForHolders();
        return 0;                                                     // RETURN
    }

    return -1;
}

                        // --------------------
                        // class CategoryHolder
                        // --------------------

// MANIPULATORS
void CategoryHolder::reset()
{
    AtomicOps::setIntRelaxed(&d_threshold, e_UNINITIALIZED_CATEGORY);
    AtomicOps::setPtrRelease(&d_category_p, 0);
    AtomicOps::setPtrRelease(&d_next_p, 0);
}

}  // close package namespace
}  // close enterprise namespace

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
