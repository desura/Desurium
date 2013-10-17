/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>
*/


#include "Common.h"

#include "util/UtilMisc.h"
#include "string.h"

#include "third_party/GeneralHashFunctions.h"
#include "MD5Wrapper.h"

#include "boost/date_time/posix_time/conversion.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

#include "boost/date_time/local_time_adjustor.hpp"
#include "boost/date_time/c_local_time_adjustor.hpp"

namespace bpt = boost::posix_time;


unsigned int FastRSHash(const char* buff, size_t size);

namespace UTIL
{
namespace MISC
{

uint8 getCoreCount()
{

#ifdef WIN32
	SYSTEM_INFO sysinfo;
	GetSystemInfo( &sysinfo );

	return (uint8)sysinfo.dwNumberOfProcessors;
#endif

#ifdef MACOS
	uint8 numCPU = 1;

	nt mib[4];
	size_t len; 

	/* set the mib for hw.ncpu */
	mib[0] = CTL_HW;
	mib[1] = HW_AVAILCPU;  // alternatively, try HW_NCPU;

	/* get the number of CPUs from the system */
	sysctl(mib, 2, &numCPU, &len, NULL, 0);

	if( numCPU < 1 ) 
	{
		 mib[1] = HW_NCPU;
		 sysctl( mib, 2, &numCPU, &len, NULL, 0 );

		 if( numCPU < 1 )
		 {
			  numCPU = 1;
		 }
	}
	return numCPU;
#endif

#ifdef __linux 
	return (uint8)sysconf( _SC_NPROCESSORS_ONLN );
#endif
}


int64 atoll(const char *ca )
{
	int64 ig=0;
	int8 sign=1;

	/* test for prefixing white space */
	while (*ca == ' ' || *ca == '\t' )
		ca++;

	/* Check sign entered or no */
	if ( *ca == '-' )
		sign = -1;

	/* convert string to int */
	while (*ca != '\0')
	{
		if (*ca >= '0' && *ca <= '9')
			ig = ig * 10LL + *ca++ - '0';
		else
			ca++;
	}

	return (ig*(int64)sign);
}

bool matchList(const char* str, const char **list)
{
	if (!str || str[0] == '\0')
		return false;

	int x=0;

	while (list[x])
	{
		if (strcmp(str,list[x])==0)
			return true;

		x++;
	}

	return false;
}



uint32 pow(uint32 num, uint32 power)
{
	if (power == 0)
		return 1;

	uint32 res = 1;
	for (uint32 x=0; x<power; x++)
		res *= num;

	return res;
}

uint32 hextoDec(const char* hexstr, uint32 size)
{
	uint32 *arr = new uint32[size+1];

	for (uint32 x = 0; x<size; x++)
	{
		switch (hexstr[x])
		{
		case '0': arr[x] = 0; break;
		case '1': arr[x] = 1; break;
		case '2': arr[x] = 2; break;
		case '3': arr[x] = 3; break;
		case '4': arr[x] = 4; break;
		case '5': arr[x] = 5; break;
		case '6': arr[x] = 6; break;
		case '7': arr[x] = 7; break;
		case '8': arr[x] = 8; break;
		case '9': arr[x] = 9; break;
		case 'A': arr[x] = 10; break;
		case 'B': arr[x] = 11; break;
		case 'C': arr[x] = 12; break;
		case 'D': arr[x] = 13; break;
		case 'E': arr[x] = 14; break;
		case 'F': arr[x] = 15; break;
		}
	}

	uint32 res = 0;

	for (uint32 x = 0; x<size; x++)
	{
		//printf("[%d, %d, %d]", arr[x], size-x-1, arr[x]*POW(16, size-x-1));
		res += arr[x]*pow(16, size-x-1);
	}

	delete [] arr;

	return res;
}



uint32 xtod(char c)
{
	if (c>='0' && c<='9')
		return c-'0';

	if (c>='A' && c<='F')
		return c-'A'+10;

	if (c>='a' && c<='f')
		return c-'a'+10;

	return 0;
}

uint32 doHexToDec(uint32 hex, uint32 offset)
{
	for (uint32 x=0; x<offset; x++)
		hex *= 16;

	return hex;
}

uint32 hextoDec(char *hex, uint32& depth)
{
	if (hex[0]=='\0')
		return 0;

	uint32 res = hextoDec(hex+1, depth) + doHexToDec(xtod(hex[0]), depth);
	depth++;

	return res;
}

uint32 hextoDec(char *hex)
{
	uint32 depth = 0;
	return hextoDec(hex, depth);
}

std::string niceSizeStr(uint64 size, bool capAtMib)
{
	char temp[100];
	if (size <= 1024)
	{
		Safe::snprintf(temp, 100, "%lld Bytes", size);
	}
	else if (size <= 1024*1024)
	{
		Safe::snprintf(temp, 100, "%0.2f KiB", (float)size/1024.0);
	}
	else if (size <= 1024*1024*1024 || capAtMib)
	{
		Safe::snprintf(temp, 100, "%0.2f MiB", (float)size/(float)(1024*1024));
	}
	else
	{
		Safe::snprintf(temp, 100, "%0.2f GiB", (float)size/(float)(1024*1024*1024));
	}

	return temp;
}

std::string genTimeString(uint8 hours, uint8 mins, uint32 rate)
{
	std::string out;

	if (mins == (uint8)-1 || hours == (uint8)-1)
		return "Unknown";

	if (mins == 0 && hours == 0)
		out = "Under a Minute";
	else if (mins == 1 && hours == 0)
		out = "1 Minute";
	else if (hours == 0)
		out = gcString("{0} Minutes", (uint32)mins);
	else if (hours == 1 && mins == 0)
		out = "1 Hour";
	else if (hours == 1)
		out = gcString("1 Hour, {0} Minutes", (uint32)mins);
	else
		out = gcString("{0} Hours, {1} Minutes", (uint32)hours, (uint32)mins);

	if (rate != 0)
	{
		const char *rateExt = NULL;

		uint32 unit = 0;
		double dRate = rate;

		while (dRate > 1024.0 && unit <= 3)
		{
			dRate /= 1024.0;
			unit++;
		}

		switch (unit)
		{
			default:
			case 0: rateExt = "B/s"; break;
			case 1: rateExt = "KiB/s"; break;
			case 2: rateExt = "MiB/s"; break;
			case 3: rateExt = "GiB/s"; break;
		}


		char temp[100] = {0};
		Safe::snprintf(temp, 100, " [ %0.1f %s ]", dRate, rateExt);

		out += temp;
	}

	return out;
}


bool isWebURL(const char* str)
{
	if (!str)
		return false;

	return (strncmp(str, "http://", 7) == 0 || strncmp(str, "ftp://", 6) == 0 || strncmp(str, "https://", 8) == 0);
}





IMAGE_TYPES isValidImage(const unsigned char h[5])
{
	
	//GIF8
	if (h[0] == 71 && h[1] == 73 && h[2] == 70 && h[3] == 56)
	{
		return IMAGE_GIF;
	}

	//89 PNG
	if (h[0] == 137 && h[1] == 80 && h[2] == 78 && h[3] == 71)
	{
		return IMAGE_PNG;
	}
	
	//FFD8
	if (h[0] == 255 && h[1] == 216)
	{
		return IMAGE_JPG;
	}

	return IMAGE_VOID;
}





bool isValidEmail(const char* email)
{
	if (!email)
		return false;

	const char* atSym = strstr(email, "@");

	if (!atSym)
		return false;

	const char* dotSym = strstr(atSym, ".");

	if (!dotSym)
		return false;

	return true;
}

void getTimeDiffFromNow(const char* dateTime, uint32 &days, uint32 &hours, bool dtIsUTC)
{
	days = 0;
	hours = 0;

	if (!dateTime)
		return;

	try
	{
		bpt::ptime t(bpt::from_iso_string(dateTime));
		bpt::ptime now(bpt::second_clock::universal_time());

		t = boost::date_time::local_adjustor<bpt::ptime, 0, bpt::no_dst>::utc_to_local(t);

		if (dtIsUTC)
			t = boost::date_time::c_local_adjustor<bpt::ptime>::utc_to_local(t);

		bpt::time_duration diff = t - now;

		if (diff.is_negative())
			return;

		days = diff.hours()/24;
		hours = diff.hours() - days*24;
	}
	catch (...)
	{
		return;
	}
}



uint32 RSHash_CSTR(const char* buff, size_t size)
{
	return ::FastRSHash(buff, size);
}

uint32 RSHash_CSTR(const std::string &str)
{
	return ::RSHash(str);
}

uint64 FNVHash64(const std::string &str)
{
	return ::FNVHash64(str);
}

void hashString(const char* str, uint32 size, char out[16])
{
	md5wrapper::getHashFromBuffer((const unsigned char*)str, size, (unsigned char*)out);
}

std::string hashString(const char* str, uint32 size)
{
	return md5wrapper::getHashFromBuffer((const unsigned char*)str, size);
}

std::string hashFile(const std::string file)
{
	return md5wrapper::getHashFromFile(file);
}

std::string hashFile(FHANDLE file, uint64 size)
{
#ifdef WIN32
	return md5wrapper::getHashFromFile((HANDLE)file, size);
#else
	return md5wrapper::getHashFromFile((FILE*)file, size);
#endif
}



//crc table from http://www.codeproject.com/KB/recipes/crc32.aspx
const unsigned long ulTable[256] =
{
	0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
	0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
	0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
	0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
	0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
	0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
	0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
	0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
	0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
	0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
	0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
	0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
	0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
	0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
	0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
	0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,

	0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
	0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
	0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
	0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
	0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
	0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
	0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
	0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
	0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
	0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
	0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
	0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
	0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
	0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
	0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
	0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,

	0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
	0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
	0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
	0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
	0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
	0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
	0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
	0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
	0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
	0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
	0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
	0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
	0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
	0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
	0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
	0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,

	0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
	0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
	0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
	0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
	0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
	0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
	0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
	0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
	0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
	0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
	0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
	0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
	0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
	0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
	0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
	0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D,
};



unsigned long CRC32(const unsigned char byte, unsigned long dwCrc32)
{
	return ((dwCrc32) >> 8) ^ ulTable[(byte) ^ ((dwCrc32) & 0x000000FF)];
}

unsigned long CRC32(const unsigned char* str, uint32 len)
{
	unsigned long ulCRC = 0xFFFFFFFF; //Initilaize the CRC.

	for (uint32 x=0; x<len; x++)
		ulCRC = ((ulCRC) >> 8) ^ ulTable[(str[x]) ^ ((ulCRC) & 0x000000FF)];

	return ~ulCRC; //Finalize the CRC and return.;
}














class BufferData
{
public:
	BufferData(size_t size)
	{
		this->size = size;
		data = (char*)malloc(size+1);
		data[size] = 0;
	}

	~BufferData()
	{
		free(data);
	}

	size_t size;
	char* data;
};



Buffer::Buffer(size_t size, bool zero)
{
	m_pData = new BufferData(size);

	if (zero)
		this->zero();
}

Buffer::~Buffer()
{
	safe_delete(m_pData);
}
		
void Buffer::resize(size_t newSize)
{
	m_pData->data = (char*)realloc(m_pData->data, newSize);
	m_pData->size = newSize;
}

void Buffer::zero()
{
	memset(m_pData->data, 0, m_pData->size);
}

size_t Buffer::size()
{
	return m_pData->size;
}

char* Buffer::data()
{
	return m_pData->data;
}

Buffer::operator char*()
{
	return m_pData->data;
}




}
}
