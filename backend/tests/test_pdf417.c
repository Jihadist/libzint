/*
    libzint - the open source barcode library
    Copyright (C) 2019 - 2021 Robin Stuart <rstuart114@gmail.com>

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name of the project nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
 */
/* vim: set ts=4 sw=4 et : */

#include "testcommon.h"

static void test_large(int index, int debug) {

    struct item {
        int symbology;
        int option_1;
        int option_2;
        int option_3;
        char *pattern;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
        char *expected_errtxt;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_PDF417, 0, -1, -1, "A", 1850, 0, 32, 562, "" },
        /*  1*/ { BARCODE_PDF417, 0, -1, -1, "A", 1851, ZINT_ERROR_TOO_LONG, -1, -1, "Error 464: Input string too long" },
        /*  2*/ { BARCODE_PDF417, 0, -1, -1, "\200", 1108, 0, 32, 562, "" },
        /*  3*/ { BARCODE_PDF417, 0, -1, -1, "\200", 1109, ZINT_ERROR_TOO_LONG, -1, -1, "Error 464: Input string too long" },
        /*  4*/ { BARCODE_PDF417, 0, -1, -1, "1", 2710, 0, 32, 562, "" },
        /*  5*/ { BARCODE_PDF417, 0, -1, -1, "1", 2711, ZINT_ERROR_TOO_LONG, -1, -1, "Error 463: Input string too long" },
        /*  6*/ { BARCODE_PDF417, 0, -1, 59, "A", 1850, ZINT_ERROR_TOO_LONG, -1, -1, "Error 465: Data too long for specified number of rows" },
        /*  7*/ { BARCODE_PDF417, 0, 1, 3, "A", 1850, ZINT_ERROR_TOO_LONG, 32, 562, "Error 745: Data too long for specified number of columns" },
        /*  8*/ { BARCODE_PDF417, 0, -1, 3, "A", 1850, ZINT_WARN_INVALID_OPTION, 32, 562, "Warning 746: Rows increased from 3 to 32" },
        /*  9*/ { BARCODE_PDF417, 0, 30, -1, "A", 1850, ZINT_ERROR_TOO_LONG, 32, 562, "Error 747: Data too long for specified number of columns" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char data_buf[4096];

    testStart("test_large");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        if (data[i].length != -1) {
            testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
            assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));
        } else {
            strcpy(data_buf, data[i].pattern);
        }

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, data[i].option_2, data[i].option_3, -1 /*output_options*/, data_buf, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data_buf, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, symbol->errtxt);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
        }
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected_errtxt);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_options(int index, int debug) {

    struct item {
        int symbology;
        int option_1;
        int option_2;
        int option_3;
        int warn_level;
        struct zint_structapp structapp;
        char *data;
        int ret_encode;
        int ret_vector;

        int expected_rows;
        int expected_width;
        const char *expected_errtxt;
        int compare_previous;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_PDF417, -1, -1, -1, 0, { 0, 0, "" }, "12345", 0, 0, 6, 103, "", -1 }, // ECC auto-set to 2, cols auto-set to 2
        /*  1*/ { BARCODE_PDF417, -1, -1, 928, 0, { 0, 0, "" }, "12345", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 466: Number of rows out of range (3 to 90)", -1 }, // Option 3 no longer ignored
        /*  2*/ { BARCODE_PDF417, -1, -1, 1, 0, { 0, 0, "" }, "12345", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 466: Number of rows out of range (3 to 90)", -1 }, // Option 3 no longer ignored
        /*  3*/ { BARCODE_PDF417, 3, -1, -1, 0, { 0, 0, "" }, "12345", 0, 0, 7, 120, "", -1 }, // ECC 3, cols auto-set to 3
        /*  4*/ { BARCODE_PDF417, 3, 2, -1, 0, { 0, 0, "" }, "12345", 0, 0, 10, 103, "", -1 }, // ECC 3, cols 2
        /*  5*/ { BARCODE_PDF417, 8, 2, -1, 0, { 0, 0, "" }, "12345", ZINT_WARN_INVALID_OPTION, 0, 86, 171, "Warning 748: Columns increased from 2 to 6", -1 }, // ECC 8, cols 2, used to fail, now auto-upped to 3 with warning
        /*  6*/ { BARCODE_PDF417, 8, 2, -1, WARN_FAIL_ALL, { 0, 0, "" }, "12345", ZINT_ERROR_INVALID_OPTION, 0, 86, 171, "Error 748: Columns increased from 2 to 6", -1 },
        /*  7*/ { BARCODE_PDF417, 7, 2, -1, 0, { 0, 0, "" }, "12345", ZINT_WARN_INVALID_OPTION, 0, 87, 120, "Warning 748: Columns increased from 2 to 3", -1 }, // ECC 7, cols 2 auto-upped to 3 but now with warning
        /*  8*/ { BARCODE_PDF417, 7, 2, -1, WARN_FAIL_ALL, { 0, 0, "" }, "12345", ZINT_ERROR_INVALID_OPTION, 0, 87, 120, "Error 748: Columns increased from 2 to 3", -1 },
        /*  9*/ { BARCODE_PDF417, -1, 10, -1, 0, { 0, 0, "" }, "12345", 0, 0, 3, 239, "", -1 }, // ECC auto-set to 2, cols 10
        /* 10*/ { BARCODE_PDF417, 9, -1, -1, 0, { 0, 0, "" }, "12345", ZINT_WARN_INVALID_OPTION, 0, 6, 103, "Warning 460: Security value out of range", -1 }, // Invalid ECC, auto-set
        /* 11*/ { BARCODE_PDF417, -1, 31, -1, 0, { 0, 0, "" }, "12345", ZINT_WARN_INVALID_OPTION, 0, 6, 103, "Warning 461: Number of columns out of range (1 to 30)", 0 }, // Invalid cols, auto-set
        /* 12*/ { BARCODE_PDF417, -1, -1, 2, 0, { 0, 0, "" }, "12345", ZINT_ERROR_INVALID_OPTION, 0, 0, 0, "Error 466: Number of rows out of range (3 to 90)", -1 }, // Invalid rows, error
        /* 13*/ { BARCODE_PDF417, -1, -1, 91, 0, { 0, 0, "" }, "12345", ZINT_ERROR_INVALID_OPTION, 0, 0, 0, "Error 466: Number of rows out of range (3 to 90)", -1 }, // Invalid rows, error
        /* 14*/ { BARCODE_PDF417, 9, -1, -1, WARN_FAIL_ALL, { 0, 0, "" }, "12345", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 460: Security value out of range", -1 }, // Invalid ECC
        /* 15*/ { BARCODE_PDF417, -1, 31, -1, WARN_FAIL_ALL, { 0, 0, "" }, "12345", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 461: Number of columns out of range (1 to 30)", -1 }, // Invalid cols
        /* 16*/ { BARCODE_PDF417, -1, 30, 31, 0, { 0, 0, "" }, "12345", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 475: Columns x rows out of range (1 to 928)", -1 }, // Rows * cols (930) > 928
        /* 17*/ { BARCODE_PDF417, -1, 1, -1, 0, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHI", ZINT_WARN_INVALID_OPTION, 0, 65, 120, "Warning 748: Columns increased from 1 to 3", -1 }, // Cols 1 too small, used to fail, now auto-upped to 3 with warning
        /* 18*/ { BARCODE_PDF417, -1, -1, 4, 0, { 0, 0, "" }, "12345", 0, 0, 4, 120, "", -1 }, // Specify rows 4 (cols 3)
        /* 19*/ { BARCODE_PDF417, -1, 3, 4, 0, { 0, 0, "" }, "12345", 0, 0, 4, 120, "", 0 }, // Specify cols 3 & rows 4
        /* 20*/ { BARCODE_PDF417, -1, -1, 90, 0, { 0, 0, "" }, "12345", 0, 0, 90, 86, "", -1 }, // Specify rows 90 (cols 1)
        /* 21*/ { BARCODE_PDF417, 0, -1, 3, 0, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQR", 0, 0, 3, 579, "", -1 }, // Specify rows 3, max cols 30
        /* 22*/ { BARCODE_PDF417, 0, 30, 3, 0, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQR", 0, 0, 3, 579, "", 0 }, // Specify rows 3, cols 30
        /* 23*/ { BARCODE_PDF417, 0, 29, 3, 0, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQR", ZINT_WARN_INVALID_OPTION, 0, 4, 562, "Warning 746: Rows increased from 3 to 4", -1 }, // Specify rows 3, cols 29, rows auto-upped to 4
        /* 24*/ { BARCODE_MICROPDF417, -1, 5, -1, 0, { 0, 0, "" }, "12345", ZINT_WARN_INVALID_OPTION, 0, 11, 38, "Warning 468: Specified width out of range", -1 }, // Invalid cols, auto-set to 1
        /* 25*/ { BARCODE_MICROPDF417, -1, 5, -1, WARN_FAIL_ALL, { 0, 0, "" }, "12345", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 468: Specified width out of range", -1 }, // Invalid cols
        /* 26*/ { BARCODE_MICROPDF417, -1, 5, 3, 0, { 0, 0, "" }, "12345", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 476: Cannot specify rows for MicroPDF417", -1 }, // Rows option not available
        /* 27*/ { BARCODE_MICROPDF417, -1, 1, -1, 0, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLM", ZINT_WARN_INVALID_OPTION, 0, 17, 55, "Warning 469: Specified symbol size too small for data", -1 }, // Cols 1 too small, auto-upped to 2 with warning
        /* 28*/ { BARCODE_MICROPDF417, -1, 1, -1, WARN_FAIL_ALL, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLM", ZINT_ERROR_INVALID_OPTION, 0, 0, 0, "Error 469: Specified symbol size too small for data", -1 }, // Cols 1 too small
        /* 29*/ { BARCODE_MICROPDF417, -1, 2, -1, 0, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWX", ZINT_WARN_INVALID_OPTION, 0, 15, 99, "Warning 470: Specified symbol size too small for data", -1 }, // Cols 2 too small, auto-upped to 4 with warning
        /* 30*/ { BARCODE_MICROPDF417, -1, 2, -1, WARN_FAIL_ALL, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWX", ZINT_ERROR_INVALID_OPTION, 0, 0, 0, "Error 470: Specified symbol size too small for data", -1 }, // Cols 2 too small
        /* 31*/ { BARCODE_MICROPDF417, -1, 3, -1, 0, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKL", ZINT_WARN_INVALID_OPTION, 0, 32, 99, "Warning 471: Specified symbol size too small for data", -1 }, // Cols 3 too small, auto-upped to 4 with warning
        /* 32*/ { BARCODE_MICROPDF417, -1, 3, -1, WARN_FAIL_ALL, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKL", ZINT_ERROR_INVALID_OPTION, 0, 0, 0, "Error 471: Specified symbol size too small for data", -1 }, // Cols 3 too small
        /* 33*/ { BARCODE_PDF417, -1, 1, -1, 0, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGH", ZINT_WARN_INVALID_OPTION, 0, 89, 103, "Warning 748: Columns increased from 1 to 2", -1 }, // Cols 1 auto-upped to 2 just fits, now with warning
        /* 34*/ { BARCODE_PDF417, -1, 1, -1, 0, { 1, 2, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGH", ZINT_WARN_INVALID_OPTION, 0, 67, 120, "Warning 748: Columns increased from 1 to 3", -1 }, // Cols 1 too small with Structured Append, used to fail, now auto-upped to 3 with warning
        /* 35*/ { BARCODE_PDF417, -1, 1, -1, 0, { 1, 2, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRST", ZINT_WARN_INVALID_OPTION, 0, 89, 103, "Warning 748: Columns increased from 1 to 2", -1 }, // Cols 1 with Structured Append auto-upped to 2 just fits, now with warning
        /* 36*/ { BARCODE_PDF417, -1, 1, -1, 0, { 2, 2, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTU", ZINT_WARN_INVALID_OPTION, 0, 65, 120, "Warning 748: Columns increased from 1 to 3", -1 }, // Cols 1 too small with Structured Append as last symbol (uses extra terminating codeword), used to fail, now auto-upped to 3 with warning
        /* 37*/ { BARCODE_PDF417, -1, 1, -1, 0, { 2, 2, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQR", ZINT_WARN_INVALID_OPTION, 0, 89, 103, "Warning 748: Columns increased from 1 to 2", -1 }, // Cols 1 with Structured Append as last symbol just fits with 1 less character pair when auto-upped to 2, now with warning
        /* 38*/ { BARCODE_PDF417, -1, 1, -1, 0, { 3, 2, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRST", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 741: Structured Append index out of range (1-2)", -1 },
        /* 39*/ { BARCODE_PDF417, -1, 1, -1, 0, { 1, 1, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRST", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 740: Structured Append count out of range (2-99999)", -1 },
        /* 40*/ { BARCODE_PDF417, -1, 1, -1, 0, { 1, 100000, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRST", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 740: Structured Append count out of range (2-99999)", -1 },
        /* 41*/ { BARCODE_PDF417, -1, 1, -1, 0, { 0, 2, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRST", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 741: Structured Append index out of range (1-2)", -1 },
        /* 42*/ { BARCODE_PDF417, -1, 1, -1, 0, { 1, 2, "1" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQR", ZINT_WARN_INVALID_OPTION, 0, 89, 103, "Warning 748: Columns increased from 1 to 2", -1 }, // Now with warning
        /* 43*/ { BARCODE_PDF417, -1, 1, -1, 0, { 1, 2, "123123" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOP", ZINT_WARN_INVALID_OPTION, 0, 89, 103, "Warning 748: Columns increased from 1 to 2", -1 }, // Now with warning
        /* 44*/ { BARCODE_PDF417, -1, 1, -1, 0, { 1, 2, "123123123123123123123123123123" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", ZINT_WARN_INVALID_OPTION, 0, 89, 103, "Warning 748: Columns increased from 1 to 2", -1 }, // Now with warning
        /* 45*/ { BARCODE_PDF417, -1, 1, -1, 0, { 1, 2, "1231231231231231231231231231231" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 742: Structured Append ID too long (30 digit maximum)", -1 },
        /* 46*/ { BARCODE_PDF417, -1, 1, -1, 0, { 1, 2, "23123123123123123123123123123" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", ZINT_WARN_INVALID_OPTION, 0, 89, 103, "Warning 748: Columns increased from 1 to 2", -1 }, // Now with warning
        /* 47*/ { BARCODE_PDF417, -1, 1, -1, 0, { 1, 2, "A" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQR", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 743: Invalid Structured Append ID (digits only)", -1 },
        /* 48*/ { BARCODE_PDF417, -1, 1, -1, 0, { 1, 2, "900" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQR", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 744: Structured Append ID triplet 1 '900' out of range (000-899)", -1 },
        /* 49*/ { BARCODE_PDF417, -1, 1, -1, 0, { 1, 2, "123123123123123123123123901123" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 744: Structured Append ID triplet 9 '901' out of range (000-899)", -1 },
        /* 50*/ { BARCODE_MICROPDF417, -1, -1, -1, 0, { 1, 2, "1231231231231231231231231231231" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGH", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 742: Structured Append ID too long (30 digit maximum)", -1 }, // Micro PDF417 same error checking code
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    struct zint_symbol previous_symbol;

    testStart("test_options");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1, data[i].option_1, data[i].option_2, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);
        if (data[i].warn_level) {
            symbol->warn_level = data[i].warn_level;
        }
        if (data[i].structapp.count) {
            symbol->structapp = data[i].structapp;
        }

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret_encode, symbol->errtxt);

        if (data[i].option_3 != -1) {
            assert_equal(symbol->option_3, data[i].option_3, "i:%d symbol->option_3 %d != %d\n", i, symbol->option_3, data[i].option_3); // Unchanged
        } else {
            assert_zero(symbol->option_3, "i:%d symbol->option_3 %d != 0\n", i, symbol->option_3);
        }

        assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, symbol->errtxt);
        assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, symbol->errtxt);
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected_errtxt);

        if (index == -1 && data[i].compare_previous != -1) {
            ret = testUtilSymbolCmp(symbol, &previous_symbol);
            assert_equal(!ret, !data[i].compare_previous, "i:%d testUtilSymbolCmp !ret %d != %d\n", i, ret, data[i].compare_previous);
        }
        memcpy(&previous_symbol, symbol, sizeof(previous_symbol));

        if (data[i].ret_vector != -1) {
            ret = ZBarcode_Buffer_Vector(symbol, 0);
            assert_equal(ret, data[i].ret_vector, "i:%d ZBarcode_Buffer_Vector ret %d != %d\n", i, ret, data[i].ret_vector);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_reader_init(int index, int generate, int debug) {

    struct item {
        int symbology;
        int input_mode;
        int output_options;
        char *data;
        int ret;
        int expected_rows;
        int expected_width;
        char *expected;
        char *comment;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_PDF417, UNICODE_MODE, READER_INIT, "A", 0, 6, 103, "(12) 4 921 900 29 60 257 719 198 75 123 199 98", "Outputs Test Alpha flag 900" },
        /*  1*/ { BARCODE_MICROPDF417, UNICODE_MODE, READER_INIT, "A", 0, 11, 38, "(11) 921 900 29 900 179 499 922 262 777 478 300", "Outputs Test Alpha flag 900" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];

    testStart("test_reader_init");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1 /*option_2*/, -1, data[i].output_options, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, %s, %s, \"%s\", %s, %d, %d, \"%s\", \"%s\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), testUtilOutputOptionsName(data[i].output_options),
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, symbol->errtxt, data[i].comment);
        } else {
            if (ret < ZINT_ERROR) {
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);
            }
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_input(int index, int generate, int debug) {

    struct item {
        int symbology;
        int input_mode;
        int eci;
        int option_1;
        int option_2;
        struct zint_structapp structapp;
        char *data;
        int ret;
        int expected_eci;
        int expected_rows;
        int expected_width;
        char *expected;
        char *comment;
    };
    // é U+00E9 (\351, 233), UTF-8 C3A9
    // β U+03B2 in ISO 8859-7 Greek (but not other ISO 8859 or Win page) (\342, 226), UTF-8 CEB2
    struct item data[] = {
        /*  0*/ { BARCODE_PDF417, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "é", 0, 0, 6, 103, "(12) 4 913 233 900 398 878 279 350 217 295 231 77", "" },
        /*  1*/ { BARCODE_PDF417, UNICODE_MODE, 3, -1, -1, { 0, 0, "" }, "é", 0, 3, 7, 103, "(14) 6 927 3 913 233 900 162 81 551 529 607 384 164 108", "" },
        /*  2*/ { BARCODE_PDF417, UNICODE_MODE, 26, -1, -1, { 0, 0, "" }, "é", 0, 26, 7, 103, "(14) 6 927 26 901 195 169 574 701 519 908 84 241 360 642", "" },
        /*  3*/ { BARCODE_PDF417, UNICODE_MODE, 9, -1, -1, { 0, 0, "" }, "β", 0, 9, 7, 103, "(14) 6 927 9 913 226 900 487 92 418 278 838 500 576 84", "" },
        /*  4*/ { BARCODE_PDF417, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "β", ZINT_WARN_USES_ECI, 9, 7, 103, "Warning (14) 6 927 9 913 226 900 487 92 418 278 838 500 576 84", "" },
        /*  5*/ { BARCODE_PDF417, UNICODE_MODE, 3, -1, -1, { 0, 0, "" }, "β", ZINT_ERROR_INVALID_DATA, 3, 0, 0, "Error 244: Invalid character in input data for ECI 3", "" },
        /*  6*/ { BARCODE_PDF417, UNICODE_MODE, 899, -1, -1, { 0, 0, "" }, "A", 0, 899, 7, 103, "(14) 6 927 899 900 29 900 727 69 915 482 371 771 641 35", "" },
        /*  7*/ { BARCODE_PDF417, UNICODE_MODE, 900, -1, -1, { 0, 0, "" }, "A", 0, 900, 7, 103, "(14) 6 926 0 0 900 29 56 795 921 763 468 267 410 129", "" },
        /*  8*/ { BARCODE_PDF417, UNICODE_MODE, 810899, -1, -1, { 0, 0, "" }, "A", 0, 810899, 7, 103, "(14) 6 926 899 899 900 29 847 901 749 718 89 792 660 273", "" },
        /*  9*/ { BARCODE_PDF417, UNICODE_MODE, 810900, -1, -1, { 0, 0, "" }, "A", 0, 810900, 7, 103, "(14) 6 925 0 900 29 900 652 613 857 390 38 450 415 899", "" },
        /* 10*/ { BARCODE_PDF417, UNICODE_MODE, 811799, -1, -1, { 0, 0, "" }, "A", 0, 811799, 7, 103, "(14) 6 925 899 900 29 900 456 300 328 160 510 753 157 159", "" },
        /* 11*/ { BARCODE_PDF417, UNICODE_MODE, 811800, -1, -1, { 0, 0, "" }, "A", ZINT_ERROR_INVALID_OPTION, 811800, 0, 0, "Error 472: Invalid ECI", "" },
        /* 12*/ { BARCODE_MICROPDF417, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "é", 0, 0, 11, 38, "(11) 913 233 900 900 805 609 847 211 598 4 603", "" },
        /* 13*/ { BARCODE_MICROPDF417, UNICODE_MODE, 3, -1, -1, { 0, 0, "" }, "é", 0, 3, 11, 38, "(11) 927 3 913 233 803 477 85 249 824 813 830", "" },
        /* 14*/ { BARCODE_MICROPDF417, UNICODE_MODE, 26, -1, -1, { 0, 0, "" }, "é", 0, 26, 6, 82, "(18) 927 26 901 195 169 900 288 96 509 365 709 784 713 403 219 81 851 866", "" },
        /* 15*/ { BARCODE_MICROPDF417, UNICODE_MODE, 9, -1, -1, { 0, 0, "" }, "β", 0, 9, 11, 38, "(11) 927 9 913 226 23 103 74 194 394 667 324", "" },
        /* 16*/ { BARCODE_MICROPDF417, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "β", ZINT_WARN_USES_ECI, 9, 11, 38, "Warning (11) 927 9 913 226 23 103 74 194 394 667 324", "" },
        /* 17*/ { BARCODE_MICROPDF417, UNICODE_MODE, 3, -1, -1, { 0, 0, "" }, "β", ZINT_ERROR_INVALID_DATA, 3, 0, 0, "Error 244: Invalid character in input data for ECI 3", "" },
        /* 18*/ { BARCODE_MICROPDF417, UNICODE_MODE, 899, -1, -1, { 0, 0, "" }, "A", 0, 899, 11, 38, "(11) 927 899 900 29 533 437 884 3 617 241 747", "" },
        /* 19*/ { BARCODE_MICROPDF417, UNICODE_MODE, 900, -1, -1, { 0, 0, "" }, "A", 0, 900, 6, 82, "(18) 926 0 0 900 29 900 913 543 414 141 214 886 461 1 419 422 54 495", "" },
        /* 20*/ { BARCODE_MICROPDF417, UNICODE_MODE, 810899, -1, -1, { 0, 0, "" }, "A", 0, 810899, 6, 82, "(18) 926 899 899 900 29 900 351 555 241 509 787 583 3 326 41 628 534 151", "" },
        /* 21*/ { BARCODE_MICROPDF417, UNICODE_MODE, 810900, -1, -1, { 0, 0, "" }, "A", 0, 810900, 11, 38, "(11) 925 0 900 29 233 533 43 483 708 659 704", "" },
        /* 22*/ { BARCODE_MICROPDF417, UNICODE_MODE, 811800, -1, -1, { 0, 0, "" }, "A", ZINT_ERROR_INVALID_OPTION, 811800, 0, 0, "Error 472: Invalid ECI", "" },
        /* 23*/ { BARCODE_HIBC_PDF, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, ",", ZINT_ERROR_INVALID_DATA, 0, 0, 0, "Error 203: Invalid character in data (alphanumerics, space and \"-.$/+%\" only)", "" },
        /* 24*/ { BARCODE_HIBC_MICPDF, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, ",", ZINT_ERROR_INVALID_DATA, 0, 0, 0, "Error 203: Invalid character in data (alphanumerics, space and \"-.$/+%\" only)", "" },
        /* 25*/ { BARCODE_PDF417, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "AB{}  C#+  de{}  {}F  12{}  G{}  H", 0, 0, 12, 120, "(36) 28 1 865 807 896 782 855 626 807 94 865 807 896 808 776 839 176 808 32 776 839 806 208", "" },
        /* 26*/ { BARCODE_PDF417, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "{}  #+ de{}  12{}  {}  H", 0, 0, 10, 120, "(30) 22 865 807 896 808 470 807 94 865 807 896 808 32 776 839 806 865 807 896 787 900 900", "" },
        /* 27*/ { BARCODE_PDF417, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "A", 0, 0, 5, 103, "(10) 2 29 478 509 903 637 74 490 760 21", "" },
        /* 28*/ { BARCODE_PDF417, UNICODE_MODE, -1, 0, -1, { 0, 0, "" }, "A", 0, 0, 4, 86, "(4) 2 29 347 502", "" },
        /* 29*/ { BARCODE_PDF417, UNICODE_MODE, -1, 1, -1, { 0, 0, "" }, "A", 0, 0, 6, 86, "(6) 2 29 752 533 551 139", "" },
        /* 30*/ { BARCODE_PDF417, UNICODE_MODE, -1, 2, -1, { 0, 0, "" }, "A", 0, 0, 5, 103, "(10) 2 29 478 509 903 637 74 490 760 21", "" },
        /* 31*/ { BARCODE_PDF417, UNICODE_MODE, -1, 3, -1, { 0, 0, "" }, "A", 0, 0, 9, 103, "(18) 2 29 290 888 64 789 390 182 22 197 347 41 298 467 387 917 455 196", "" },
        /* 32*/ { BARCODE_PDF417, UNICODE_MODE, -1, 4, -1, { 0, 0, "" }, "A", 0, 0, 12, 120, "(36) 4 29 900 900 702 212 753 721 695 584 222 459 110 594 813 465 718 912 667 349 852 602", "" },
        /* 33*/ { BARCODE_PDF417, UNICODE_MODE, -1, 5, -1, { 0, 0, "" }, "A", 0, 0, 14, 154, "(70) 6 29 900 900 900 900 774 599 527 418 850 374 921 763 922 772 572 661 584 902 578 696", "" },
        /* 34*/ { BARCODE_PDF417, UNICODE_MODE, -1, 6, -1, { 0, 0, "" }, "A", 0, 0, 19, 188, "(133) 5 29 900 900 900 113 261 822 368 600 652 404 869 860 902 184 702 611 323 195 794 566", "" },
        /* 35*/ { BARCODE_PDF417, UNICODE_MODE, -1, 7, -1, { 0, 0, "" }, "A", 0, 0, 29, 222, "(261) 5 29 900 900 900 384 614 456 20 422 177 78 492 215 859 765 864 755 572 621 891 97 538", "" },
        /* 36*/ { BARCODE_PDF417, UNICODE_MODE, -1, 8, -1, { 0, 0, "" }, "A", 0, 0, 40, 290, "(520) 8 29 900 900 900 900 900 900 255 576 871 499 885 500 866 196 784 681 589 448 428 108", "" },
        /* 37*/ { BARCODE_PDF417, UNICODE_MODE, -1, 8, -1, { 1, 4, "017053" }, "A", 0, 0, 41, 290, "(533) 21 29 900 900 900 900 900 900 900 900 900 900 928 111 100 17 53 923 1 111 104 903 71", "H.4 example" },
        /* 38*/ { BARCODE_PDF417, UNICODE_MODE, -1, 8, -1, { 4, 4, "017053" }, "A", 0, 0, 41, 290, "(533) 21 29 900 900 900 900 900 900 900 900 900 928 111 103 17 53 923 1 111 104 922 772 754", "H.4 example last segment" },
        /* 39*/ { BARCODE_PDF417, UNICODE_MODE, -1, 8, -1, { 2, 4, "" }, "A", 0, 0, 41, 290, "(533) 21 29 900 900 900 900 900 900 900 900 900 900 900 900 928 111 101 923 1 111 104 583", "No ID" },
        /* 40*/ { BARCODE_PDF417, UNICODE_MODE, -1, 8, -1, { 99998, 99999, "12345" }, "A", 0, 0, 41, 290, "(533) 21 29 900 900 900 900 900 900 900 900 900 900 928 222 197 123 45 923 1 222 199 198", "IDs '123', '045'" },
        /* 41*/ { BARCODE_MICROPDF417, UNICODE_MODE, -1, -1, -1, { 1, 4, "017053" }, "A", 0, 0, 6, 99, "(24) 900 29 900 928 111 100 17 53 923 1 111 104 430 136 328 218 796 853 32 421 712 477 363", "H.4 example" },
        /* 42*/ { BARCODE_MICROPDF417, UNICODE_MODE, -1, -1, -1, { 4, 4, "017053" }, "A", 0, 0, 6, 99, "(24) 900 29 928 111 103 17 53 923 1 111 104 922 837 837 774 835 701 445 926 428 285 851 334", "H.4 example last segment" },
        /* 43*/ { BARCODE_MICROPDF417, UNICODE_MODE, -1, -1, -1, { 3, 4, "" }, "A", 0, 0, 17, 38, "(17) 900 29 900 928 111 102 923 1 111 104 343 717 634 693 618 860 618", "No ID" },
        /* 44*/ { BARCODE_MICROPDF417, UNICODE_MODE, -1, -1, -1, { 99999, 99999, "100200300" }, "A", 0, 0, 11, 55, "(22) 900 29 928 222 198 100 200 300 923 1 222 199 922 693 699 895 719 637 154 478 399 638", "IDs '100', '200', '300'" },
        /* 45*/ { BARCODE_PDF417, DATA_MODE, -1, -1, -1, { 0, 0, "" }, "123456", 0, 0, 7, 103, "(14) 6 902 1 348 256 900 759 577 359 263 64 409 852 154", "" },
        /* 46*/ { BARCODE_PDF417, DATA_MODE, -1, -1, -1, { 0, 0, "" }, "12345678901234567890", 0, 0, 9, 103, "(18) 10 902 211 358 354 304 269 753 190 900 327 902 163 367 231 586 808 731", "" },
        /* 47*/ { BARCODE_PDF417, DATA_MODE, -1, -1, -1, { 0, 0, "" }, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, 0, 12, 137, "(48) 40 902 491 81 137 450 302 67 15 174 492 862 667 475 869 12 434 685 326 422 57 117 339", "" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];

    testStart("test_input");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, data[i].eci, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);
        if (data[i].structapp.count) {
            symbol->structapp = data[i].structapp;
        }

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, %s, %d, %d, %d, { %d, %d, \"%s\" }, \"%s\", %s, %d, %d, %d, \"%s\", \"%s\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), data[i].eci, data[i].option_1, data[i].option_2,
                    data[i].structapp.index, data[i].structapp.count, data[i].structapp.id,
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret),
                    symbol->eci, symbol->rows, symbol->width, symbol->errtxt, data[i].comment);
        } else {
            if (ret < ZINT_ERROR) {
                assert_equal(symbol->eci, data[i].expected_eci, "i:%d symbol->eci %d != %d (%s)\n", i, symbol->eci, data[i].expected_eci, data[i].data);
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);
            }
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_encode(int index, int generate, int debug) {

    struct item {
        int symbology;
        int eci;
        int input_mode;
        int option_1;
        int option_2;
        int option_3;
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
        int bwipp_cmp;
        char *comment;
        char *expected;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_PDF417, -1, UNICODE_MODE, 1, 2, -1, "PDF417 Symbology Standard", 0, 10, 103, 0, "ISO 15438:2015 Figure 1, same, BWIPP uses different encodation, same codeword count",
                    "1111111101010100011101010011100000111010110011110001110111011001100011110101011110000111111101000101001"
                    "1111111101010100011111010100110000110100001110001001111010001010000011111010100110000111111101000101001"
                    "1111111101010100011101010111111000101100110111100001110111111000101011010100111110000111111101000101001"
                    "1111111101010100010101111101111100100000100100000101000101000100000010101111001111000111111101000101001"
                    "1111111101010100011010111000100000111100100000101001001000011111011011010111000100000111111101000101001"
                    "1111111101010100011110101111010000100111100001010001100111110010010011110101111001000111111101000101001"
                    "1111111101010100010100111001110000101110001110100001111001101000111011010011101111000111111101000101001"
                    "1111111101010100011010111111011110111101011001100001010011111101110011010111111011110111111101000101001"
                    "1111111101010100011010011011111100110000101001111101101111100010001010100110011111000111111101000101001"
                    "1111111101010100010100011000001100100010111101111001100011100011001011010001100011100111111101000101001"
                },
        /*  1*/ { BARCODE_PDF417, -1, UNICODE_MODE, 1, 2, -1, "PDF417", 0, 5, 103, 1, "ISO 15438:2015 Annex Q example for generating ECC",
                    "1111111101010100011110101011110000110101000110000001110111011001100011110101011110000111111101000101001"
                    "1111111101010100011111101010011100110100001110001001111010001010000011111101010111000111111101000101001"
                    "1111111101010100011101010111111000101100110011110001100011111001001011101010011111100111111101000101001"
                    "1111111101010100010101111001111000101011101110000001100001101000100010101111001111000111111101000101001"
                    "1111111101010100011101011100011000100001101011111101111110110001011011101011100110000111111101000101001"
                },
        /*  2*/ { BARCODE_PDF417, -1, UNICODE_MODE, 0, 1, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ ", 0, 17, 86, 1, "Text Compaction Alpha",
                    "11111111010101000111110101001111101101011001110000011101010111000000111111101000101001"
                    "11111111010101000111111010101110001111110101011100011110101000100000111111101000101001"
                    "11111111010101000110101011111000001010011001111100011101010111111000111111101000101001"
                    "11111111010101000111010010111000001101000001011000011010111101111100111111101000101001"
                    "11111111010101000111010111001100001010000111110011011110101110001110111111101000101001"
                    "11111111010101000111110101110000101011001110011111011110101111010000111111101000101001"
                    "11111111010101000101001110001110001100100001101110010100111011100000111111101000101001"
                    "11111111010101000101011111000011001101100111100100010101111110001110111111101000101001"
                    "11111111010101000101001101111100001001100001000111011010011011111100111111101000101001"
                    "11111111010101000110100011101111001001100111100111010100011000110000111111101000101001"
                    "11111111010101000111010011101100001001100101111110011110100111001110111111101000101001"
                    "11111111010101000110100010011111001000001010111100010100010001111000111111101000101001"
                    "11111111010101000110100000101100001000110011001000010100001100001100111111101000101001"
                    "11111111010101000111101000101000001110001100111010011111010001001100111111101000101001"
                    "11111111010101000111101000011110101100011110010011011101000011111010111111101000101001"
                    "11111111010101000110010110111000001100011000100001011100101000111000111111101000101001"
                    "11111111010101000101000111100100001110000101100010010100011110000100111111101000101001"
                },
        /*  3*/ { BARCODE_PDF417, -1, UNICODE_MODE, 1, 1, -1, "abcdefghijklmnopqrstuvwxyz ", 0, 19, 86, 1, "Text Compaction Lower",
                    "11111111010101000110101000110000001101011001110000011101010111000000111111101000101001"
                    "11111111010101000111110101001100001100000101110010011111010100011000111111101000101001"
                    "11111111010101000110101011111000001111101011110110011010100111110000111111101000101001"
                    "11111111010101000111101001011110001010001110111000011010111101111100111111101000101001"
                    "11111111010101000110101110001000001010001111000001011010111000010000111111101000101001"
                    "11111111010101000111110101110000101110010011111001011110101111001000111111101000101001"
                    "11111111010101000110100111000111101011001111000111010100111011100000111111101000101001"
                    "11111111010101000110101111110111101111101100100100011110100101000000111111101000101001"
                    "11111111010101000101001101111100001011100100000011010100110011111000111111101000101001"
                    "11111111010101000101000111001110001100010001001100010100011000110000111111101000101001"
                    "11111111010101000110100111001000001110001001110011011010011100010000111111101000101001"
                    "11111111010101000110100010011111001101111000100110010100010000111100111111101000101001"
                    "11111111010101000111010000010111001000110111101110010100001100001100111111101000101001"
                    "11111111010101000111111010001011101100011011110010011110100010001000111111101000101001"
                    "11111111010101000111101000011110101000001100101110011010000010111110111111101000101001"
                    "11111111010101000111001011011110001000010010000001011100101000111000111111101000101001"
                    "11111111010101000101000111100010001111010000101000010100011110000010111111101000101001"
                    "11111111010101000111111001011101101010000001001111010010111001111110111111101000101001"
                    "11111111010101000111011010000110001000100111001110011110110100111000111111101000101001"
                },
        /*  4*/ { BARCODE_PDF417, -1, UNICODE_MODE, 2, 2, -1, "abcdefgABCDEFG", 0, 9, 103, 1, "Text Compaction Lower Alpha",
                    "1111111101010100011111010101111100110101000001100001000001010000010011110101011110000111111101000101001"
                    "1111111101010100011110101000010000111101011100111001110100111001100011110101001000000111111101000101001"
                    "1111111101010100011101010111111000111110010111101101000001110100110010101000011110000111111101000101001"
                    "1111111101010100011010111100111110111101010111100001010011100111000010101111001111000111111101000101001"
                    "1111111101010100011010111000001000111110100010011001101000000111001011110101110011100111111101000101001"
                    "1111111101010100011110101111010000100111111001110101011111000011010011110101111101100111111101000101001"
                    "1111111101010100011101001110111110110010000101100001001110111101100011010011101111000111111101000101001"
                    "1111111101010100011111101001011100111111011010110001011100111111010010101111110111000111111101000101001"
                    "1111111101010100011010011011111100100011101100011101010111011111100011111010011101000111111101000101001"
                },
        /*  5*/ { BARCODE_PDF417, -1, UNICODE_MODE, 1, 4, -1, "0123456&\015\011,:#-.$/+%*=^ 789", 0, 5, 137, 1, "Text Compaction Mixed",
                    "11111111010101000111101010111100001110101100111100010000110111001100110101111001111101010001110111000011101010011100000111111101000101001"
                    "11111111010101000111111010100111001010001111000001011101101111001100110110011110010001110010000011010011111101010111000111111101000101001"
                    "11111111010101000110101001111100001100111010000111011011110010110000100000101011110001101111101010000011101010011111100111111101000101001"
                    "11111111010101000101011110011110001000010000100001010010011000011000110010000100110001000011000110010010101111101111100111111101000101001"
                    "11111111010101000111010111000110001001111001001111010000101111101100100011110010111101001111110110111011101011100110000111111101000101001"
                },
        /*  6*/ { BARCODE_PDF417, -1, UNICODE_MODE, 3, 2, -1, ";<>@[\\]_'~!\015\011,:\012-.$/\"|*()?{", 0, 16, 103, 1, "Text Compaction Punctuation",
                    "1111111101010100011111010100111110111010110011110001000111011100100011110101011110000111111101000101001"
                    "1111111101010100011111010100001100111111010101110001101011111101111011110101000100000111111101000101001"
                    "1111111101010100011101010111111000101000001000111101011011001111000011010100001111100111111101000101001"
                    "1111111101010100011101001011100000110000110010100001100100001101110010101111001111000111111101000101001"
                    "1111111101010100011101011100000110110110011110010001110010000011010011110101110001110111111101000101001"
                    "1111111101010100011110101111010000110011101000011101101111001011000011101011111001000111111101000101001"
                    "1111111101010100010100111000111000110001101000100001000110011001000011010011101111000111111101000101001"
                    "1111111101010100011110100100100000111000110011101001110000010111011010101111110001110111111101000101001"
                    "1111111101010100011010011011111100110011111010100001001111000010010011111101001110110111111101000101001"
                    "1111111101010100011010001110111100110101000110000001100011010010000011010001100011100111111101000101001"
                    "1111111101010100011101001110000110110100111110011101001000011110100011110100111001110111111101000101001"
                    "1111111101010100010100010001111000110010011011111101000101000001111010100011001111100111111101000101001"
                    "1111111101010100011010000010110000110001011001110001100100010011000010100001100000110111111101000101001"
                    "1111111101010100011110100010000010110000010001110101111010011000110011111010001001100111111101000101001"
                    "1111111101010100011101000011111010111111010001101001011000010011100010010101111000000111111101000101001"
                    "1111111101010100011001011011100000110011001100001101100100101100000011110010100011110111111101000101001"
                },
        /*  7*/ { BARCODE_PDF417, -1, UNICODE_MODE, 4, 2, -1, "\015\015\015\015\010\015", 0, 20, 103, 0, "Text Compaction Punctuation 1 Mixed -> Text Byte; BWIPP uses Byte only",
                    "1111111101010100011010100011000000110101000011000001110001110110110011110101011110000111111101000101001"
                    "1111111101010100011110101101100000101100101111110001111001000110110011111010100011000111111101000101001"
                    "1111111101010100011101010111111000110011111101100101010100001111000011010110111111000111111101000101001"
                    "1111111101010100011110100101111000111010110111100001000011000110010010101111001111000111111101000101001"
                    "1111111101010100011110101111011110111001011100110001010011110010000011010111000010000111111101000101001"
                    "1111111101010100011110101111010000110011111001000101011100100110000011010111111010000111111101000101001"
                    "1111111101010100011010011100011110110010001100011101000100111000111011010011101111000111111101000101001"
                    "1111111101010100011111010010001100111000110011101001111000100001010011110100101000000111111101000101001"
                    "1111111101010100011010011011111100110001110010011101000001111010100011111010011100010111111101000101001"
                    "1111111101010100010100011100111000100001101100000101101000110011100011010001100011100111111101000101001"
                    "1111111101010100011010011110110000110010000111000101110100110000100011010011100010000111111101000101001"
                    "1111111101010100010100010001111000100000101101111101000111010000110011111010001110010111111101000101001"
                    "1111111101010100011101000001011100111100110100111001011110001111001010100001100000110111111101000101001"
                    "1111111101010100011101000110010000111000001101110101101011110000011011110100010001000111111101000101001"
                    "1111111101010100011101000011111010101101000111000001101111000001011011001010011111000111111101000101001"
                    "1111111101010100011100101101111000100001100110010001010011000110000011110010100011110111111101000101001"
                    "1111111101010100011111010000101100111000001011101101000101111000001010100011110000010111111101000101001"
                    "1111111101010100010010111011111100111000110101111101101110100011100011001011111101000111111101000101001"
                    "1111111101010100011101101000011000100000100011001101011001111101111011111011010011110111111101000101001"
                    "1111111101010100011110100000110110111110111011001001111100001010011010100001111101100111111101000101001"
                },
        /*  8*/ { BARCODE_PDF417, -1, UNICODE_MODE, 4, 3, -1, "??????ABCDEFG??????abcdef??????%%%%%%", 0, 19, 120, 1, "Text Compaction Punctuation Alpha Punctuation Lower Punctuation Mixed",
                    "111111110101010001101010001100000011010111001111000100011101110010001100111000110010011111010101111100111111101000101001"
                    "111111110101010001111010100000010011111001110011010111110011100110101101111100101111011111010100011000111111101000101001"
                    "111111110101010001010100111100000011111010111101100101000100000111101111100101111011010101101111100000111111101000101001"
                    "111111110101010001111010010111100010001110111001000110011100011001001100111000110010011010111100111110111111101000101001"
                    "111111110101010001110101111011100011111001110011010100111110100011101111110101011100011010111000010000111111101000101001"
                    "111111110101010001111101011110110010100110011111000101000001000111101000000110100111011101011111000010111111101000101001"
                    "111111110101010001101001110001111011001110001100100110011100011001001100111000110010011101001110111110111111101000101001"
                    "111111110101010001111010010001000010001111001011110111100010000010101111000100000101011110100101000000111111101000101001"
                    "111111110101010001111110100110010010001110110001110100001001011110001001111001101100010100111001111110111111101000101001"
                    "111111110101010001010001110011100011000110110000110110001011100111101100111001100001010100011000011000111111101000101001"
                    "111111110101010001010011110100000011011110111111010100000101111000101110110111001000011010011100010000111111101000101001"
                    "111111110101010001101000100011111011000101101111110110111110010001001011010011100000010100011101111110111111101000101001"
                    "111111110101010001110100000101110010010001000000100100000100110110001100110001010000011010000111011110111111101000101001"
                    "111111110101010001111101000110111010011110001011110111110101000011001111001001000010011110100010001000111111101000101001"
                    "111111110101010001010000010111100010111100010001000111001001111101001110011111101010010010100111100000111111101000101001"
                    "111111110101010001110010110111100010100011000110000110001110001100101000010010010000010010100001000000111111101000101001"
                    "111111110101010001111010000101000010000010111101000111010001101000001111000001001001010100011110000010111111101000101001"
                    "111111110101010001111100101110010010011100000100110100111010000110001100110100001111011100101111100010111111101000101001"
                    "111111110101010001110110100001100010011001111001110101111011110001001011011100011000011011010001000000111111101000101001"
                },
        /*  9*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, ";;;;;é;;;;;", 0, 7, 120, 0, "BWIPP different encodation",
                    "111111110101010001111101010111110011101011011110000100011101110010001110101011100000011111010101111100111111101000101001"
                    "111111110101010001111101010001100011111010101100000111110101110111101011111100100011011110101001000000111111101000101001"
                    "111111110101010001010100111100000010110010000001110110001111100100101000000110100111011010100011111000111111101000101001"
                    "111111110101010001101011110011111011101010111000000111010101110000001010111101111000011010111100111110111111101000101001"
                    "111111110101010001101011100001000010111111010110000111000011011101001100001011110110011110101110011100111111101000101001"
                    "111111110101010001111101011110110010111000110001110111011111100100101111101001111011011110101111000010111111101000101001"
                    "111111110101010001110100111011111010110011001000000100001100110001001001110011100100011101001110111110111111101000101001"
                },
        /* 10*/ { BARCODE_PDF417, -1, UNICODE_MODE, 2, 3, -1, "12345678901234", 0, 5, 120, 1, "Numeric Compaction",
                    "111111110101010001111010101111000011101010001110000100111101111010001001011100001110011111010101111100111111101000101001"
                    "111111110101010001111110101000111011010000001110010111111011010011001111010100000010011111101010111000111111101000101001"
                    "111111110101010001010100111100000010111000110011100101110011000011101110001111110101011101010001111110111111101000101001"
                    "111111110101010001010111100111100010001100001100010100001100011101101110101100111100011010111100111110111111101000101001"
                    "111111110101010001110101110000110011000000101110010110001001110000101011001000111111011101011100110000111111101000101001"
                },
        /* 11*/ { BARCODE_PDF417, -1, UNICODE_MODE, 2, 3, -1, "1234567890123456789012345678901234567890123", 0, 9, 120, 1, "Numeric Compaction 43 consecutive",
                    "111111110101010001111101010111110011010110001110000100111101111010001101001101110000011111010101111100111111101000101001"
                    "111111110101010001111010100001000011010011100001000110100111101100001110000101100001011110101001000000111111101000101001"
                    "111111110101010001010100111100000011111010111000010110010010011111001000011010000111010101000011110000111111101000101001"
                    "111111110101010001101011110011111011100010001001110110011011000110001001000110011000011010111100111110111111101000101001"
                    "111111110101010001101011100000100010010111101000000111000010111011001001101111101000011110101110011100111111101000101001"
                    "111111110101010001111101011110110010001111001000100110110010111100001100011111001001011110101111101100111111101000101001"
                    "111111110101010001110100111011111010000110001100100110010001110111101001001110111000011101001110111110111111101000101001"
                    "111111110101010001111110100101110010100111100001000110000101111001101110010110000100010101111110111000111111101000101001"
                    "111111110101010001111110100110010011100100111110100100111110011000101001111000010001011111010011101000111111101000101001"
                },
        /* 12*/ { BARCODE_PDF417, -1, UNICODE_MODE, 2, 3, -1, "12345678901234567890123456789012345678901234", 0, 9, 120, 1, "Numeric Compaction 44 consecutive",
                    "111111110101010001111101010111110011010110001110000100111101111010001000100011000011011111010101111100111111101000101001"
                    "111111110101010001111010100001000011101001100100000111010001100001001110010000001101011110101001000000111111101000101001"
                    "111111110101010001010100111100000011111100010110100101001100001111101010110011111000010101000011110000111111101000101001"
                    "111111110101010001101011110011111010010111100111100100110011100001101000111011101000011010111100111110111111101000101001"
                    "111111110101010001101011100000100011111000010100110110001011100100001011111001011100011110101110011100111111101000101001"
                    "111111110101010001111101011110110010101101111100000101110001100111001100011111001001011110101111101100111111101000101001"
                    "111111110101010001110100111011111010000110001100100100001011011000001000100000100100011101001110111110111111101000101001"
                    "111111110101010001111110100101110011100110011101000100110001011111101101001110000001010101111110111000111111101000101001"
                    "111111110101010001111110100110010010111001100011100101000110111110001001100001000111011111010011101000111111101000101001"
                },
        /* 13*/ { BARCODE_PDF417, -1, UNICODE_MODE, 2, 3, -1, "123456789012345678901234567890123456789012345", 0, 9, 120, 1, "Numeric Compaction 45 consecutive",
                    "111111110101010001111101010111110011010110001110000100111101111010001000100011000011011111010101111100111111101000101001"
                    "111111110101010001111010100001000011101001100100000111010001100001001110010000001101011110101001000000111111101000101001"
                    "111111110101010001010100111100000011111100010110100101001100001111101010110011111000010101000011110000111111101000101001"
                    "111111110101010001101011110011111010010111100111100100110011100001101000111011101000011010111100111110111111101000101001"
                    "111111110101010001101011100000100011111000010100110110001011100100001011111001011100011110101110011100111111101000101001"
                    "111111110101010001111101011110110010101101111100000101110001100111001010110011111000011110101111101100111111101000101001"
                    "111111110101010001110100111011111010000110001100100100001001000010001000010001001000011101001110111110111111101000101001"
                    "111111110101010001111110100101110011110001001101100111101001110011101101111010111110010101111110111000111111101000101001"
                    "111111110101010001111110100110010011011110011001110110011100100011101100100100011111011111010011101000111111101000101001"
                },
        /* 14*/ { BARCODE_PDF417, -1, UNICODE_MODE, 2, 3, -1, "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567", 0, 14, 120, 1, "Numeric Compaction 87 consecutive",
                    "111111110101010001111010100111100011111010111111010100111101111010001000100011000011011111010101111100111111101000101001"
                    "111111110101010001111110101000111011101001100100000111010001100001001110010000001101011111101010011100111111101000101001"
                    "111111110101010001010100111100000011111100010110100101001100001111101010110011111000011101010001111110111111101000101001"
                    "111111110101010001111101011111101010010111100111100100110011100001101000111011101000011010111100111110111111101000101001"
                    "111111110101010001110101110000110011111000010100110110001011100100001011111001011100011101011100011000111111101000101001"
                    "111111110101010001111101011110110010101101111100000101110001100111001111101001110100011101011111010000111111101000101001"
                    "111111110101010001101001110011110011000100011011100111101110000101101101101000010000011101001110111110111111101000101001"
                    "111111110101010001111101001011000010011000111110100101110000101111101100000100011101010101111110011100111111101000101001"
                    "111111110101010001111110100110010010000110011011110110011111010001001000011110110110010100110000111110111111101000101001"
                    "111111110101010001010001110111000011011110111000010100110001100000101111101011111101010100011000011000111111101000101001"
                    "111111110101010001110100111000110010111101101111100111110000101001101011111101011000011101001110011000111111101000101001"
                    "111111110101010001101000100011111011000111110010010100000100010111101011111001100100011010001101111110111111101000101001"
                    "111111110101010001010000010100000011110111001001100110100000100110001110111100011010011010000111011110111111101000101001"
                    "111111110101010001111101000100011011100101110011000111100011001101001000001011110001011110100010010000111111101000101001"
                },
        /* 15*/ { BARCODE_PDF417, -1, UNICODE_MODE, 2, 3, -1, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678", 0, 14, 120, 1, "Numeric Compaction 88 consecutive",
                    "111111110101010001111010100111100011111010111111010100111101111010001000100011000011011111010101111100111111101000101001"
                    "111111110101010001111110101000111011101001100100000111010001100001001110010000001101011111101010011100111111101000101001"
                    "111111110101010001010100111100000011111100010110100101001100001111101010110011111000011101010001111110111111101000101001"
                    "111111110101010001111101011111101010010111100111100100110011100001101000111011101000011010111100111110111111101000101001"
                    "111111110101010001110101110000110011111000010100110110001011100100001011111001011100011101011100011000111111101000101001"
                    "111111110101010001111101011110110010101101111100000101110001100111001111011111010111011101011111010000111111101000101001"
                    "111111110101010001101001110011110011101100001001100110001001100011101110100110001111011101001110111110111111101000101001"
                    "111111110101010001111101001011000010100111110001100111110110010100001101100111100001010101111110011100111111101000101001"
                    "111111110101010001111110100110010011001000100111110101111110111001001011111011001000010100110000111110111111101000101001"
                    "111111110101010001010001110111000011111001010111110100000100011011001110110000010110010100011000011000111111101000101001"
                    "111111110101010001110100111000110011100010011001000100100011110100001011111101011000011101001110011000111111101000101001"
                    "111111110101010001101000100011111011000111110010010100011110100000101001110011011100011010001101111110111111101000101001"
                    "111111110101010001010000010100000011100010110011110111011001100111001110011010000110011010000111011110111111101000101001"
                    "111111110101010001111101000100011011110010110000110111011100111100101111010000110011011110100010010000111111101000101001"
                },
        /* 16*/ { BARCODE_PDF417, -1, UNICODE_MODE, 2, 3, -1, "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789", 0, 14, 120, 1, "Numeric Compaction 89 consecutive",
                    "111111110101010001111010100111100011111010111111010100111101111010001000100011000011011111010101111100111111101000101001"
                    "111111110101010001111110101000111011101001100100000111010001100001001110010000001101011111101010011100111111101000101001"
                    "111111110101010001010100111100000011111100010110100101001100001111101010110011111000011101010001111110111111101000101001"
                    "111111110101010001111101011111101010010111100111100100110011100001101000111011101000011010111100111110111111101000101001"
                    "111111110101010001110101110000110011111000010100110110001011100100001011111001011100011101011100011000111111101000101001"
                    "111111110101010001111101011110110010101101111100000101110001100111001111011111010111011101011111010000111111101000101001"
                    "111111110101010001101001110011110011101100001001100110001001100011101110100110001111011101001110111110111111101000101001"
                    "111111110101010001111101001011000010100111110001100111110110010100001101100111100001010101111110011100111111101000101001"
                    "111111110101010001111110100110010011001000100111110101111110111001001011111011001000010100110000111110111111101000101001"
                    "111111110101010001010001110111000011111001010111110100000100011011001110110000010110010100011000011000111111101000101001"
                    "111111110101010001110100111000110011100010011001000100100011110100001110101100010000011101001110011000111111101000101001"
                    "111111110101010001101000100011111011000111110010010110010001011111001111010011110001011010001101111110111111101000101001"
                    "111111110101010001010000010100000011100110011001110100111011110110001100010000100110011010000111011110111111101000101001"
                    "111111110101010001111101000100011011100101110001100110001001110100001010001111000001011110100010010000111111101000101001"
                },
        /* 17*/ { BARCODE_PDF417, -1, UNICODE_MODE, 0, 3, -1, "AB{}  C#+  de{}  {}F  12{}  G{}  H", 0, 10, 120, 0, "Text Compaction newtable, BWIPP uses PUNCT_SHIFT better for less codewords",
                    "111111110101010001110101001110000011010111000111100111101010111100001000111011100100011111010101111100111111101000101001"
                    "111111110101010001111101010110000011100000101100010100111110100111001110001100011101011111010100110000111111101000101001"
                    "111111110101010001010100111100000010111111001110100100001101011100001001111101101000011010101111100000111111101000101001"
                    "111111110101010001010111110111110010100011101110000100011101110010001100000101001100011010111100111110111111101000101001"
                    "111111110101010001101011100100000010011111010011100110000010111010001111101111011101011010111000100000111111101000101001"
                    "111111110101010001111101011110110010111111011100100101101000011100001100111110110110011111010111000010111111101000101001"
                    "111111110101010001010011100111000011010111100111110110001100001000101100001101110111011101001110111110111111101000101001"
                    "111111110101010001101011111000111011100000101100100110100000011101001111101111011101011010111111011110111111101000101001"
                    "111111110101010001111110100110010010111111011100100110001111001011001011001100111100010100110111110000111111101000101001"
                    "111111110101010001010001100000110010000110001100100110011100110100001100100100110000010100011000011000111111101000101001"
                },
        /* 18*/ { BARCODE_PDF417, -1, UNICODE_MODE, 1, 4, -1, "\177\177\177\177\177", 0, 3, 137, 1, "Byte Compaction",
                    "11111111010101000111010101110000001101010000110000010000010000100010101000001001000001010000010010000011101010011100000111111101000101001"
                    "11111111010101000111101010001000001111101000100011011111010001000110111110100010001101011111101011000011111010101100000111111101000101001"
                    "11111111010101000110101001111100001100011110101100011001101011110000100000111010110001011110011100111010101000111100000111111101000101001"
                },
        /* 19*/ { BARCODE_PDF417, -1, UNICODE_MODE, 1, 4, -1, "\177\177\177\177\177\177", 0, 3, 137, 1, "Byte Compaction, mod 6 == 0 (924 emitted)",
                    "11111111010101000111010101110000001101010000110000011000111000110100111001001100111101000010100001000011101010011100000111111101000101001"
                    "11111111010101000111101010001000001110010000111011010100111110000110111101001100001101011111101011000011111010101100000111111101000101001"
                    "11111111010101000110101001111100001011111011101100010000001110100110110000011010111101111010111100001010101000111100000111111101000101001"
                },
        /* 20*/ { BARCODE_PDF417, -1, UNICODE_MODE, 1, 4, -1, "\177\177\177\177\177\177\177", 0, 3, 137, 1, "Byte Compaction",
                    "11111111010101000111010101110000001101010000110000010000010000100010111001001100111101000010100001000011101010011100000111111101000101001"
                    "11111111010101000111101010001000001110010000111011010100111110000110111101001100001101111101000100011011111010101100000111111101000101001"
                    "11111111010101000110101001111100001011000110011110010110001000111000100011010000111001001100100001110010101000111100000111111101000101001"
                },
        /* 21*/ { BARCODE_PDF417, -1, UNICODE_MODE, 1, 4, -1, "\177\177\177\177\177\177\177\177\177\177\177", 0, 4, 137, 1, "Byte Compaction",
                    "11111111010101000111101010111100001101011011100000010000010000100010111001001100111101000010100001000011101010011100000111111101000101001"
                    "11111111010101000111110101001100001110010000111011010100111110000110111101001100001101111101000100011011111101010111000111111101000101001"
                    "11111111010101000110101001111100001010000001011110010100000010111100101000000101111001010000001011110011010100111110000111111101000101001"
                    "11111111010101000101011110011110001010001000001000011011000010100000111000110001001101100111000110010010101111101111100111111101000101001"
                },
        /* 22*/ { BARCODE_PDF417, -1, UNICODE_MODE, 1, 4, -1, "\177\177\177\177\177\177\177\177\177\177\177\177", 0, 4, 137, 1, "Byte Compaction, mod 6 == 0 (924 emitted)",
                    "11111111010101000111101010111100001101011011100000011000111000110100111001001100111101000010100001000011101010011100000111111101000101001"
                    "11111111010101000111110101001100001110010000111011010100111110000110111101001100001101111001010010000011111101010111000111111101000101001"
                    "11111111010101000110101001111100001001110000100110010011000100001110101000011001111101101000101111100011010100111110000111111101000101001"
                    "11111111010101000101011110011110001101000100011000010011000111001100110001100001000101110100010111000010101111101111100111111101000101001"
                },
        /* 23*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, 5, -1, "1\177", 0, 3, 154, 1, "Byte Compaction, 1 Numeric, 1 Byte",
                    "1111111101010100011101010111000000111010100011100001000001000010001011010011011100000101000001001000001000011000110010011110101001111000111111101000101001"
                    "1111111101010100011110101000010000101111110101100001011111101011000011101001110110000111000010110100001110000110011101011111010101100000111111101000101001"
                    "1111111101010100011101010011111100100111100000100101101100010011110010111100000110110101101110111110001001101011100000010101000011110000111111101000101001"
                },
        /* 24*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, 5, -1, "ABCDEF1234567890123\177\177\177\177VWXYZ", 0, 6, 154, 1, "Text, Numeric, Byte, Text",
                    "1111111101010100011110101011110000110101110111100001111010101111000010100111001110000110100000101100001001111011110100011110101001111000111111101000101001"
                    "1111111101010100011110101000010000111101011001100001010011110000100011111100011101010110000010111000101111001011011000011111101010111000111111101000101001"
                    "1111111101010100011101010011111100110011111101100101010000001011110010100000010111100101000000101111001010000001011110010101000011110000111111101000101001"
                    "1111111101010100010101111001111000100001100011001001000110111101110011110111101101100110111100111000101000011000110010011111010111111010111111101000101001"
                    "1111111101010100011010111000001000101111110101100001011111101011000011001011111001110111100100100100001011111101011000011101011100110000111111101000101001"
                    "1111111101010100011111010111100110110111110110011001101001011111000010101110011111100100100001000111101011000000101110011110101111101100111111101000101001"
                },
        /* 25*/ { BARCODE_PDF417, -1, UNICODE_MODE, 6, 5, -1, "ABCDEF1234567890123\177\177\177\177VWXYZ", 0, 30, 154, 1, "ECC 6: Text, Numeric, Byte, Text",
                    "1111111101010100010101000001000000110101110111100001111010101111000010100111001110000110100000101100001001111011110100011110101001111000111111101000101001"
                    "1111111101010100011110101100011000111101011001100001010011110000100011111100011101010110000010111000101111001011011000011111010100001100111111101000101001"
                    "1111111101010100011101010011111100110011111101100101010000001011110010100000010111100101000000101111001010000001011110011111101011000010111111101000101001"
                    "1111111101010100011101001001110000100001100011001001000110111101110011110111101101100110111100111000101000011000110010011111010111111010111111101000101001"
                    "1111111101010100010101111000010000101111110101100001011111101011000011110001010000010100001001111000101011110011011111011101011100000110111111101000101001"
                    "1111111101010100011111010111100110100111011000111001111010001111010011001111001011000111001111110100101101110000101110011101001011111100111111101000101001"
                    "1111111101010100010100111100111100110011000010000101010000100000100011110011110101000110111100111001001101000110111000011010011100111100111111101000101001"
                    "1111111101010100011111010011011100101000111100100001101001110010000010000001011110100110111001111101001001100010111111011110100100100000111111101000101001"
                    "1111111101010100011010011001111110111111001011101101011111000110100011010101111100000100001110010001101010111001111110011101001111100100111111101000101001"
                    "1111111101010100010100011110111100111101111010001001101000110001110011110100101111000111101100101110001001001000010000010100011101110000111111101000101001"
                    "1111111101010100010100111100001000111000010110010001110011000111001011100101100000100100111011111101001111110111001010011101001110000110111111101000101001"
                    "1111111101010100010100010000011110111001000111110101001111001100011010100001101111100100110000001011101011100110000111010100001011110000111111101000101001"
                    "1111111101010100011101000001001110100001011000001101101111000111001010100001110001110100010100001000001010000110110000010100000101000000111111101000101001"
                    "1111111101010100011101000111011000110011111101011101101001111011000010001110111111010111110111011000101111011101110010011110100010000010111111101000101001"
                    "1111111101010100010100000100111100110000111110100101101111010001100010001111000100100101110000010011001000101000111100011111100101101000111111101000101001"
                    "1111111101010100011001011001110000100111000111010001011000011010000011001100011001100100010100010000001010111110111110010010100000100000111111101000101001"
                    "1111111101010100011101000011001000101111110100110001111101011011100010011101001111100110000001001110101111100010101100010100011111001100111111101000101001"
                    "1111111101010100011111001011100010110000111010111001101010000111110011111010111000100101111011001100001000011101000110010110100111000000111111101000101001"
                    "1111111101010100011101101000001100101110011100010001111001111010100010110011110111000100101000001000001011110011110100011110110100011100111111101000101001"
                    "1111111101010100010100000111101000111110001010011001111100011001001010110111111000110111100100100100001111110001101011011111010000010110111111101000101001"
                    "1111111101010100010110110111100000101111000010100001000111101000010011011010000111100100011010011100001001111000001001010110111011111000111111101000101001"
                    "1111111101010100010110111000011000101111001111001001000011011100011011100001101000110110011000011001101101001000001100010110111001100000111111101000101001"
                    "1111111101010100011100101100001000110110011110000101110010011101100011110100011000110111110110010100001011100010011111011111001010000110111111101000101001"
                    "1111111101010100011110010011110010100011111001101001001111000110011010001110110011100110000101011111001111110101101000010110010000111000111111101000101001"
                    "1111111101010100011001000100110000110100111001111001011100111101100011010000100011000110011011000110001101011001110000011011001000000100111111101000101001"
                    "1111111101010100011001011111001110110110111110011001110001001100100010010001111010000111101000100100001011111101101110010010111100100000111111101000101001"
                    "1111111101010100011111100100011010110100000101111101011001110111110010111000110111000110001001111110101111011011111001010110001001110000111111101000101001"
                    "1111111101010100010010001110001110100110000110010001100001001100111011101001110111110110111100011100101110101001110000010010001110111000111111101000101001"
                    "1111111101010100011100100110100000110101111011000001100110011110100010000010001111010111001011001000001001000011110100011110110110100000111111101000101001"
                    "1111111101010100010110001110111110111110001011100101111101100111101010000111010110000110110010111100001111101011100100010110000011011110111111101000101001"
                },
        /* 26*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, 5, -1, "ABCDEF1234567890123\177\177\177\177YZ1234567890123", 0, 6, 154, 1, "Text, Numeric, Byte, 2 Text, Numeric",
                    "1111111101010100011110101011110000110101110111100001111010101111000010100111001110000110100000101100001001111011110100011110101001111000111111101000101001"
                    "1111111101010100011110101000010000111101011001100001010011110000100011111100011101010110000010111000101111001011011000011111101010111000111111101000101001"
                    "1111111101010100011101010011111100100000111100101001111011011111010010011100001001100100110001000011101110100011111001010101000011110000111111101000101001"
                    "1111111101010100010101111001111000101101111011100001001111011110100011110101100111110110100001000011001000001001101100011111010111111010111111101000101001"
                    "1111111101010100011010111000001000110000010111000101111001011011000010110001111101000111010011100011001110110111010000011101011100110000111111101000101001"
                    "1111111101010100011111010111100110100001100110111101100110101111000010110011011110000111011100010111101001001001111000011110101111101100111111101000101001"
                },
        /* 27*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "ABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZ", 0, 14, 154, 1, "ECC 3",
                    "1111111101010100011110101001111000111010011001111001111010101111000010100111001110000110100000101100001111011010000111011110101001111000111111101000101001"
                    "1111111101010100011110101000001000100101111001000001110110111100110011101100111100110111000101100100001110001001110011011111101010011100111111101000101001"
                    "1111111101010100011101010011111100110111100010011001000001000101111010011110011000110101111100011001001111101011110110010101000001111000111111101000101001"
                    "1111111101010100011111010111111010101000111011100001110010110111100010010011000011000101100111100011101011000011000010011111010111111010111111101000101001"
                    "1111111101010100011010111000000100111001000001101001001101011111100010011001011111100111110111001100101100001011110110011101011100011000111111101000101001"
                    "1111111101010100011111010111100110110111110001000101110101011111100010100110011111000101000001000111101011011001111000011110101111100110111111101000101001"
                    "1111111101010100011010011100111100110010001001100001100100001101110010001011000110000110001000100110001001100011001000011010011100111100111111101000101001"
                    "1111111101010100011111010010011000110011001111010001111001100001101011000110111100100110000100011101001111010111001110010101111110011100111111101000101001"
                    "1111111101010100011010011001111110101000100000111101111100101111011011100100111110010101100001000011101001101110111110010100111011111100111111101000101001"
                    "1111111101010100010100011101110000111101110101100001001100111100111011100110000100110110001101000100001000110011001000010100011101110000111111101000101001"
                    "1111111101010100011010011100000100111000110011101001011111101011000010111111010110000101111110101100001111010100001000011101001110011000111111101000101001"
                    "1111111101010100010100010000011110100100110001111101011011000111100011000111100101100110010101111100001111101000111010010100011000111110111111101000101001"
                    "1111111101010100010100000101000000111101100100011101000011011010000011001000000101100110100000010011001010000110000011010100000101000000111111101000101001"
                    "1111111101010100011101000110100000100111101001111001111100100101100011110101101100000111100010011001101111000100000101011110100010010000111111101000101001"
                },
        /* 28*/ { BARCODE_PDF417, -1, UNICODE_MODE, 8, 29, 32, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 32, 562, 1, "Max codewords (with padding)",
                    "1111111101010100011010100000110000111000100110111101111010101111000010100111001110000110100000101100001111011010000111011001000100110000110010000110111001110001011001111011110111010110000100110011110011101110011000010011011000110100010000100011001100100001100111001110011010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010011010111000111100111111101000101001"
                    "1111111101010100011110101100000110101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000011110101000001000111111101000101001"
                    "1111111101010100011111010111000100110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011111010111001000111111101000101001"
                    "1111111101010100011110100100111100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010100110000110000111111101000101001"
                    "1111111101010100011010111110111000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000011010111000000100111111101000101001"
                    "1111111101010100010100100000111100110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011010010001111100111111101000101001"
                    "1111111101010100010100111100011110100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010011010001100111000111111101000101001"
                    "1111111101010100011110100110001100101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000011111010010011000111111101000101001"
                    "1111111101010100011101000101111110110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011010011111100010111111101000101001"
                    "1111111101010100011010000101100000100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010100001100011000111111101000101001"
                    "1111111101010100010100111110011000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000011010011100000100111111101000101001"
                    "1111111101010100010100001100111110110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001010100001000011110111111101000101001"
                    "1111111101010100010100000100010000100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010011110010100111100111111101000101001"
                    "1111111101010100011101000111000110101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000011101000110100000111111101000101001"
                    "1111111101010100010010110000111110110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100001010111110010011000000101110110001111101001001100111111011001010000011100010110101100001101111001011110001100011011010110111111000100111110011000101111100101110100010001111000101000100010001101111101011110000011011011011110101100000111001110100111101011110001100110011001001011111000110010100001111101000110100001110011001011001111110111111101000101001"
                    "1111111101010100011100101100111100100110011010000001001011000011000010110001110110000101000110000110001000110111000110010110001111001110111010010000111001000001010100000011010000100011000101110011100010001010000010000010010100011000110000110000110100010001101101101100000011001001000011000110000100001011001100001100101000010000101000100000101110011111011101001000110000110010000010100000100100111011100100001101100010000001011110011110010010100110000111011001000110011110111011111001010111110100111001110010001101000111011110011111011010111100111111101000101001"
                    "1111111101010100011101000011101100110000001011101001011111010011100011111000110001010100010001111010001000110111110100010001101111100010111001011001000001101101111100110011111011001010000100100111111011101101110101111110011101000011010000110111101111110101111101101001000011100100111101110111110110001001001111110110101100011010111100110000101111100010011101111001101110011011100100110010000101011110000100001100001000011101011100100011100110110110011110001001110101100010000010011111011011110110001011111011101011111101101110010100011111000110111111101000101001"
                    "1111111101010100010110100000111000101111010000100001000001110100011010001111000010100110111110000101001000101011110000010100001100111110111111011001110101110100011111001011101111010011100111101011111001101001000010011110011111100101101000111011110100011101100111110100100010111000110011100110001111001011001000011110100100010000111101001000110100111111010001011110000100100010001111101100100100101100001111101011110000101000010111111011101000111001011111000101110100111110001010110011100111110111110010111001001011110000100010011101101000111110111111101000101001"
                    "1111111101010100011001001011000000111110101011111001110011110011001010001000110000110110011011100111001011100011100010010110111110111100100011000011101101100010000100011010001000010010000110110000100010001100010001001100011100111000100010110010000110111001101000011001110010111011100100000101000000101000001100001000110111011001101001000000110110010010000001110100001101111011100110010000110110010111101111101101000100001100010111000011101000110011001001000001101111001110010011101011100111110111110110010111101111101100101111011001001101110000111111101000101001"
                    "1111111101010100011110100000010010110001100111100101011111010001110011100000100011010101110111111001001111010001000010011101101110010000110101110010000001101000111010000010110101111110000101111100101110001100101110000001010111101100111110111001001110110001010001111110111011100011011101000110000100111101101110010011000010011100001101110100111101000010100001011000111111011010111011111100010110101111100011101111101110111011011111100101001110101110011011111101010001111101100010001111101011100110100011100100001101000011100010011110100000100100111111101000101001"
                    "1111111101010100011111011011110100100011110101000001010010111100000010001010000011110110111110001000101110110010011111011001111011001110110000011101011101011101110111100011111000101110100101100000101110001000110100001110011011110100000110101111101100001001000111000001011011000111100010110101110011000111001111000101111010010010111011111100110111100010001101101111000101100010000010101111000111111001011010001000111110011010010000111100101000101100111001111101001111001000010011111101001100100101111011100011101011100100000110010110111000111110111111101000101001"
                    "1111111101010100010010011101110000111001000100011101011000111100111011010000111011110111001100010110001100111001101000011010000010001100110110010000100001001000110110000011011011100111000110111000110001001000010000100100010001100110000100100100111100111101001111011110010011001101110011100110011000010100001011000110000010011110001101011100110001101100001101111101101001111010110000111011000111110101111110101001110000111010011000100010000110110000010010011001110100000100111011000110100000100100110011110111001100001100001001011011001000100000111111101000101001"
                    "1111111101010100011100101110110000110011011110100001111110111010100010111011001111110100110001111101001100001001110001011111010100110000111001100111001001001111101011100011110010001101100111110010101100001111000100100001011111010100011000111000001001100101111100110100010010110001111110110111110110110011001111101000100110010001100101111110101001111101100001111101000010011011111001000010110101110111111001001111101100110011010011110101111000111011001110000101110010110000001010111101000111100110001000111000101001011110001000011110010100000100111111101000101001"
                    "1111111101010100011001000100111110100111010001100001001101000001110010100110011111000110100001001111101100011010011110010011000010011100110111100001011001111010011110001011001111000010110101101001110000001000111100010100011101110100011110110011111001000101111001001111001010111100000100100100111001011000001110011010111110010110000110111100111111010011001001100111101000110011011101000111000101111000100010001101110000010111010000111011001110100111100001010001101000111111010010000001001011110100010010000111101101110110011110011001000101111100111111101000101001"
                    "1111111101010100011100100010011100110001110011000101100001100010100011000010001101110111010101110000001001000011011000010110111100011100110100001000110001010000110000110010111001110100000110110010001000001000101000100000010001101100001000111000110010001101101000010110000010011100111101100110101000001100001110110000101100011011001000000100111011100001000101000011011000010011000110100010000110010111001111001110110001001100011000010101100000111010001100111101000001000011011010110111011000000111011000001001101100010100110000010110011100110000111111101000101001"
                    "1111111101010100011111011010100000101101111110011001111001010001000011000100111101100111100101100011001111000101101100010001101111101000111101011000011001111000010110011011110000010010010111100101101100001110100011000010010111110100001110110001001110000101110100111100111011110101101100000111010011100011001101011110000110010011110110111110111100101110111001001111010011110011100101111011100111101110001110101111101000100110011000100111010000111101110111000101101000111000010010011111100101100111000101100000101100010001110010011001011110011000111111101000101001"
                    "1111111101010100010110001101111000111011110101110001100101111110010011000001101011110110111001000011101000010100011110010111000001001100110011110001011001011010000111000011111000100111010100110101110000001100110010011110011111010011101000111111010110000101101111100100100010000011110101000100100010000111101100011111010010010110000001011100110001101001111001000011100110111010110110111100000100111011101111001010000100011110011110010011110100111100010111101001100111101001100011100100101111110101100100000111001000111110011010010110001000001110111111101000101001"
                    "1111111101010100010110011110001110100111001111011001100011011100111010000101000010000110100001110111101111011110100100010000110111101110110110110110000001111011000100111010001000011101110100011000011001001110001100001011011110110000101110101110111001000001101100110110000011110011110010010110100010001100001100100000100011010110001110011000111000101000111001100111000110100010010111100111100100001001100001101010000100010000010111100011110100111000111011011001001000011000011011000110010100000111001100000101101101110011001000010010000100100000111111101000101001"
                    "1111111101010100011110010011001100101100010111111001111001001100011011101001100010000100111101101111101110100001110011011010000111010000111111011101101101001100011111001010100111111001110100010111100100001011000100111111011011111010011110110011011110100001110110000011101011001101111001000101100011111010001110110111001000011111000101001100101000111100100001100100011100100011101001101000000101000111100100001101111110100111010001111100101110100101111100001101000101111101100011011100011111010101110000101111101011010001111110011110010010010000111111101000101001"
                    "1111111101010100010001010001111000110101100111111001110100011111010010111100011000110110001110010011101000110101110000011111100100011010101100111011111001011001101111000011000111110100100101111011011000001010111011111100010011100011011100110111000101110001100011111001001010011010011100000100111001000001101111010111100010010111100010001000100111001000001101101000001011111010111001000110000110111101011000001011100110111000011101010111111000110011110001011001001110000010011010011100110111000100111000010001101111110101100100011100010101111110111111101000101001"
                    "1111111101010100010010000110110000110011010000010001010000100001000011010110111000000111011000001001101100010001001100011001100001010000111100010010111101101111011110011010001000110000110110110001000001001110110101100000011000110001001000110010001101110001010010000010000011111011110101100100010000010100001000010110001100011100010110111100110110010000001001111101100101111010000100001010000110010001100111001101110001101000011101000010111000111001000101110001000001000100100010101110001110000101000110000110001110011010000110011101100001000110111111101000101001"
                    "1111111101010100011011011111001100101111001001111001101110011111010011100110011100100110000100011101001111010011000110011110101110001110100010011110000101100101110000010010111101000111100111010011010000001101011111000111011110101101100000110001001111001101110100000011010010001000111101000111100010010100001111010110011000011001011111011100101000111101000001111001000001010011111001010001100111011100011110101110010000001101011011000011110010110000101110010001001001111001000010100001111100110110100111000010001011010011111100011011011110010000111111101000101001"
                },
        /* 29*/ { BARCODE_PDF417COMP, -1, UNICODE_MODE, 1, 2, -1, "PDF417 APK", 0, 6, 69, 0, "ISO 15438:2015 Figure G.1, same, BWIPP uses different encodation, same codeword count",
                    "111111110101010001111010101111000011010100001100000111011101100110001"
                    "111111110101010001111010100010000011010000111000100111101000101000001"
                    "111111110101010001110101011111100010110011011110000100111110011000101"
                    "111111110101010001010111100111100011001100100000010100001100011001001"
                    "111111110101010001111010111000111011011000001111010110010011101000001"
                    "111111110101010001111010111101000011110100111101000110010010011111001"
                },
        /* 30*/ { BARCODE_PDF417COMP, -1, UNICODE_MODE, 4, 4, -1, "ABCDEFG", 0, 10, 103, 1, "",
                    "1111111101010100011101010011100000110101000011000001111010101111000010100111001110000110100000101100001"
                    "1111111101010100011110101000000100110100000011100101011111101011000010111111010110000101111110101100001"
                    "1111111101010100011010100111110000101111001100011001000001111010100010011111001100100111001011111001001"
                    "1111111101010100010101111101111100100001011000110001100011100110010011011011100111000101111000111100101"
                    "1111111101010100011101011110111000111110011010000101110000010110010011100100011001000100011111001011101"
                    "1111111101010100011110101111001000111111010111011001110111111010100010100000010011110100111100110001101"
                    "1111111101010100010100111001110000100001101101000001101100110000110011011000110011000101000000101000001"
                    "1111111101010100011110100100010000111101001000010001111101011011100011100100001100100101001111100011001"
                    "1111111101010100010100110011111000100110000110111101100111000010111010010001011110000110011111010001001"
                    "1111111101010100010100011000001100110001101010000001100011000110011011001001101110000111110111110101001"
                },
        /* 31*/ { BARCODE_HIBC_PDF, -1, UNICODE_MODE, -1, 3, -1, "H123ABC01234567890D", 0, 8, 120, 0, "BWIPP uses different encodation, same codeword count but zint half-pad shorter",
                    "111111110101010001111101010111110011101011001111000100000100010010001110001110100010011111010101111100111111101000101001"
                    "111111110101010001111110101000111011110000010001010110101111110111101111100011101101011110101001000000111111101000101001"
                    "111111110101010001010100111100000011111010111101100100001111000101001100101000011111011101010001111110111111101000101001"
                    "111111110101010001101011110011111010000100000101000110001110110010001100100101100000011010111100111110111111101000101001"
                    "111111110101010001110101110000110010111111010110000110100111100110001011111101011000011110101110011100111111101000101001"
                    "111111110101010001111101011110110011000111110010010110010100111110001000001001001111011101011111010000111111101000101001"
                    "111111110101010001110100111011111010100110001100000110100011100111101111010010111100011101001110111110111111101000101001"
                    "111111110101010001111101001011000011100001001100100111010000011001001111011000110100010101111110111000111111101000101001"
                },
        /* 32*/ { BARCODE_HIBC_PDF, -1, UNICODE_MODE, 1, 3, -1, "A123BJC5D6E71", 0, 6, 120, 1, "BWIPP example",
                    "111111110101010001111010101111000011110101101111100100000100010010001000011011100110011111010101111100111111101000101001"
                    "111111110101010001111010100010000011110000010001010110101111110111101111000001000101011111101010111000111111101000101001"
                    "111111110101010001010100111100000010110001100011110101111110111101101000111100011011010101000111100000111111101000101001"
                    "111111110101010001010111100111100011100011101001000100001101111011101110001110100010011010111100111110111111101000101001"
                    "111111110101010001111010111000111011010111110011100110100000011100101111110101000111011101011100110000111111101000101001"
                    "111111110101010001111101011110110010011100110011100100011110110011001011001011100000011110101111000100111111101000101001"
                },
        /* 33*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 1, -1, "ABCDEFGHIJKLMNOPQRSTUV", 0, 20, 38, 1, "ISO 24728:2006 Figure 1 1st 1x20, same",
                    "11110101001000011000110010011110101001"
                    "11100101001111110101011100011100101001"
                    "11101101001010011001111100011101101001"
                    "11101001001101000001011000011101001001"
                    "11101001101010000111110011011101001101"
                    "11101011101011001110011111011101011101"
                    "11101011001100100001101110011101011001"
                    "11101010001101100111100100011101010001"
                    "11001010001001100001000111011001010001"
                    "11001011001001100111100111011001011001"
                    "11001011101001100101111110011001011101"
                    "11001001101000001010111100011001001101"
                    "11001101101111001100100111011001101101"
                    "11101101101011100111111010011101101101"
                    "11100101101110011110100111011100101101"
                    "11000101101110001010011100011000101101"
                    "11000101001100010111100110011000101001"
                    "11001101001000011001000111011001101001"
                    "11011101001111011111011010011011101001"
                    "11011001001100010001110100011011001001"
                },
        /* 34*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 2, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCD", 0, 20, 55, 1, "ISO 24728:2006 Figure 1 2nd 2x20, same",
                    "1111010100100001100011001001111010101111000011110101001"
                    "1110010100110101111110111101111101000100110011100101001"
                    "1110110100101101100111100001011001110011111011101101001"
                    "1110100100110010000110111001110001011001111011101001001"
                    "1110100110111001000001101001001101011111100011101001101"
                    "1110101110110111100101100001000001010111100011101011101"
                    "1110101100100011001100100001100111001110011011101011001"
                    "1110101000111111010101110001101011111101111011101010001"
                    "1100101000101000001000111101011011001111000011001010001"
                    "1100101100110010001001100001100100001101110011001011001"
                    "1100101110110110011110010001110010000011010011001011101"
                    "1100100110110011101000011101101111001011000011001001101"
                    "1100110110110001101000100001000110011001000011001101101"
                    "1110110110111000110011101001111110101011100011101101101"
                    "1110010110101001100111110001000110010011100011100101101"
                    "1100010110110110011011000001100110100010000011000101101"
                    "1100010100111111001010011101110000100110100011000101001"
                    "1100110100111001111100101101111110010011010011001101001"
                    "1101110100111010110011110001000001001101100011011101001"
                    "1101100100111100110110100001001001111001000011011001001"
                },
        /* 35*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 3, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMN", 0, 20, 82, 1, "ISO 24728:2006 Figure 1 3rd 3x20",
                    "1100100010100001100011001001011110010111101010111100001010011100111000011100101101"
                    "1110100010111110100010011001011110110101000011111001101001011110010000011000101101"
                    "1110110010111100010111101001001110110110111011001111001001100001000111011000101001"
                    "1100110010100110011110011101001110100111001100001001101100011010001000011001101001"
                    "1101110010110000101111011001001100100111000110011101001111110101011100011011101001"
                    "1101111010101001100111110001001100110101000001000111101011011001111000011011001001"
                    "1100111010110010001001100001001000110110010000110111001110001011001111011011001101"
                    "1110111010111001000001101001001000010100110101111110001001100101111110011011011101"
                    "1110011010100000101011110001011000010110111110101000001101111100010001011011011001"
                    "1111011010111101010111100001011100010101001110011100001101000001011000011011010001"
                    "1111001010101000011111001101011100110100101111001000001110110111100110011010010001"
                    "1110001010110111011001111001011100100101111100110010001110011111010110011010110001"
                    "1100001010111100111101010001011101100111100111100101001111101110100111011010111001"
                    "1100011010111001101110010001001101100101001111000001001111010001100110011010111101"
                    "1100010010110010001111110101000101100110011100001011101111001000111101011010011101"
                    "1110010010100110001100000101000101000110111101110001001101100110110000011010011001"
                    "1111010010100111010001111101001101000100110111110100001111001110111010011010001001"
                    "1111010110101100100000011101011101000110001101000111101000000100101111011010001101"
                    "1111010100101111011110100001011001000111110011010111101011110111110110011010000101"
                    "1110010100110010001111011001011001100111000010111011001110001011100110011011000101"
                },
        /* 36*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZAB", 0, 20, 99, 1, "ISO 24728:2006 Figure 1 4th 4x20, same",
                    "110010001010000110001100100111101010111100001011110010101001110011100001101000001011000011100101101"
                    "111010001010100001111100110100101111001000001011110110111011011110011001101100111100100011000101101"
                    "111011001010011000010001110110011101000011101001110110110111100101100001000001010111100011000101001"
                    "110011001010001100110010000110011100111001101001110100111101010111100001010011100111000011001101001"
                    "110111001011111010001001100101000011111001101001100100100101111001000001110110111100110011011101001"
                    "110111101011011101100111100100110000100011101001100110110011101000011101101111001011000011011001001"
                    "110011101011000110100010000100011001100100001001000110110011100111001101111010101111000011011001101"
                    "111011101011010111111011110111110100010011001001000010101000011111001101001011110010000011011011101"
                    "111001101011110001011110100110111011001111001011000010100110000100011101100111010000111011011011001"
                    "111101101011100110000100110110001101000100001011100010100011001100100001100111001110011011011010001"
                    "111100101011111101010111000110101111110111101011100110111110100010011001010000111110011011010010001"
                    "111000101010110011100111110111100010111101001011100100110111011001111001001100001000111011010110001"
                    "110000101010011001111001110111001100001001101011101100110001101000100001000110011001000011010111001"
                    "110001101011100011001110100111111010101110001001101100111101001110111001011111011001111011010111101"
                    "110001001011110101111101100101111001110011101000101100101110110111000001000010000101111011010011101"
                    "111001001010000010100000100111000111010010001000101000110010000010110001101100010000001011010011001"
                    "111101001011110111001110100110000010011100101001101000111101100110001001011110100001111011010001001"
                    "111101011010000001010011110100110110001111001011101000110000111100101101111010011110010011010001101"
                    "111101010011100011101010000110001011101111001011001000111110111101011001100101110111100011010000101"
                    "111001010010001000001111010111100010100001001011001100100111101101111101001110100111110011011000101"
                },
        /* 37*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 1, -1, "123456789012345", 0, 14, 38, 1, "Number Compaction",
                    "11101110101011111101001100011101110101"
                    "11100110101110101011111100011100110101"
                    "11110110101000001011001100011110110101"
                    "11110010101111100100110111011110010101"
                    "11100010101000111110110010011100010101"
                    "11000010101010000010100000011000010101"
                    "11000110101110001100111010011000110101"
                    "11000100101000100001001111011000100101"
                    "11100100101011110011110010011100100101"
                    "11110100101110011000011101011110100101"
                    "11110101101000101000001111011110101101"
                    "11110101001111001010011110011110101001"
                    "11100101001101011110000110011100101001"
                    "11101101001101000111111001011101101001"
                },
        /* 38*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 2, -1, "\177\177\177", 0, 8, 55, 1, "Byte Compaction",
                    "1100100010100000100001000101010000010010000011001000101"
                    "1110100010111110100010001101111101000100011011101000101"
                    "1110110010110001111100100101100011111001001011101100101"
                    "1100110010100001100011001001000011000110010011001100101"
                    "1101110010111001000111011001011011111101100011011100101"
                    "1101111010111010011111101101001110010011000011011110101"
                    "1100111010111001111001100101000001001101100011001110101"
                    "1110111010111000101111011101110001000011010011101110101"
                },
        /* 39*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 2, -1, "\177\177\177\177\177\177", 0, 8, 55, 1, "Byte Compaction, mod 6 == 0 (924 emitted)",
                    "1100100010110001110001101001110010011001111011001000101"
                    "1110100010100010001111010001110010000111011011101000101"
                    "1110110010101000011001111101101000101111100011101100101"
                    "1100110010100001100011001001000011000110010011001100101"
                    "1101110010101110001001111101101000111000001011011100101"
                    "1101111010100011001101111001010010000111100011011110101"
                    "1100111010100100010000100001110111101100001011001110101"
                    "1110111010111110011010100001101100001111010011101110101"
                },
        /* 40*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 3, -1, "ABCDEFG\177\177\177", 0, 8, 82, 1, "Text & Byte Compaction",
                    "1100111010100001100011001001000010110111101010111100001010011100111000011001110101"
                    "1110111010111110100010011001000010010110100000011100101101111110101110011101110101"
                    "1110011010101000000101111001000011010101000000101111001010000001011110011100110101"
                    "1111011010100001100011001001000111010100100001010000001111001110100011011110110101"
                    "1111001010111000001001100101000110010111010011000010001110100110000100011110010101"
                    "1110001010101111010001000001000100010101100010000011101000000111001011011100010101"
                    "1100001010111110111010111001001100010110011100011000101101100001100110011000010101"
                    "1100011010110100011100001001001110010110110000111101001100011011110010011000110101"
                },
        /* 41*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 4, -1, "\177\177\177abcdefgh1234567890123", 0, 8, 99, 1, "Byte & Text & Numeric Compaction",
                    "110011101010000010000100010101000001001000001000010110101000001001000001010000010010000011001110101"
                    "111011101010111111010110000110000010111001001000010010111101011100111001110100111001100011101110101"
                    "111001101011111001011110110101100110011110001000011010100001111000101001111110101100010011100110101"
                    "111101101011010000100001100100000100110110001000111010100000101101100001001001110011100011110110101"
                    "111100101010111111010110000101111110101100001000110010101111101011100001110011000111010011110010101"
                    "111000101010111111011110110110111111000110101000100010100111000000101101001111101101000011100010101"
                    "110000101011000011010000100100000101101100001001100010101110111110111001111001110010110011000010101"
                    "110001101011101110111100010100100011110100001001110010100000101111000101111001010010000011000110101"
                },
        /* 42*/ { BARCODE_HIBC_MICPDF, -1, UNICODE_MODE, -1, 4, -1, "H123ABC01234567890D", 0, 8, 99, 0, "BWIPP uses different encodation, same codeword count but zint full-pad shorter",
                    "110011101010000110001100100100000100010010001000010110111000111010001001000001001100011011001110101"
                    "111011101011010111111011110111110001110110101000010010111101011100111001011111101001100011101110101"
                    "111001101011001010000111110100011110101000001000011010100111110001101001011011000111100011100110101"
                    "111101101010000110001100100101000010001000001000111010100001100011001001000011000110010011110110101"
                    "111100101010111111010110000101111110101100001000110010111001001100001001111110001101011011110010101"
                    "111000101010001100101110000110010010011111001000100010100111100001101101111110101100010011100010101"
                    "110000101010110110001000000111000101100111101001100010110111101110000101100010101100000011000010101"
                    "110001101011110110000011010111100100001101101001110010101101011111100001111001000110011011000110101"
                },
        /* 43*/ { BARCODE_HIBC_MICPDF, -1, UNICODE_MODE, -1, 1, -1, "/EAH783", 0, 17, 38, 1, "HIBC Provider Applications Standard (PAS) example",
                    "11001101001100011111001001011001101001"
                    "11011101001000001000100100011011101001"
                    "11011001001000100011110100011011001001"
                    "11011001101111010000111101011011001101"
                    "11011011101101100100010000011011011101"
                    "11011011001111001010000100011011011001"
                    "11011010001010000110011111011011010001"
                    "11010010001101001100001110011010010001"
                    "11010110001011111101011000011010110001"
                    "11010111001100011111001001011010111001"
                    "11010111101100110001000010011010111101"
                    "11010011101100001000111001011010011101"
                    "11010011001011110111000111011010011001"
                    "11010001001110001100100011011010001001"
                    "11010001101110010000110100011010001101"
                    "11010000101101100100001111011010000101"
                    "11011000101110111000100010011011000101"
                },
        /* 44*/ { BARCODE_PDF417, 9, DATA_MODE, -1, -1, -1, "\342", 0, 7, 103, 1, "β",
                    "1111111101010100011111010101111100110101000110000001100011100011001011110101011110000111111101000101001"
                    "1111111101010100011111010100011000111110101000011001011111100100011011110101001000000111111101000101001"
                    "1111111101010100011101010111111000110110010011110001100011111001001011010100011111000111111101000101001"
                    "1111111101010100011010111100111110100110011100110001010001100001100010101111001111000111111101000101001"
                    "1111111101010100011010111000010000110110001111000101111110010010111011110101110011100111111101000101001"
                    "1111111101010100011110101111010000100011110001000101000110010111000011110101111000010111111101000101001"
                    "1111111101010100011101001110111110101110001110001001010001101100000011010011101111000111111101000101001"
                },
        /* 45*/ { BARCODE_MICROPDF417, 9, DATA_MODE, -1, 1, -1, "\342\343", 0, 14, 38, 1, "βγ",
                    "11101110101001111110010110011101110101"
                    "11100110101101010000111110011100110101"
                    "11110110101000001000010001011110110101"
                    "11110010101111001011001100011110010101"
                    "11100010101110110010011111011100010101"
                    "11000010101000011000110010011000010101"
                    "11000110101011111101011000011000110101"
                    "11000100101001111000101000011000100101"
                    "11100100101000100000101000011100100101"
                    "11110100101101110010111111011110100101"
                    "11110101101101100101111000011110101101"
                    "11110101001101011100111100011110101001"
                    "11100101001011100101111100011100101001"
                    "11101101001101001001111100011101101001"
                },
        /* 46*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 1, -1, "12345678", 0, 11, 38, 0, "1 columns x 11 rows, variant 1; BWIPP uses byte compaction TODO: investigate",
                    "11001000101001111011110100011100110101"
                    "11101000101110100011000001011110110101"
                    "11101100101000011010011100011110010101"
                    "11001100101100110011000011011100010101"
                    "11011100101111100001010110011000010101"
                    "11011110101101100100111100011000110101"
                    "11001110101011110011111011011000100101"
                    "11101110101000110111110010011100100101"
                    "11100110101011011110111111011110100101"
                    "11110110101001000001000010011110101101"
                    "11110010101110001001110110011110101001"
                },
        /* 47*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 1, -1, "123456789012345678901234567890", 0, 20, 38, 1, "1 columns x 20 rows, variant 4",
                    "11110101001001111011110100011110101001"
                    "11100101001111101010011000011100101001"
                    "11101101001111110010011001011101101001"
                    "11101001001001110011110110011101001001"
                    "11101001101111100010100110011101001101"
                    "11101011101010000010111100011101011101"
                    "11101011001011000111100111011101011001"
                    "11101010001110100000111011011101010001"
                    "11001010001110011101001111011001010001"
                    "11001011001100101110011110011001011001"
                    "11001011101100111011111010011001011101"
                    "11001001101011011000111100011001001101"
                    "11001101101100110000010010011001101101"
                    "11101101101100100011110110011101101101"
                    "11100101101001111110111001011100101101"
                    "11000101101001110011100100011000101101"
                    "11000101001010000111100100011000101001"
                    "11001101001011111101110001011001101001"
                    "11011101001011110111101000011011101001"
                    "11011001001010001111000010011011001001"
                },
        /* 48*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 1, -1, "1234567890123456789012345678901234567890", 0, 24, 38, 1, "1 columns x 24 rows, variant 5",
                    "11100110101000011110001010011110100101"
                    "11110110101101001000011000011110101101"
                    "11110010101101000011100010011110101001"
                    "11100010101001000011011111011100101001"
                    "11000010101011000011001000011101101001"
                    "11000110101100010111000100011101001001"
                    "11000100101110110101111100011101001101"
                    "11100100101101011100011110011101011101"
                    "11110100101001001111000001011101011001"
                    "11110101101110001110101111011101010001"
                    "11110101001101100001000010011001010001"
                    "11100101001110111011110100011001011001"
                    "11101101001111110100111011011001011101"
                    "11101001001000010000110011011001001101"
                    "11101001101111010000010010011001101101"
                    "11101011101100011111001001011101101101"
                    "11101011001000010011100111011100101101"
                    "11101010001111101100001001011000101101"
                    "11001010001011101101110000011000101001"
                    "11001011001000010011011000011001101001"
                    "11001011101100110111100100011011101001"
                    "11001001101001110011011100011011001001"
                    "11001101101000101101100000011011001101"
                    "11101101101111000010010010011011011101"
                },
        /* 49*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 1, -1, "12345678901234567890123456789012345678901234567890", 0, 28, 38, 1, "1 columns x 28 rows, variant 6",
                    "11101011001001111011110100011100101101"
                    "11101010001100010111110111011000101101"
                    "11001010001110100111110001011000101001"
                    "11001011001010000011000110011001101001"
                    "11001011101110010000001101011011101001"
                    "11001001101111110001011010011011001001"
                    "11001101101010011110111100011011001101"
                    "11101101101111110101101111011011011101"
                    "11100101101101101000111100011011011001"
                    "11000101101001100111000011011011010001"
                    "11000101001101111101001111011010010001"
                    "11001101001000111001011000011010110001"
                    "11011101001100010001000011011010111001"
                    "11011001001011111001011100011010111101"
                    "11011001101010110111110000011010011101"
                    "11011011101110111010100000011010011001"
                    "11011011001111110101011100011010001001"
                    "11011010001000011100001011011010001101"
                    "11010010001010001100011000011010000101"
                    "11010110001011111101011000011011000101"
                    "11010111001110111110101100011001000101"
                    "11010111101101011110111110011101000101"
                    "11010011101011110100111100011101100101"
                    "11010011001011101110111100011001100101"
                    "11010001001000101100001100011011100101"
                    "11010001101111101000100011011011110101"
                    "11010000101001111001100110011001110101"
                    "11011000101110101011100000011101110101"
                },
        /* 50*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 2, -1, "ABCDEFGHIJKLMNOPQRSTU", 0, 11, 55, 1, "2 columns x 11 rows, variant 8",
                    "1100100010100001100011001001111010101111000011100110101"
                    "1110100010110101111110111101111101000100110011110110101"
                    "1110110010101101100111100001011001110011111011110010101"
                    "1100110010110010000110111001110001011001111011100010101"
                    "1101110010111001000001101001001101011111100011000010101"
                    "1101111010110111100101100001100001101001111011000110101"
                    "1100111010100001100011001001110010000101110011000100101"
                    "1110111010111101110111010001111100001010110011100100101"
                    "1110011010110010001001111101101100101111000011110100101"
                    "1111011010111000110100001101000101110111000011110101101"
                    "1111001010110001011100000101000100011110001011110101001"
                },
        /* 51*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 2, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZA", 0, 14, 55, 1, "2 columns x 14 rows, variant 9",
                    "1110111010101111110101100001111110101011100011101110101"
                    "1110011010101001100111110001010000010001111011100110101"
                    "1111011010111101101000011101100100010011000011110110101"
                    "1111001010111011011110011001101100111100100011110010101"
                    "1110001010100110000100011101100111010000111011100010101"
                    "1100001010111001100001001101100011010001000011000010101"
                    "1100011010110000101111011001110001100111010011000110101"
                    "1100010010101011100011111101100011111001001011000100101"
                    "1110010010100001100011001001000011000110010011100100101"
                    "1111010010101111110101100001110111000111101011110100101"
                    "1111010110101111110111001001111001011110100011110101101"
                    "1111010100101010000010000001110100001001110011110101001"
                    "1110010100101111100110111101110110011110011011100101001"
                    "1110110100100000011010111001100111101000011011101101001"
                },
        /* 52*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 2, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKL", 0, 17, 55, 1, "2 columns x 17 rows, variant 10",
                    "1100110100110001111100100101110101011111100011001101001"
                    "1101110100101001110011100001101000001011000011011101001"
                    "1101100100101000011111001101001011110010000011011001001"
                    "1101100110111100010111101001101110110011110011011001101"
                    "1101101110111101110101100001001100111100111011011011101"
                    "1101101100100110010111111001111101110011001011011011001"
                    "1101101000110111110101000001101111100010001011011010001"
                    "1101001000111101010111100001010011100111000011010010001"
                    "1101011000111110100010011001010000111110011011010110001"
                    "1101011100101100111001111101111000101111010011010111001"
                    "1101011110100001100011001001000011000110010011010111101"
                    "1101001110101111110101100001011111101011000011010011101"
                    "1101001100101110100000110001001111001010000011010011001"
                    "1101000100111001000110111101110011000010011011010001001"
                    "1101000110111111001010111001111000101100110011010001101"
                    "1101000010101110011101111001010010011110000011010000101"
                    "1101100010100101111101111101000110000011010011011000101"
                },
        /* 53*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 2, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", 0, 23, 55, 1, "2 columns x 23 rows, variant 12",
                    "1110011010110001111100100101110101011111100011110100101"
                    "1111011010101001110011100001101000001011000011110101101"
                    "1111001010101000011111001101001011110010000011110101001"
                    "1110001010111100010111101001101110110011110011100101001"
                    "1100001010111101110101100001001100111100111011101101001"
                    "1100011010100110010111111001111101110011001011101001001"
                    "1100010010110111110101000001101111100010001011101001101"
                    "1110010010111101010111100001010011100111000011101011101"
                    "1111010010111110100010011001010000111110011011101011001"
                    "1111010110101100111001111101111000101111010011101010001"
                    "1111010100111000101100111101111011101011000011001010001"
                    "1110010100100110101111110001001100101111110011001011001"
                    "1110110100100000101011110001101111101010000011001011101"
                    "1110100100110011100111001101111010101111000011001001101"
                    "1110100110110101111110111101111101000100110011001101101"
                    "1110101110111110010011100101100011111001001011101101101"
                    "1110101100100001100011001001110111100110010011100101101"
                    "1110101000110111011111001001110000100011010011000101101"
                    "1100101000100011010011100001011000100000111011000101001"
                    "1100101100111010001001110001101000101100000011001101001"
                    "1100101110111010000001101001011001111100010011011101001"
                    "1100100110101100011011110001111101101111010011011001001"
                    "1100110110100001000010000101100010001000011011011001101"
                },
        /* 54*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 2, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQ", 0, 26, 55, 1, "2 columns x 26 rows, variant 13",
                    "1100101000110001111100100101110101011111100011000101001"
                    "1100101100101001110011100001101000001011000011001101001"
                    "1100101110101000011111001101001011110010000011011101001"
                    "1100100110111100010111101001101110110011110011011001001"
                    "1100110110111101110101100001001100111100111011011001101"
                    "1110110110100110010111111001111101110011001011011011101"
                    "1110010110110111110101000001101111100010001011011011001"
                    "1100010110111101010111100001010011100111000011011010001"
                    "1100010100111110100010011001010000111110011011010010001"
                    "1100110100101100111001111101111000101111010011010110001"
                    "1101110100111000101100111101111011101011000011010111001"
                    "1101100100100110101111110001001100101111110011010111101"
                    "1101100110100000101011110001101111101010000011010011101"
                    "1101101110110011100111001101111010101111000011010011001"
                    "1101101100110101111110111101111101000100110011010001001"
                    "1101101000101101100111100001011001110011111011010001101"
                    "1101001000110010000110111001110001011001111011010000101"
                    "1101011000111001000001101001111001101100100011011000101"
                    "1101011100110001111100100101101001000011111011001000101"
                    "1101011110100100001101100001011100011100010011101000101"
                    "1101001110100010011110100001111110110100110011101100101"
                    "1101001100111101011111001101100011101001110011001100101"
                    "1101000100110111011110011101011100011100010011011100101"
                    "1101000110100111011011111101110110011101000011011110101"
                    "1101000010100010111011111101010100011110000011001110101"
                    "1101100010110001000001011001110101000111000011101110101"
                },
        /* 55*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 3, -1, "ABCDEFGHIJ", 0, 6, 82, 1, "3 columns x 6 rows, variant 14",
                    "1100100010100001100011001001011001110111101010111100001010011100111000011001000101"
                    "1110100010111110100010011001001001110101000011111001101001011110010000011101000101"
                    "1110110010110100010001111101001101110110000101001111101001000100111100011101100101"
                    "1100110010111101101001110001000101110100100011101110001011110011110001011001100101"
                    "1101110010111010011110011101000100110111011100111100101110100111001100011011100101"
                    "1101111010100001111010000101000110110100110100000011101100100111111001011011110101"
                },
        /* 56*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 3, -1, "ABCDEFGHIJKLMNOPQRSTU", 0, 10, 82, 1, "3 columns x 10 rows, variant 16",
                    "1100010010110001111100100101001111010111010101111110001010011001111100011000100101"
                    "1110010010110100000101100001011111010111101101000011101100100010011000011100100101"
                    "1111010010111011011110011001011110010110110011110010001110010000011010011110100101"
                    "1111010110110011101000011101011110110110111100101100001100001101001111011110101101"
                    "1111010100100001100011001001001110110100001100011001001101110001101000011110101001"
                    "1110010100110000100111010001001110100111010001110110001101011100000100011100101001"
                    "1110110100110100110011111101001100100110011100001011101000110010000111011101101001"
                    "1110100100101011110011110001001100110100001001001000001101101000001000011101001001"
                    "1110100110101011111001100001001000110101110010011111001111101100101000011101001101"
                    "1110101110110111101100111001001000010110111110000101001001110001101110011101011101"
                },
        /* 57*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 3, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCD", 0, 12, 82, 1, "3 columns x 12 rows, variant 17",
                    "1110101100100001100011001001011000010111101010111100001010011100111000011101011001"
                    "1110101000111110100010011001011100010101000011111001101001011110010000011101010001"
                    "1100101000111100010111101001011100110110111011001111001001100001000111011001010001"
                    "1100101100100110011110011101011100100111001100001001101100011010001000011001011001"
                    "1100101110110000101111011001011101100111000110011101001111110101011100011001011101"
                    "1100100110101001100111110001001101100110001111100100101100011111001001011001001101"
                    "1100110110110011010000100001000101100100011101110010001101100100000010011001101101"
                    "1110110110111101011000110001000101000111101110111010001111110111010100011101101101"
                    "1110010110100011010111000001001101000101110000001011001110101111101000011100101101"
                    "1100010110101100001110001101011101000110111000111001101110001011011110011000101101"
                    "1100010100111101100011000101011001000110110001111000101100011001111001011000101001"
                    "1100110100101010000011110001011001100111001111101011001111010111100010011001101001"
                },
        /* 58*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 3, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHI", 0, 15, 82, 1, "3 columns x 15 rows, variant 18",
                    "1101110100100001100011001001011000100111101010111100001010011100111000011011101001"
                    "1101100100111110100010011001011000110101000011111001101001011110010000011011001001"
                    "1101100110111100010111101001010000110110111011001111001001100001000111011011001101"
                    "1101101110100110011110011101010001110111001100001001101100011010001000011011011101"
                    "1101101100110000101111011001010001100111000110011101001111110101011100011011011001"
                    "1101101000101001100111110001010011100101000001000111101011011001111000011011010001"
                    "1101001000110110011100111001010011000100001100011001001000011000110010011010010001"
                    "1101011000101111110101100001010111000101111110101100001011111101011000011010110001"
                    "1101011100110111100100001101010110000111011010011111001101111001011000011010111001"
                    "1101011110110011100110000101010010000100010110000011001011110001111001011010111101"
                    "1101001110111010001100100001011010000100111110100011101011010001111110011010011101"
                    "1101001100110011110010110001001010000110011111101100101000111000101100011010011001"
                    "1101000100100001100110010001001011000100000100001010001111010000101111011010001001"
                    "1101000110111110101101110001001011100101111110101100001111100110100001011010001101"
                    "1101000010111011101011110001011011100100011100000101101011000110001111011010000101"
                },
        /* 59*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 3, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRST", 0, 26, 82, 1, "3 columns x 26 rows, variant 20",
                    "1100100010100001100011001001000011010111101010111100001010011100111000011110100101"
                    "1110100010111110100010011001000111010101000011111001101001011110010000011110101101"
                    "1110110010111100010111101001000110010110111011001111001001100001000111011110101001"
                    "1100110010100110011110011101000100010111001100001001101100011010001000011100101001"
                    "1101110010110000101111011001001100010111000110011101001111110101011100011101101001"
                    "1101111010101001100111110001001110010101000001000111101011011001111000011101001001"
                    "1100111010110010001001100001001111010110010000110111001110001011001111011101001101"
                    "1110111010111001000001101001011111010100110101111110001001100101111110011101011101"
                    "1110011010100000101011110001011110010110111110101000001101111100010001011101011001"
                    "1111011010111101010111100001011110110101001110011100001101000001011000011101010001"
                    "1111001010101000011111001101001110110100101111001000001110110111100110011001010001"
                    "1110001010110111011001111001001110100100110000100011101100111010000111011001011001"
                    "1100001010111001100001001101001100100100001100011001001000011000110010011001011101"
                    "1100011010101111110101100001001100110101111110101100001011111101011000011001001101"
                    "1100010010110001111100100101001000110110001111100100101100011111001001011001101101"
                    "1110010010100001100011001001001000010101000000100000101001111011110100011101101101"
                    "1111010010100111001011111001011000010111101101100100001111010111000111011100101101"
                    "1111010110101101001110000001011100010110011111101101001100011101000111011000101101"
                    "1111010100111001110001010001011100110100111000111010001111001101000111011000101001"
                    "1110010100111000101100001001011100100111000110011101001100011011110001011001101001"
                    "1110110100100111000010011001011101100100000110110111101010011011111000011011101001"
                    "1110100100110010000110011101001101100111001111011000101110010001011100011011001001"
                    "1110100110110001011110001101000101100111100001011001101111001101101000011011001101"
                    "1110101110110011110010001101000101000100111101001000001101111101100110011011011101"
                    "1110101100100110011010000001001101000111011110001100101100110000100100011011011001"
                    "1110101000111100010100100001011101000101111101001110001111100100010011011011010001"
                },
        /* 60*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 3, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 32, 82, 1, "3 columns x 26 rows, variant 20",
                    "1110110100110001111100100101011101100111010101111110001010011001111100011011101001"
                    "1110100100110100000101100001001101100111101101000011101100100010011000011011001001"
                    "1110100110111011011110011001000101100110110011110010001110010000011010011011001101"
                    "1110101110110011101000011101000101000110111100101100001000001010111100011011011101"
                    "1110101100100011001100100001001101000110011100111001101111010101111000011011011001"
                    "1110101000110101111110111101011101000111110100010011001010000111110011011011010001"
                    "1100101000101100111001111101011001000111100010111101001101110110011110011010010001"
                    "1100101100111101110101100001011001100100110011110011101110011000010011011010110001"
                    "1100101110111110111001100101011000100110000101111011001110001100111010011010111001"
                    "1100100110111010101111110001011000110101001100111110001010000010001111011010111101"
                    "1100110110111101101000011101010000110110010001001100001100100001101110011010011101"
                    "1110110110110110011110010001010001110111001000001101001001101011111100011010011001"
                    "1110010110110111100101100001010001100100000101011110001101111101010000011010001001"
                    "1100010110110011100111001101010011100111101010111100001010011100111000011010001101"
                    "1100010100111110100010011001010011000101000011111001101001011110010000011010000101"
                    "1100110100111100010111101001010111000110111011001111001001100001000111011011000101"
                    "1101110100100110011110011101010110000111001100001001101100011010001000011001000101"
                    "1101100100110000101111011001010010000111000110011101001011111101011000011101000101"
                    "1101100110110001111100100101011010000110001111100100101100011111001001011101100101"
                    "1101101110100001100011001001001010000111101111101101001110110000100011011001100101"
                    "1101101100100001011111001101001011000111010000111001101110011011101000011011100101"
                    "1101101000110011111101100101001011100101111100011101101100100100111110011011110101"
                    "1101001000101000010001000001011011100100110111001100001110111010100000011001110101"
                    "1101011000101011111100111001011011110111110001110110101111110101001110011101110101"
                    "1101011100100011001001110001011001110100011110101000001100111110010010011100110101"
                    "1101011110111011100010000101001001110101101111001110001001100111000011011110110101"
                    "1101001110100111111010011001001101110100011001111101001111000100110011011110010101"
                    "1101001100100011101011000001000101110111010111110010001000010010011110011100010101"
                    "1101000100111000010010011101000100110100001011000001101000101111001111011000010101"
                    "1101000110111110010000101101000110110111001101111001101111110110001011011000110101"
                    "1101000010110010100001111101000010110100111100110011001111010000111101011000100101"
                    "1101100010101110011101000001000010010110000100010001101110001101000011011100100101"
                },
        /* 61*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 3, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 38, 82, 1, "3 columns x 38 rows, variant 22",
                    "1100010010110001111100100101000101100111010101111110001010011001111100011010011101"
                    "1110010010110100000101100001000101000111101101000011101100100010011000011010011001"
                    "1111010010111011011110011001001101000110110011110010001110010000011010011010001001"
                    "1111010110110011101000011101011101000110111100101100001000001010111100011010001101"
                    "1111010100100011001100100001011001000110011100111001101111010101111000011010000101"
                    "1110010100110101111110111101011001100111110100010011001010000111110011011011000101"
                    "1110110100101100111001111101011000100111100010111101001101110110011110011001000101"
                    "1110100100111101110101100001011000110100110011110011101110011000010011011101000101"
                    "1110100110111110111001100101010000110110000101111011001110001100111010011101100101"
                    "1110101110111010101111110001010001110101001100111110001010000010001111011001100101"
                    "1110101100111101101000011101010001100110010001001100001100100001101110011011100101"
                    "1110101000110110011110010001010011100111001000001101001001101011111100011011110101"
                    "1100101000110111100101100001010011000100000101011110001101111101010000011001110101"
                    "1100101100110011100111001101010111000111101010111100001010011100111000011101110101"
                    "1100101110111110100010011001010110000101000011111001101001011110010000011100110101"
                    "1100100110111100010111101001010010000110111011001111001001100001000111011110110101"
                    "1100110110100110011110011101011010000111001100001001101100011010001000011110010101"
                    "1110110110110000101111011001001010000111000110011101001111110101011100011100010101"
                    "1110010110101001100111110001001011000101000001000111101011011001111000011000010101"
                    "1100010110110010001001100001001011100110010000110111001110001011001111011000110101"
                    "1100010100111001000001101001011011100100110101111110001001100101111110011000100101"
                    "1100110100100000101011110001011011110110111110101000001101111100010001011100100101"
                    "1101110100100001100011001001011001110100001100011001001000011000110010011110100101"
                    "1101100100101111110101100001001001110110110011110001001110000101100100011110101101"
                    "1101100110101111000001001001001101110111111001011000101000110001011100011110101001"
                    "1101101110110111000110010001000101110101001111001111001100100101100000011100101001"
                    "1101101100101100011111000101000100110101111110011011101011100011111101011101101001"
                    "1101101000101111010000001001000110110101110011101111001001110011101111011101001001"
                    "1101001000111110111010011101000010110110110011100111001100100001001100011101001101"
                    "1101011000100101111000010001000010010100111111001011001101001110010000011101011101"
                    "1101011100100011101000110001000011010110011100001011101011101001100000011101011001"
                    "1101011110110110000110011001000111010111000011010001101001000100001000011101010001"
                    "1101001110110101110000001001000110010110010011111011101111010001100110011001010001"
                    "1101001100111110001011101001000100010100011001001110001011111011100011011001011001"
                    "1101000100100100001100001101001100010111011000010001101001011100111000011001011101"
                    "1101000110110000010011100101001110010101100111110001001111010111000111011001001101"
                    "1101000010110100101111100001001111010101000000100111101011101111011111011001101101"
                    "1101100010111011101100001101011111010100000101000010001011110111100010011101101101"
                },
        /* 62*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 3, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 44, 82, 1, "3 columns x 44 rows, variant 23",
                    "1100100010100001100011001001011000010111101010111100001010011100111000011010001001"
                    "1110100010111110100010011001011100010101000011111001101001011110010000011010001101"
                    "1110110010111100010111101001011100110110111011001111001001100001000111011010000101"
                    "1100110010100110011110011101011100100111001100001001101100011010001000011011000101"
                    "1101110010110000101111011001011101100111000110011101001111110101011100011001000101"
                    "1101111010101001100111110001001101100101000001000111101011011001111000011101000101"
                    "1100111010110010001001100001000101100110010000110111001110001011001111011101100101"
                    "1110111010111001000001101001000101000100110101111110001001100101111110011001100101"
                    "1110011010100000101011110001001101000110111110101000001101111100010001011011100101"
                    "1111011010111101010111100001011101000101001110011100001101000001011000011011110101"
                    "1111001010101000011111001101011001000100101111001000001110110111100110011001110101"
                    "1110001010110111011001111001011001100100110000100011101100111010000111011101110101"
                    "1100001010111001100001001101011000100110001101000100001000110011001000011100110101"
                    "1100011010111000110011101001011000110111111010101110001101011111101111011110110101"
                    "1100010010101000001000111101010000110101101100111100001011001110011111011110010101"
                    "1110010010110010000110111001010001110111000101100111101111011101011000011100010101"
                    "1111010010100110101111110001010001100100110010111111001111101110011001011000010101"
                    "1111010110110111110101000001010011100110111110001000101110101011111100011000110101"
                    "1111010100101001110011100001010011000110100000101100001111011010000111011000100101"
                    "1110010100100101111001000001010111000111011011110011001101100111100100011100100101"
                    "1110110100100110000100011101010110000110011101000011101101111001011000011110100101"
                    "1110100100110001101000100001010010000100011001100100001100111001110011011110101101"
                    "1110100110111111010101110001011010000110101111110111101111101000100110011110101001"
                    "1110101110101101100111100001001010000101100111001111101111000101111010011100101001"
                    "1110101100111000101100111101001011000111101110101100001001100111100111011101101001"
                    "1110101000100110010111111001001011100111110111001100101100001011110110011101001001"
                    "1100101000110111110001000101011011100110001111100100101100011111001001011101001101"
                    "1100101100100001100011001001011011110111100100100111101110010000101110011101011101"
                    "1100101110111100010001101101011001110111011100111101001111100010101100011101011001"
                    "1100100110111001100101111101001001110110010100001111101011000010000111011101010001"
                    "1100110110100011000110100001001101110101100110001000001110001001000111011001010001"
                    "1110110110101101001111110001000101110111011000111101101111000101000010011001011001"
                    "1110010110100111100000100101000100110111000100111110101001100010000111011001011101"
                    "1100010110100010000100100001000110110111010011001111001000101000010000011001001101"
                    "1100010100101111000100111101000010110110001011100100001111000001010001011001101101"
                    "1100110100101110100110000001000010010101111011000011001000110011001111011101101101"
                    "1101110100110001110110010001000011010100011011100110001001100001100100011100101101"
                    "1101100100100111110001011101000111010100011110101111001110100110000100011000101101"
                    "1101100110101100110111100001000110010111001001111100101011110010001000011000101001"
                    "1101101110111011000001011001000100010101000111000111001001110001110010011001101001"
                    "1101101100101111101100111101001100010111101000100000101110100111100111011011101001"
                    "1101101000110111100001011001001110010111110100001110101001100100001110011011001001"
                    "1101001000111000100010011101001111010110000010010011001111011101000110011011001101"
                    "1101011000100100011111011001011111010111011100111100101001011110100000011011011101"
                },
        /* 63*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 4, -1, "ABCDEFG", 0, 4, 99, 1, "4 columns x 4 rows, variant 24",
                    "110100111010111111010110000111111010101110001001110110110101111110111101111101000100110011010010001"
                    "110100110011111001001110010110001111100100101001110100110001111100100101100011111001001011010110001"
                    "110100010011000110100010000101101111101111001001100100110100110000111001110011100101000011010111001"
                    "110100011011111011001001000110100111000010001001100110101011111101110001110110111100110011010111101"
                },
        /* 64*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRS", 0, 6, 99, 1, "4 columns x 6 rows, variant 25",
                    "110010001010000110001100100111101010111100001011001110101001110011100001101000001011000011001000101"
                    "111010001010100001111100110100101111001000001001001110111011011110011001101100111100100011101000101"
                    "111011001010011000010001110110011101000011101001101110101111001000000101100011111001001011101100101"
                    "110011001010010001000000100110100001000011001000101110110111011000010001110010111011111011001100101"
                    "110111001011000110011110010101101111100001001000100110111110001010110001111101100010001011011100101"
                    "110111101010001110110001110100000100101111001000110110110111100000101101111000101111001011011110101"
                },
        /* 65*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJK", 0, 10, 99, 1, "4 columns x 10 rows, variant 27",
                    "110001001011000111110010010111010101111110001001111010101001100111110001010000010001111011000100101"
                    "111001001011110110100001110110010001001100001011111010110010000110111001110001011001111011100100101"
                    "111101001011100100000110100100110101111110001011110010100110010111111001111101110011001011110100101"
                    "111101011011011111010100000110111110001000101011110110111010101111110001010011001111100011110101101"
                    "111101010011010000010110000111101101000011101001110110110010001001100001101100001000010011110101001"
                    "111001010010111111010110000101111110101100001001110100101111110101100001011111101011000011100101001"
                    "111011010010001110111011110111110101110001001001100100100110001000011101110010111111011011101101001"
                    "111010010010001000100000010100100010000100001001100110110110000100010001110001001000111011101001001"
                    "111010011011101100000111010110111111010111001001000110100010011111011001111010110000110011101001101"
                    "111010111011001111110110010100011101000011001001000010101111100110010001011100001100111011101011101"
                },
        /* 66*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCD", 0, 12, 99, 1, "4 columns x 12 rows, variant 28",
                    "111010110010000110001100100111101010111100001011000010101001110011100001101000001011000011101011001"
                    "111010100010100001111100110100101111001000001011100010111011011110011001101100111100100011101010001"
                    "110010100010011000010001110110011101000011101011100110110111100101100001000001010111100011001010001"
                    "110010110010001100110010000110011100111001101011100100111101010111100001010011100111000011001011001"
                    "110010111011111010001001100101000011111001101011101100100101111001000001110110111100110011001011101"
                    "110010011011011101100111100100110000100011101001101100110011101000011101101111001011000011001001101"
                    "110011011011000110100010000100011001100100001000101100110011100111001101111010101111000011001101101"
                    "111011011011010111111011110101111110101100001000101000111101100111001101011100100011111011101101101"
                    "111001011011101100100111110100010110001111101001101000111101011110000101110010111110001011100101101"
                    "110001011011000001000010110111010101110000001011101000111110100101111101101100011100111011000101101"
                    "110001010010001111001011110111101101100010001011001000111100111011101001111001100011001011000101001"
                    "110011010011101111110100100101011100111111001011001100100001111000101001001111101110110011001101001"
                },
        /* 67*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHI", 0, 15, 99, 1, "4 columns x 15 rows, variant 29",
                    "110111010010000110001100100111101010111100001011000100101001110011100001101000001011000011011101001"
                    "110110010010100001111100110100101111001000001011000110111011011110011001101100111100100011011001001"
                    "110110011010011000010001110110011101000011101010000110110111100101100001000001010111100011011001101"
                    "110110111010001100110010000110011100111001101010001110111101010111100001010011100111000011011011101"
                    "110110110011111010001001100101000011111001101010001100100101111001000001110110111100110011011011001"
                    "110110100011011101100111100100110000100011101010011100110011101000011101101111001011000011011010001"
                    "110100100011000110100010000100011001100100001010011000110011100111001101111010101111000011010010001"
                    "110101100011010111111011110111110100010011001010111000101000011111001101111110110100110011010110001"
                    "110101110011000111110010010110001111100100101010110000110001111100100101100011111001001011010111001"
                    "110101111010000110001100100100001100011001001010010000100001100011001001110110100110000011010111101"
                    "110100111010111100101111000100111110100111001011010000111001101110100001011001111100010011010011101"
                    "110100110010000110010011100111001011111000101001010000110011100100111001110011111101001011010011001"
                    "110100010011000111011001000110100110000111001001011000111001000010111001111001101000111011010001001"
                    "110100011010111010000111110110001000111001001001011100111101100000110101100100011101000011010001101"
                    "110100001010011110100100000100111110011010001011011100111011111101001001011101100011100011010000101"
                },
        /* 68*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 20, 99, 1, "4 columns x 20 rows, variant 30",
                    "110010001010000110001100100111101010111100001011110010101001110011100001101000001011000011100101101"
                    "111010001010100001111100110100101111001000001011110110111011011110011001101100111100100011000101101"
                    "111011001010011000010001110110011101000011101001110110110111100101100001000001010111100011000101001"
                    "110011001010001100110010000110011100111001101001110100111101010111100001010011100111000011001101001"
                    "110111001011111010001001100101000011111001101001100100100101111001000001110110111100110011011101001"
                    "110111101011011101100111100100110000100011101001100110110011101000011101101111001011000011011001001"
                    "110011101011000110100010000100011001100100001001000110110011100111001101111010101111000011011001101"
                    "111011101011010111111011110111110100010011001001000010101000011111001101001011110010000011011011101"
                    "111001101011110001011110100110111011001111001011000010100110000100011101100111010000111011011011001"
                    "111101101011100110000100110110001101000100001011100010100011001100100001100111001110011011011010001"
                    "111100101010111111010110000101111110101100001011100110101111110101100001011111101011000011010010001"
                    "111000101011000111110010010110001111100100101011100100110001111100100101100011111001001011010110001"
                    "110000101010000110001100100100001100011001001011101100100001100011001001000011000110010011010111001"
                    "110001101010111111010110000101111110101100001001101100111110110010010001101111101001111011010111101"
                    "110001001011111101011100110111100010111100101000101100100011011011110001010010000011110011010011101"
                    "111001001010000100001100110110011100011010001000101000110111101111001101100001001100111011010011001"
                    "111101001010111101000011110111100011001101001001101000110100001110000101101111110100111011010001001"
                    "111101011010111100000110110100110001100111101011101000101100010000011101000001111011011011010001101"
                    "111101010011001100001010000100010001000000101011001000100001100110100001110111001000100011010000101"
                    "111001010011100101100100000111000110111000101011001100111110010010001101110001011101100011011000101"
                },
        /* 69*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 26, 99, 1, "4 columns x 26 rows, variant 31",
                    "110010001010000110001100100111101010111100001000011010101001110011100001101000001011000011110100101"
                    "111010001010100001111100110100101111001000001000111010111011011110011001101100111100100011110101101"
                    "111011001010011000010001110110011101000011101000110010110111100101100001000001010111100011110101001"
                    "110011001010001100110010000110011100111001101000100010111101010111100001010011100111000011100101001"
                    "110111001011111010001001100101000011111001101001100010100101111001000001110110111100110011101101001"
                    "110111101011011101100111100100110000100011101001110010110011101000011101101111001011000011101001001"
                    "110011101011000110100010000100011001100100001001111010110011100111001101111010101111000011101001101"
                    "111011101011010111111011110111110100010011001011111010101000011111001101001011110010000011101011101"
                    "111001101011110001011110100110111011001111001011110010100110000100011101100111010000111011101011001"
                    "111101101011100110000100110110001101000100001011110110100011001100100001100111001110011011101010001"
                    "111100101011111101010111000110101111110111101001110110111110100010011001010000111110011011001010001"
                    "111000101010110011100111110111100010111101001001110100110111011001111001001100001000111011001011001"
                    "110000101010011001111001110111001100001001101001100100110001101000100001000110011001000011001011101"
                    "110001101011100011001110100111111010101110001001100110110101111110111101111101000100110011001001101"
                    "110001001010110110011110000101100111001111101001000110111100010111101001101110110011110011001101101"
                    "111001001011110111010110000100110011110011101001000010111001100001001101100011010001000011101101101"
                    "111101001011000010111101100111000110011101001011000010101111110101100001011111101011000011100101101"
                    "111101011011000111110010010110001111100100101011100010110001111100100101100011111001001011000101101"
                    "111101010011000111011000010101100001110011001011100110101100111011000001000010010000100011000101001"
                    "111001010011100100111000110110000100011100101011100100111100101010000001001111100100111011001101001"
                    "111011010010110100011100000101101000000111001011101100100000110010011101100111110001010011011101001"
                    "111010010010010000110000110110000100010110001001101100100001000110011001010011101110000011011001001"
                    "111010011011110001100110100110001000011100101000101100110000010111010001111101100110011011011001101"
                    "111010111010011111001100100100010100001111001000101000100111100000101001001111100111011011011011101"
                    "111010110011000110000100100100100001001000001001101000100000100011001101000011001100001011011011001"
                    "111010100011110100010010000111000001101110101011101000111001001110011001101111101001111011011010001"
                },
        /* 70*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 32, 99, 1, "4 columns x 32 rows, variant 32",
                    "111011010011000111110010010111010101111110001011101100101001100111110001010000010001111011011101001"
                    "111010010011110110100001110110010001001100001001101100110010000110111001110001011001111011011001001"
                    "111010011011100100000110100100110101111110001000101100100110010111111001111101110011001011011001101"
                    "111010111011011111010100000110111110001000101000101000111010101111110001010011001111100011011011101"
                    "111010110011010000010110000111101101000011101001101000110010001001100001100100001101110011011011001"
                    "111010100011011001111001000111001000001101001011101000100110101111110001001100101111110011011010001"
                    "110010100010000010101111000110111110101000001011001000110111110001000101110101011111100011010010001"
                    "110010110010100111001110000110100000101100001011001100111101101000011101100100010011000011010110001"
                    "110010111011101101111001100110110011110010001011000100111001000001101001001101011111100011010111001"
                    "110010011011011110010110000100000101011110001011000110110111110101000001101111100010001011010111101"
                    "110011011011110101011110000101001110011100001010000110110100000101100001111011010000111011010011101"
                    "111011011010010111100100000111011011110011001010001110110110011110010001110010000011010011010011001"
                    "111001011011001110100001110110111100101100001010001100100000101011110001101111101010000011010001001"
                    "110001011011001110011100110111101010111100001010011100101001110011100001101000001011000011010001101"
                    "110001010010100001111100110100101111001000001010011000111011011110011001101100111100100011010000101"
                    "110011010010011000010001110110011101000011101010111000110111100101100001000001010111100011011000101"
                    "110111010010001100110010000110011100111001101010110000111101010111100001010011100111000011001000101"
                    "110110010011111010001001100101000011111001101010010000100101111001000001110110111100110011101000101"
                    "110110011011011101100111100100110000100011101011010000110011101000011101101111001011000011101100101"
                    "110110111011000110100010000100011001100100001001010000110011100111001101000011000110010011001100101"
                    "110110110010111111010110000101111110101100001001011000101111110101100001011111101011000011011100101"
                    "110110100011000111110010010110001111100100101001011100110001111100100101100011111001001011011110101"
                    "110100100010000110001100100100001100011001001011011100110111100111000101100001100010100011001110101"
                    "110101100011111010000101100111001000001101001011011110111100011011010001111101010001100011101110101"
                    "110101110010000001110010110101110100001100001011001110101001001111000001111110101100100011100110101"
                    "110101111011000010110011100101101111100111101001001110110011000100001001001101110110000011110110101"
                    "110100111011110100011011000100101111100110001001101110100100111100001001111000100100100011110010101"
                    "110100110011100111101001110111100101111101101000101110100000100100111101111110101100010011100010101"
                    "110100010010011000011010000100001100011001001000100110100110000110100001101100100100000011000010101"
                    "110100011010110111111000110111110000101011001000110110111010001100000101111000110110001011000110101"
                    "110100001010110000100001110101110000011011101000010110101110000010110001101111110011010011000100101"
                    "110110001010010000001001000110011101100001001000010010111110010101111101110001000100111011100100101"
                },
        /* 71*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 38, 99, 1, "4 columns x 38 rows, variant 33",
                    "110001001011000111110010010111010101111110001000101100101001100111110001010000010001111011010011101"
                    "111001001011110110100001110110010001001100001000101000110010000110111001110001011001111011010011001"
                    "111101001011100100000110100100110101111110001001101000100110010111111001111101110011001011010001001"
                    "111101011011011111010100000110111110001000101011101000111010101111110001010011001111100011010001101"
                    "111101010011010000010110000111101101000011101011001000110010001001100001100100001101110011010000101"
                    "111001010011011001111001000111001000001101001011001100100110101111110001001100101111110011011000101"
                    "111011010010000010101111000110111110101000001011000100110111110001000101110101011111100011001000101"
                    "111010010010100111001110000110100000101100001011000110111101101000011101100100010011000011101000101"
                    "111010011011101101111001100110110011110010001010000110111001000001101001001101011111100011101100101"
                    "111010111011011110010110000100000101011110001010001110110111110101000001101111100010001011001100101"
                    "111010110011110101011110000101001110011100001010001100110100000101100001111011010000111011011100101"
                    "111010100010010111100100000111011011110011001010011100110110011110010001110010000011010011011110101"
                    "110010100011001110100001110110111100101100001010011000100000101011110001101111101010000011001110101"
                    "110010110011001110011100110111101010111100001010111000101001110011100001101000001011000011101110101"
                    "110010111010100001111100110100101111001000001010110000111011011110011001101100111100100011100110101"
                    "110010011010011000010001110110011101000011101010010000110111100101100001000001010111100011110110101"
                    "110011011010001100110010000110011100111001101011010000111101010111100001010011100111000011110010101"
                    "111011011011111010001001100101000011111001101001010000100101111001000001110110111100110011100010101"
                    "111001011011011101100111100100110000100011101001011000110011101000011101101111001011000011000010101"
                    "110001011011000110100010000100011001100100001001011100110011100111001101111010101111000011000110101"
                    "110001010011010111111011110111110100010011001011011100101000011111001101001011110010000011000100101"
                    "110011010011110001011110100110111011001111001011011110100110000100011101100111010000111011100100101"
                    "110111010011100110000100110110001101000100001011001110100011001100100001100111001110011011110100101"
                    "110110010010111111010110000101111110101100001001001110101111110101100001011111101011000011110101101"
                    "110110011011000111110010010110001111100100101001101110110001111100100101100011111001001011110101001"
                    "110110111010000110001100100100001100011001001000101110100001100011001001000011000110010011100101001"
                    "110110110010111111010110000101111110101100001000100110101111110101100001011111101011000011101101001"
                    "110110100011011111101101000100001100101110001000110110110110010111100001001111011000110011101001001"
                    "110100100011100011101000100110000010001001101000010110111101111010000101000001000011011011101001101"
                    "110101100011100010001110110110110001111000101000010010101111101000011101001000011110010011101011101"
                    "110101110011010100001111100101111101110011001000011010101110000010110001011000100000111011101011001"
                    "110101111010000100010100000110001101100001101000111010111101110100001101011101111001100011101010001"
                    "110100111011101001110000110100100111101000001000110010111111001010111001110100000011010011001010001"
                    "110100110010001111010000100111111010011000101000100010101001000011110001011110010001000011001011001"
                    "110100010011110111101001000110011110111000101001100010110011001010000001101110110000100011001011101"
                    "110100011011001011110000110100110001111100101001110010101110011011111101001011110100000011001001101"
                    "110100001011001111001011000110011111010001001001111010100100101111000001011100000100011011001101101"
                    "110110001011011101111001110111101101100111101011111010110100011001110001100110100001000011101101101"
                },
        /* 72*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 44, 99, 1, "4 columns x 44 rows, variant 34",
                    "110010001010000110001100100111101010111100001011000010101001110011100001101000001011000011010001001"
                    "111010001010100001111100110100101111001000001011100010111011011110011001101100111100100011010001101"
                    "111011001010011000010001110110011101000011101011100110110111100101100001000001010111100011010000101"
                    "110011001010001100110010000110011100111001101011100100111101010111100001010011100111000011011000101"
                    "110111001011111010001001100101000011111001101011101100100101111001000001110110111100110011001000101"
                    "110111101011011101100111100100110000100011101001101100110011101000011101101111001011000011101000101"
                    "110011101011000110100010000100011001100100001000101100110011100111001101111010101111000011101100101"
                    "111011101011010111111011110111110100010011001000101000101000011111001101001011110010000011001100101"
                    "111001101011110001011110100110111011001111001001101000100110000100011101100111010000111011011100101"
                    "111101101011100110000100110110001101000100001011101000100011001100100001100111001110011011011110101"
                    "111100101011111101010111000110101111110111101011001000111110100010011001010000111110011011001110101"
                    "111000101010110011100111110111100010111101001011001100110111011001111001001100001000111011101110101"
                    "110000101010011001111001110111001100001001101011000100110001101000100001000110011001000011100110101"
                    "110001101011100011001110100111111010101110001011000110110101111110111101111101000100110011110110101"
                    "110001001010110110011110000101100111001111101010000110111100010111101001101110110011110011110010101"
                    "111001001011110111010110000100110011110011101010001110111001100001001101100011010001000011100010101"
                    "111101001011000010111101100111000110011101001010001100111111010101110001101011111101111011000010101"
                    "111101011010100000100011110101101100111100001010011100101100111001111101111000101111010011000110101"
                    "111101010011100010110011110111101110101100001010011000100110011110011101110011000010011011000100101"
                    "111001010011111011100110010110000101111011001010111000111000110011101001111110101011100011100100101"
                    "111011010010100110011111000101000001000111101010110000101101100111100001011001110011111011110100101"
                    "111010010011001000011011100111000101100111101010010000111101110101100001001100111100111011110101101"
                    "111010011010011001011111100111110111001100101011010000110000101111011001110001100111010011110101001"
                    "111010111011101010111111000101001100111110001001010000101000001000111101011011001111000011100101001"
                    "111010110011001000100110000110010000110111001001011000111000101100111101111011101011000011101101001"
                    "111010100010011010111111000100110010111111001001011100111110111001100101100001011110110011101001001"
                    "110010100011011111000100010111010101111110001011011100101001100111110001010000010001111011101001101"
                    "110010110011110110100001110110010001001100001011011110110010000110111001110001011001111011101011101"
                    "110010111011100100000110100100110101111110001011001110100110010111111001111101110011001011101011001"
                    "110010011011011111010100000110111110001000101001001110110001111100100101100011111001001011101010001"
                    "110011011010000110001100100100001100011001001001101110100001100011001001000011000110010011001010001"
                    "111011011010111111010110000101111110101100001000101110111000100001101001111011110111101011001011001"
                    "111001011010101100011111000101111000010010001000100110101000100011110001101110110011110011001011101"
                    "110001011010001011000110000111001000110111101000110110111101010111100001011000011100110011001001101"
                    "110001010011001001110100000111100010100000101000010110111100111011101001111001010000001011001101101"
                    "110011010010011110110001100100011101101110001000010010100101000001111001111010111110110011101101101"
                    "110111010011100011001011000110000110110011001000011010101111001111010001100001001100111011100101101"
                    "110110010011110001000110110111101001000010001000111010110001001110100001011111010000111011000101101"
                    "110110011010001101001110000101110000010001101000110010100111000110011101000011010000111011000101001"
                    "110110111010000010100010000110001110110000101000100010110110110110000001000011000110100011001101001"
                    "110110110011110001100110010111101001110111001001100010111000100110100001001110100111110011011101001"
                    "110110100011000101101111110100100001101111101001110010110000011101011101001011011111000011011001001"
                    "110100100011100011101100110111010111001111101001111010100001000110110001110100110111100011011001101"
                    "110101100011001000011101000111110100101100001011111010110101111101110001101101111010000011011011101"
                },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];
    char cmp_buf[32768];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); // Only do BWIPP test if asked, too slow otherwise
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); // Only do ZXing-C++ test if asked, too slow otherwise

    testStart("test_encode");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, data[i].eci, data[i].option_1, data[i].option_2, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, %d, %s, %d, %d, %d, \"%s\", %s, %d, %d, %d, \"%s\",\n",
                    i, testUtilBarcodeName(data[i].symbology), data[i].eci, testUtilInputModeName(data[i].input_mode),
                    data[i].option_1, data[i].option_2, data[i].option_3,
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret),
                    symbol->rows, symbol->width, data[i].bwipp_cmp, data[i].comment);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < ZINT_ERROR) {
                int width, row;

                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, ret, width, row, data[i].data);

                if (do_bwipp && testUtilCanBwipp(i, symbol, data[i].option_1, data[i].option_2, data[i].option_3, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        ret = testUtilBwipp(i, symbol, data[i].option_1, data[i].option_2, data[i].option_3, data[i].data, length, NULL, cmp_buf, sizeof(cmp_buf));
                        assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, data[i].expected);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, data[i].expected);
                    }
                }
                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, debug)) {
                    int cmp_len, ret_len;
                    char modules_dump[2710 * 8 + 1];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilZXingCPP(i, symbol, data[i].data, modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length, NULL /*primary*/, escaped, &ret_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

// #181 Nico Gunkel OSS-Fuzz
static void test_fuzz(int index, int debug) {

    struct item {
        int symbology;
        char *data;
        int length;
        int option_1;
        int ret;
    };
    struct item data[] = {
        /* 0*/ { BARCODE_PDF417,
                    "\060\075\204\060\204\060\204\041\060\075\060\204\060\075\060\075\204\060\075\060\103\204\060\204\060\003\120\060\075\060\004\060\204\060\074\204\060\204\060\075"
                    "\204\060\075\060\103\204\060\214\060\204\060\075\060\031\060\073\060\025\060\075\060\204\060\103\204\060\075\060\204\060\000\075\060\226\060\100\204\060\204\060"
                    "\204\060\075\204\060\120\214\060\204\060\074\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\000\060\204\060\120\214\060\204\060\074\204"
                    "\060\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\204\060\204\060\126\060\075\060\204\060\177\060\103\204\060\204\060\377\060\262\060"
                    "\000\075\060\226\060\100\060\073\060\204\060\000\075\060\226\060\100\060\103\204\060\204\060\075\204\060\204\060\204\041\060\110\060\160\060\075\060\075\204\060"
                    "\075\060\103\204\060\204\060\003\120\060\075\060\004\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\073\060\074\060\075"
                    "\060\204\060\103\204\060\075\060\204\060\204\060\075\204\060\120\214\060\204\060\074\204\060\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041"
                    "\060\000\060\000\060\200\060\204\060\214\060\204\060\075\060\141\060\000\060\204\060\120\214\060\204\060\075\060\204\060\075\204\060\204\060\204\075\060\075\060"
                    "\204\060\075\060\075\204\060\075\060\103\204\060\204\060\372\120\060\124\060\000\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060"
                    "\075\060\073\060\075\060\204\060\103\204\060\075\060\204\060\204\060\122\060\000\060\075\060\000\076\060\100\000\060\075\060\103\204\060\214\060\204\060\075\060"
                    "\200\060\204\075\060\075\060\204\060\000\075\060\226\060\100\204\060\204\060\075\204\060\204\060\204\075\060\075\060\204\060\134\060\075\204\060\040\060\103\204"
                    "\060\372\120\060\124\060\004\060\103\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\000\060\113\060\377\060\235\060\075\060"
                    "\204\060\103\204\060\204\060\075\060\204\060\204\060\075\204\060\075\214\060\204\060\074\204\060\204\060\075\204\060\075\060\103\211\060\214\060\204\060\075\060"
                    "\041\060\204\060\204\060\120\060\075\060\204\060\003\060\103\204\060\204\060\377\060\350\060\223\060\000\075\060\226\060\103\204\060\204\060\204\120\060\075\060"
                    "\204\060\000\075\060\226\060\100\060\103\204\060\204\060\075\204\060\204\060\204\041\060\075\060\204\060\075\060\075\204\060\075\060\103\204\060\204\060\003\120"
                    "\060\075\060\004\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\031\060\155\060\000\075\060\226\060\100\204\060\204\060"
                    "\204\060\075\204\060\120\214\060\204\060\074\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\000\060\204\060\120\214\060\204\060\074\204"
                    "\060\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\204\060\204\060\126\060\075\060\204\060\177\060\103\204\060\204\060\377\060\262\060"
                    "\000\075\060\226\060\100\204\060\204\060\204\075\060\073\060\204\060\000\075\060\226\060\100\060\103\204\060\204\060\075\204\060\204\060\204\075\060\110\060\160"
                    "\060\075\060\075\204\060\075\060\103\204\060\204\060\372\120\060\124\060\000\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075"
                    "\060\073\060\074\060\075\060\204\060\103\204\060\075\060\204\060\204\060\075\204\060\075\214\060\204\060\074\204\060\204\060\075\204\060\075\060\103\214\060\214"
                    "\060\204\060\075\060\041\060\000\060\000\060\200\060\204\060\214\060\204\060\075\060\141\060\000\060\204\060\075\214\060\204\060\075\060\204\060\075\204\060\204"
                    "\060\204\041\060\075\060\204\060\075\060\075\204\060\075\060\103\204\060\204\060\003\120\060\075\060\004\060\204\060\074\204\060\204\060\075\204\060\075\060\103"
                    "\204\060\214\060\204\060\075\060\073\060\075\060\204\060\103\204\060\075\060\204\060\204\060\122\060\000\060\075\060\000\076\060\100\000\060\004\060\103\204\060"
                    "\204\060\003\060\204\075\060\120\214\060\204\060\004\060\103\204\060\204\060\003\060\211\074\060\120\060\124\060\351\060\120\060\075\060\351\060\072\375\060\204\060",
                    1001, -1, ZINT_ERROR_TOO_LONG }, // Original OSS-Fuzz triggering data
        /* 1*/ { BARCODE_PDF417COMP,
                    "\060\075\204\060\204\060\204\041\060\075\060\204\060\075\060\075\204\060\075\060\103\204\060\204\060\003\120\060\075\060\004\060\204\060\074\204\060\204\060\075"
                    "\204\060\075\060\103\204\060\214\060\204\060\075\060\031\060\073\060\025\060\075\060\204\060\103\204\060\075\060\204\060\000\075\060\226\060\100\204\060\204\060"
                    "\204\060\075\204\060\120\214\060\204\060\074\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\000\060\204\060\120\214\060\204\060\074\204"
                    "\060\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\204\060\204\060\126\060\075\060\204\060\177\060\103\204\060\204\060\377\060\262\060"
                    "\000\075\060\226\060\100\060\073\060\204\060\000\075\060\226\060\100\060\103\204\060\204\060\075\204\060\204\060\204\041\060\110\060\160\060\075\060\075\204\060"
                    "\075\060\103\204\060\204\060\003\120\060\075\060\004\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\073\060\074\060\075"
                    "\060\204\060\103\204\060\075\060\204\060\204\060\075\204\060\120\214\060\204\060\074\204\060\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041"
                    "\060\000\060\000\060\200\060\204\060\214\060\204\060\075\060\141\060\000\060\204\060\120\214\060\204\060\075\060\204\060\075\204\060\204\060\204\075\060\075\060"
                    "\204\060\075\060\075\204\060\075\060\103\204\060\204\060\372\120\060\124\060\000\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060"
                    "\075\060\073\060\075\060\204\060\103\204\060\075\060\204\060\204\060\122\060\000\060\075\060\000\076\060\100\000\060\075\060\103\204\060\214\060\204\060\075\060"
                    "\200\060\204\075\060\075\060\204\060\000\075\060\226\060\100\204\060\204\060\075\204\060\204\060\204\075\060\075\060\204\060\134\060\075\204\060\040\060\103\204"
                    "\060\372\120\060\124\060\004\060\103\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\000\060\113\060\377\060\235\060\075\060"
                    "\204\060\103\204\060\204\060\075\060\204\060\204\060\075\204\060\075\214\060\204\060\074\204\060\204\060\075\204\060\075\060\103\211\060\214\060\204\060\075\060"
                    "\041\060\204\060\204\060\120\060\075\060\204\060\003\060\103\204\060\204\060\377\060\350\060\223\060\000\075\060\226\060\103\204\060\204\060\204\120\060\075\060"
                    "\204\060\000\075\060\226\060\100\060\103\204\060\204\060\075\204\060\204\060\204\041\060\075\060\204\060\075\060\075\204\060\075\060\103\204\060\204\060\003\120"
                    "\060\075\060\004\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\031\060\155\060\000\075\060\226\060\100\204\060\204\060"
                    "\204\060\075\204\060\120\214\060\204\060\074\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\000\060\204\060\120\214\060\204\060\074\204"
                    "\060\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\204\060\204\060\126\060\075\060\204\060\177\060\103\204\060\204\060\377\060\262\060"
                    "\000\075\060\226\060\100\204\060\204\060\204\075\060\073\060\204\060\000\075\060\226\060\100\060\103\204\060\204\060\075\204\060\204\060\204\075\060\110\060\160"
                    "\060\075\060\075\204\060\075\060\103\204\060\204\060\372\120\060\124\060\000\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075"
                    "\060\073\060\074\060\075\060\204\060\103\204\060\075\060\204\060\204\060\075\204\060\075\214\060\204\060\074\204\060\204\060\075\204\060\075\060\103\214\060\214"
                    "\060\204\060\075\060\041\060\000\060\000\060\200\060\204\060\214\060\204\060\075\060\141\060\000\060\204\060\075\214\060\204\060\075\060\204\060\075\204\060\204"
                    "\060\204\041\060\075\060\204\060\075\060\075\204\060\075\060\103\204\060\204\060\003\120\060\075\060\004\060\204\060\074\204\060\204\060\075\204\060\075\060\103"
                    "\204\060\214\060\204\060\075\060\073\060\075\060\204\060\103\204\060\075\060\204\060\204\060\122\060\000\060\075\060\000\076\060\100\000\060\004\060\103\204\060"
                    "\204\060\003\060\204\075\060\120\214\060\204\060\004\060\103\204\060\204\060\003\060\211\074\060\120\060\124\060\351\060\120\060\075\060\351\060\072\375\060\204\060",
                    1001, -1, ZINT_ERROR_TOO_LONG },
        /* 2*/ { BARCODE_MICROPDF417,
                    "\060\075\204\060\204\060\204\041\060\075\060\204\060\075\060\075\204\060\075\060\103\204\060\204\060\003\120\060\075\060\004\060\204\060\074\204\060\204\060\075"
                    "\204\060\075\060\103\204\060\214\060\204\060\075\060\031\060\073\060\025\060\075\060\204\060\103\204\060\075\060\204\060\000\075\060\226\060\100\204\060\204\060"
                    "\204\060\075\204\060\120\214\060\204\060\074\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\000\060\204\060\120\214\060\204\060\074\204"
                    "\060\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\204\060\204\060\126\060\075\060\204\060\177\060\103\204\060\204\060\377\060\262\060"
                    "\000\075\060\226\060\100\060\073\060\204\060\000\075\060\226\060\100\060\103\204\060\204\060\075\204\060\204\060\204\041\060\110\060\160\060\075\060\075\204\060"
                    "\075\060\103\204\060\204\060\003\120\060\075\060\004\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\073\060\074\060\075"
                    "\060\204\060\103\204\060\075\060\204\060\204\060\075\204\060\120\214\060\204\060\074\204\060\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041"
                    "\060\000\060\000\060\200\060\204\060\214\060\204\060\075\060\141\060\000\060\204\060\120\214\060\204\060\075\060\204\060\075\204\060\204\060\204\075\060\075\060"
                    "\204\060\075\060\075\204\060\075\060\103\204\060\204\060\372\120\060\124\060\000\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060"
                    "\075\060\073\060\075\060\204\060\103\204\060\075\060\204\060\204\060\122\060\000\060\075\060\000\076\060\100\000\060\075\060\103\204\060\214\060\204\060\075\060"
                    "\200\060\204\075\060\075\060\204\060\000\075\060\226\060\100\204\060\204\060\075\204\060\204\060\204\075\060\075\060\204\060\134\060\075\204\060\040\060\103\204"
                    "\060\372\120\060\124\060\004\060\103\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\000\060\113\060\377\060\235\060\075\060"
                    "\204\060\103\204\060\204\060\075\060\204\060\204\060\075\204\060\075\214\060\204\060\074\204\060\204\060\075\204\060\075\060\103\211\060\214\060\204\060\075\060"
                    "\041\060\204\060\204\060\120\060\075\060\204\060\003\060\103\204\060\204\060\377\060\350\060\223\060\000\075\060\226\060\103\204\060\204\060\204\120\060\075\060"
                    "\204\060\000\075\060\226\060\100\060\103\204\060\204\060\075\204\060\204\060\204\041\060\075\060\204\060\075\060\075\204\060\075\060\103\204\060\204\060\003\120"
                    "\060\075\060\004\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\031\060\155\060\000\075\060\226\060\100\204\060\204\060"
                    "\204\060\075\204\060\120\214\060\204\060\074\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\000\060\204\060\120\214\060\204\060\074\204"
                    "\060\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\204\060\204\060\126\060\075\060\204\060\177\060\103\204\060\204\060\377\060\262\060"
                    "\000\075\060\226\060\100\204\060\204\060\204\075\060\073\060\204\060\000\075\060\226\060\100\060\103\204\060\204\060\075\204\060\204\060\204\075\060\110\060\160"
                    "\060\075\060\075\204\060\075\060\103\204\060\204\060\372\120\060\124\060\000\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075"
                    "\060\073\060\074\060\075\060\204\060\103\204\060\075\060\204\060\204\060\075\204\060\075\214\060\204\060\074\204\060\204\060\075\204\060\075\060\103\214\060\214"
                    "\060\204\060\075\060\041\060\000\060\000\060\200\060\204\060\214\060\204\060\075\060\141\060\000\060\204\060\075\214\060\204\060\075\060\204\060\075\204\060\204"
                    "\060\204\041\060\075\060\204\060\075\060\075\204\060\075\060\103\204\060\204\060\003\120\060\075\060\004\060\204\060\074\204\060\204\060\075\204\060\075\060\103"
                    "\204\060\214\060\204\060\075\060\073\060\075\060\204\060\103\204\060\075\060\204\060\204\060\122\060\000\060\075\060\000\076\060\100\000\060\004\060\103\204\060"
                    "\204\060\003\060\204\075\060\120\214\060\204\060\004\060\103\204\060\204\060\003\060\211\074\060\120\060\124\060\351\060\120\060\075\060\351\060\072\375\060\204\060",
                    1001, -1, ZINT_ERROR_TOO_LONG },
        /* 3*/ { BARCODE_PDF417,
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "1234567890",
                    2710, 0, 0 }, // Max numerics with ECC 0
        /* 4*/ { BARCODE_PDF417,
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "12345678901",
                    2711, 0, ZINT_ERROR_TOO_LONG },
        /* 5*/ { BARCODE_PDF417,
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678",
                    2528, -1, 0 }, // Max numerics with ECC 5
        /* 6*/ { BARCODE_PDF417,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCD",
                    1850, 0, 0 }, // Max text with ECC 0
        /* 7*/ { BARCODE_PDF417,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG",
                    1853, 0, ZINT_ERROR_TOO_LONG },
        /* 8*/ { BARCODE_PDF417,
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240",
                    1108, 0, 0 }, // Max bytes with ECC 0
        /* 9*/ { BARCODE_PDF417,
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240",
                    1111, 0, ZINT_ERROR_TOO_LONG },
        /*10*/ { BARCODE_MICROPDF417,
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456",
                    366, -1, 0 }, // Max numerics
        /*11*/ { BARCODE_MICROPDF417,
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "1234567890123456789012345678901234567890123456789012345678901234567",
                    367, -1, ZINT_ERROR_TOO_LONG },
        /*12*/ { BARCODE_MICROPDF417,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOP",
                    250, -1, 0 }, // Max text
        /*13*/ { BARCODE_MICROPDF417,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQ",
                    251, -1, ZINT_ERROR_TOO_LONG },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_fuzz");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        if (data[i].option_1 != -1) {
            symbol->option_1 = data[i].option_1;
        }
        symbol->debug |= debug;

        length = data[i].length;
        if (length == -1) {
            length = (int) strlen(data[i].data);
        }

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

#include <time.h>

#define TEST_PERF_ITER_MILLES   5
#define TEST_PERF_ITERATIONS    (TEST_PERF_ITER_MILLES * 1000)
#define TEST_PERF_TIME(arg)     (((arg) * 1000.0) / CLOCKS_PER_SEC)

// Not a real test, just performance indicator
static void test_perf(int index, int debug) {

    struct item {
        int symbology;
        int input_mode;
        int option_1;
        int option_2;
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_PDF417, -1, -1, -1, "1234567890", 0, 7, 103, "10 numerics" },
        /*  1*/ { BARCODE_PDF417, -1, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^ABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLM"
                    "NOPQRSTUVWXYZ;<>@[]_`~!||()?{}'123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJK"
                    "LMNOPQRSTUVWXYZ12345678912345678912345678912345678900001234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFG"
                    "HIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567"
                    "890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcde"
                    "fghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNO",
                    0, 40, 307, "960 chars, text/numeric" },
        /*  2*/ { BARCODE_PDF417, DATA_MODE, -1, -1,
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240",
                    0, 51, 358, "960 chars, byte" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    clock_t start;
    clock_t total_create = 0, total_encode = 0, total_buffer = 0, total_buf_inter = 0, total_print = 0;
    clock_t diff_create, diff_encode, diff_buffer, diff_buf_inter, diff_print;
    int comment_max = 0;

    if (!(debug & ZINT_DEBUG_TEST_PERFORMANCE)) { /* -d 256 */
        return;
    }

    for (i = 0; i < data_size; i++) if ((int) strlen(data[i].comment) > comment_max) comment_max = (int) strlen(data[i].comment);

    printf("Iterations %d\n", TEST_PERF_ITERATIONS);

    for (i = 0; i < data_size; i++) {
        int j;

        if (index != -1 && i != index) continue;

        diff_create = diff_encode = diff_buffer = diff_buf_inter = diff_print = 0;

        for (j = 0; j < TEST_PERF_ITERATIONS; j++) {
            start = clock();
            symbol = ZBarcode_Create();
            diff_create += clock() - start;
            assert_nonnull(symbol, "Symbol not created\n");

            length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

            start = clock();
            ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
            diff_encode += clock() - start;
            assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

            start = clock();
            ret = ZBarcode_Buffer(symbol, 0 /*rotate_angle*/);
            diff_buffer += clock() - start;
            assert_zero(ret, "i:%d ZBarcode_Buffer ret %d != 0 (%s)\n", i, ret, symbol->errtxt);

            symbol->output_options |= OUT_BUFFER_INTERMEDIATE;
            start = clock();
            ret = ZBarcode_Buffer(symbol, 0 /*rotate_angle*/);
            diff_buf_inter += clock() - start;
            assert_zero(ret, "i:%d ZBarcode_Buffer OUT_BUFFER_INTERMEDIATE ret %d != 0 (%s)\n", i, ret, symbol->errtxt);
            symbol->output_options &= ~OUT_BUFFER_INTERMEDIATE; // Undo

            start = clock();
            ret = ZBarcode_Print(symbol, 0 /*rotate_angle*/);
            diff_print += clock() - start;
            assert_zero(ret, "i:%d ZBarcode_Print ret %d != 0 (%s)\n", i, ret, symbol->errtxt);
            assert_zero(remove(symbol->outfile), "i:%d remove(%s) != 0\n", i, symbol->outfile);

            ZBarcode_Delete(symbol);
        }

        printf("%*s: encode % 8gms, buffer % 8gms, buf_inter % 8gms, print % 8gms, create % 8gms\n", comment_max, data[i].comment,
                TEST_PERF_TIME(diff_encode), TEST_PERF_TIME(diff_buffer), TEST_PERF_TIME(diff_buf_inter), TEST_PERF_TIME(diff_print), TEST_PERF_TIME(diff_create));

        total_create += diff_create;
        total_encode += diff_encode;
        total_buffer += diff_buffer;
        total_buf_inter += diff_buf_inter;
        total_print += diff_print;
    }
    if (index == -1) {
        printf("%*s: encode % 8gms, buffer % 8gms, buf_inter % 8gms, print % 8gms, create % 8gms\n", comment_max, "totals",
                TEST_PERF_TIME(total_encode), TEST_PERF_TIME(total_buffer), TEST_PERF_TIME(total_buf_inter), TEST_PERF_TIME(total_print), TEST_PERF_TIME(total_create));
    }
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_large", test_large, 1, 0, 1 },
        { "test_options", test_options, 1, 0, 1 },
        { "test_reader_init", test_reader_init, 1, 1, 1 },
        { "test_input", test_input, 1, 1, 1 },
        { "test_encode", test_encode, 1, 1, 1 },
        { "test_fuzz", test_fuzz, 1, 0, 1 },
        { "test_perf", test_perf, 1, 0, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
