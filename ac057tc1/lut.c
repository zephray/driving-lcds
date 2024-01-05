//
// Copyright 2024 Wenting Zhang <zephray@outlook.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#include "lut.h"

#define PRD0_LENGTH  20
#define XPS0_LENGTH  15
#define LPS0_LENGTH  10
#define MPS0_LENGTH  5
#define SPS0_LENGTH  2

// 15V pulse field (2nd field in 6bpp mode, 3rd field in 9bpp mode)

// VCOM waveform
const uint8_t lut0_vcom_20[] = {
    1, 0x60, 0x00, PRD0_LENGTH, PRD0_LENGTH, 0, 0, 0, 0, 0, 0,
};

// -15V extra long pulse
const uint8_t lut0_c0_21[] = {
    1, 0x01, 0x20, 0x00, 0x00, XPS0_LENGTH, (PRD0_LENGTH-XPS0_LENGTH), PRD0_LENGTH, 0, 0, 0, 0, 0,
};

// +15V long pulse
const uint8_t lut0_c1_23[] = {
    1, 0x01, 0x20, 0x00, 0x00, LPS0_LENGTH, (PRD0_LENGTH-LPS0_LENGTH), PRD0_LENGTH, 0, 0, 0, 0, 0,
};

// +15V medium pulse
const uint8_t lut0_c2_24[] = {
    1, 0x01, 0x20, 0x00, 0x00, MPS0_LENGTH, (PRD0_LENGTH-MPS0_LENGTH), PRD0_LENGTH, 0, 0, 0, 0, 0,
};

// +15V short pulse
const uint8_t lut0_c3_22[] = {
    1, 0x01, 0x20, 0x00, 0x00, SPS0_LENGTH, (PRD0_LENGTH-SPS0_LENGTH), PRD0_LENGTH, 0, 0, 0, 0, 0,
};

// no change
const uint8_t lut0_c4_25[] = {
    1, 0x12, 0x00, 0x00, 0x00, PRD0_LENGTH, PRD0_LENGTH, 0, 0, 0, 0, 0, 0,
};

// -15V medium pulse
const uint8_t lut0_c5_26[] = {
    1, 0x10, 0x20, 0x00, 0x00, PRD0_LENGTH, MPS0_LENGTH, (PRD0_LENGTH-MPS0_LENGTH), 0, 0, 0, 0, 0,
};

// -15V long pulse
const uint8_t lut0_c6_27[] = {
    1, 0x10, 0x20, 0x00, 0x00, PRD0_LENGTH, LPS0_LENGTH, (PRD0_LENGTH-LPS0_LENGTH), 0, 0, 0, 0, 0,
};

// -15V extra long pulse
const uint8_t lut0_c7_28[] = {
    1, 0x10, 0x20, 0x00, 0x00, PRD0_LENGTH, XPS0_LENGTH, (PRD0_LENGTH-XPS0_LENGTH), 0, 0, 0, 0, 0,
};

const uint8_t lut0_xon_29[] = {
    1, 0xff, PRD0_LENGTH, PRD0_LENGTH, 0,  0,  0,  0,  0,  0,
};

const lut_t lut0[10] = {
    lut0_vcom_20, sizeof(lut0_vcom_20),
    lut0_c0_21, sizeof(lut0_c0_21),
    lut0_c3_22, sizeof(lut0_c3_22),
    lut0_c1_23, sizeof(lut0_c1_23),
    lut0_c2_24, sizeof(lut0_c2_24),
    lut0_c4_25, sizeof(lut0_c4_25),
    lut0_c5_26, sizeof(lut0_c5_26),
    lut0_c6_27, sizeof(lut0_c6_27),
    lut0_c7_28, sizeof(lut0_c7_28),
    lut0_xon_29, sizeof(lut0_xon_29)
};

// 25V pulse field (2nd field in 9bpp mode)

#define PRD1_LENGTH  20
#define XPS1_LENGTH  15
#define LPS1_LENGTH  10
#define MPS1_LENGTH  5
#define SPS1_LENGTH  2

// VCOM waveform
const uint8_t lut1_vcom_20[] = {
    1, 0x60, 0x00, PRD0_LENGTH, PRD0_LENGTH, 0, 0, 0, 0, 0, 0,
};

// -25V extra long pulse
const uint8_t lut1_c0_21[] = {
    1, 0x41, 0x20, 0x00, 0x00, XPS1_LENGTH, (PRD1_LENGTH-XPS1_LENGTH), PRD1_LENGTH, 0, 0, 0, 0, 0,
};

// +25V long pulse
const uint8_t lut1_c1_23[] = {
    1, 0x41, 0x20, 0x00, 0x00, LPS1_LENGTH, (PRD1_LENGTH-LPS1_LENGTH), PRD1_LENGTH, 0, 0, 0, 0, 0,
};

// +25V medium pulse
const uint8_t lut1_c2_24[] = {
    1, 0x41, 0x20, 0x00, 0x00, MPS1_LENGTH, (PRD1_LENGTH-MPS1_LENGTH), PRD1_LENGTH, 0, 0, 0, 0, 0,
};

// +25V short pulse
const uint8_t lut1_c3_22[] = {
    1, 0x41, 0x20, 0x00, 0x00, SPS1_LENGTH, (PRD1_LENGTH-SPS1_LENGTH), PRD1_LENGTH, 0, 0, 0, 0, 0,
};

// no change
const uint8_t lut1_c4_25[] = {
    1, 0x12, 0x00, 0x00, 0x00, PRD1_LENGTH, PRD1_LENGTH, 0, 0, 0, 0, 0, 0,
};

// -25V medium pulse
const uint8_t lut1_c5_26[] = {
    1, 0x13, 0x20, 0x00, 0x00, PRD1_LENGTH, SPS1_LENGTH, (PRD1_LENGTH-SPS1_LENGTH), 0, 0, 0, 0, 0,
};

// -25V long pulse
const uint8_t lut1_c6_27[] = {
    1, 0x13, 0x20, 0x00, 0x00, PRD1_LENGTH, MPS1_LENGTH, (PRD1_LENGTH-MPS1_LENGTH), 0, 0, 0, 0, 0,
};

// -25V extra long pulse
const uint8_t lut1_c7_28[] = {
    1, 0x13, 0x20, 0x00, 0x00, PRD1_LENGTH, LPS1_LENGTH, (PRD1_LENGTH-LPS1_LENGTH), 0, 0, 0, 0, 0,
};

const uint8_t lut1_xon_29[] = {
    1, 0xff, PRD1_LENGTH, PRD1_LENGTH, 0,  0,  0,  0,  0,  0,
};

const lut_t lut1[10] = {
    lut1_vcom_20, sizeof(lut1_vcom_20),
    lut1_c0_21, sizeof(lut1_c0_21),
    lut1_c3_22, sizeof(lut1_c3_22),
    lut1_c1_23, sizeof(lut1_c1_23),
    lut1_c2_24, sizeof(lut1_c2_24),
    lut1_c4_25, sizeof(lut1_c4_25),
    lut1_c5_26, sizeof(lut1_c5_26),
    lut1_c6_27, sizeof(lut1_c6_27),
    lut1_c7_28, sizeof(lut1_c7_28),
    lut1_xon_29, sizeof(lut1_xon_29)
};