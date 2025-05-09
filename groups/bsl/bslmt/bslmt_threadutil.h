// bslmt_threadutil.h                                                 -*-C++-*-
#ifndef INCLUDED_BSLMT_THREADUTIL
#define INCLUDED_BSLMT_THREADUTIL

#include <bsls_ident.h>
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide platform-independent utilities related to threading.
//
//@CLASSES:
//  bslmt::ThreadUtil: namespace for portable thread management utilities
//
//@SEE_ALSO: bslmt_threadattributes, bslmt_configuration
//
//@DESCRIPTION: This component defines a utility `struct`, `bslmt::ThreadUtil`,
// that serves as a name space for a suite of pure functions to create threads,
// join them (make one thread block and wait for another thread to exit),
// manipulate thread handles, manipulate the current thread, and (on some
// platforms) access thread-local storage.
//
///Creating a Simple Thread with Default Attributes
///------------------------------------------------
// Clients call `bslmt::ThreadUtil::create()` to create threads.  Threads may
// be started using a "C" linkage function pointer (of a type defined by
// `bslmt::ThreadUtil::ThreadFunction`) and a `void` pointer to `userData` to
// be passed to the function; or an "invokable" object of parameterized type
// (any copy-constructible type on which `operator()` may be invoked).  The
// invoked function becomes the main driver for the new thread; when it
// returns, the thread terminates.
//
///Thread Identity
///---------------
// A thread is identified by an object of the opaque type
// `bslmt::ThreadUtil::Handle`.  A handle of this type is returned when a
// thread is created (using `bslmt::ThreadUtil::create`).  A client can also
// retrieve a `Handle` for the "current" thread via the `self` method:
// ```
// bslmt::ThreadUtil::Handle myHandle = bslmt::ThreadUtil::self();
// ```
// Several thread manipulation functions in `bslmt::ThreadUtil` take a thread
// handle, or pointer to a thread handle, as an argument.  To facilitate
// compatibility with existing systems and allow for non-portable operations,
// clients also have access to the `bslmt::ThreadUtil::NativeHandle` type,
// which exposes the underlying, platform-specific thread identifier type:
// ```
// bslmt::ThreadUtil::NativeHandle myNativeHandle;
// myNativeHandle = bslmt::ThreadUtil::nativeHandle();
// ```
// Note that the returned native handle may not be a globally unique identifier
// for the thread, and, e.g., should not be converted to an integer identifier,
// or used as a key in a map.
//
///Setting Thread Priorities
///-------------------------
// `bslmt::ThreadUtil` allows clients to configure the priority of newly
// created threads by setting the `inheritSchedule`, `schedulingPolicy`, and
// `schedulingPriority` of a thread attributes object supplied to the `create`
// method.  The range of legal values for `schedulingPriority` depends on both
// the platform and the value of `schedulingPolicy`, and can be obtained from
// the `getMinSchedulingPriority` and `getMaxSchedulingPriority` methods.  Both
// `schedulingPolicy` and `schedulingPriority` are ignored unless
// `inheritSchedule` is `false` (the default value is `true`).  Note that not
// only is effective setting of thread priorities workable on only some
// combinations of platforms and user privileges, but setting the thread policy
// and priority appropriately for one platform may cause thread creation to
// fail on another platform.  Also note that an unset thread priority may be
// interpreted as being outside the valid range defined by
// `[ getMinSchedulingPriority(policy), getMaxSchedulingPriority(policy) ]`.
// ```
// Platform      Restrictions
// ------------  --------------------------------------------------------------
// Solaris 5.10  None.
//
// Solaris 5.11  Spawning of threads fails if `schedulingPolicy` is
//               `BSLMT_SCHED_FIFO` or `BSLMT_SCHED_RR`.  Thread priorities
//               should not be used on Solaris 5.11 as it is not clear that
//               they have any detectable effect.  Note that
//               `getMinSchedulingPriority` and `getMaxSchedulingPriority`
//               return different values than on Solaris 5.10.
//
// AIX           For non-privileged clients, spawning of threads fails if
//               `schedulingPolicy` is `BSLMT_SCHED_FIFO` or `BSLMT_SCHED_RR`.
//
// Linux         Non-privileged clients *can* *not* make effective use of
//               thread priorities -- spawning of threads fails if
//               `schedulingPolicy` is `BSLMT_SCHED_FIFO` or `BSLMT_SCHED_RR`,
//               and `getMinSchedulingPriority == getMaxSchedulingPriority` if
//               the policy has any other value.
//
// Darwin        Non-privileged clients *can* *not* make effective use of
//               thread priorities -- there is no observable difference in
//               urgency between high priority and low priority threads.
//               Spawning of threads does succeed, however, for all scheduling
//               policies.
//
// Windows       Clients *can* *not* make effective use of thread priorities --
//               `schedulingPolicy`, `schedulingPriority`, and
//               `inheritSchedule` are ignored for all clients.
// ```
//
///Supported Clock-Types
///---------------------
// `bsls::SystemClockType` supplies the enumeration indicating the system clock
// on which timeouts supplied to other methods should be based.  If the clock
// type indicated at construction is `bsls::SystemClockType::e_REALTIME`, the
// `absTime` argument passed to the `timedWait` method of the various
// synchronization primitives offered in `bslmt` should be expressed as an
// *absolute* offset since 00:00:00 UTC, January 1, 1970 (which matches the
// epoch used in `bsls::SystemTime::now(bsls::SystemClockType::e_REALTIME)`.
// If the clock type indicated at construction is
// `bsls::SystemClockType::e_MONOTONIC`, the `absTime` argument passed to the
// `timedWait` method of the various synchronization primitives offered in
// `bslmt` should be expressed as an *absolute* offset since the epoch of this
// clock (which matches the epoch used in
// `bsls::SystemTime::now(bsls::SystemClockType::e_MONOTONIC)`.
//
///Usage
///-----
// This section illustrates the intended use of this component.
//
///Example 1: Creating a Simple Thread with Default Attributes
///- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// In this example, we create a thread using the default attribute settings.
// Upon creation, the thread executes the user-supplied C-linkage function
// `myThreadFunction` that counts 5 seconds before terminating:
//
// First, we create a function that will run in the spawned thread:
// ```
// /// Print to standard output "Another second has passed" every second
// /// for five seconds, and return 0.
// extern "C" void *myThreadFunction(void *)
// {
//     for (int i = 0; i < 3; ++i) {
//         bslmt::ThreadUtil::microSleep(0, 1);
//         bsl::cout << "Another second has passed." << bsl::endl;
//     }
//     return 0;
// }
// ```
// Now, we show how to create and join the thread.
//
// After creating the thread, the `main` routine *joins* the thread, which, in
// effect, causes `main` to wait for execution of `myThreadFunction` to
// complete, and guarantees that the output from `main` will follow the last
// output from the user-supplied function:
// ```
// int main()
// {
//     bslmt::Configuration::setDefaultThreadStackSize(
//                 bslmt::Configuration::recommendedDefaultThreadStackSize());
//
//     bslmt::ThreadUtil::Handle handle;
//
//     bslmt::ThreadAttributes attr;
//     attr.setStackSize(1024 * 1024);
//
//     int rc = bslmt::ThreadUtil::create(&handle, attr, myThreadFunction, 0);
//     assert(0 == rc);
//
//     bslmt::ThreadUtil::yield();
//
//     rc = bslmt::ThreadUtil::join(handle);
//     assert(0 == rc);
//
//     bsl::cout << "A three second interval has elapsed\n";
//
//     return 0;
// }
// ```
// Finally, the output of this program is:
// ```
// Another second has passed.
// Another second has passed.
// Another second has passed.
// A three second interval has elapsed.
// ```
//
///Example 2: Creating a Simple Thread with User-Specified Attributes
/// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// In this example, we will choose to override the default thread attribute
// values.
//
// The attributes of a thread can be specified explicitly by supplying a
// `bslmt::ThreadAttributes` object to the `create` method.  For instance, we
// could specify a smaller stack size for a thread to conserve system resources
// if we know that we will require not require the platform's default stack
// size.
//
// First, we define our thread function, noting that it doesn't need much stack
// space:
// ```
// /// Initialize a small object on the stack and do some work.
// extern "C" void *mySmallStackThreadFunction(void *threadArg)
// {
//     char *initValue = (char *)threadArg;
//     char Small[8];
//     bsl::memset(&Small[0], *initValue, 8);
//     // do some work ...
//     return 0;
// }
// ```
// Finally, we show how to create a detached thread running the function just
// created with a small stack size:
// ```
// /// Create a detached thread with a small stack size and perform some work.
// void createSmallStackSizeThread()
// {
//     enum { k_STACK_SIZE = 16384 };
//     bslmt::ThreadAttributes attributes;
//     attributes.setDetachedState(
//                            bslmt::ThreadAttributes::e_CREATE_DETACHED);
//     attributes.setStackSize(k_STACK_SIZE);
//
//     char initValue = 1;
//     bslmt::ThreadUtil::Handle handle;
//     int status = bslmt::ThreadUtil::create(&handle,
//                                           attributes,
//                                           mySmallStackThreadFunction,
//                                           &initValue);
// }
// ```
//
///Example 3: Setting Thread Priorities
/// - - - - - - - - - - - - - - - - - -
// In this example we demonstrate creating 3 threads with different priorities.
// We use the `convertToSchedulingPriority` function to translate a normalized,
// floating-point priority in the range `[ 0.0, 1.0 ]` to an integer priority
// in the range `[ getMinSchedulingPriority, getMaxSchedulingPriority ]` to set
// the `schedulingPriority` attribute.
// ```
// /// Create 3 threads with different priorities and then wait for them
// /// all to finish.
// void runSeveralThreads()
// {
//     enum { k_NUM_THREADS = 3 };
//
//     bslmt::ThreadUtil::Handle handles[k_NUM_THREADS];
//     bslmt_ThreadFunction functions[k_NUM_THREADS] = {
//                                               MostUrgentThreadFunctor,
//                                               FairlyUrgentThreadFunctor,
//                                               LeastUrgentThreadFunctor };
//     double priorities[k_NUM_THREADS] = { 1.0, 0.5, 0.0 };
//
//     bslmt::ThreadAttributes attributes;
//     attributes.setInheritSchedule(false);
//     const bslmt::ThreadAttributes::SchedulingPolicy policy =
//                                 bslmt::ThreadAttributes::e_SCHED_OTHER;
//     attributes.setSchedulingPolicy(policy);
//
//     for (int i = 0; i < k_NUM_THREADS; ++i) {
//         attributes.setSchedulingPriority(
//              bslmt::ThreadUtil::convertToSchedulingPriority(policy,
//                                                            priorities[i]));
//         int rc = bslmt::ThreadUtil::create(&handles[i],
//                                            attributes,
//                                            functions[i], 0);
//         assert(0 == rc);
//     }
//
//     for (int i = 0; i < k_NUM_THREADS; ++i) {
//         int rc = bslmt::ThreadUtil::join(handles[i]);
//         assert(0 == rc);
//     }
// }
// ```

#include <bslscm_version.h>

#include <bslmt_entrypointfunctoradapter.h>
#include <bslmt_platform.h>
#include <bslmt_threadattributes.h>
#include <bslmt_threadutilimpl_pthread.h>
#include <bslmt_threadutilimpl_win32.h>

#include <bsla_maybeunused.h>

#include <bslma_allocator.h>
#include <bslma_default.h>

#include <bsls_assert.h>
#include <bsls_libraryfeatures.h>
#include <bsls_systemclocktype.h>
#include <bsls_systemtime.h>
#include <bsls_timeinterval.h>
#include <bsls_types.h>

#include <bsl_string.h>

#ifdef BSLS_LIBRARYFEATURES_HAS_CPP11_BASELINE_LIBRARY
#include <bslmt_chronoutil.h>

#include <bsl_chrono.h>
#endif

namespace BloombergLP {

extern "C" {
    /// `bslmt_ThreadFunction` is an alias for a function type taking a
    /// single `void` pointer argument and returning `void *`.  Such
    /// functions are suitable to be specified as thread entry-point
    /// functions to `bslmt::ThreadUtil::create`.  Note that `create` also
    /// accepts any invokable C++ "functor" object.
    typedef void *(*bslmt_ThreadFunction)(void *);

    /// `bslmt_KeyDestructorFunction` is an alias for a function type taking
    /// a single `void` pointer argument and returning `void`.  Such
    /// functions are suitable to be specified as thread-specific key
    /// destructor functions to `bslmt::ThreadUtil::createKey`.
    typedef void (*bslmt_KeyDestructorFunction)(void *);
}  // extern "C"

namespace bslmt {

template <class THREAD_POLICY>
struct ThreadUtilImpl;

                            // =================
                            // struct ThreadUtil
                            // =================

/// This `struct` provides a suite of portable utility functions for
/// managing threads.
struct ThreadUtil {

  public:
    // PUBLIC TYPES

    /// Platform-specific implementation type.
    typedef ThreadUtilImpl<Platform::ThreadPolicy> Imp;

    /// Thread handle type.  Use this type to refer to a thread in a
    /// platform-independent way.
    typedef Imp::Handle                            Handle;

    /// Platform-specific thread handle type.
    typedef Imp::NativeHandle                      NativeHandle;

    /// Thread identifier type - distinguished from a `Handle` in that it
    /// does not have any resources associated with it, whereas `Handle`
    /// may, depending on platform.
    typedef Imp::Id                                Id;

    /// Prototype for thread entry-point functions.
    typedef bslmt_ThreadFunction                   ThreadFunction;

    /// Thread-specific key type, used to refer to thread-specific storage.
    typedef Imp::Key                               Key;

    /// Prototype for thread-specific key destructors.
    typedef bslmt_KeyDestructorFunction            Destructor;

  public:
    // PUBLIC CLASS METHODS
                // *** Thread Management ***

    /// Return an integer scheduling priority appropriate for the specified
    /// `normalizedSchedulingPriority` and the specified `policy`.  If
    /// either the minimum or maximum priority for this platform cannot be
    /// determined, return `ThreadAttributes::e_UNSET_PRIORITY`.  Higher
    /// values of `normalizedSchedulingPriority` are considered to represent
    /// more urgent priorities.  The behavior is undefined unless `policy`
    /// is a valid `ThreadAttributes::SchedulingPolicy` and
    /// `normalizedSchedulingPriority` is in the range `[ 0.0, 1.0 ]`.
    static int convertToSchedulingPriority(
              ThreadAttributes::SchedulingPolicy policy,
              double                             normalizedSchedulingPriority);

    /// Create a new thread of program control whose entry point will be the
    /// specified `function`, and which will be passed the specified
    /// `userData` as its sole argument, and load into the specified
    /// `handle` an identifier that may be used to refer to this thread in
    /// calls to other `ThreadUtil` methods.  Optionally specify
    /// `attributes` describing the properties for the new thread.  If
    /// `attributes` is not supplied, a default `ThreadAttributes` object is
    /// used.  Use the global allocator to supply memory.  Return 0 on
    /// success, and a non-zero value otherwise.  `bslmt::Configuration` is
    /// used to determine the created thread's default stack-size if either
    /// `attributes` is not supplied or if `attributes.stackSize()` has the
    /// unset value.  The behavior is undefined unless `attributes`, if
    /// specified, has a `stackSize` that is either greater than 0 or
    /// `e_UNSET_STACK_SIZE`.  Note that unless the created thread is
    /// explicitly "detached" (by invoking the `detach` class method with
    /// `handle`) or the `k_CREATE_DETACHED` attribute is specified, a call
    /// to `join` must be made to reclaim any system resources associated
    /// with the newly-created thread.  Also note that users are encouraged
    /// to either explicitly provide a stack size attribute, or configure a
    /// `bslmt`-wide default using `bslmt::Configuration`, because the
    /// default stack size is surprisingly small on some platforms.
    static int create(Handle                  *handle,
                      ThreadFunction           function,
                      void                    *userData);
    static int create(Handle                  *handle,
                      const ThreadAttributes&  attributes,
                      ThreadFunction           function,
                      void                    *userData);

    /// Create a new thread of program control whose entry point will invoke
    /// the specified `function` object, and load into the specified
    /// `handle` an identifier that may be used to refer to this thread in
    /// calls to other `ThreadUtil` methods.  Optionally specify
    /// `attributes` describing the properties for the new thread.  If
    /// `attributes` is not supplied, a default `ThreadAttributes` object is
    /// used.  Use the global allocator to supply memory.  Return 0 on
    /// success, and a non-zero value otherwise.  `function` shall be a
    /// reference to a type, `INVOKABLE`, that can be copy-constructed, and
    /// where the expression `(void)function()` will execute a function call
    /// (i.e., either a `void()()` function, or a functor object
    /// implementing `void operator()()`).  `bslmt::Configuration` is used
    /// to determine the created thread's default stack-size if either
    /// `attributes` is not supplied or if `attributes.stackSize()` has the
    /// unset value.  The behavior is undefined unless `attributes`, if
    /// specified, has a `stackSize` that is either greater than 0 or
    /// `e_UNSET_STACK_SIZE`.  Note that unless the created thread is
    /// explicitly "detached" (by invoking the `detach` class method with
    /// `handle`) or the `k_CREATE_DETACHED` attribute is specified, a call
    /// to `join` must be made to reclaim any system resources associated
    /// with the newly-created thread.  Also note that users are encouraged
    /// to either explicitly provide a stack size attribute, or configure a
    /// `bslmt`-wide default using `bslmt::Configuration`, because the
    /// default stack size is surprisingly small on some platforms.
    template <class INVOKABLE>
    static int create(Handle                  *handle,
                      const INVOKABLE&         function);
    template <class INVOKABLE>
    static int create(Handle                  *handle,
                      const ThreadAttributes&  attributes,
                      const INVOKABLE&         function);

    /// Create a new thread of program control whose entry point will be the
    /// specified `function`, and which will be passed the specified
    /// `userData` as its sole argument, and load into the specified
    /// `handle` an identifier that may be used to refer to this thread in
    /// calls to other `ThreadUtil` methods.  Optionally specify
    /// `attributes` describing the properties for the new thread.  If
    /// `attributes` is not supplied, a default `ThreadAttributes` object is
    /// used.  Use the specified `allocator` to supply memory.  Return 0 on
    /// success, and a non-zero value otherwise.  `bslmt::Configuration` is
    /// used to determine the created thread's default stack-size if either
    /// `attributes` is not supplied or if `attributes.stackSize()` has the
    /// unset value.  The behavior is undefined unless `attributes`, if
    /// specified, has a `stackSize` that is either greater than 0 or
    /// `e_UNSET_STACK_SIZE`.  Note that unless the created thread is
    /// explicitly "detached" (by invoking the `detach` class method with
    /// `handle`) or the `k_CREATE_DETACHED` attribute is specified, a call
    /// to `join` must be made to reclaim any system resources associated
    /// with the newly-created thread.  Also note that users are encouraged
    /// to either explicitly provide a stack size attribute, or configure a
    /// `bslmt`-wide default using `bslmt::Configuration`, because the
    /// default stack size is surprisingly small on some platforms.
    static int createWithAllocator(Handle                  *handle,
                                   ThreadFunction           function,
                                   void                    *userData,
                                   bslma::Allocator        *allocator);
    static int createWithAllocator(Handle                  *handle,
                                   const ThreadAttributes&  attributes,
                                   ThreadFunction           function,
                                   void                    *userData,
                                   bslma::Allocator        *allocator);

    /// Create a new thread of program control whose entry point will invoke
    /// the specified `function` object (using the specified `allocator` to
    /// supply memory to copy `function`), and load into the specified
    /// `handle` an identifier that may be used to refer to this thread in
    /// calls to other `ThreadUtil` methods.  Optionally specify
    /// `attributes` describing the properties for the new thread.  If
    /// `attributes` is not supplied, a default `ThreadAttributes` object is
    /// used.  Return 0 on success, and a non-zero value otherwise.
    /// `function` shall be a reference to a type, `INVOKABLE`, that can be
    /// copy-constructed, and where the expression `(void)function()` will
    /// execute a function call (i.e., either a `void()()` function, or a
    /// functor object implementing `void operator()()`).
    /// `bslmt::Configuration` is used to determine the created thread's
    /// default stack-size if either `attributes` is not supplied or if
    /// `attributes.stackSize()` has the unset value.  The behavior is
    /// undefined unless `attributes`, if specified, has a `stackSize` that
    /// is either greater than 0 or `e_UNSET_STACK_SIZE`.  Note that unless
    /// the created thread is explicitly "detached" (by invoking the
    /// `detach` class method with `handle`) or the `k_CREATE_DETACHED`
    /// attribute is specified, a call to `join` must be made to reclaim any
    /// system resources associated with the newly-created thread.  Also
    /// note that the lifetime of `allocator` must exceed the lifetime of
    /// the thread.  Also note that users are encouraged to either
    /// explicitly provide a stack size attribute, or configure a
    /// `bslmt`-wide default using `bslmt::Configuration`, because the
    /// default stack size is surprisingly small on some platforms.
    template <class INVOKABLE>
    static int createWithAllocator(Handle                  *handle,
                                   const INVOKABLE&         function,
                                   bslma::Allocator        *allocator);
    template <class INVOKABLE>
    static int createWithAllocator(Handle                  *handle,
                                   const ThreadAttributes&  attributes,
                                   const INVOKABLE&         function,
                                   bslma::Allocator        *allocator);

    /// "Detach" the thread identified by the specified `handle` such that
    /// when it terminates, the resources associated with that thread will
    /// automatically be reclaimed.  The behavior is undefined unless
    /// `handle` was obtained by a call to `create` or `self`.  Note that
    /// once a thread is "detached", it is no longer possible to `join` the
    /// thread to retrieve its exit status.
    static int detach(Handle& handle);

    /// Exit the current thread and return the specified `status`.  If the
    /// current thread is not "detached", then a call to `join` must be made
    /// to reclaim any resources used by the thread, and to retrieve the
    /// exit status.  Note that the preferred method of exiting a thread is
    /// to return from the entry point function.
    static void exit(void *status);

    /// Return the minimum available priority for the specified `policy`,
    /// where `policy` is of type `ThreadAttributes::SchedulingPolicy`.
    /// Return `ThreadAttributes::e_UNSET_PRIORITY` if the minimum
    /// scheduling priority cannot be determined.  Note that, for some
    /// platform / policy combinations, `getMinSchedulingPriority(policy)`
    /// and `getMaxSchedulingPriority(policy)` return the same value.
    static int getMinSchedulingPriority(
                                    ThreadAttributes::SchedulingPolicy policy);

    /// Return the maximum available priority for the specified `policy`,
    /// where `policy` is of type `ThreadAttributes::SchedulingPolicy`.
    /// Return `ThreadAttributes::e_UNSET_PRIORITY` if the maximum
    /// scheduling priority cannot be determined.  Note that, for some
    /// platform / policy combinations, `getMinSchedulingPriority(policy)`
    /// and `getMaxSchedulingPriority(policy)` return the same value.
    static int getMaxSchedulingPriority(
                                    ThreadAttributes::SchedulingPolicy policy);

    /// Load the name of the current thread into the specified
    /// `*threadName`.  Note that this method clears `*threadName` on
    /// platforms other than Linux, Solaris, Darwin, and Windows.
    static void getThreadName(bsl::string *threadName);

    /// Suspend execution of the current thread until the thread referred to
    /// by the specified `threadHandle` terminates, and reclaim any system
    /// resources associated with `threadHandle`.  Return 0 on success, and
    /// a non-zero value otherwise.  If the optionally specified `status` is
    /// not 0, load into `*status` the value returned by the function
    /// supplied at the creation of the thread identified by `threadHandle`.
    /// The behavior is undefined unless `threadHandle` was obtained by a
    /// call to `create`.
    static int join(Handle& threadHandle, void **status = 0);

    /// Suspend execution of the current thread for a period of at least the
    /// specified `microseconds` and the optionally specified `seconds`
    /// (relative time), or an interrupting signal is received.  Note that
    /// the actual time suspended depends on many factors including system
    /// scheduling and system timer resolution, and may be significantly
    /// longer than the time requested.
    static void microSleep(int microseconds, int seconds = 0);

    /// Set the name of the current thread to the specified `threadName`.
    /// On platforms other than Linux, Solaris, Darwin and Windows this
    /// method has no effect.  Note that on those two platforms `threadName`
    /// will be truncated to a length of 15 bytes, not including the
    /// terminating '\0'.
    static void setThreadName(const bslstl::StringRef& threadName);

    /// Suspend execution of the current thread for a period of at least the
    /// specified (relative) `sleepTime`, or an interrupting signal is
    /// received.  Note that the actual time suspended depends on many
    /// factors including system scheduling and system timer resolution.
    static void sleep(const bsls::TimeInterval& sleepTime);

#ifdef BSLS_LIBRARYFEATURES_HAS_CPP11_BASELINE_LIBRARY
    /// Suspend execution of the current thread for a period of at least the
    /// specified (relative) `sleepTime`, or an interrupting signal is
    /// received.  Note that the actual time suspended depends on many
    /// factors including system scheduling and system timer resolution.
    template <class REP_TYPE, class PERIOD_TYPE>
    static void sleep(
                const bsl::chrono::duration<REP_TYPE, PERIOD_TYPE>& sleepTime);
#endif

    /// Suspend execution of the current thread until the specified
    /// `absoluteTime`, or an interrupting signal is received.  Optionally
    /// specify `clockType` which determines the epoch from which the
    /// interval `absoluteTime` is measured (see {Supported Clock-Types} in
    /// the component documentation).  The behavior is undefined unless
    /// `absoluteTime` represents a time after January 1, 1970 and before
    /// the end of December 31, 9999 (i.e., a time interval greater than or
    /// equal to 0, and less than 253,402,300,800 seconds).  Note that the
    /// actual time suspended depends on many factors including system
    /// scheduling and system timer resolution.
    static void sleepUntil(const bsls::TimeInterval&   absoluteTime,
                           bsls::SystemClockType::Enum clockType
                                          = bsls::SystemClockType::e_REALTIME);

#ifdef BSLS_LIBRARYFEATURES_HAS_CPP11_BASELINE_LIBRARY
    /// Suspend execution of the current thread until the specified
    /// `absoluteTime`, which is an *absolute* time represented as an
    /// interval from some epoch, determined by the clock associated with
    /// the time point.  The behavior is undefined unless `absoluteTime`
    /// represents a time after January 1, 1970 and before the end of
    /// December 31, 9999.  Note that the actual time suspended depends on
    /// many factors including system scheduling and system timer
    /// resolution.
    template <class CLOCK, class DURATION>
    static void sleepUntil(
                 const bsl::chrono::time_point<CLOCK, DURATION>& absoluteTime);
#endif

    /// Move the current thread to the end of the scheduler's queue and
    /// schedule another thread to run.  Note that this allows cooperating
    /// threads of the same priority to share CPU resources equally.
    static void yield();

                       // *** Thread Identification ***

    /// Return `true` if the specified `a` and `b` thread handles identify
    /// the same thread, or if both `a` and `b` are invalid handles, and
    /// `false` otherwise.  Note that if *either* of `a` or `b` is an
    /// invalid handle, but not both, this method returns `false`.
    static bool areEqual(const Handle& a, const Handle& b);

    /// Return `true` if the specified `a` thread identifier is associated
    /// with the same thread as the specified `b` thread identifier, and
    /// `false` otherwise.
    static bool areEqualId(const Id& a, const Id& b);

    /// Return the unique identifier of the thread having the specified
    /// `threadHandle` within the current process.  The behavior is
    /// undefined unless `handle` was obtained by a call to `create` or
    /// `self`.  Note that this value is valid only until the thread
    /// terminates, and may be reused thereafter.
    static Id handleToId(const Handle& threadHandle);

    /// Return the unique integral identifier of a thread uniquely
    /// identified by the specified `threadId` within the current process.
    /// Note that this representation is particularly useful for logging
    /// purposes.  Also note that this value is only valid until the thread
    /// terminates and may be reused thereafter.
    static bsls::Types::Uint64 idAsUint64(const Id& threadId);

    /// Return the unique integral identifier of a thread uniquely
    /// identified by the specified `threadId` within the current process.
    /// Note that this representation is particularly useful for logging
    /// purposes.  Also note that this value is only valid until the thread
    /// terminates and may be reused thereafter.
    ///
    /// DEPRECATED: use `idAsUint64`.
    static int idAsInt(const Id& threadId);

    /// Return a reference to the non-modifiable `Handle` object that is
    /// guaranteed never to be a valid thread handle.
    static const Handle& invalidHandle();

    /// Return `true` if the specified `a` and `b` thread handles identify
    /// the same thread, or if both `a` and `b` are invalid handles, and
    /// `false` otherwise.  Note that if *either* of `a` or `b` is an
    /// invalid handle, but not both, this method returns `false`.
    ///
    /// DEPRECATED: use `areEqual` instead.
    static bool isEqual(const Handle& a, const Handle& b);

    /// Return `true` if the specified `lhs` thread identifier is associated
    /// with the same thread as the specified `rhs` thread identifier, and
    /// `false` otherwise.
    ///
    /// DEPRECATED: use `areEqualId` instead.
    static bool isEqualId(const Id& a, const Id& b);

    /// Return the platform-specific identifier associated with the thread
    /// referred to by the specified `handle`.  The behavior is undefined
    /// unless `handle` was obtained by a call to `create` or `self`.  Note
    /// that the returned native handle may not be a globally unique
    /// identifier for the thread (see `selfIdAsUint`).
    static NativeHandle nativeHandle(const Handle& handle);

    /// Return an opaque thread identifier that can be used to refer to the
    /// current thread in calls to other `ThreadUtil` methods.  Note that
    /// identifier may only be used to refer to the current thread from the
    /// current thread (the handle returned is not valid in other threads).
    static Handle self();

    /// Return an identifier that can be used to uniquely identify the
    /// current thread within the current process.  Note that the identifier
    /// is only valid until the thread terminates and may be reused
    /// thereafter.
    static Id selfId();

    /// Return an integral identifier that can be used to uniquely identify
    /// the current thread within the current process.  Note that this
    /// representation is particularly useful for logging purposes.  Also
    /// note that this value is only valid until the thread terminates and
    /// may be reused thereafter.
    ///
    /// DEPRECATED: use `selfIdAsUint64` instead.
    static bsls::Types::Uint64 selfIdAsInt();

    /// Return an integral identifier that can be used to uniquely identify
    /// the current thread within the current process.  Note that this
    /// representation is particularly useful for logging purposes.  Also
    /// note that this value is valid only until the thread terminates, and
    /// may be reused thereafter.
    static bsls::Types::Uint64 selfIdAsUint64();



                // *** Thread-Specific (Local) Storage (TSS or TLS) ***

    /// Load into the specified `key` a new process-wide identifier that can
    /// be used to store (via `setSpecific`) and retrieve (via
    /// `getSpecific`) a pointer value local to each thread, and associate
    /// with the new key the specified `threadKeyCleanupFunction`, which
    /// will be called by each thread, if `threadKeyCleanupFunction` is
    /// non-zero and the value associated with `key` for that thread is
    /// non-zero, with the associated value as an argument, after the
    /// function passed to `create` has returned and before the thread
    /// terminates.  Return 0 on success, and a non-zero value otherwise.
    /// Note that multiple keys can be defined, which can result in multiple
    /// thread key cleanup functions being called for a given thread.
    static int createKey(Key *key, Destructor threadKeyCleanupFunction);

    /// Delete the specified `key` from the calling process, and
    /// disassociate all threads from the thread key cleanup function
    /// supplied when `key` was created (see `createKey`).  Return 0 on
    /// success, and a non-zero value otherwise.  The behavior is undefined
    /// unless `key` was obtained from a successful call to `createKey` and
    /// has not already been deleted.  Note that deleting a key does not
    /// delete any data referred to by the pointer values associated with
    /// that key in any thread.
    static int deleteKey(Key& key);

    /// Return the thread-local value associated with the specified `key`.
    /// A `key` is shared among all threads and the value associated with
    /// `key` for each thread is 0 until it is set by that thread using
    /// `setSpecific`.  The behavior is undefined unless this method is
    /// called outside any thread key cleanup function associated with any
    /// key by `createKey`, `key` was obtained from a successful call to
    /// `createKey`, and `key` has not been deleted.
    static void *getSpecific(const Key& key);

    /// Associate the specified thread-local `value` with the specified
    /// process-wide `key`.  Return 0 on success, and a non-zero value
    /// otherwise.  The value associated with a thread for a given key is 0
    /// until it has been set by that thread using `setSpecific`.  The
    /// behavior is undefined unless this method is called outside any
    /// thread key cleanup function associated with any key by `createKey`,
    /// `key` was obtained from a successful call to `createKey`, and `key`
    /// has not been deleted.
    static int setSpecific(const Key& key, const void *value);

    /// Return a *hint* at the number of concurrent threads supported by
    /// this platform on success, and 0 otherwise.
    static unsigned int hardwareConcurrency();
};

// ============================================================================
//                             INLINE DEFINITIONS
// ============================================================================

                            // -----------------
                            // struct ThreadUtil
                            // -----------------

                    // *** Thread Management ***

// CLASS METHODS
inline
int ThreadUtil::create(Handle         *handle,
                       ThreadFunction  function,
                       void           *userData)
{
    BSLS_ASSERT_SAFE(handle);

    return Imp::create(handle, function, userData);
}

template <class INVOKABLE>
inline
int ThreadUtil::create(Handle           *handle,
                       const INVOKABLE&  function)
{
    BSLS_ASSERT_SAFE(handle);

    return createWithAllocator(handle,
                               function,
                               bslma::Default::globalAllocator());
}

template <class INVOKABLE>
inline
int ThreadUtil::create(Handle                  *handle,
                       const ThreadAttributes&  attributes,
                       const INVOKABLE&         function)
{
    BSLS_ASSERT_SAFE(handle);

    return createWithAllocator(handle,
                               attributes,
                               function,
                               bslma::Default::globalAllocator());
}

inline
int ThreadUtil::createWithAllocator(
                                 Handle                             *handle,
                                 ThreadFunction                      function,
                                 void                               *userData,
                                 BSLA_MAYBE_UNUSED bslma::Allocator *allocator)
{
    BSLS_ASSERT_SAFE(handle);
    BSLS_ASSERT_OPT(allocator);

    // 'allocator' is unused in this function, which is provided for symmetry
    // and in case this function comes to need an allocator at sometime in the
    // future.

    return Imp::create(handle, function, userData);
}

template <class INVOKABLE>
inline
int ThreadUtil::createWithAllocator(Handle                  *handle,
                                    const ThreadAttributes&  attributes,
                                    const INVOKABLE&         function,
                                    bslma::Allocator        *allocator)
{
    BSLS_ASSERT_SAFE(handle);
    BSLS_ASSERT_OPT(allocator);

    bslma::ManagedPtr<EntryPointFunctorAdapter<INVOKABLE> > threadData;
    EntryPointFunctorAdapterUtil::allocateAdapter(&threadData,
                                                  function,
                                                  attributes.threadName(),
                                                  allocator);

    int rc = Imp::create(handle,
                         attributes,
                         bslmt_EntryPointFunctorAdapter_invoker,
                         threadData.ptr());
    if (0 == rc) {
        threadData.release();
    }
    return rc;
}

template <class INVOKABLE>
inline
int ThreadUtil::createWithAllocator(Handle           *handle,
                                    const INVOKABLE&  function,
                                    bslma::Allocator *allocator)
{
    BSLS_ASSERT_SAFE(handle);
    BSLS_ASSERT_OPT(allocator);

    bslma::ManagedPtr<EntryPointFunctorAdapter<INVOKABLE> > threadData;
    EntryPointFunctorAdapterUtil::allocateAdapter(&threadData,
                                                  function,
                                                  bslstl::StringRef(),
                                                  allocator);

    int rc = Imp::create(handle, bslmt_EntryPointFunctorAdapter_invoker,
                         threadData.ptr());
    if (0 == rc) {
        threadData.release();
    }
    return rc;
}

inline
int ThreadUtil::detach(Handle& handle)
{
    return Imp::detach(handle);
}

inline
void ThreadUtil::exit(void *status)
{
    Imp::exit(status);
}

inline
int ThreadUtil::getMinSchedulingPriority(
                                     ThreadAttributes::SchedulingPolicy policy)
{
    return Imp::getMinSchedulingPriority(policy);
}

inline
int ThreadUtil::getMaxSchedulingPriority(
                                     ThreadAttributes::SchedulingPolicy policy)
{
    return Imp::getMaxSchedulingPriority(policy);
}

inline
void ThreadUtil::getThreadName(bsl::string *threadName)
{
    BSLS_ASSERT_SAFE(threadName);

    return Imp::getThreadName(threadName);
}

inline
int ThreadUtil::join(Handle& threadHandle, void **status)
{
    return Imp::join(threadHandle, status);
}

inline
void ThreadUtil::microSleep(int microseconds, int seconds)
{
    Imp::microSleep(microseconds, seconds);
}

inline
void ThreadUtil::setThreadName(const bslstl::StringRef& threadName)
{
    Imp::setThreadName(threadName);
}

inline
void ThreadUtil::sleep(const bsls::TimeInterval& sleepTime)
{
    Imp::sleep(sleepTime);
}

#ifdef BSLS_LIBRARYFEATURES_HAS_CPP11_BASELINE_LIBRARY
template <class REP_TYPE, class PERIOD_TYPE>
inline
void ThreadUtil::sleep(
                 const bsl::chrono::duration<REP_TYPE, PERIOD_TYPE>& sleepTime)
{
    ThreadUtil::sleep(ChronoUtil::durationToTimeInterval(sleepTime));
}
#endif

inline
void ThreadUtil::sleepUntil(const bsls::TimeInterval&   absoluteTime,
                                   bsls::SystemClockType::Enum clockType)
{
    int status = Imp::sleepUntil(absoluteTime, clockType);
    (void) status;  // Suppress an unused variable error.
    BSLS_ASSERT(0 == status);
}

#ifdef BSLS_LIBRARYFEATURES_HAS_CPP11_BASELINE_LIBRARY
template <class CLOCK, class DURATION>
void ThreadUtil::sleepUntil(
                  const bsl::chrono::time_point<CLOCK, DURATION>& absoluteTime)
{
    typename CLOCK::time_point        now = CLOCK::now();
    const bsls::SystemClockType::Enum bslsClockType
                                          = bsls::SystemClockType::e_REALTIME;

    // Iteration is necessary because the specified 'CLOCK' type may not
    // progress at the same rate as the realtime system clock.

    while (absoluteTime > now) {
        bsls::TimeInterval ti = bsls::SystemTime::now(bslsClockType)
                                              .addDuration(absoluteTime - now);
        ThreadUtil::sleepUntil(ti);
        now = CLOCK::now();
    }
}
#endif

inline
void ThreadUtil::yield()
{
    Imp::yield();
}

                    // *** Thread Identification ***

inline
bool ThreadUtil::areEqual(const Handle& a, const Handle& b)
{
    // Some implementations (notably pthreads) do not define the result of
    // comparing invalid handles.  We avoid undefined behavior by explicitly
    // checking for invalid handles.

    return Imp::INVALID_HANDLE == a
           ? (Imp::INVALID_HANDLE == b)
           : (Imp::INVALID_HANDLE == b ? false : Imp::areEqual(a, b));
}

inline
bool ThreadUtil::areEqualId(const Id& a, const Id& b)
{
    return Imp::areEqualId(a, b);
}

inline
ThreadUtil::Id ThreadUtil::handleToId(const Handle& threadHandle)
{
    return Imp::handleToId(threadHandle);
}

inline
bsls::Types::Uint64 ThreadUtil::idAsUint64(const Id& threadId)
{
    return Imp::idAsUint64(threadId);
}

inline
int ThreadUtil::idAsInt(const Id& threadId)
{
    return Imp::idAsInt(threadId);
}

inline
const ThreadUtil::Handle& ThreadUtil::invalidHandle()
{
    return Imp::INVALID_HANDLE;
}

inline
bool ThreadUtil::isEqual(const Handle& a, const Handle& b)
{
    return Imp::areEqual(a, b);
}

inline
bool ThreadUtil::isEqualId(const Id& a, const Id& b)
{
    return Imp::areEqualId(a, b);
}

inline
ThreadUtil::NativeHandle
ThreadUtil::nativeHandle(const Handle& handle)
{
    return Imp::nativeHandle(handle);
}

inline
ThreadUtil::Handle ThreadUtil::self()
{
    return Imp::self();
}

inline
ThreadUtil::Id ThreadUtil::selfId()
{
    return Imp::selfId();
}

inline
bsls::Types::Uint64 ThreadUtil::selfIdAsInt()
{
    return Imp::selfIdAsInt();
}

inline
bsls::Types::Uint64 ThreadUtil::selfIdAsUint64()
{
    return Imp::selfIdAsUint64();
}

            // *** Thread-Specific (Local) Storage (TSS or TLS) ***

inline
int ThreadUtil::createKey(Key *key, Destructor threadKeyCleanupFunction)
{
    return Imp::createKey(key, threadKeyCleanupFunction);
}

inline
int ThreadUtil::deleteKey(Key& key)
{
    return Imp::deleteKey(key);
}

inline
void *ThreadUtil::getSpecific(const Key& key)
{
    return Imp::getSpecific(key);
}

inline
int ThreadUtil::setSpecific(const Key& key, const void *value)
{
    return Imp::setSpecific(key, value);
}

inline
unsigned int ThreadUtil::hardwareConcurrency()
{
    return Imp::hardwareConcurrency();
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
