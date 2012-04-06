///////////// Copyright © 2010 DesuraNet. All rights reserved. /////////////
//
//   Project     : mcf_util
//   File        : CreateMCF.cpp
//   Description :
//      [TODO: Write the purpose of CreateMCF.cpp.]
//
//   Created On: 4/5/2011 4:13:28 PM
//   Created By:  <mailto:>
////////////////////////////////////////////////////////////////////////////

#include "Common.h"
#include "UtilFunction.h"



class CreateMCF : public UtilFunction
{
public:
	virtual uint32 getNumArgs()
	{
		return 2;
	}

	virtual const char* getArgDesc(size_t index)
	{
		if (index == 0)
			return "Src Folder";

		return "Dest Mcf";
	}

	virtual const char* getFullArg()
	{
		return "create";
	}

	virtual const char getShortArg()
	{
		return 'c';
	}

	virtual const char* getDescription()
	{
		return "Creates a new mcf from a folder";
	}

	virtual int performAction(std::vector<std::string> &args)
	{
		MCFCore::MCFI* mcfHandle = mcfFactory();
		mcfHandle->getProgEvent() += delegate((UtilFunction*)this, &UtilFunction::printProgress);
		mcfHandle->getErrorEvent() += delegate((UtilFunction*)this, &UtilFunction::mcfError);

		mcfHandle->setFile(args[1].c_str());
		mcfHandle->parseFolder(args[0].c_str());
		mcfHandle->saveMCF();

		mcfDelFactory(mcfHandle);
		return 0;
	}
};

class CreateMCFDiff : public UtilFunction
{
public:
	virtual uint32 getNumArgs()
	{
		return 3;
	}

	virtual const char* getArgDesc(size_t index)
	{
		if (index == 0)
			return "vA Mcf";
		else if (index == 1)
			return "vB Mcf";

		return "Out Mcf";
	}

	virtual const char* getFullArg()
	{
		return "creatediff";
	}

	virtual const char getShortArg()
	{
		return 'd';
	}

	virtual const char* getDescription()
	{
		return "Creates a new mcf from a folder and the diffs as well";
	}

	virtual int performAction(std::vector<std::string> &args)
	{
		MCFCore::MCFI* mcfA = mcfFactory();
		MCFCore::MCFI* mcfB = mcfFactory();

		mcfA->setFile(args[0].c_str());
		mcfA->parseMCF();

		mcfB->setFile(args[1].c_str());
		mcfB->parseMCF();

		mcfB->createCourgetteDiffs(mcfA, args[2].c_str());

		mcfDelFactory(mcfA);
		mcfDelFactory(mcfB);
		return 0;
	}
};

class CreateNCMCF : public UtilFunction
{
public:
	virtual uint32 getNumArgs()
	{
		return 2;
	}

	virtual const char* getArgDesc(size_t index)
	{
		if (index == 0)
			return "Src Folder";

		return "Dest Mcf";
	}

	virtual const char* getFullArg()
	{
		return "createnc";
	}

	virtual const char getShortArg()
	{
		return 'n';
	}

	virtual const char* getDescription()
	{
		return "Creates a new mcf that is not compressed from a folder";
	}

	virtual int performAction(std::vector<std::string> &args)
	{

		MCFCore::MCFI* mcfHandle = mcfFactory();
		mcfHandle->getProgEvent() += delegate((UtilFunction*)this, &UtilFunction::printProgress);
		mcfHandle->getErrorEvent() += delegate((UtilFunction*)this, &UtilFunction::mcfError);

		mcfHandle->setWorkerCount(2);
		mcfHandle->setFile(args[1].c_str());
		mcfHandle->disableCompression();

		mcfHandle->parseFolder(args[0].c_str());
		mcfHandle->saveMCF();


		mcfDelFactory(mcfHandle);
		return 0;
	}
};


REG_FUNCTION(CreateMCFDiff)
REG_FUNCTION(CreateMCF)
REG_FUNCTION(CreateNCMCF)
