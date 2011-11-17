// MD5.CC - source code for the C++/object oriented translation and 
//          modification of MD5.

// Translation and modification (c) 1995 by Mordechai T. Abzug 

// This translation/ modification is provided "as is," without express or 
// implied warranty of any kind.

// The translator/ modifier does not claim (1) that MD5 will do what you think 
// it does; (2) that this translation/ modification is accurate; or (3) that 
// this software is "merchantible."  (Language for this disclaimer partially 
// copied from the disclaimer below).

/* based on:

MD5C.C - RSA Data Security, Inc., MD5 message-m_Digest algorithm
MDDRIVER.C - test driver for MD2, MD4 and MD5


Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-m_Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-m_Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.

*/


#include "md5.h"
#include "Common.h" // KP 28apr2011 - For Safe:snprintf

#include <assert.h>
#include <string>
#include <string.h>

// Encodes input (unsigned int) into output (unsigned char). Assumes len is
// a multiple of 4.
void encode(unsigned char *output, unsigned int *input, size_t len) 
{
	for (size_t i = 0, j = 0; j < len; i++, j += 4) 
	{
		output[j]   = (unsigned char)(input[i] & 0xff);
		output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
		output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
		output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
	}
}

// Decodes input (unsigned char) into output (unsigned int). Assumes len is
// a multiple of 4.
void decode(unsigned int *output, unsigned char *input, unsigned int len)
{
	unsigned int i, j;

	for (i = 0, j = 0; j < len; i++, j += 4)
	{
		output[i] = ((unsigned int)input[j]) | (((unsigned int)input[j+1]) << 8) | (((unsigned int)input[j+2]) << 16) | (((unsigned int)input[j+3]) << 24);
	}
}

// ROTATE_LEFT rotates x left n bits.
inline unsigned int rotate_left(unsigned int x, unsigned int n)
{
	return (x << n) | (x >> (32-n));
}

inline unsigned int F(unsigned int x, unsigned int y, unsigned int z)
{
	return (x & y) | (~x & z);
}

inline unsigned int G(unsigned int x, unsigned int y, unsigned int z)
{
	return (x & z) | (y & ~z);
}

inline unsigned int H(unsigned int x, unsigned int y, unsigned int z)
{
	return x ^ y ^ z;
}

inline unsigned int I(unsigned int x, unsigned int y, unsigned int z)
{
	return y ^ (x | ~z);
}

// FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
// Rotation is separate from addition to prevent recomputation.
inline void FF(unsigned int& a, unsigned int b, unsigned int c, unsigned int d, unsigned int x, unsigned int  s, unsigned int ac)
{
	a += F(b, c, d) + x + ac;
	a = rotate_left(a, s) +b;
}

inline void GG(unsigned int& a, unsigned int b, unsigned int c, unsigned int d, unsigned int x, unsigned int s, unsigned int ac)
{
	a += G(b, c, d) + x + ac;
	a = rotate_left(a, s) +b;
}

inline void HH(unsigned int& a, unsigned int b, unsigned int c, unsigned int d, unsigned int x, unsigned int s, unsigned int ac)
{
	a += H(b, c, d) + x + ac;
	a = rotate_left(a, s) +b;
}

inline void II(unsigned int& a, unsigned int b, unsigned int c, unsigned int d, unsigned int x, unsigned int s, unsigned int ac)
{
	a += I(b, c, d) + x + ac;
	a = rotate_left(a, s) +b;
}

static unsigned char PADDING[64]=
{
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


// Constants for MD5Transform routine.
// Although we could use C++ style constants, defines are actually better,
// since they let us easily evade scope clashes.

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21




std::string MD5::CalcMD5(const unsigned char *string, size_t length)
{
	MD5 md5;

	md5.update(string, length);
	md5.finalize();

	return md5.getMd5();
}

std::string MD5::CalcMD5(FILE *file)
{
	MD5 md5;

	md5.update(file);
	md5.finalize();

	return md5.getMd5();
}

void MD5::CalcMD5(const unsigned char *string, size_t length, unsigned char out[16])
{
	MD5 md5;

	md5.update(string, length);
	md5.finalize();

	unsigned char* d = md5.getDigest();
	memcpy(out, d, 16);
}


MD5::MD5()
{
	m_Finalized=0;

	m_Count[0] = 0;
	m_Count[1] = 0;

	// Load magic initialization constants.
	m_State[0] = 0x67452301;
	m_State[1] = 0xefcdab89;
	m_State[2] = 0x98badcfe;
	m_State[3] = 0x10325476;
}

// MD5 block update operation. Continues an MD5 message-m_Digest
// operation, processing another message block, and updating the
// context.
void MD5::update(const unsigned char *input, size_t input_length) 
{
	if (m_Finalized)
		return;

	unsigned int input_index;
	unsigned int index;
	unsigned int buffSpace;


	// Compute number of bytes mod 64
	index = (unsigned int)((m_Count[0] >> 3) & 0x3F);

	// Update number of bits
	if ((m_Count[0] += ((unsigned int) input_length << 3)) < ((unsigned int) input_length << 3))
		m_Count[1]++;

	m_Count[1] += ((unsigned int)input_length >> 29);
	buffSpace = 64 - index;  // how much space is left in m_Buffer

	// Transform as many times as possible.
	if (input_length >= buffSpace) 
	{ 
		// ie. we have enough to fill the m_Buffer
		// fill the rest of the m_Buffer and transform
		memcpy(m_Buffer + index, input, buffSpace);
		transform(m_Buffer);

		// now, transform each 64-byte piece of the input, bypassing the m_Buffer
		for (input_index = buffSpace; input_index + 63 < input_length; input_index += 64)
		{
			transform(const_cast<unsigned char*>(input)+input_index);
		}

		index = 0;  // so we can m_Buffer remaining
	}
	else
	{
		input_index=0;     // so we can m_Buffer the whole input
	}

	// and here we do the m_Buffering:
	memcpy(m_Buffer+index, input+input_index, input_length-input_index);
}

// MD5 update for files.
// Like above, except that it works on files (and uses above as a primitive.)
void MD5::update(FILE *file)
{
	unsigned char buf[1024] = {0};
	size_t len;
	size_t total = 0;

	do
	{
		len=fread(buf, 1, 1024, file);

		if (len == 0)
			break;

		update(buf, len);
		total += len;
	}
	while (true);
}

// MD5 finalization. Ends an MD5 message-m_Digest operation, writing the
// the message m_Digest and zeroizing the context.
void MD5::finalize()
{
	if (m_Finalized)
		return;
	
	unsigned char bits[8] = {0};
	unsigned int index;
	unsigned int padLen;
	
	// Save number of bits
	encode(bits, m_Count, 8);

	// Pad out to 56 mod 64.
	index = (unsigned int) ((m_Count[0] >> 3) & 0x3f);
	padLen = (index < 56) ? (56 - index) : (120 - index);
	update(PADDING, padLen);

	update(bits, 8);
	encode(m_Digest, m_State, 16);
	memset(m_Buffer, 0, sizeof(*m_Buffer));

	m_Finalized=true;
}

std::string MD5::getMd5()
{
	char s[33] = {0};

	if (!m_Finalized)
		return "-1";

	for (size_t i=0; i<16; i++)
		Safe::snprintf(s+i*2, 3, "%02x", m_Digest[i]); // KP 28apr2011 - Changed to Safe::snprintf instead of sprintf_s

	s[32]='\0';
	return s;
}

unsigned char* MD5::getDigest()
{
	return m_Digest;
}

// MD5 basic transformation. Transforms m_State based on block.
void MD5::transform(unsigned char block[64])
{
	unsigned int a = m_State[0], b = m_State[1], c = m_State[2], d = m_State[3], x[16];

	decode(x, block, 64);
	assert(!m_Finalized);  // not just a user error, since the method is private

	/* Round 1 */
	FF(a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
	FF(d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
	FF(c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
	FF(b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
	FF(a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
	FF(d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
	FF(c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
	FF(b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
	FF(a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
	FF(d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
	FF(c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
	FF(b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
	FF(a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
	FF(d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
	FF(c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
	FF(b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

	/* Round 2 */
	GG(a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
	GG(d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
	GG(c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
	GG(b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
	GG(a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
	GG(d, a, b, c, x[10], S22,  0x2441453); /* 22 */
	GG(c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
	GG(b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
	GG(a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
	GG(d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
	GG(c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
	GG(b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
	GG(a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
	GG(d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
	GG(c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
	GG(b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

	/* Round 3 */
	HH(a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
	HH(d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
	HH(c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
	HH(b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
	HH(a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
	HH(d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
	HH(c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
	HH(b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
	HH(a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
	HH(d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
	HH(c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
	HH(b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
	HH(a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
	HH(d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
	HH(c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
	HH(b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

	/* Round 4 */
	II(a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
	II(d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
	II(c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
	II(b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
	II(a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
	II(d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
	II(c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
	II(b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
	II(a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
	II(d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
	II(c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
	II(b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
	II(a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
	II(d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
	II(c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
	II(b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

	m_State[0] += a;
	m_State[1] += b;
	m_State[2] += c;
	m_State[3] += d;

	// Zeroize sensitive information.
	memset(x, 0, sizeof(x));
}
