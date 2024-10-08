// bslmt_meteredmutex.h                                               -*-C++-*-
#ifndef INCLUDED_BSLMT_METEREDMUTEX
#define INCLUDED_BSLMT_METEREDMUTEX

#include <bsls_ident.h>
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide a mutex capable of keeping track of wait and hold time.
//
//@CLASSES:
// bslmt::MeteredMutex: mutex capable of keeping track of wait and hold time
//
//@SEE_ALSO:
//
//@DESCRIPTION: This component provides a class, `bslmt::MeteredMutex`, that
// functions as a mutex and has additional capability to keep track of wait
// time and hold time.  This class can be used, for example, in evaluating the
// performance of an application, based on its lock contention behavior.
//
///Precise Definitions of Wait and Hold Time
///-----------------------------------------
// Wait time is defined as the sum of the time intervals between each call to
// `lock` (or `tryLock`) on the underlying mutex and the return of that call.
// Note that if one or more threads are waiting for the lock at the point when
// `waitTime` is called, those waiting time intervals are *not* included in the
// returned wait time.  Hold time is defined as the sum of the time intervals
// between return from each call to `lock` (or a successful call to `tryLock`)
// on the underlying mutex and the subsequent call to `unlock`.  Note that if a
// thread is holding the lock at the point when `holdTime` is called, then that
// holding time is *not* included in the returned hold time.
//
///Performance
///-----------
// It should be noted that the overhead in keeping track of wait and hold time
// is very small.  We do not use additional mutexes to manipulate these times,
// instead, we use atomic data types (which have very small overhead compared
// to a mutex) to update these times atomically.
//
///Inaccuracy of `waitTime` and `holdTime`
///---------------------------------------
// Times reported by `waitTime` and `holdTime` are (close) approximate times
// and *not* 100% accurate.  This inaccuracy can sometime cause surprising
// behavior.  For example, one can incorrectly assume `lock()` and
// `while (tryLock() != 0);` to be effectively the same (both disallowing the
// thread to advance until the lock is acquired) but the wait time reported in
// the first case can be much more accurate than that of the second because the
// `lock` is called only once (and thus computation error is introduced only
// once) in the first case.
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: Basic Usage
/// - - - - - - - - - - -
// In the following example, we have `NUM_THREADS` threads (that are
// sequentially numbered from `0` to `NUM_THREADS-1`) and two counters
// `evenCount` and `oddCount`.  `evenCount` is incremented by the even numbered
// threads and `oddCount` is incremented by the odd ones.  We considers two
// strategies to increment these counters.  In the first strategy (strategy1),
// we use two mutexes (one for each counter) and in the second strategy
// (strategy2), we use a single mutex for both counters.
// ```
// int oddCount = 0;
// int evenCount = 0;
//
// typedef bslmt::MeteredMutex Obj;
// Obj oddMutex;
// Obj evenMutex;
// Obj globalMutex;
//
// enum { k_USAGE_NUM_THREADS = 4, k_USAGE_SLEEP_TIME = 100000 };
// bslmt::Barrier usageBarrier(k_USAGE_NUM_THREADS);
//
// /// Create the specified `numThreads`, each executing the specified
// /// `function`.  Number each thread (sequentially from 0 to
// /// `numThreads - 1`) by passing i to i'th thread.  Finally join all the
// /// threads.
// void executeInParallel(int                               numThreads,
//                        bslmt::ThreadUtil::ThreadFunction function)
// {
//     bslmt::ThreadUtil::Handle *threads =
//                                  new bslmt::ThreadUtil::Handle[numThreads];
//     assert(threads);
//
//     for (int i = 0; i < numThreads; ++i) {
//         bslmt::ThreadUtil::create(&threads[i], function, (void*)i);
//     }
//     for (int i = 0; i < numThreads; ++i) {
//         bslmt::ThreadUtil::join(threads[i]);
//     }
//
//     delete [] threads;
// }
//
// extern "C" {
//     void *strategy1(void *arg)
//     {
//         usageBarrier.wait();
//         int remainder = (int)(bsls::Types::IntPtr)arg % 2;
//         if (remainder == 1) {
//             oddMutex.lock();
//             ++oddCount;
//             bslmt::ThreadUtil::microSleep(k_USAGE_SLEEP_TIME);
//             oddMutex.unlock();
//         }
//         else {
//             evenMutex.lock();
//             ++evenCount;
//             bslmt::ThreadUtil::microSleep(k_USAGE_SLEEP_TIME);
//             evenMutex.unlock();
//         }
//         return NULL;
//     }
// } // extern "C"
//
// extern "C" {
//     void *strategy2(void *arg)
//     {
//         usageBarrier.wait();
//         int remainder = (int)(bsls::Types::IntPtr)arg % 2;
//         if (remainder == 1) {
//             globalMutex.lock();
//             ++oddCount;
//             bslmt::ThreadUtil::microSleep(k_USAGE_SLEEP_TIME);
//             globalMutex.unlock();
//         }
//         else {
//             globalMutex.lock();
//             ++evenCount;
//             bslmt::ThreadUtil::microSleep(k_USAGE_SLEEP_TIME);
//             globalMutex.unlock();
//         }
//         return NULL;
//     }
// } // extern "C"
// ```
// Then in the application `main`:
// ```
// executeInParallel(k_USAGE_NUM_THREADS, strategy1);
// bsls::Types::Int64 waitTimeForStrategy1 =
//                                 oddMutex.waitTime() + evenMutex.waitTime();
//
// executeInParallel(k_USAGE_NUM_THREADS, strategy2);
// bsls::Types::Int64 waitTimeForStrategy2 = globalMutex.waitTime();
//
// assert(waitTimeForStrategy2 > waitTimeForStrategy1);
// if (veryVerbose) {
//     P(waitTimeForStrategy1);
//     P(waitTimeForStrategy2);
// }
// ```
// We measured the wait times for each strategy.  Intuitively, the wait time
// for the second strategy should be greater than that of the first.  The
// output was consistent with our expectation.
// ```
// waitTimeForStrategy1 = 400787000
// waitTimeForStrategy2 = 880765000
// ```

#include <bslscm_version.h>

#include <bslmt_mutex.h>

#include <bsls_atomic.h>
#include <bsls_timeutil.h>
#include <bsls_types.h>

namespace BloombergLP {
namespace bslmt {

                            // ==================
                            // class MeteredMutex
                            // ==================

/// This class implements a mutex, that has the additional capability to
/// keep track of hold time and wait time.  The hold time is defined as the
/// cumulative duration for which the mutex was in the locked state.  The
/// wait time is defined as the duration for which threads waited for the
/// mutex.
class MeteredMutex {

    // DATA
    Mutex               d_mutex;          // underlying mutex
    bsls::AtomicInt64   d_waitTime;       // wait time
    bsls::AtomicInt64   d_holdTime;       // hold time
    bsls::Types::Int64  d_startHoldTime;  // starting point of hold time
    bsls::AtomicInt64   d_lastResetTime;  // last reset time

    // NOT IMPLEMENTED
    MeteredMutex(const MeteredMutex&);
    MeteredMutex& operator=(const MeteredMutex&);

  public:
    // CREATORS

    /// Create a metered mutex in the unlocked state.
    MeteredMutex();

    /// Destroy this metered mutex.
    ~MeteredMutex();

    // MANIPULATORS

    /// Acquire the lock on this metered mutex.  If this mutex is currently
    /// locked, suspend the execution of the current thread until the lock
    /// can be acquired.  Update the wait and hold time appropriately.  The
    /// behavior is undefined if the calling thread already owns the lock.
    void lock();

    /// Reset the wait and hold time to zero and record the current time.
    /// All subsequent calls (that are made before a subsequent call to
    /// `resetMetrics`) to `waitTime` (or `holdTime`) will return the wait
    /// (or hold) time, accumulated since this call.  Also, all subsequent
    /// calls (that are made before a subsequent call to `resetMetrics`) to
    /// `lastResetTime` will return the time of this call.
    void resetMetrics();

    /// Attempt to acquire the lock on this metered mutex.  Return 0 on
    /// success, and a non-zero value if this mutex is already locked, or if
    /// an error occurs.  Update the wait and hold time appropriately.  The
    /// behavior is undefined if the calling thread already owns the lock.
    int tryLock();

    /// Release the lock on this mutex that was previously acquired through
    /// a successful call to `lock` or `tryLock`.  Update the hold time
    /// appropriately.  The behavior is undefined unless the calling thread
    /// currently owns the lock.
    void unlock();

    // ACCESSORS

    /// Return the hold time (in nanoseconds) accumulated since the most
    /// recent call to `resetMetrics` (or `MeteredMutex` if `resetMetrics`
    /// was never called).
    bsls::Types::Int64 holdTime() const;

    /// Return the time in nanoseconds (referenced to an arbitrary but fixed
    /// origin) of the most recent invocation to `resetMetrics` (or creation
    /// time if `resetMetrics` was never invoked).  User can calculate the
    /// difference (in nanoseconds) between the current time and the last
    /// reset time by expression
    /// `bsls::TimeUtil::getTimer() - clientMutex.lastResetTime()`.
    bsls::Types::Int64 lastResetTime() const;

    /// Return the wait time (in nanoseconds), accumulated since the most
    /// recent call to `resetMetrics` (or `MeteredMutex` if `resetMetrics`
    /// was never called).
    bsls::Types::Int64 waitTime() const;
};

// ============================================================================
//                             INLINE DEFINITIONS
// ============================================================================

                                // ------------
                                // MeteredMutex
                                // ------------
// CREATORS
inline
MeteredMutex::MeteredMutex()
: d_lastResetTime(bsls::TimeUtil::getTimer())
{
}

inline
MeteredMutex::~MeteredMutex()
{
}

// MANIPULATORS
inline
void MeteredMutex::lock()
{
    bsls::Types::Int64 t1 = bsls::TimeUtil::getTimer();
    d_mutex.lock();
    d_startHoldTime = bsls::TimeUtil::getTimer();
    d_waitTime += (d_startHoldTime - t1);
}

inline
int MeteredMutex::tryLock()
{
    bsls::Types::Int64 t1 = bsls::TimeUtil::getTimer();
    int returnStatus = d_mutex.tryLock();
    bsls::Types::Int64 t2 = bsls::TimeUtil::getTimer();
    d_waitTime += t2 - t1;
    if (returnStatus == 0) {
        d_startHoldTime = t2;
    }
    return returnStatus;
}

inline
void MeteredMutex::unlock()
{
    d_holdTime += (bsls::TimeUtil::getTimer() - d_startHoldTime);
    d_mutex.unlock();
}

// ACCESSORS
inline
bsls::Types::Int64 MeteredMutex::holdTime() const
{
    return d_holdTime;
}

inline
bsls::Types::Int64 MeteredMutex::lastResetTime() const
{
    return d_lastResetTime;
}

inline
bsls::Types::Int64 MeteredMutex::waitTime() const
{
    return d_waitTime;
}

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
