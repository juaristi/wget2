/*
 * Copyright(c) 2012 Tim Ruehsen
 * Copyright(c) 2015-2016 Free Software Foundation, Inc.
 *
 * This file is part of libwget.
 *
 * Libwget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Libwget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libwget.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * base64 conversion routines
 *
 * Changelog
 * 21.12.2012  Tim Ruehsen  created
 *
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stddef.h>
#include <stdarg.h>

#include <wget.h>
#include "private.h"

/**
 * \file
 * \brief Base64 functions
 * \defgroup libwget-base64 Base64 functions
 * @{
 *
 * This is a collection base64 encoding/decoding functions used in Wget2.
 */

static const unsigned char base64_2_bin[256] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 62, 0, 0, 0, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0, 0, 0, 0, 0,
	0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static int G_GNUC_WGET_CONST _isbase64(char c)
{
	// isalnum(c) does not work for all locales
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '+' || c == '/';
}

/**
 * \param[in] src String to be checked
 * \return 1 if \p src is a base64 string, 0 otherwise
 *
 * Checks wether \p src is a base64 string.
 * Returns 0 if \p src is NULL.
 */
int wget_base64_is_string(const char *src)
{
	if (src) {
		while (_isbase64(*src)) src++;

		if (!*src || (*src == '=' && src[1]) || (*src == '=' && src[1] == '=' && src[2]))
			return 1;
	}

	return 0;
}

/**
 * \param[out] dst Output buffer
 * \param[in] src Base64 string to be decoded
 * \param[in] n Length of \p src
 * \return Number of bytes written into \p dst
 *
 * Decodes \p n bytes of the base64 string \p src.
 * The decoded bytes are written into \p dst and are 0-terminated.
 *
 * The size of \p dst has to be at minimum the number of bytes returned
 * by wget_base64_get_decoded_length().
 */
size_t wget_base64_decode(char *dst, const char *src, int n)
{
	const unsigned char *usrc = (const unsigned char *)src;
	char *old = dst;
	int extra;

	// trim '=' at the end
	while (n > 0 && !_isbase64(usrc[n - 1]))
		n--;

	extra = n & 3;

	for (n /= 4; --n >= 0; usrc += 4) {
		*dst++ = base64_2_bin[usrc[0]] << 2 | base64_2_bin[usrc[1]] >> 4;
		*dst++ = (base64_2_bin[usrc[1]]&0x0F) << 4 | base64_2_bin[usrc[2]] >> 2;
		*dst++ = (base64_2_bin[usrc[2]]&0x03) << 6 | base64_2_bin[usrc[3]];
	}

	switch (extra) {
	case 1:
		// this should not happen
		*dst++ = base64_2_bin[usrc[0]] << 2;
		break;
	case 2:
		*dst++ = base64_2_bin[usrc[0]] << 2 | base64_2_bin[usrc[1]] >> 4;
		*dst = (base64_2_bin[usrc[1]]&0x0F) << 4;
		if (*dst) dst++;
		break;
	case 3:
		*dst++ = base64_2_bin[usrc[0]] << 2 | base64_2_bin[usrc[1]] >> 4;
		*dst++ = (base64_2_bin[usrc[1]]&0x0F) << 4 | base64_2_bin[usrc[2]] >> 2;
		*dst = (base64_2_bin[usrc[2]]&0x03) << 6;
		if (*dst) dst++;
		break;
	default: // 0: ignore
		break;
	}

	*dst = 0;
	return (size_t) (dst - old);
}

/**
 * \param[in] src Base64 string to be decoded
 * \param[in] n Length of \p src
 * \return Decoded bytes, zero terminated
 *
 * Decodes \p n bytes of the base64 string \p src.
 * The decoded bytes are returned in an allocated buffer.
 *
 * You should free() the returned string when not needed any more.
 */
char *wget_base64_decode_alloc(const char *src, int n)
{
	char *dst = xmalloc(wget_base64_get_decoded_length(n));

	wget_base64_decode(dst, src, n);

	return dst;
}

/**
 * \param[out] dst Base64 output string
 * \param[in] src Input buffer
 * \param[in] n Number of bytes to be encoded
 * \return Length of output string \p dst
 *
 * Encodes \p n bytes from \p src into a base64 string.
 * The encoded string is written into \p dst (0-terminated).
 *
 * The length of \p dst has to be at minimum the number of bytes returned
 * by wget_base64_get_encoded_length().
 */
size_t wget_base64_encode(char *dst, const char *src, int n)
{
	static const char base64[64] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	const unsigned char *usrc = (const unsigned char *)src;
	char *start = dst;
	int extra = n % 3;

	// convert line by line
	for (n /= 3; --n >= 0; usrc += 3) {
		*dst++ = base64[usrc[0] >> 2];
		*dst++ = base64[((usrc[0]&3) << 4) | (usrc[1] >> 4)];
		*dst++ = base64[((usrc[1]&15) << 2) | (usrc[2] >> 6)];
		*dst++ = base64[usrc[2]&0x3f];
	}

	// special case
	if (extra == 1) {
		*dst++ = base64[usrc[0] >> 2];
		*dst++ = base64[(usrc[0]&3) << 4];
		*dst++ = '=';
		*dst++ = '=';
	} else if (extra == 2) {
		*dst++ = base64[usrc[0] >> 2];
		*dst++ = base64[((usrc[0]&3) << 4) | (usrc[1] >> 4)];
		*dst++ = base64[((usrc[1]&15) << 2)];
		*dst++ = '=';
	}

	*dst = 0;

	return (size_t) (dst - start);
}

/**
 * \param[in] src Input buffer
 * \param[in] n Number of bytes to be encoded
 * \return Base64 encoded string
 *
 * Encodes \p n bytes from input buffer \p src.
 * The encoded string is returned in an allocated buffer.
 *
 * You should free() the returned string when not needed any more.
 */
char *wget_base64_encode_alloc(const char *src, int n)
{
	char *dst = xmalloc(wget_base64_get_encoded_length(n));

	wget_base64_encode(dst, src, n);

	return dst;
}

/**
 * \param[in] fmt Printf-like format string
 * \param[in] args Argument list
 * \return Base64 encoded string
 *
 * Encodes the string constructed by \p fmt and \p args.
 * The encoded string is returned in an allocated buffer.
 *
 * You should free() the returned string when not needed any more.
 */
char *wget_base64_encode_vprintf_alloc(const char *fmt, va_list args)
{
	char *data = NULL;
	int n;

	n = wget_vasprintf(&data, fmt, args);

	if (data) {
		char *dst = wget_base64_encode_alloc(data, n);
		xfree(data);
		return dst;
	}

	return NULL;
}

/**
 * \param[in] fmt Printf-like format string
 * \param[in] ... Argument list
 * \return Base64 encoded string
 *
 * Encodes the string constructed by \p fmt and the arguments.
 * The encoded string is returned in an allocated buffer.
 *
 * You should free() the returned string when not needed any more.
 */
char *wget_base64_encode_printf_alloc(const char *fmt, ...)
{
	char *dst;
	va_list args;

	va_start(args, fmt);
	dst = wget_base64_encode_vprintf_alloc(fmt, args);
	va_end(args);

	return dst;
}

/**@}*/
