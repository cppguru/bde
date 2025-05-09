// bdlt_time.h                                                        -*-C++-*-
#ifndef INCLUDED_BDLT_TIME
#define INCLUDED_BDLT_TIME

#include <bsls_ident.h>
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide a value-semantic type representing time-of-day.
//
//@CLASSES:
//  bdlt::Time: time-of-day type (with microsecond resolution)
//
//@DESCRIPTION: This component implements a value-semantic time class,
// `bdlt::Time`, that can represent the time of day to a resolution of one
// microsecond (using a 24-hour clock).  Valid time values range from
// 00:00:00.000000 (i.e., midnight) through 23:59:59.999999 (i.e., one
// microsecond before midnight).  A time value can be specified via five
// separate integer attribute values denoting hours `[0 .. 23]`, minutes
// `[0 .. 59]`, seconds `[0 .. 59]`, milliseconds `[0 .. 999]`, and
// microseconds `[0 .. 999]`.  In addition, the `bdlt::Time` type has one more
// valid value, 24:00:00.000000, that can be set explicitly and accessed.  The
// value 24:00:00.000000 behaves, in most cases, as if it were the value
// 00:00:00.000000; however, for all relational comparison operators,
// 24:00:00.000000 is not a valid argument and, therefore, would result in
// undefined behavior.  Each of the `add` manipulators, along with modifying
// the value of the object, return the (signed) number of times that the
// 23:59:59.999999 - 00:00:00.000000 boundary was crossed in performing the
// addition.
//
///ISO Standard Text Representation
///--------------------------------
// A common standard text representation of a date and time value is described
// by ISO 8601.  BDE provides the `bdlt_iso8601util` component for conversion
// to and from the standard ISO8601 format.
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: Basic `bdlt::Time` Usage
///- - - - - - - - - - - - - - - - - -
// This example demonstrates how to create and use a `bdlt::Time` object.
//
// First, create an object `t1` having the default value, and then verify that
// it represents the value 24:00:00.000000:
// ```
// bdlt::Time t1;               assert(24 == t1.hour());
//                              assert( 0 == t1.minute());
//                              assert( 0 == t1.second());
//                              assert( 0 == t1.millisecond());
//                              assert( 0 == t1.microsecond());
// ```
// Then, set `t1` to the value 2:34pm (14:34:00.000000):
// ```
// t1.setTime(14, 34);          assert(14 == t1.hour());
//                              assert(34 == t1.minute());
//                              assert( 0 == t1.second());
//                              assert( 0 == t1.millisecond());
//                              assert( 0 == t1.microsecond());
// ```
// Next, use `setTimeIfValid` to attempt to assign the invalid value 24:15 to
// `t1`, then verify the method returns an error status and the value of `t1`
// is unmodified:
// ```
// int ret = t1.setTimeIfValid(24, 15);
//                              assert( 0 != ret);          // 24:15 is not
//                                                          // valid
//
//                              assert(14 == t1.hour());    // no effect
//                              assert(34 == t1.minute());  // on the
//                              assert( 0 == t1.second());  // object
//                              assert( 0 == t1.millisecond());
//                              assert( 0 == t1.microsecond());
// ```
// Then, create `t2` as a copy of `t1`:
// ```
// bdlt::Time t2(t1);            assert(t1 == t2);
// ```
// Next, add 5 minutes and 7 seconds to the value of `t2` (in two steps), and
// confirm the value of `t2`:
// ```
// t2.addMinutes(5);
// t2.addSeconds(7);
//                              assert(14 == t2.hour());
//                              assert(39 == t2.minute());
//                              assert( 7 == t2.second());
//                              assert( 0 == t2.millisecond());
//                              assert( 0 == t2.microsecond());
// ```
// Then, subtract `t1` from `t2` to yield a `bdlt::DatetimeInterval` `dt`
// representing the time-interval between those two times, and verify the value
// of `dt` is 5 minutes and 7 seconds (or 307 seconds):
// ```
// bdlt::DatetimeInterval dt = t2 - t1;
//                              assert(307 == dt.totalSeconds());
// ```
// Finally, stream the value of `t2` to `stdout`:
// ```
// bsl::cout << t2 << bsl::endl;
// ```
// The streaming operator produces the following output on `stdout`:
// ```
// 14:39:07.000000
// ```

#include <bdlscm_version.h>

#include <bdlt_datetimeinterval.h>
#include <bdlt_timeunitratio.h>

#include <bdlb_bitutil.h>

#include <bslh_hash.h>

#include <bslmf_integralconstant.h>
#include <bslmf_isbitwisecopyable.h>

#include <bsls_assert.h>
#include <bsls_atomic.h>
#include <bsls_log.h>
#include <bsls_performancehint.h>
#include <bsls_platform.h>
#include <bsls_review.h>
#include <bsls_stackaddressutil.h>
#include <bsls_types.h>

#include <bsl_iosfwd.h>
#include <bsl_cstring.h> // memset
#include <bsl_sstream.h>

namespace BloombergLP {
namespace bdlt {

                                 // ==========
                                 // class Time
                                 // ==========

/// This class implements a value-semantic type that represents the time of
/// day to a resolution of one microsecond.  Each object of this (almost)
/// simply constrained attribute class *always* represents a valid time
/// value to a resolution of one microsecond.  The valid range for times is
/// 00:00:00.000000 through 23:59:59.999999, except that 24:00:00.000000
/// represents the default-constructed value.  The value 24:00:00.000000
/// behaves, in most cases, as if it were the value 00:00:00.000000;
/// however, for all relational comparison operators, 24:00:00.000000 is not
/// a valid argument and, therefore, would result in undefined behavior.
/// Each add operation on a `Time` object returns the (signed) number of
/// times that the 23:59:59.999999 - 00:00:00.000000 boundary was crossed
/// while performing the operation.  Attempting to construct a `Time` with
/// any attribute outside its valid range (or with an hour attribute value
/// of 24 and any other attribute non-zero) has undefined behavior.
class Time {

    // PRIVATE TYPES
    enum {
        k_DEFAULT_FRACTIONAL_SECOND_PRECISION = 6
    };

    // CLASS DATA
    static const bsls::Types::Int64 k_REP_MASK  = 0x0000004000000000ULL;

    static bsls::AtomicInt64 s_invalidRepresentationCount;

    // DATA
    bsls::Types::Int64 d_value;  // encoded offset from 00:00:00.000000

    // FRIENDS
    friend DatetimeInterval operator-(const Time&, const Time&);
    friend bool operator==(const Time&, const Time&);
    friend bool operator!=(const Time&, const Time&);
    friend bool operator< (const Time&, const Time&);
    friend bool operator<=(const Time&, const Time&);
    friend bool operator>=(const Time&, const Time&);
    friend bool operator> (const Time&, const Time&);
    template <class HASHALG>
    friend void hashAppend(HASHALG& hashAlg, const Time&);

    // PRIVATE MANIPULATORS

    /// Assign to `d_value` the representation of time such that the
    /// difference between this representation of time and 00:00:00.000000
    /// is the specified `totalMicroseconds`.  If
    /// `TimeUnitRatio::k_US_PER_D == totalMicroseconds`, assign to
    /// `d_value` the representation of 24:00:00.000000.  The behavior is
    /// undefined unless
    /// `0 <= totalMicroseconds <= TimeUnitRatio::k_US_PER_D`.
    void setMicrosecondsFromMidnight(bsls::Types::Int64 totalMicroseconds);

    // PRIVATE ACCESSORS

    /// Return the difference, in microseconds, between the value of this
    /// object and 00:00:00.000000.  If the value of this object is
    /// 24:00:00.000000, it is treated as 00:00:00.000000.
    bsls::Types::Int64 microsecondsFromMidnight() const;

    /// Invoke a review failure notifying that a `bdlt::Time` instance is
    /// being used in an invalid state.  The behavior is undefined unless
    /// this object has the old represenation (`k_REP_MASK > d_value`) and
    /// `BSLS_ASSERT_SAFE` is not active.
    bsls::Types::Int64 invalidMicrosecondsFromMidnight() const;

    /// If `d_value` was stored using the current representation scheme,
    /// return `d_value`.  Otherwise, return the representation of the time
    /// corresponding to `d_value` total milliseconds since 00:00:00.000000
    /// (i.e., convert from the old representation scheme to the current
    /// scheme), or return the current representation of 24:00:00.000000 if
    /// `d_value` is the old representation of the default-constructed
    /// value.
    bsls::Types::Int64 updatedRepresentation() const;

  public:
    // CLASS METHODS

    /// Return `true` if the specified `hour`, and the optionally specified
    /// `minute`, `second`, `millisecond`, and `microsecond`, represent a
    /// valid `Time` value, and `false` otherwise.  Unspecified arguments
    /// default to 0.  The `hour`, `minute`, `second`, `millisecond`, and
    /// `microsecond` attributes comprise a valid `Time` value if
    /// `0 <= hour < 24`, `0 <= minute < 60`, `0 <= second < 60`,
    /// `0 <= millisecond < 1000`, and `0 <= microsecond < 1000`.
    /// Additionally, 24:00:00.000000 also represents a valid `Time` value.
    static bool isValid(int hour,
                        int minute = 0,
                        int second = 0,
                        int millisecond = 0,
                        int microsecond = 0);

                                  // Aspects

    /// Return the maximum valid BDEX format version, as indicated by the
    /// specified `versionSelector`, to be passed to the `bdexStreamOut`
    /// method.  Note that it is highly recommended that `versionSelector`
    /// be formatted as "YYYYMMDD", a date representation.  Also note that
    /// `versionSelector` should be a *compile*-time-chosen value that
    /// selects a format version supported by both externalizer and
    /// unexternalizer.  See the `bslx` package-level documentation for more
    /// information on BDEX streaming of value-semantic types and
    /// containers.
    static int maxSupportedBdexVersion(int versionSelector);

    // CREATORS

    /// Create a `Time` object having the value 24:00:00.000000.
    Time();

    /// Create a `Time` object having the (valid) value represented by the
    /// specified `hour`, and the optionally specified `minute`, `second`,
    /// `millisecond`, and `microsecond`.  Unspecified arguments default to
    /// 0.  The behavior is undefined unless all of the specified values are
    /// within their valid ranges (see `isValid`).
    explicit
    Time(int hour,
         int minute = 0,
         int second = 0,
         int millisecond = 0,
         int microsecond = 0);

    /// Create a `Time` object having the value of the specified `original`
    /// time.
    Time(const Time& original);

    /// Destroy this 'Time' object.
    //! ~Time() = default;

    // MANIPULATORS

    /// Assign to this time object the value of the specified `rhs` object,
    /// and return a reference providing modifiable access to this object.
    Time& operator=(const Time& rhs);

    /// Add to this time object the value of the specified `rhs` datetime
    /// interval, and return a reference providing modifiable access to this
    /// object.
    Time& operator+=(const DatetimeInterval& rhs);

    /// Subtract from this time object the value of the specified `rhs`
    /// datetime interval, and return a reference providing modifiable
    /// access to this object.
    Time& operator-=(const DatetimeInterval& rhs);

    /// Increase the value of this time object by the specified number of
    /// `hours`, and return the (signed) number of times that the
    /// 23:59:59.999999 - 00:00:00.000000 boundary was crossed in performing
    /// the operation.  Note that `hours` may be negative.
    int addHours(int hours);

    /// Increase the value of this time object by the specified number of
    /// `minutes`, and return the (signed) number of times that the
    /// 23:59:59.999999 - 00:00:00.000000 boundary was crossed in performing
    /// the operation.  Note that `minutes` may be negative.
    int addMinutes(int minutes);

    /// Increase the value of this time object by the specified number of
    /// `seconds`, and return the (signed) number of times that the
    /// 23:59:59.999999 - 00:00:00.000000 boundary was crossed in performing
    /// the operation.  Note that `seconds` may be negative.
    int addSeconds(int seconds);

    /// Increase the value of this time object by the specified number of
    /// `milliseconds`, and return the (signed) number of times that the
    /// 23:59:59.999999 - 00:00:00.000000 boundary was crossed in performing
    /// the operation.  Note that `milliseconds` may be negative.
    int addMilliseconds(int milliseconds);

    /// Increase the value of this time object by the specified number of
    /// `microseconds`, and return the (signed) number of times that the
    /// 23:59:59.999999 - 00:00:00.000000 boundary was crossed in performing
    /// the operation.  Note that `microseconds` may be negative.
    int addMicroseconds(bsls::Types::Int64 microseconds);

    /// Increase the value of this time object by the specified `interval`
    /// of time, and return the (signed) number of times that the
    /// 23:59:59.999999 - 00:00:00.000000 boundary was crossed in performing
    /// the operation.  The behavior is undefined unless the number of
    /// crossings that would be returned can be represented by an `int`.
    int addInterval(const DatetimeInterval& interval);

    /// Add to the value of this time object the specified (signed) number
    /// of `hours`, and the optionally specified (signed) numbers of
    /// `minutes`, `seconds`, `milliseconds`, and `microseconds`; return the
    /// (signed) number of times that the 23:59:59.999999 -
    /// 00:00:00.000000 boundary was crossed in performing the operation.
    /// Unspecified arguments default to 0.
    int addTime(int                hours,
                int                minutes = 0,
                int                seconds = 0,
                int                milliseconds = 0,
                bsls::Types::Int64 microseconds = 0);

    /// Set the "hour" attribute of this time object to the specified
    /// `hour`; if `hour` is 24, set the remaining attributes of this object
    /// to 0.  The behavior is undefined unless `0 <= hour <= 24`.
    void setHour(int hour);

    /// Set the "hour" attribute of this time object to the specified `hour`
    /// value *if* `0 <= hour <= 24`.  If `24 == hour`, set the remaining
    /// attributes to 0.  Return 0 on success, and a non-zero value (with no
    /// effect) otherwise.
    int setHourIfValid(int hour);

    /// Set the "minute" attribute of this time object to the specified
    /// `minute`; if the `hour` attribute is 24, set the `hour` attribute to
    /// 0.  The behavior is undefined unless `0 <= minute < 60`.
    void setMinute(int minute);

    /// Set the "minute" attribute of this time object to the specified
    /// `minute` *if* `0 <= minute < 60`; if the `hour` attribute is 24, set
    /// the `hour` attribute to 0.  Return 0 on success, and a non-zero
    /// value (with no effect) otherwise.
    int setMinuteIfValid(int minute);

    /// Set the "second" attribute of this time object to the specified
    /// `second`; if the `hour` attribute is 24, set the `hour` attribute to
    /// 0.  The behavior is undefined unless `0 <= second < 60`.
    void setSecond(int second);

    /// Set the "second" attribute of this time object to the specified
    /// `second` *if* `0 <= second < 60`; if the `hour` attribute is 24, set
    /// the `hour` attribute to 0.  Return 0 on success, and a non-zero
    /// value (with no effect) otherwise.
    int setSecondIfValid(int second);

    /// Set the "millisecond" attribute of this time object to the specified
    /// `millisecond`; if the `hour` attribute is 24, set the `hour`
    /// attribute to 0.  The behavior is undefined unless
    /// `0 <= millisecond < 1000`.
    void setMillisecond(int millisecond);

    /// Set the "millisecond" attribute of this time object to the specified
    /// `millisecond` *if* `0 <= millisecond < 1000`; if the `hour`
    /// attribute is 24, set the `hour` attribute to 0.  Return 0 on
    /// success, and a non-zero value (with no effect) otherwise.
    int setMillisecondIfValid(int millisecond);

    /// Set the "microsecond" attribute of this time object to the specified
    /// `microsecond`; if the `hour` attribute is 24, set the `hour`
    /// attribute to 0.  The behavior is undefined unless
    /// `0 <= microsecond < 1000`.
    void setMicrosecond(int microsecond);

    /// Set the "microsecond" attribute of this time object to the specified
    /// `microsecond` *if* `0 <= microsecond < 1000`; if the `hour`
    /// attribute is 24, set the `hour` attribute to 0.  Return 0 on
    /// success, and a non-zero value (with no effect) otherwise.
    int setMicrosecondIfValid(int microsecond);

    /// Set the value of this time object to the specified `hour`, and the
    /// optionally specified `minute`, `second`, `millisecond`, and
    /// `microsecond`.  Unspecified arguments default to 0.  The behavior is
    /// undefined unless all of the specified values are within their valid
    /// ranges (see `isValid`).
    void setTime(int hour,
                 int minute = 0,
                 int second = 0,
                 int millisecond = 0,
                 int microsecond = 0);

    /// Set the value of this time object to the specified `hour`, and the
    /// optionally specified `minute`, `second`, `millisecond`, and
    /// `microsecond`, if they would comprise a valid `Time` value (see
    /// `isValid`).  Return 0 on success, and a non-zero value (with no
    /// effect) otherwise.  Unspecified arguments default to 0.
    int setTimeIfValid(int hour,
                       int minute = 0,
                       int second = 0,
                       int millisecond = 0,
                       int microsecond = 0);

                                  // Aspects

    /// Assign to this object the value read from the specified input
    /// `stream` using the specified `version` format, and return a
    /// reference to `stream`.  If `stream` is initially invalid, this
    /// operation has no effect.  If `version` is not supported, this object
    /// is unaltered and `stream` is invalidated, but otherwise unmodified.
    /// If `version` is supported but `stream` becomes invalid during this
    /// operation, this object has an undefined, but valid, state.  Note
    /// that no version is read from `stream`.  See the `bslx` package-level
    /// documentation for more information on BDEX streaming of
    /// value-semantic types and containers.
    template <class STREAM>
    STREAM& bdexStreamIn(STREAM& stream, int version);

    // ACCESSORS

    /// Load, into the specified `hour`, and the optionally specified
    /// `minute`, `second`, `millisecond`, and `microsecond`, the respective
    /// `hour`, `minute`, `second`, `millisecond`, and `microsecond`
    /// attribute values from this time object.  Unspecified arguments
    /// default to 0.  Supplying 0 for an address argument suppresses the
    /// loading of the value for the corresponding attribute, but has no
    /// effect on the loading of other attribute values.
    void getTime(int *hour,
                 int *minute = 0,
                 int *second = 0,
                 int *millisecond = 0,
                 int *microsecond = 0) const;

    /// Return the value of the `hour` attribute of this time object.
    int hour() const;

    /// Return the value of the `minute` attribute of this time object.
    int minute() const;

    /// Return the value of the `second` attribute of this time object.
    int second() const;

    /// Return the value of the `millisecond` attribute of this time object.
    int millisecond() const;

    /// Return the value of the `microsecond` attribute of this time object.
    int microsecond() const;

    /// Efficiently write to the specified `result` buffer no more than the
    /// specified `numBytes` of a representation of the value of this
    /// object.  Optionally specify `fractionalSecondPrecision` digits to
    /// indicate how many fractional second digits to output.  If
    /// `fractionalSecondPrecision` is not specified then 6 fractional
    /// second digits will be output (3 digits for milliseconds and 3 digits
    /// for microseconds).  Return the number of characters (not including
    /// the null character) that would have been written if the limit due to
    /// `numBytes` were not imposed.  `result` is null-terminated unless
    /// `numBytes` is 0.  The behavior is undefined unless `0 <= numBytes`,
    /// `0 <= fractionalSecondPrecision <= 6`, and `result` refers to at
    /// least `numBytes` contiguous bytes.  Note that the return value is
    /// greater than or equal to `numBytes` if the output representation was
    /// truncated to avoid `result` overrun.
    int printToBuffer(char *result,
                      int   numBytes,
                      int   fractionalSecondPrecision = 6) const;

                                  // Aspects

    /// Write the value of this object, using the specified `version`
    /// format, to the specified output `stream`, and return a reference to
    /// `stream`.  If `stream` is initially invalid, this operation has no
    /// effect.  If `version` is not supported, `stream` is invalidated, but
    /// otherwise unmodified.  Note that `version` is not written to
    /// `stream`.  See the `bslx` package-level documentation for more
    /// information on BDEX streaming of value-semantic types and
    /// containers.
    template <class STREAM>
    STREAM& bdexStreamOut(STREAM& stream, int version) const;

    /// Write the value of this object to the specified output `stream` in a
    /// human-readable format, and return a reference to `stream`.
    /// Optionally specify an initial indentation `level`, whose absolute
    /// value is incremented recursively for nested objects.  If `level` is
    /// specified, optionally specify `spacesPerLevel`, whose absolute value
    /// indicates the number of spaces per indentation level for this and
    /// all of its nested objects.  If `level` is negative, suppress
    /// indentation of the first line.  If `spacesPerLevel` is negative,
    /// format the entire output on one line, suppressing all but the
    /// initial indentation (as governed by `level`).  If `stream` is not
    /// valid on entry, this operation has no effect.  Note that this
    /// human-readable format is not fully specified, and can change without
    /// notice.
    bsl::ostream& print(bsl::ostream& stream,
                        int           level = 0,
                        int           spacesPerLevel = 4) const;

#ifndef BDE_OPENSOURCE_PUBLICATION  // pending deprecation

    // DEPRECATED METHODS

    /// Return the most current BDEX streaming version number supported by
    /// this class.
    ///
    /// @DEPRECATED: Use `maxSupportedBdexVersion(int)` instead.
    static int maxSupportedBdexVersion();

#endif // BDE_OPENSOURCE_PUBLICATION -- pending deprecation
#ifndef BDE_OMIT_INTERNAL_DEPRECATED  // BDE2.22

    /// Return the most current BDEX streaming version number supported by
    /// this class.
    ///
    /// @DEPRECATED: Use `maxSupportedBdexVersion(int)` instead.
    static int maxSupportedVersion();

    /// Format this time to the specified output `stream`, and return a
    /// reference to the modifiable `stream`.
    ///
    /// @DEPRECATED: Use `print` instead.
    bsl::ostream& streamOut(bsl::ostream& stream) const;

    /// Set the value of this time object to the specified `hour`, and the
    /// optionally specified `minute`, `second`, and `millisecond`, if they
    /// would comprise a valid `Time` value (see `isValid`).  Return 0 on
    /// success, and a non-zero value (with no effect) otherwise.
    /// Unspecified arguments default to 0.
    ///
    /// @DEPRECATED: Use `setTimeIfValid` instead.
    int validateAndSetTime(int hour,
                           int minute = 0,
                           int second = 0,
                           int millisecond = 0);

#endif  // BDE_OMIT_INTERNAL_DEPRECATED -- BDE2.22

};

// FREE OPERATORS

/// Pass the specified `object` to the specified `hashAlg`.  This function
/// integrates with the `bslh` modular hashing system and effectively
/// provides a `bsl::hash` specialization for `Time`.
template <class HASHALG>
void hashAppend(HASHALG& hashAlg, const Time& object);

/// Return a `Time` value that is the sum of the specified `lhs` time and
/// the specified `rhs` datetime interval.
Time operator+(const Time& lhs, const DatetimeInterval& rhs);

/// Return a `Time` value that is the sum of the specified `lhs` datetime
/// interval and the specified `rhs` time.
Time operator+(const DatetimeInterval& lhs, const Time& rhs);

/// Return a `Time` value that is the difference between the specified `lhs`
/// time and the specified `rhs` datetime interval.
Time operator-(const Time& lhs, const DatetimeInterval& rhs);

/// Return a `DatetimeInterval` object initialized with the difference
/// between the specified `lhs` and `rhs` time values.
DatetimeInterval operator-(const Time& lhs, const Time& rhs);

/// Return `true` if the specified `lhs` and `rhs` time objects have the
/// same value, and `false` otherwise.  Two time objects have the same value
/// if each of their corresponding `hour`, `minute`, `second`,
/// `millisecond`, and `microsecond` attributes respectively have the same
/// value.
bool operator==(const Time& lhs, const Time& rhs);

/// Return `true` if the specified `lhs` and `rhs` time objects do not have
/// the same value, and `false` otherwise.  Two time objects do not have the
/// same value if any of their corresponding `hour`, `minute`, `second`,
/// `millisecond`, and `microsecond` attributes respectively do not have the
/// same value.
bool operator!=(const Time& lhs, const Time& rhs);

/// Return `true` if the specified `lhs` time value is less than the
/// specified `rhs` time value, and `false` otherwise.  The behavior is
/// undefined unless `lhs != Time()` and `rhs != Time()` (i.e., they do not
/// have the, default, value 24:00:00.000000).
bool operator<(const Time& lhs, const Time& rhs);

/// Return `true` if the specified `lhs` time value is less than or equal to
/// the specified `rhs` time value, and `false` otherwise.  The behavior is
/// undefined unless `lhs != Time()` and `rhs != Time()` (i.e., they do not
/// have the, default, value 24:00:00.000000).
bool operator<=(const Time& lhs, const Time& rhs);

/// Return `true` if the specified `lhs` time value is greater than the
/// specified `rhs` time value, and `false` otherwise.  The behavior is
/// undefined unless `lhs != Time()` and `rhs != Time()` (i.e., they do not
/// have the, default, value 24:00:00.000000).
bool operator>(const Time& lhs, const Time& rhs);

/// Return `true` if the specified `lhs` time value is greater than or equal
/// to the specified `rhs` time value, and `false` otherwise.  The behavior
/// is undefined unless `lhs != Time()` and `rhs != Time()` (i.e., they do
/// not have the, default, value 24:00:00.000000).
bool operator>=(const Time& lhs, const Time& rhs);

/// Write the value of the specified `time` object to the specified output
/// `stream` in a single-line format, and return a reference to `stream`.
/// If `stream` is not valid on entry, this operation has no effect.  Note
/// that this human-readable format is not fully specified, can change
/// without notice, and is logically equivalent to:
/// ```
/// print(stream, 0, -1);
/// ```
bsl::ostream& operator<<(bsl::ostream& stream, const Time& time);

// ============================================================================
//                             INLINE DEFINITIONS
// ============================================================================

                                 // ----------
                                 // class Time
                                 // ----------

// PRIVATE MANIPULATORS
inline
void Time::setMicrosecondsFromMidnight(bsls::Types::Int64 totalMicroseconds)
{
    BSLS_REVIEW(                        0 <= totalMicroseconds);
    BSLS_REVIEW(TimeUnitRatio::k_US_PER_D >= totalMicroseconds);

    d_value = totalMicroseconds | k_REP_MASK;
}

// PRIVATE ACCESSORS
inline
bsls::Types::Int64 Time::microsecondsFromMidnight() const
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(k_REP_MASK > d_value)) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;
        return invalidMicrosecondsFromMidnight();                     // RETURN
    }

    return d_value & (~k_REP_MASK);
}

inline
bsls::Types::Int64 Time::updatedRepresentation() const
{
    return microsecondsFromMidnight() | k_REP_MASK;
}

// CLASS METHODS
inline
bool Time::isValid(int hour,
                   int minute,
                   int second,
                   int millisecond,
                   int microsecond)
{
    return (0 <= hour        && hour    < bdlt::TimeUnitRatio::k_H_PER_D_32 &&
            0 <= minute      && minute  < bdlt::TimeUnitRatio::k_M_PER_H_32 &&
            0 <= second      && second  < bdlt::TimeUnitRatio::k_S_PER_M_32 &&
            0 <= millisecond && millisecond
                                        < bdlt::TimeUnitRatio::k_MS_PER_S_32 &&
            0 <= microsecond && microsecond
                                        < bdlt::TimeUnitRatio::k_US_PER_MS_32)
        || (bdlt::TimeUnitRatio::k_H_PER_D_32 == hour &&
            0                                 == minute &&
            0                                 == second &&
            0                                 == millisecond &&
            0                                 == microsecond);
}

                                  // Aspects

inline
int Time::maxSupportedBdexVersion(int versionSelector)
{
    if (versionSelector >= 20170401) {
        return 2;                                                     // RETURN
    }
    return 1;
}

// CREATORS
inline
Time::Time()
: d_value(TimeUnitRatio::k_US_PER_D | k_REP_MASK)
{
}

inline
Time::Time(int hour, int minute, int second, int millisecond, int microsecond)
{
    BSLS_ASSERT_SAFE(isValid(hour, minute, second, millisecond, microsecond));

    setMicrosecondsFromMidnight(  TimeUnitRatio::k_US_PER_H  * hour
                                + TimeUnitRatio::k_US_PER_M  * minute
                                + TimeUnitRatio::k_US_PER_S  * second
                                + TimeUnitRatio::k_US_PER_MS * millisecond
                                + microsecond);
}

inline
Time::Time(const Time& original)
: d_value(original.d_value)
{
    d_value = updatedRepresentation();
}

// MANIPULATORS
inline
Time& Time::operator=(const Time& rhs)
{
    d_value = rhs.d_value;
    d_value = updatedRepresentation();

    return *this;
}

inline
Time& Time::operator+=(const DatetimeInterval& rhs)
{
    addInterval(rhs);
    return *this;
}

inline
Time& Time::operator-=(const DatetimeInterval& rhs)
{
    addInterval(-rhs);
    return *this;
}

inline
int Time::setHourIfValid(int hour)
{
    enum { k_SUCCESS = 0, k_FAILURE = -1 };

    if (0 <= hour && hour <= 24) {
        setHour(hour);
        return k_SUCCESS;                                             // RETURN
    }
    return k_FAILURE;
}

inline
int Time::setMicrosecondIfValid(int microsecond)
{
    enum { k_SUCCESS = 0, k_FAILURE = -1 };

    if (0 <= microsecond && microsecond <= 999) {
        setMicrosecond(microsecond);
        return k_SUCCESS;                                             // RETURN
    }
    return k_FAILURE;
}

inline
int Time::setMillisecondIfValid(int millisecond)
{
    enum { k_SUCCESS = 0, k_FAILURE = -1 };

    if (0 <= millisecond && millisecond <= 999) {
        setMillisecond(millisecond);
        return k_SUCCESS;                                             // RETURN
    }
    return k_FAILURE;
}

inline
int Time::setMinuteIfValid(int minute)
{
    enum { k_SUCCESS = 0, k_FAILURE = -1 };

    if (0 <= minute && minute <= 59) {
        setMinute(minute);
        return k_SUCCESS;                                             // RETURN
    }
    return k_FAILURE;
}

inline
int Time::setSecondIfValid(int second)
{
    enum { k_SUCCESS = 0, k_FAILURE = -1 };

    if (0 <= second && second <= 59) {
        setSecond(second);
        return k_SUCCESS;                                             // RETURN
    }
    return k_FAILURE;
}

inline
int Time::setTimeIfValid(int hour,
                         int minute,
                         int second,
                         int millisecond,
                         int microsecond)
{
    enum { k_SUCCESS = 0, k_FAILURE = -1 };

    if (isValid(hour, minute, second, millisecond, microsecond)) {
        setTime(hour, minute, second, millisecond, microsecond);
        return k_SUCCESS;                                             // RETURN
    }

    return k_FAILURE;
}

                                  // Aspects

template <class STREAM>
STREAM& Time::bdexStreamIn(STREAM& stream, int version)
{
    if (stream) {
        switch (version) { // switch on the schema version
          case 2: {
            bsls::Types::Int64 tmp = 0;
            stream.getInt64(tmp);

            if (   stream
                && 0 <= tmp
                && TimeUnitRatio::k_US_PER_D >= tmp) {
                setMicrosecondsFromMidnight(tmp);
            }
            else {
                stream.invalidate();
            }
          } break;
          case 1: {
            int tmp = 0;
            stream.getInt32(tmp);

            if (   stream
                && static_cast<unsigned int>(tmp) <=
                   static_cast<unsigned int>(TimeUnitRatio::k_MS_PER_D_32)) {
                setMicrosecondsFromMidnight(TimeUnitRatio::k_US_PER_MS * tmp);
            }
            else {
                stream.invalidate();
            }
          } break;
          default: {
            stream.invalidate();  // unrecognized version number
          }
        }
    }
    return stream;
}

// ACCESSORS
inline
int Time::hour() const
{
    return static_cast<int>(microsecondsFromMidnight()
                                                  / TimeUnitRatio::k_US_PER_H);
}

inline
int Time::microsecond() const
{
    return static_cast<int>(  microsecondsFromMidnight()
                            % TimeUnitRatio::k_US_PER_MS);
}

inline
int Time::millisecond() const
{
    return static_cast<int>(  microsecondsFromMidnight()
                            / TimeUnitRatio::k_US_PER_MS
                            % TimeUnitRatio::k_MS_PER_S);
}

inline
int Time::minute() const
{
    return static_cast<int>(  microsecondsFromMidnight()
                            / TimeUnitRatio::k_US_PER_M
                            % TimeUnitRatio::k_M_PER_H);
}

inline
int Time::second() const
{
    return static_cast<int>(  microsecondsFromMidnight()
                            / TimeUnitRatio::k_US_PER_S
                            % TimeUnitRatio::k_S_PER_M);
}

                                  // Aspects

template <class STREAM>
STREAM& Time::bdexStreamOut(STREAM& stream, int version) const
{
    if (stream) {
        switch (version) { // switch on the schema version
          case 2: {
            stream.putInt64(microsecondsFromMidnight());
          } break;
          case 1: {
            stream.putInt32(static_cast<int>(microsecondsFromMidnight()
                                                / TimeUnitRatio::k_US_PER_MS));
          } break;
          default: {
            stream.invalidate();  // unrecognized version number
          }
        }
    }
    return stream;
}

#ifndef BDE_OPENSOURCE_PUBLICATION  // pending deprecation

// DEPRECATED METHODS
inline
int Time::maxSupportedBdexVersion()
{
    return maxSupportedBdexVersion(0);
}

#endif // BDE_OPENSOURCE_PUBLICATION -- pending deprecation
#ifndef BDE_OMIT_INTERNAL_DEPRECATED  // BDE2.22
inline
int Time::maxSupportedVersion()
{
    return maxSupportedBdexVersion(0);
}

inline
bsl::ostream& Time::streamOut(bsl::ostream& stream) const
{
    return stream << *this;
}

inline
int Time::validateAndSetTime(int hour, int minute, int second, int millisecond)
{
    return setTimeIfValid(hour, minute, second, millisecond);
}

#endif  // BDE_OMIT_INTERNAL_DEPRECATED -- BDE2.22

}  // close package namespace

// FREE OPERATORS
inline
bdlt::Time bdlt::operator+(const Time& lhs, const DatetimeInterval& rhs)
{
    Time result(lhs);
    return result += rhs;
}

inline
bdlt::Time bdlt::operator+(const DatetimeInterval& lhs, const Time& rhs)
{
    Time result(rhs);
    return result += lhs;
}

inline
bdlt::Time bdlt::operator-(const Time& lhs, const DatetimeInterval& rhs)
{
    Time result(lhs);
    return result -= rhs;
}

inline
bdlt::DatetimeInterval bdlt::operator-(const Time& lhs, const Time& rhs)
{
    DatetimeInterval timeInterval;

    timeInterval.setTotalMicroseconds(
             lhs.microsecondsFromMidnight() % bdlt::TimeUnitRatio::k_US_PER_D
           - rhs.microsecondsFromMidnight() % bdlt::TimeUnitRatio::k_US_PER_D);

    return timeInterval;
}

inline
bool bdlt::operator==(const Time& lhs, const Time& rhs)
{
    bsls::Types::Int64 lhsValue = lhs.microsecondsFromMidnight();
    bsls::Types::Int64 rhsValue = rhs.microsecondsFromMidnight();

    return lhsValue == rhsValue;
}

inline
bool bdlt::operator!=(const Time& lhs, const Time& rhs)
{
    bsls::Types::Int64 lhsValue = lhs.microsecondsFromMidnight();
    bsls::Types::Int64 rhsValue = rhs.microsecondsFromMidnight();

    return lhsValue != rhsValue;
}

inline
bool bdlt::operator<(const Time& lhs, const Time& rhs)
{
    BSLS_ASSERT_SAFE(bdlt::TimeUnitRatio::k_US_PER_D
                                            != lhs.microsecondsFromMidnight());
    BSLS_ASSERT_SAFE(bdlt::TimeUnitRatio::k_US_PER_D
                                            != rhs.microsecondsFromMidnight());

    bsls::Types::Int64 lhsValue = lhs.microsecondsFromMidnight();
    bsls::Types::Int64 rhsValue = rhs.microsecondsFromMidnight();

    return lhsValue < rhsValue;
}

inline
bool bdlt::operator<=(const Time& lhs, const Time& rhs)
{
    BSLS_ASSERT_SAFE(bdlt::TimeUnitRatio::k_US_PER_D
                                            != lhs.microsecondsFromMidnight());
    BSLS_ASSERT_SAFE(bdlt::TimeUnitRatio::k_US_PER_D
                                            != rhs.microsecondsFromMidnight());

    bsls::Types::Int64 lhsValue = lhs.microsecondsFromMidnight();
    bsls::Types::Int64 rhsValue = rhs.microsecondsFromMidnight();

    return lhsValue <= rhsValue;
}

inline
bool bdlt::operator>(const Time& lhs, const Time& rhs)
{
    BSLS_ASSERT_SAFE(bdlt::TimeUnitRatio::k_US_PER_D
                                            != lhs.microsecondsFromMidnight());
    BSLS_ASSERT_SAFE(bdlt::TimeUnitRatio::k_US_PER_D
                                            != rhs.microsecondsFromMidnight());

    bsls::Types::Int64 lhsValue = lhs.microsecondsFromMidnight();
    bsls::Types::Int64 rhsValue = rhs.microsecondsFromMidnight();

    return lhsValue > rhsValue;
}

inline
bool bdlt::operator>=(const Time& lhs, const Time& rhs)
{
    BSLS_ASSERT_SAFE(bdlt::TimeUnitRatio::k_US_PER_D
                                            != lhs.microsecondsFromMidnight());
    BSLS_ASSERT_SAFE(bdlt::TimeUnitRatio::k_US_PER_D
                                            != rhs.microsecondsFromMidnight());

    bsls::Types::Int64 lhsValue = lhs.microsecondsFromMidnight();
    bsls::Types::Int64 rhsValue = rhs.microsecondsFromMidnight();

    return lhsValue >= rhsValue;
}

inline
bsl::ostream& bdlt::operator<<(bsl::ostream& stream, const Time& time)
{
    return time.print(stream, 0, -1);
}

// FREE FUNCTIONS
template <class HASHALG>
inline
void bdlt::hashAppend(HASHALG& hashAlg, const Time& object)
{
    using ::BloombergLP::bslh::hashAppend;
    hashAppend(hashAlg, object.microsecondsFromMidnight());
}

namespace bslmf {

// TRAITS

/// This template specialization for `IsBitwiseCopyable` indicates that
/// `bdlt::Time` is a bitwise copyable type.
template <>
struct IsBitwiseCopyable<BloombergLP::bdlt::Time> : bsl::true_type {
};

}  // close namespace bslmf
}  // close enterprise namespace

#endif

// ----------------------------------------------------------------------------
// Copyright 2017 Bloomberg Finance L.P.
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
