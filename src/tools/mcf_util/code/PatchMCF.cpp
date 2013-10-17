///////////// Copyright 2010 DesuraNet. All rights reserved. /////////////
//
//   Project     : mcf_util
//   File        : PatchMCF.cpp
//   Description :
//      [TODO: Write the purpose of PatchMCF.cpp.]
//
//   Created On: 4/5/2011 4:57:30 PM
//   Created By:  <mailto:>
////////////////////////////////////////////////////////////////////////////

#include "Common.h"
#include "UtilFunction.h"


class MakePatchMCF : public UtilFunction
{
public:
	virtual uint32 getNumArgs()
	{
		return 3;
	}

	virtual const char* getArgDesc(size_t index)
	{
		if (index == 0)
			return "Src Folder";

		else if (index == 1)
			return "Dest Mcf";

		return "Prev Mcf";
	}

	virtual const char* getFullArg()
	{
		return "patch";
	}

	virtual const char getShortArg()
	{
		return 'p';
	}

	virtual const char* getDescription()
	{
		return "Makes a patch from a folder and previouse mcf";
	}

	virtual int performAction(std::vector<std::string> &args)
	{
		MCFCore::MCFI* mcfSrc = mcfFactory();
		MCFCore::MCFI* mcfPrev = mcfFactory();

		mcfSrc->setFile(args[1].c_str());
		mcfPrev->setFile(args[2].c_str());

		mcfSrc->parseFolder(args[0].c_str());
		mcfSrc->hashFiles();

		mcfPrev->parseMCF();

		mcfSrc->makePatch(mcfPrev);
		mcfSrc->saveMCF();


		mcfDelFactory(mcfSrc);
		mcfDelFactory(mcfPrev);

		return 0;
	}
};


class MakeBackPatchMCF : public UtilFunction
{
public:
	virtual uint32 getNumArgs()
	{
		return 3;
	}

	virtual const char* getArgDesc(size_t index)
	{
		if (index == 0)
			return "New Full Mcf";
		else if (index == 1)
			return "Old Full Mcf";

		return "Dest Mcf";
	}

	virtual const char* getFullArg()
	{
		return "backpatch";
	}

	virtual const char getShortArg()
	{
		return 'b';
	}

	virtual const char* getDescription()
	{
		return "Creates a back patch from a full and old mcf";
	}

	virtual int performAction(std::vector<std::string> &args)
	{
		MCFCore::MCFI* mcfPatch = mcfFactory();
		MCFCore::MCFI* mcfFull = mcfFactory();

		mcfPatch->setFile(args[0].c_str());
		mcfPatch->parseMCF();

		mcfFull->setFile(args[1].c_str());
		mcfFull->parseMCF();

		mcfPatch->makeBackPatchMCF(mcfFull, args[2].c_str());

		mcfDelFactory(mcfFull);
		mcfDelFactory(mcfPatch);

		return 0;
	}
};


class MergePatchMCF : public UtilFunction
{
public:
	virtual uint32 getNumArgs()
	{
		return 3;
	}

	virtual const char* getArgDesc(size_t index)
	{
		if (index == 0)
			return "Src Mcf";
		else if (index == 1)
			return "Patch Mcf";

		return "Dest Mcf";
	}

	virtual const char* getFullArg()
	{
		return "mergepatch";
	}

	virtual const char getShortArg()
	{
		return 'm';
	}

	virtual const char* getDescription()
	{
		return "Merges a patch and a old full mcf to make a new full mcf";
	}

	virtual int performAction(std::vector<std::string> &args)
	{
		MCFCore::MCFI* mcfSrc = mcfFactory();
		MCFCore::MCFI* mcfPatch = mcfFactory();

		printf("Parsing MCF: %s\n", args[0].c_str());
		mcfSrc->setFile(args[0].c_str());
		mcfSrc->parseMCF();

		printf("Parsing MCF: %s\n", args[1].c_str());
		mcfPatch->setFile(args[1].c_str());
		mcfPatch->parseMCF();

		printf("Merging patch!\n");
		mcfSrc->makeFullFile(mcfPatch, args[2].c_str());

		mcfDelFactory(mcfSrc);
		mcfDelFactory(mcfPatch);

		return 0;
	}
};

REG_FUNCTION(MakePatchMCF)
REG_FUNCTION(MakeBackPatchMCF)
REG_FUNCTION(MergePatchMCF)
