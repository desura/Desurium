///////////// Copyright 2010 DesuraNet. All rights reserved. /////////////
//
//   Project     : courgette
//   File        : Courgette.cpp
//   Description :
//      [TODO: Write the purpose of Courgette.cpp.]
//
//   Created On: 5/24/2011 12:59:39 PM
//   Created By:  <mailto:>
////////////////////////////////////////////////////////////////////////////


#include <stdlib.h>
#include <string.h>

#include "courgette/third_party/bsdiff.h"
#include "courgette/courgette.h"
#include "courgette/streams.h"

#include "CourgetteI.h"

#ifdef _WIN32
	#define CEXPORT __declspec(dllexport)
	#define CIMPORT __declspec(dllimport)
#else
	#define CEXPORT
	#define CIMPORT
#endif

namespace courgette
{
	class Varint 
	{
	public:
		static const int kMax32 = 5;
		static const uint8* Parse32WithLimit(const uint8* source, const uint8* limit, uint32* output);
		static uint8* Encode32(uint8* destination, uint32 value);
	};
}


class SinkStream : public courgette::SinkStream
{
public:
	SinkStream(CourgetteCallbackI* callback)
	{
		m_uiDone = 0;
		m_pCallback = callback;
	}

	virtual void Write(const void* data, size_t byte_count)
	{
		m_uiDone += byte_count;
		m_pCallback->write(static_cast<const char*>(data), byte_count);
	}

	virtual size_t Length() const 
	{ 
		return m_uiDone;
	}

	virtual const uint8* Buffer() const 
	{
		return NULL;
	}

	virtual void Reserve(size_t length) 
	{
	}

	size_t m_uiDone;
	CourgetteCallbackI* m_pCallback;
};

class Courgette : public CourgetteI
{
public:
	virtual void destory()
	{
		delete this;
	}

	virtual bool createDiff(CourgetteBufferI* oldBuffer, CourgetteBufferI* newBuffer, CourgetteCallbackI* callback)
	{
		if (!callback || !oldBuffer || !newBuffer)
			return false;

		courgette::SourceStream old_stream;
		courgette::SourceStream new_stream;

		old_stream.Init(oldBuffer->getBuffer(), oldBuffer->getSize());
		new_stream.Init(newBuffer->getBuffer(), newBuffer->getSize());

		SinkStream out_stream(callback);

		courgette::Status status = courgette::GenerateEnsemblePatch(&old_stream, &new_stream, &out_stream);

		return (status == courgette::C_OK);
	}

	virtual bool applyDiff(CourgetteBufferI* oldBuffer, CourgetteBufferI* diffBuffer, CourgetteCallbackI* callback)
	{
		if (!callback || !oldBuffer || !diffBuffer)
			return false;

		courgette::SourceStream old_stream;
		courgette::SourceStream diff_stream;

		old_stream.Init(oldBuffer->getBuffer(), oldBuffer->getSize());
		diff_stream.Init(diffBuffer->getBuffer(), diffBuffer->getSize());
		
		SinkStream out_stream(callback);

		courgette::Status status = courgette::ApplyEnsemblePatch(&old_stream, &diff_stream, &out_stream);

		//ignore crc as desura will check its own and has issues with null buffers
		if (status == courgette::C_BAD_ENSEMBLE_CRC)
			status = courgette::C_OK;

		return (status == courgette::C_OK);
	}

	static void* FactoryBuilder(const char* name)
	{
		if (strcmp(name, COURGETTE)==0)
		{
			return static_cast<void*>(new Courgette());
		}

		return NULL;
	}
};

extern "C"
{
	CEXPORT void* FactoryBuilderCourgette(const char* name)
	{
		return Courgette::FactoryBuilder(name);
	}
}
