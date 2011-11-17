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

MD5.H - header file for MD5C.C
MDDRIVER.C - test driver for MD2, MD4 and MD5

Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.

*/

#include <stdio.h>
#include <string>

class MD5 
{
public:
	static std::string CalcMD5(const unsigned char *string, size_t length);
	static std::string CalcMD5(FILE *file);

	static void CalcMD5(const unsigned char *string, size_t length, unsigned char out[16]);

	MD5();

	void  update(const unsigned char *input, size_t length);
	void  update(FILE *file);
	void  finalize();

	std::string getMd5();
	unsigned char* getDigest();

protected:
	void transform(unsigned char *buffer);

private:
	unsigned int m_State[4];
	unsigned int m_Count[2];
	unsigned char m_Buffer[64];
	unsigned char m_Digest[16];
	bool m_Finalized;
};
