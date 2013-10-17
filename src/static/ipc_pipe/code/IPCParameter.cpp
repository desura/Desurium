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
#include "IPCParameter.h"

IPC::IPCParameterI* newParameterMacro(IPC::PVoid val)
{
	return new IPC::PVoid();
}

IPC::IPCParameterI* new_void()
{
	return new IPC::PVoid();
}

IPC::AutoReg<void> a = IPC::AutoReg<void>( &new_void );

//REGTYPE( PVoid, void );
REGTYPE( IPC::PUint32, uint8 );
REGTYPE( IPC::PUint32, uint32 );
REGTYPE( IPC::PInt32, int32 );
REGTYPE( IPC::PUint64, uint64 );
REGTYPE( IPC::PBool, bool );
REGTYPE( IPC::PException, gcException );
REGTYPEP( IPC::PString, char );
REGTYPEC( IPC::PString, char const, charconst );
REGTYPE( IPC::PDouble, double );

IPC::IPCParameterI* new_ipc_blob ()
{
	return new IPC::PBlob();
}

IPC::AutoReg<IPC::PBlob> ar_ipc_blob = IPC::AutoReg<IPC::PBlob>( new_ipc_blob );

IPC::IPCParameterI* newParameterMacro(IPC::PBlob val)
{
	return new IPC::PBlob(val);
}



namespace IPC
{

std::map<uint32, newFunc> *g_vParameterType = NULL;

class AutoCleanUp
{
public:
	~AutoCleanUp()
	{
		safe_delete(g_vParameterType);
	}
};

AutoCleanUp acu;

void RegType(const std::type_info& type, newFunc funct)
{
	if (!g_vParameterType)
	{
		g_vParameterType = new std::map<uint32, newFunc>;
	}

	uint32 hash = UTIL::MISC::RSHash_CSTR(type.name());
	(*g_vParameterType)[ hash ] = funct;
}

IPCParameterI* newParameter(uint32 type, const char* buff, uint32 size)
{
	if (!g_vParameterType)
		return new PVoid();

	std::map<uint32, newFunc>::iterator it = g_vParameterType->find(type);

	if (it == g_vParameterType->end())
		return new PVoid();

	IPCParameterI* p = it->second();
	p->deserialize(buff, size);

	return p;
}







PVoid::PVoid()
{
}

char* PVoid::serialize(uint32 &size)
{
	size = 0;
	return NULL;
}

uint32 PVoid::deserialize(const char* buffer, uint32 size)
{
	return 0;
}

uint64 PVoid::getValue(bool dup)
{
	return 0;
}


PBool::PBool()
{
	m_bValue = false;
}

PBool::PBool(bool val)
{
	m_bValue = val;
}

char* PBool::serialize(uint32 &size)
{
	size = 1;
	char* str = new char[1];
	str[0] = m_bValue;
	return str;
}

uint32 PBool::deserialize(const char* buffer, uint32 size)
{
	if (size >= 1)
	{
		m_bValue = buffer[0]?true:false;
			return 1;
	}

	return 0;
}

uint64 PBool::getValue(bool dup)
{
	return (uint64)m_bValue;
}


PUint32::PUint32()
{
	m_uiValue = 0;
}

PUint32::PUint32(uint32 val)
{
	m_uiValue = val;
}

char* PUint32::serialize(uint32 &size)
{
	size = 4;
	char* str = new char[4];
	str[0] = m_uiValue&0xFF;
	str[1] = (m_uiValue>>8)&0xFF;
	str[2] = (m_uiValue>>16)&0xFF;
	str[3] = (m_uiValue>>24)&0xFF;
	return str;
}

uint32 PUint32::deserialize(const char* buffer, uint32 size)
{
	if (size >= 4)
	{
		m_uiValue = buffToUint32(buffer);
		return 4;
	}

	return 0;
}

uint64 PUint32::getValue(bool dup)
{
	return (uint64)m_uiValue;
}






PInt32::PInt32()
{
	m_iValue = 0;
}

PInt32::PInt32(int32 value)
{
	m_iValue = value;
}

char* PInt32::serialize(uint32 &size)
{
	size = 4;

	char* str = new char[4];
	str[0] = m_iValue&0xFF;
	str[1] = (m_iValue>>8)&0xFF;
	str[2] = (m_iValue>>16)&0xFF;
	str[3] = (m_iValue>>24)&0xFF;
	return str;
}

uint32 PInt32::deserialize(const char* buffer, uint32 size)
{
	if (size >= 4)
	{
		m_iValue = buffToInt32(buffer);
		return 4;
	}

	return 0;
}

uint64 PInt32::getValue(bool dup)
{
	return (uint64)m_iValue;
}





typedef union
{
	char data[8];
	uint64 num;
} uint64_u;


PUint64::PUint64()
{
	m_uiValue = 0;
}

PUint64::PUint64(uint64 val)
{
	m_uiValue = val;
}

char* PUint64::serialize(uint32 &size)
{
	size = 8;
	char* str = new char[8];

	uint64_u u;
	u.num = m_uiValue;

	memcpy(str, u.data, 8);
	return str;
}

uint32 PUint64::deserialize(const char* buffer, uint32 size)
{
	if (size >= 8)
	{
		uint64_u u;
		memcpy(u.data, buffer, 8);
		m_uiValue = u.num;
		return 8;
	}

	return 0;
}

uint64 PUint64::getValue(bool dup)
{
	return (uint64)m_uiValue;
}


typedef union
{
	char data[8];
	double num;
} double_u;


PDouble::PDouble()
{
	m_dValue = 0.0;
}

PDouble::PDouble(double val)
{
	m_dValue = val;
}

char* PDouble::serialize(uint32 &size)
{
	size = 8;
	char* str = new char[8];

	double_u u;
	u.num = m_dValue;

	memcpy(str, u.data, 8);
	return str;
}

uint32 PDouble::deserialize(const char* buffer, uint32 size)
{
	if (size >= 8)
	{
		double_u u;
		memcpy(u.data, buffer, 8);
		m_dValue = u.num;
		return 8;
	}

	return 0;
}

uint64 PDouble::getValue(bool dup)
{
	return (uint64)m_dValue;
}




PString::PString()
{
	m_szValue = NULL;
}

PString::PString(const char* v)
{
	m_szValue = NULL;

	if (v)
	{
		size_t len = strlen(v);
		m_szValue = new char[len+1];
		Safe::strcpy(m_szValue, len+1, v);
		m_szValue[len]='\0';
	}
}

PString::~PString()
{
	delete [] m_szValue;
}




char* PString::serialize(uint32 &size)
{
	if (!m_szValue)
	{
		size = 0;
		return NULL;
	}

	uint32 strsize = strlen(m_szValue);

	if (strsize == 0)
	{
		size = 0;
		return NULL;
	}

	size = strsize+4;


	char* str = new char[strsize+4];

	str[0] = strsize&0xFF;
	str[1] = (strsize>>8)&0xFF;
	str[2] = (strsize>>16)&0xFF;
	str[3] = (strsize>>24)&0xFF;

	memcpy(str+4, m_szValue, strsize);
	return str;
}

uint32 PString::deserialize(const char* buffer, uint32 size)
{
	if (size < 4)
		return 0;

	uint32 strsize = buffToInt32(buffer);

	if (strsize > size - 4)
		return 0;

	safe_delete(m_szValue);

	m_szValue = new char[strsize+1];
	memcpy(m_szValue, buffer+4, strsize);
	m_szValue[strsize] = '\0';

	return 4+strsize;
}


uint64 PString::getValue(bool dup)
{
	if (dup)
	{
		char *res = NULL;
		Safe::strcpy(&res, m_szValue, 255);
		return (uint64)res;
	}
	else
	{
		return (uint64)m_szValue;
	}
}



PException::PException()
{
	exception = new gcException();
}

PException::PException(gcException& e)
{
	exception = new gcException(e);
}

PException::~PException()
{
	safe_delete(exception);
}

char* PException::serialize(uint32 &size)
{
	uint32 e1 = exception->getErrId();
	uint32 e2 = exception->getSecErrId();

	const char* msg = exception->getErrMsg();
	uint32 msgSize = strlen(msg);

	size = msgSize + 12;

	char* str = new char[size];

	str[0] = e1&0xFF;
	str[1] = (e1>>8)&0xFF;
	str[2] = (e1>>16)&0xFF;
	str[3] = (e1>>24)&0xFF;

	str[4] = e2&0xFF;
	str[5] = (e2>>8)&0xFF;
	str[6] = (e2>>16)&0xFF;
	str[7] = (e2>>24)&0xFF;

	str[8] = msgSize&0xFF;
	str[9] = (msgSize>>8)&0xFF;
	str[10] = (msgSize>>16)&0xFF;
	str[11] = (msgSize>>24)&0xFF;

	memcpy(&str[12], msg, msgSize);

	return str;
}

uint32 PException::deserialize(const char* buffer, uint32 size)
{
	uint32 e1;
	uint32 e2;
	uint32 msgSize = 0;
	char* msg = NULL;

	if (size < 8)
		return 0;

	e1 = buffToUint32(buffer);
	e2 = buffToUint32(buffer+4);
	msgSize = buffToUint32(buffer+8);

	if (msgSize > size - 12)
		return 0;

	msg = new char[msgSize+1];
	memcpy(msg, buffer+12, msgSize);
	msg[msgSize] = '\0';

	safe_delete(exception);
	exception = new gcException((ERROR_ID)e1, e2, msg);
	safe_delete(msg);

	return 12+msgSize;
}

uint64 PException::getValue(bool dup)
{
	if (dup)
		return (uint64)(new gcException(exception));
	else
		return (uint64)exception;
}










PBlob::PBlob()
{
	m_szData = NULL;
	m_uiSize = 0;
}

PBlob::PBlob(const PBlob& e)
{
	m_uiSize = e.getSize();
	m_szData = NULL;

	if (m_uiSize > 0)
	{
		m_szData = new char[m_uiSize];
		memcpy(m_szData, e.getData(), m_uiSize);
	}
}

PBlob::PBlob(PBlob* blob)
{
	m_uiSize = blob->getSize();

	m_szData = NULL;

	if (m_uiSize > 0)
	{
		m_szData = new char[m_uiSize];
		memcpy(m_szData, blob->getData(), m_uiSize);
	}
}

PBlob::PBlob(uint64 val)
{
	PBlob* blob = (PBlob*)val;

	m_uiSize = blob->getSize();
	m_szData = NULL;

	if (m_uiSize > 0)
	{
		m_szData = new char[m_uiSize];
		memcpy(m_szData, blob->getData(), m_uiSize);
	}
}

PBlob::PBlob(const char* data, uint32 size)
{
	m_uiSize = size;
	m_szData = NULL;

	if (size > 0)
	{
		m_szData = new char[m_uiSize];
		memcpy(m_szData, data, m_uiSize);
	}
}

PBlob::~PBlob()
{
	safe_delete(m_szData);
}

char* PBlob::serialize(uint32 &size)
{
	size = m_uiSize;

	char *ret = new char[size+4];

	ret[0] = m_uiSize&0xFF;
	ret[1] = (m_uiSize>>8)&0xFF;
	ret[2] = (m_uiSize>>16)&0xFF;
	ret[3] = (m_uiSize>>24)&0xFF;

	if (m_uiSize > 0)
		memcpy(ret+4, m_szData, size);

	size+=4;
	return ret;
}

uint32 PBlob::deserialize(const char* buffer, uint32 size)
{
	if (size >= 4)
		m_uiSize = buffToInt32(buffer);
	else
		return 0;

	if (m_uiSize > size - 4)
		return 0;

	safe_delete(m_szData);

	if (m_uiSize > 0)
	{
		m_szData = new char[m_uiSize];
		memcpy(m_szData, buffer+4, m_uiSize);
	}

	return 4+m_uiSize;
}

uint64 PBlob::getValue(bool dup)
{
	if (dup)
	{
		return (uint64)new PBlob(m_szData, m_uiSize);
	}
	else
	{
		return (uint64)this;
	}
}





}


