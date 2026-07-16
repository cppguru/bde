// bdls_filepermissions.t.cpp                                         -*-C++-*-
#include <bdls_filepermissions.h>

#include <bdlsb_fixedmemoutstreambuf.h>

#include <bslim_testutil.h>

#include <bsl_cstdio.h>
#include <bsl_cstdlib.h>
#include <bsl_cstring.h>
#include <bsl_ios.h>
#include <bsl_ostream.h>

using namespace BloombergLP;

// ============================================================================
//                             TEST PLAN
// ----------------------------------------------------------------------------
//                              Overview
//                              --------
// This component provides an enumeration of file-system permission bits,
// two static predicates that validate an `int` bit mask against that
// enumeration, and standard streaming (`print` and `operator<<`) that emit
// the numeric value of a permission mask in octal notation without the
// customary leading `0` prefix.
// ----------------------------------------------------------------------------
// CLASS METHODS
// [ 2] bool FilePermissions::isValidBaseBits(int);
// [ 2] bool FilePermissions::isValid(int);
// [ 4] ostream& print(ostream& s, Enum val, int level = 0, int sPL = 4);
//
// FREE OPERATORS
// [ 3] operator<<(ostream& s, FilePermissions::Enum val);
// ----------------------------------------------------------------------------
// [ 1] BREATHING TEST: enumerator values
// [ 5] USAGE EXAMPLE

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

#define Q            BSLIM_TESTUTIL_Q
#define P            BSLIM_TESTUTIL_P
#define P_           BSLIM_TESTUTIL_P_
#define T_           BSLIM_TESTUTIL_T_
#define L_           BSLIM_TESTUTIL_L_

// ============================================================================
//                                MAIN PROGRAM
// ----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    typedef bdls::FilePermissions Obj;

    const int test    = argc > 1 ? bsl::atoi(argv[1]) : 0;
    const bool verbose = argc > 2;

    bsl::printf("TEST %s CASE %d\n", __FILE__, test);

    switch (test) { case 0:
      case 5: {
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

        if (verbose) bsl::puts("\nUSAGE EXAMPLE"
                               "\n=============");

        int perms = bdls::FilePermissions::k_OWNER_READ  |
                    bdls::FilePermissions::k_OWNER_WRITE |
                    bdls::FilePermissions::k_GROUP_READ;
        ASSERT(0640 == perms);

        ASSERT( bdls::FilePermissions::isValidBaseBits(0640));
        ASSERT(!bdls::FilePermissions::isValidBaseBits(
                                     0640 | bdls::FilePermissions::k_SET_UID));

        ASSERT( bdls::FilePermissions::isValid(
                                     0640 | bdls::FilePermissions::k_SET_UID));
        ASSERT(!bdls::FilePermissions::isValid(1 << 15));
      } break;
      case 4: {
        // --------------------------------------------------------------------
        // TESTING `print`
        //
        // Concerns:
        // 1. The `print` method writes the output to the specified stream.
        //
        // 2. The `print` method writes the numeric value of the mask in
        //    octal notation *without* the customary leading `0` prefix.
        //
        // 3. The `print` method honors the `level` and `spacesPerLevel`
        //    arguments in the same way as the BDE standard enumeration
        //    `print` method: leading indentation is `abs(level) *
        //    spacesPerLevel` spaces; a negative `level` suppresses the
        //    initial indentation; a negative `spacesPerLevel` suppresses the
        //    trailing newline (single-line format).
        //
        // 4. The default values of `level` (0) and `spacesPerLevel` (4)
        //    produce output identical to explicitly passing those values.
        //
        // 5. `print` does not leave the stream in octal (or any other
        //    non-default) formatting state.
        //
        // 6. There is no output when the stream is invalid.
        //
        // 7. The `print` method has the expected signature.
        //
        // Plan:
        // 1. Using a table-driven approach, verify that `print` produces the
        //    expected octal representation for a variety of permission
        //    masks (including named enumerators, common composite values,
        //    and boundary values) and `level`/`spacesPerLevel`
        //    combinations.  (C-1..3)
        //
        // 2. For rows where `level == 0` and `spacesPerLevel == 4`, repeat
        //    the check using default arguments.  (C-4)
        //
        // 3. After a `print` call, insert a decimal integer into the same
        //    stream and verify the value is emitted in decimal.  (C-5)
        //
        // 4. Set `badbit` on a stream and verify `print` writes nothing.
        //    (C-6)
        //
        // 5. Take the address of `print` and assign it to a variable of the
        //    appropriate function pointer type.  (C-7)
        //
        // Testing:
        //   ostream& print(ostream& s, Enum val, int level = 0, int sPL = 4);
        // --------------------------------------------------------------------

        if (verbose) bsl::puts("\nTESTING `print`"
                               "\n===============");

        typedef bdls::FilePermissions::Enum Enum;

        static const struct {
            int         d_line;   // source line number
            int         d_level;  // level
            int         d_spl;    // spaces per level
            Enum        d_value;  // permission mask
            const char *d_exp;    // expected output
        } DATA[] = {
#define NL "\n"
            //line  lvl  spl  value                      exp
            //----  ---  ---  -------------------------  -----------
            { L_,     0,   4,  Obj::k_NONE,              "0" NL       },
            { L_,     0,   4,  Obj::k_OWNER_READ,        "400" NL     },
            { L_,     0,   4,  Obj::k_OWNER_ALL,         "700" NL     },
            { L_,     0,   4,  Obj::k_ALL,               "777" NL     },
            { L_,     0,   4,  Obj::k_MASK,              "7777" NL    },
            { L_,     0,   4,  Obj::k_STICKY_BIT,        "1000" NL    },
            { L_,     0,   4,  static_cast<Enum>(0640),  "640" NL     },
            { L_,     0,   4,  static_cast<Enum>(0666),  "666" NL     },

            // Varying `level` and `spacesPerLevel` for a fixed value.
            { L_,     0,  -1,  static_cast<Enum>(0640),  "640"        },
            { L_,     0,   0,  static_cast<Enum>(0640),  "640" NL     },
            { L_,     0,   2,  static_cast<Enum>(0640),  "640" NL     },
            { L_,     1,   1,  static_cast<Enum>(0640),  " 640" NL    },
            { L_,     1,   2,  static_cast<Enum>(0640),  "  640" NL   },
            { L_,     2,   1,  static_cast<Enum>(0640),  "  640" NL   },
            { L_,     1,   3,  static_cast<Enum>(0640),  "   640" NL  },
            { L_,    -1,   2,  static_cast<Enum>(0640),  "640" NL     },
            { L_,    -2,   1,  static_cast<Enum>(0640),  "640" NL     },
#undef NL
        };
        const int NUM_DATA = sizeof DATA / sizeof *DATA;

        const int   SIZE = 128;         // big enough to hold output string
        const char  XX   = static_cast<char>(0xFF);  // value of unset `char`
              char  buf[SIZE];          // output buffer

              char  mCtrl[SIZE];  bsl::memset(mCtrl, XX, SIZE);
        const char *CTRL = mCtrl;

        for (int ti = 0; ti < NUM_DATA; ++ti) {
            const int   LINE  = DATA[ti].d_line;
            const int   LEVEL = DATA[ti].d_level;
            const int   SPL   = DATA[ti].d_spl;
            const Enum  VALUE = DATA[ti].d_value;
            const char *EXP   = DATA[ti].d_exp;

            bsl::memcpy(buf, CTRL, SIZE);  // preset buffer with sentinels

            bdlsb::FixedMemOutStreamBuf obuf(buf, sizeof buf);
            bsl::ostream                out(&obuf);
            Obj::print(out, VALUE, LEVEL, SPL) << bsl::ends;

            const bsl::size_t SZ = bsl::strlen(EXP) + 1;
            ASSERTV(LINE, ti, SZ,  SZ  < SIZE);
            ASSERTV(LINE, ti, XX == buf[SIZE - 1]);   // no overrun
            ASSERTV(LINE, ti, EXP, buf, 0 == bsl::memcmp(buf, EXP, SZ));
            ASSERTV(LINE, ti, 0 == bsl::memcmp(buf + SZ, CTRL + SZ,
                                                          SIZE - SZ));

            if (0 == LEVEL && 4 == SPL) {
                bsl::memcpy(buf, CTRL, SIZE);

                bdlsb::FixedMemOutStreamBuf obuf2(buf, sizeof buf);
                bsl::ostream                out2(&obuf2);
                Obj::print(out2, VALUE) << bsl::ends;

                ASSERTV(LINE, ti, XX == buf[SIZE - 1]);
                ASSERTV(LINE, ti, 0 == bsl::memcmp(buf, EXP, SZ));
                ASSERTV(LINE, ti, 0 == bsl::memcmp(buf + SZ, CTRL + SZ,
                                                              SIZE - SZ));
            }
        }

        if (verbose) bsl::puts("\n\t`print` restores stream format state.");
        {
            char                        buf2[SIZE];
            bsl::memcpy(buf2, CTRL, SIZE);
            bdlsb::FixedMemOutStreamBuf obuf(buf2, sizeof buf2);
            bsl::ostream                out(&obuf);

            // Print a mask (which internally switches the stream to octal),
            // then insert a decimal integer.  The trailing "10" must appear
            // in decimal, not octal (in which "10" would have been "12").
            Obj::print(out, Obj::k_OWNER_ALL, 0, -1);
            out << ':' << 10 << bsl::ends;

            ASSERT(0 == bsl::strcmp(buf2, "700:10"));
        }

        if (verbose)
            bsl::puts("\n\tNothing is written to a bad stream.");
        for (int ti = 0; ti < NUM_DATA; ++ti) {
            const int   LINE  = DATA[ti].d_line;
            const int   LEVEL = DATA[ti].d_level;
            const int   SPL   = DATA[ti].d_spl;
            const Enum  VALUE = DATA[ti].d_value;

            bsl::memcpy(buf, CTRL, SIZE);

            bdlsb::FixedMemOutStreamBuf obuf(buf, sizeof buf);
            bsl::ostream                out(&obuf);
            out.setstate(bsl::ios_base::badbit);
            Obj::print(out, VALUE, LEVEL, SPL);

            ASSERTV(LINE, ti, 0 == bsl::memcmp(buf, CTRL, SIZE));
        }

        if (verbose) bsl::puts("\n\tVerify `print` signature.");
        {
            typedef bsl::ostream& (*FuncPtr)(bsl::ostream&, Enum, int, int);

            const FuncPtr FP = &Obj::print;
            (void) FP;
        }
      } break;
      case 3: {
        // --------------------------------------------------------------------
        // TESTING `operator<<`
        //
        // Concerns:
        // 1. `operator<<` writes the output to the specified stream.
        //
        // 2. `operator<<` writes the numeric value of the mask in octal
        //    notation *without* the customary leading `0` prefix.
        //
        // 3. The output produced by `stream << value` is identical to that
        //    produced by `print(stream, value, 0, -1)` (i.e., single-line,
        //    no trailing newline, no indentation).
        //
        // 4. `operator<<` does not leave the stream in octal (or any other
        //    non-default) formatting state.
        //
        // 5. There is no output when the stream is invalid.
        //
        // 6. `operator<<` has the expected signature.
        //
        // Plan:
        // 1. Using a table of representative masks, verify `operator<<`
        //    produces the expected octal string.  (C-1, C-2)
        //
        // 2. For each mask verify that the output of `stream << value`
        //    matches the output of `print(stream, value, 0, -1)`.  (C-3)
        //
        // 3. After an `operator<<` call, insert a decimal integer into the
        //    same stream and verify the value is emitted in decimal.  (C-4)
        //
        // 4. Set `badbit` on a stream and verify `operator<<` writes
        //    nothing.  (C-5)
        //
        // 5. Take the address of `operator<<` and assign it to a variable of
        //    the appropriate function pointer type.  (C-6)
        //
        // Testing:
        //   operator<<(ostream& s, FilePermissions::Enum val);
        // --------------------------------------------------------------------

        if (verbose) bsl::puts("\nTESTING `operator<<`"
                               "\n====================");

        typedef bdls::FilePermissions::Enum Enum;

        static const struct {
            int         d_line;   // source line number
            Enum        d_value;  // permission mask
            const char *d_exp;    // expected output
        } DATA[] = {
            //line  value                                    expected
            //----  ---------------------------------------  --------
            { L_,   Obj::k_NONE,                             "0"       },
            { L_,   Obj::k_OWNER_READ,                       "400"     },
            { L_,   Obj::k_OWNER_WRITE,                      "200"     },
            { L_,   Obj::k_OWNER_EXEC,                       "100"     },
            { L_,   Obj::k_OWNER_ALL,                        "700"     },
            { L_,   Obj::k_GROUP_READ,                       "40"      },
            { L_,   Obj::k_GROUP_ALL,                        "70"      },
            { L_,   Obj::k_OTHERS_READ,                      "4"       },
            { L_,   Obj::k_OTHERS_ALL,                       "7"       },
            { L_,   Obj::k_ALL,                              "777"     },
            { L_,   Obj::k_SET_UID,                          "4000"    },
            { L_,   Obj::k_SET_GID,                          "2000"    },
            { L_,   Obj::k_STICKY_BIT,                       "1000"    },
            { L_,   Obj::k_MASK,                             "7777"    },
            { L_,   static_cast<Enum>(0640),                 "640"     },
            { L_,   static_cast<Enum>(0666),                 "666"     },
            { L_,   static_cast<Enum>(04755),                "4755"    },
        };
        const int NUM_DATA = sizeof DATA / sizeof *DATA;

        const int   SIZE = 128;
        const char  XX   = static_cast<char>(0xFF);
              char  buf[SIZE];

              char  mCtrl[SIZE];  bsl::memset(mCtrl, XX, SIZE);
        const char *CTRL = mCtrl;

        for (int ti = 0; ti < NUM_DATA; ++ti) {
            const int   LINE  = DATA[ti].d_line;
            const Enum  VALUE = DATA[ti].d_value;
            const char *EXP   = DATA[ti].d_exp;

            bsl::memcpy(buf, CTRL, SIZE);

            bdlsb::FixedMemOutStreamBuf obuf(buf, sizeof buf);
            bsl::ostream                out(&obuf);
            out << VALUE << bsl::ends;

            const bsl::size_t SZ = bsl::strlen(EXP) + 1;
            ASSERTV(LINE, ti, SZ,  SZ  < SIZE);
            ASSERTV(LINE, ti, XX == buf[SIZE - 1]);   // no overrun
            ASSERTV(LINE, ti, EXP, buf, 0 == bsl::memcmp(buf, EXP, SZ));
            ASSERTV(LINE, ti, 0 == bsl::memcmp(buf + SZ, CTRL + SZ,
                                                          SIZE - SZ));

            // `stream << value` matches `print(stream, value, 0, -1)`.
            char                        buf2[SIZE];
            bsl::memcpy(buf2, CTRL, SIZE);
            bdlsb::FixedMemOutStreamBuf obuf2(buf2, sizeof buf2);
            bsl::ostream                out2(&obuf2);
            Obj::print(out2, VALUE, 0, -1) << bsl::ends;

            ASSERTV(LINE, ti, 0 == bsl::memcmp(buf, buf2, SIZE));
        }

        if (verbose) bsl::puts("\n\t`operator<<` restores stream format.");
        {
            char                        buf2[SIZE];
            bsl::memcpy(buf2, CTRL, SIZE);
            bdlsb::FixedMemOutStreamBuf obuf(buf2, sizeof buf2);
            bsl::ostream                out(&obuf);

            out << Obj::k_OWNER_ALL << ':' << 10 << bsl::ends;

            ASSERT(0 == bsl::strcmp(buf2, "700:10"));
        }

        if (verbose)
            bsl::puts("\n\tNothing is written to a bad stream.");
        for (int ti = 0; ti < NUM_DATA; ++ti) {
            const int   LINE  = DATA[ti].d_line;
            const Enum  VALUE = DATA[ti].d_value;

            bsl::memcpy(buf, CTRL, SIZE);

            bdlsb::FixedMemOutStreamBuf obuf(buf, sizeof buf);
            bsl::ostream                out(&obuf);
            out.setstate(bsl::ios_base::badbit);
            out << VALUE;

            ASSERTV(LINE, ti, 0 == bsl::memcmp(buf, CTRL, SIZE));
        }

        if (verbose) bsl::puts("\n\tVerify `operator<<` signature.");
        {
            using namespace bdls;
            typedef bsl::ostream& (*FuncPtr)(bsl::ostream&, Enum);

            const FuncPtr FP = &operator<<;
            (void) FP;
        }
      } break;
      case 2: {
        // --------------------------------------------------------------------
        // TESTING `isValid` AND `isValidBaseBits`
        //
        // Concerns:
        // 1. `isValidBaseBits` returns `true` for every mask that is a
        //    combination of the nine base rwx bits (i.e., a subset of
        //    `k_ALL`), and `false` for every mask that has any bit outside
        //    that set, including negative values.
        //
        // 2. `isValid` returns `true` for every mask that is a combination of
        //    the twelve defined bits (i.e., a subset of `k_MASK`), and
        //    `false` for every mask that has any bit outside that set,
        //    including negative values.
        //
        // Plan:
        // 1. Verify boundary values (`k_NONE`, `k_ALL`, `k_MASK`) for both
        //    predicates.  (C-1, C-2)
        //
        // 2. For each defined enumerator, verify both predicates return the
        //    expected result.  `isValid` should always return `true`;
        //    `isValidBaseBits` should return `true` iff the enumerator is a
        //    subset of `k_ALL`.  (C-1, C-2)
        //
        // 3. Verify representative invalid masks (a bit above `k_MASK`, and
        //    `-1`) return `false` from both predicates.  (C-1, C-2)
        //
        // Testing:
        //   bool FilePermissions::isValidBaseBits(int);
        //   bool FilePermissions::isValid(int);
        // --------------------------------------------------------------------

        if (verbose) bsl::puts("\nTESTING `isValid` AND `isValidBaseBits`"
                               "\n=======================================");

        // Boundary values.
        ASSERT( Obj::isValidBaseBits(Obj::k_NONE));
        ASSERT( Obj::isValidBaseBits(Obj::k_ALL));
        ASSERT(!Obj::isValidBaseBits(Obj::k_MASK));

        ASSERT( Obj::isValid(Obj::k_NONE));
        ASSERT( Obj::isValid(Obj::k_ALL));
        ASSERT( Obj::isValid(Obj::k_MASK));

        // Every defined enumerator individually.
        static const struct Data {
            int  d_line;
            int  d_bit;
            bool d_isBaseBit;   // whether it is a base rwx bit
        } DATA[] = {
            //Line  bit                  base?
            //----  -------------------  -----
            { L_, Obj::k_NONE,           true  },
            { L_, Obj::k_OWNER_READ,     true  },
            { L_, Obj::k_OWNER_WRITE,    true  },
            { L_, Obj::k_OWNER_EXEC,     true  },
            { L_, Obj::k_OWNER_ALL,      true  },
            { L_, Obj::k_GROUP_READ,     true  },
            { L_, Obj::k_GROUP_WRITE,    true  },
            { L_, Obj::k_GROUP_EXEC,     true  },
            { L_, Obj::k_GROUP_ALL,      true  },
            { L_, Obj::k_OTHERS_READ,    true  },
            { L_, Obj::k_OTHERS_WRITE,   true  },
            { L_, Obj::k_OTHERS_EXEC,    true  },
            { L_, Obj::k_OTHERS_ALL,     true  },
            { L_, Obj::k_ALL,            true  },
            { L_, Obj::k_SET_UID,        false },
            { L_, Obj::k_SET_GID,        false },
            { L_, Obj::k_STICKY_BIT,     false },
        };
        const int NUM_DATA = sizeof DATA / sizeof *DATA;

        for (int ti = 0; ti < NUM_DATA; ++ti) {
            const int  LINE = DATA[ti].d_line;
            const int  BIT  = DATA[ti].d_bit;
            const bool BASE = DATA[ti].d_isBaseBit;

            ASSERTV(LINE, BIT, true == Obj::isValid(BIT));
            ASSERTV(LINE, BIT, BASE, BASE == Obj::isValidBaseBits(BIT));
        }

        // Representative octal masks users may pass.
        static const int VALID_BASE[] = { 0000, 0400, 0600, 0640, 0660, 0666,
                                          0700, 0770, 0777 };
        const int NUM_VALID_BASE = sizeof VALID_BASE / sizeof *VALID_BASE;
        for (int i = 0; i < NUM_VALID_BASE; ++i) {
            ASSERTV(VALID_BASE[i], true == Obj::isValid(VALID_BASE[i]));
            ASSERTV(VALID_BASE[i],
                    true == Obj::isValidBaseBits(VALID_BASE[i]));
        }

        // Masks that are valid overall but not base-only.
        static const int VALID_NOT_BASE[] = { 04000, 02000, 01000, 07000,
                                              04755, 07777 };
        const int NUM_VALID_NOT_BASE =
                            sizeof VALID_NOT_BASE / sizeof *VALID_NOT_BASE;
        for (int i = 0; i < NUM_VALID_NOT_BASE; ++i) {
            ASSERTV(VALID_NOT_BASE[i],
                    true == Obj::isValid(VALID_NOT_BASE[i]));
            ASSERTV(VALID_NOT_BASE[i],
                    false == Obj::isValidBaseBits(VALID_NOT_BASE[i]));
        }

        // Invalid masks (bits outside `k_MASK`, and negative values).
        static const int INVALID[] = { 010000, 0100000, 1 << 15, 1 << 30, -1 };
        const int NUM_INVALID = sizeof INVALID / sizeof *INVALID;
        for (int i = 0; i < NUM_INVALID; ++i) {
            ASSERTV(INVALID[i], false == Obj::isValid(INVALID[i]));
            ASSERTV(INVALID[i], false == Obj::isValidBaseBits(INVALID[i]));
        }
      } break;
      case 1: {
        // --------------------------------------------------------------------
        // BREATHING TEST
        //
        // Concerns:
        // 1. Every enumerator has the numeric value documented in the
        //    component header, matching the traditional Unix octal layout.
        //
        // 2. The aggregate enumerators (`k_OWNER_ALL`, `k_GROUP_ALL`,
        //    `k_OTHERS_ALL`, `k_ALL`, `k_MASK`) are the OR of the appropriate
        //    individual bits.
        //
        // Plan:
        // 1. Directly compare each enumerator to its documented octal value.
        //    (C-1, C-2)
        //
        // Testing:
        //   BREATHING TEST
        // --------------------------------------------------------------------

        if (verbose) bsl::puts("\nBREATHING TEST"
                               "\n==============");

        ASSERT(00000 == Obj::k_NONE);

        ASSERT(00400 == Obj::k_OWNER_READ);
        ASSERT(00200 == Obj::k_OWNER_WRITE);
        ASSERT(00100 == Obj::k_OWNER_EXEC);
        ASSERT(00700 == Obj::k_OWNER_ALL);
        ASSERT(Obj::k_OWNER_ALL ==
              (Obj::k_OWNER_READ | Obj::k_OWNER_WRITE | Obj::k_OWNER_EXEC));

        ASSERT(00040 == Obj::k_GROUP_READ);
        ASSERT(00020 == Obj::k_GROUP_WRITE);
        ASSERT(00010 == Obj::k_GROUP_EXEC);
        ASSERT(00070 == Obj::k_GROUP_ALL);
        ASSERT(Obj::k_GROUP_ALL ==
              (Obj::k_GROUP_READ | Obj::k_GROUP_WRITE | Obj::k_GROUP_EXEC));

        ASSERT(00004 == Obj::k_OTHERS_READ);
        ASSERT(00002 == Obj::k_OTHERS_WRITE);
        ASSERT(00001 == Obj::k_OTHERS_EXEC);
        ASSERT(00007 == Obj::k_OTHERS_ALL);
        ASSERT(Obj::k_OTHERS_ALL ==
             (Obj::k_OTHERS_READ | Obj::k_OTHERS_WRITE | Obj::k_OTHERS_EXEC));

        ASSERT(00777 == Obj::k_ALL);
        ASSERT(Obj::k_ALL ==
              (Obj::k_OWNER_ALL | Obj::k_GROUP_ALL | Obj::k_OTHERS_ALL));

        ASSERT(04000 == Obj::k_SET_UID);
        ASSERT(02000 == Obj::k_SET_GID);
        ASSERT(01000 == Obj::k_STICKY_BIT);

        ASSERT(07777 == Obj::k_MASK);
        ASSERT(Obj::k_MASK ==
              (Obj::k_ALL | Obj::k_SET_UID | Obj::k_SET_GID
                          | Obj::k_STICKY_BIT));
      } break;
      default: {
        bsl::fprintf(stderr, "WARNING: CASE `%d' NOT FOUND.\n", test);
        testStatus = -1;
      }
    }

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
