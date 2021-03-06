#include <stdint.h>

#include "cashaddr.h"

// Fastest CashAddr encoding library ever. EVER!
/*
 * Copyright (c) 2018 DesWurstes
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

const char *const CHARSET = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";

static void convertBitsEightToFive(const unsigned char *__restrict bytes,
	unsigned char first_byte, unsigned char *__restrict converted) {
	int a = 1, b = 0;
	converted[0] = first_byte >> 3;
	converted[1] = first_byte % (1 << 3) << 2;
	while (a < 32) {
		converted[a++] |= bytes[b] >> 6;
		converted[a++] = bytes[b] % (1 << 6) >> 1;
		converted[a] = bytes[b++] % (1 << 1) << 4;
		converted[a++] |= bytes[b] >> 4;
		converted[a] = bytes[b++] % (1 << 4) << 1;
		converted[a++] |= bytes[b] >> 7;
		converted[a++] = bytes[b] % (1 << 7) >> 2;
		converted[a] = bytes[b++] % (1 << 2) << 3;
		converted[a++] |= bytes[b] >> 5;
		converted[a++] = bytes[b++] % (1 << 5);
		converted[a++] = bytes[b] >> 3;
		converted[a] = bytes[b++] % (1 << 3) << 2;
	}
}

/* Copyright (c) 2017 Jochen Hoenicke
 * based on code Copyright (c) 2017 Peter Wuille
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

static uint64_t PolyMod(const char *input, uint64_t startValue) {
	for (unsigned int i = 0; i < 34; i++) {
		const uint64_t c0 = startValue >> 35;
		startValue = ((startValue & 0x07ffffffffULL) << 5) ^
			(uint64_t)(input[i]) ^
			(-((c0 >> 0) & 1) & 0x98f2bc8e61ULL) ^
			(-((c0 >> 1) & 1) & 0x79b76d99e2ULL) ^
			(-((c0 >> 2) & 1) & 0xf33e5fb3c4ULL) ^
			(-((c0 >> 3) & 1) & 0xae2eabe2a8ULL) ^
			(-((c0 >> 4) & 1) & 0x1e4f43e470ULL);
	}
	for (unsigned int i = 34; i < 42; i++) {
		const uint64_t c0 = startValue >> 35;
		startValue = ((startValue & 0x07ffffffffULL) << 5) ^
			(-((c0 >> 0) & 1) & 0x98f2bc8e61ULL) ^
			(-((c0 >> 1) & 1) & 0x79b76d99e2ULL) ^
			(-((c0 >> 2) & 1) & 0xf33e5fb3c4ULL) ^
			(-((c0 >> 3) & 1) & 0xae2eabe2a8ULL) ^
			(-((c0 >> 4) & 1) & 0x1e4f43e470ULL);
	}
	return startValue ^ 1;
}

// End of Jochen Hoenicke + Pieter Wuille licenses

static inline void CreateChecksum(
	const int isTestNet, const char *payload, char *result) {
	// https://play.golang.org/p/sM_CE4AQ7Vp
	const uint64_t mod =
		PolyMod(payload, isTestNet == 0 ? 1058337025301 : 584719417569);
	for (unsigned int i = 0; i < 8; ++i) {
		result[i] = (mod >> (5 * (7 - i))) & 0x1f;
	}
}

void CashAddrEncode(int isTestNet, const unsigned char *payload,
	unsigned int type, unsigned int withPrefix, char *output) {
	char *data;
	if (withPrefix) {
		output[0] = 'b';
		if (isTestNet) {
			output[1] = 'c';
			output[2] = 'h';
			output[3] = 't';
			output[4] = 'e';
			output[5] = 's';
			output[6] = 't';
			output[7] = ':';
			data = &output[8];
			output[50] = '\0';
		} else {
			output[1] = 'i';
			output[2] = 't';
			output[3] = 'c';
			output[4] = 'o';
			output[5] = 'i';
			output[6] = 'n';
			output[7] = 'c';
			output[8] = 'a';
			output[9] = 's';
			output[10] = 'h';
			output[11] = ':';
			data = &output[12];
			output[54] = '\0';
		}
	} else {
		output[42] = '\0';
		data = output;
	}
	char *checksum = &data[34];
	convertBitsEightToFive(payload, type, (unsigned char *) data);
	CreateChecksum(isTestNet, data, checksum);
	for (checksum += 8; data < checksum; data++) {
		*data = CHARSET[(int) (*data)];
	}
}
