///////////// Copyright 2010 DesuraNet. All rights reserved. /////////////
//
//   Project     : mcfcore
//   File        : CourgetteI.h
//   Description :
//      [TODO: Write the purpose of CourgetteI.h.]
//
//   Created On: 5/24/2011 12:38:49 PM
//   Created By:  <mailto:>
////////////////////////////////////////////////////////////////////////////

#ifndef DESURA_COURGETTEI_H
#define DESURA_COURGETTEI_H
#ifdef _WIN32
#pragma once
#endif

#define COURGETTE "COURGETTE_001"

typedef char* (*CreateBufferFn)(size_t);

class CourgetteCallbackI
{
public:
	//! Write output data
	//!
	virtual bool write(const char* data, size_t size)=0;
};

class CourgetteBufferI
{
public:
	virtual const char* getBuffer()=0;
	virtual size_t getSize()=0;
};

class CourgetteI
{
public:
	virtual void destory()=0;

	//! Creates a diff between an old file and new file
	//! Writes data in the diff
	//!
	virtual bool createDiff(CourgetteBufferI* oldBuffer, CourgetteBufferI* newBuffer, CourgetteCallbackI* callback)=0;

	//! Applys a diff from an old file and diff file.
	//! Writes data in new file
	//!
	//! @return bool if succeced
	//!
	virtual bool applyDiff(CourgetteBufferI* oldBuffer, CourgetteBufferI* diffBuffer, CourgetteCallbackI* callback)=0;
};

#endif //DESURA_COURGETTEI_H
