/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Karol Herbst <git@karolherbst.de>

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
#include "gcWCEvents.h"
#include "gcWebControl.h"

#include "gcJSBase.h"
#include "wx_controls/gcCustomMenu.h"

void BrowserUICallback(ChromiumDLL::CallbackI* callback);


class ContextResultCallback : public ChromiumDLL::CallbackI
{
public:
	ContextResultCallback(ChromiumDLL::JavaScriptContextI* context, ChromiumDLL::JSObjHandle t, ChromiumDLL::JSObjHandle funct) : m_pThis(t), m_pFunction(funct)
	{
		m_pContext = context;
		m_iResult = -1;
	}

	virtual void destroy()
	{
		if (m_pContext)
			m_pContext->destroy();

		delete this;
	}

	virtual void setResult(int32 res)
	{
		m_iResult = res;
	}

	virtual void run()
	{
		if (!m_pFunction || !m_pFunction->isFunction() || !m_pContext)
			return;

		m_pContext->enter();

		ChromiumDLL::JSObjHandle res = m_pContext->getFactory()->CreateInt(m_iResult);

		ChromiumDLL::JavaScriptFunctionArgs args;
		args.function = NULL;
		args.context = m_pContext;
		args.argc = 1;
		args.argv = &res;
		args.factory = NULL;
		args.object = m_pThis;

		ChromiumDLL::JSObjHandle ret = m_pFunction->executeFunction(&args);

		m_pContext->exit();
	}

private:
	int32 m_iResult;

	ChromiumDLL::JSObjHandle m_pThis;
	ChromiumDLL::JSObjHandle m_pFunction;
	ChromiumDLL::JavaScriptContextI* m_pContext;
};


static const int baseId = 100;

class ContextClientData : public wxClientData, public ContextClientDataI
{
public:
	ContextClientData(ContextResultCallback* crc)
	{
		m_pCRC = crc;
	}

	virtual ~ContextClientData()
	{
		if (m_pCRC)
			m_pCRC->destroy();
	}

	virtual bool processResult(uint32 res)
	{
		bool processed = false;

		if (res >= (uint32)baseId)
		{
			m_pCRC->setResult(res-baseId);
			BrowserUICallback(m_pCRC);
			processed = true;
		}
		else
		{
			m_pCRC->destroy();
		}

		m_pCRC = NULL;
		return processed;
	}

	virtual void destroy()
	{
		delete this;
	}

	ContextResultCallback *m_pCRC;
};


class CrumbExtender : public DesuraJSBase<CrumbExtender>
{
public:
	CrumbExtender(gcWebControlI* parent) : DesuraJSBase<CrumbExtender>("CrumbExtender", "")
	{
		m_pParent = parent;

		REG_SIMPLE_JS_VOIDFUNCTION( clearCrumbs, CrumbExtender );
		REG_SIMPLE_JS_VOIDFUNCTION( addCrumb, CrumbExtender );
	}

	//dont use. Need for template
	CrumbExtender() : DesuraJSBase<CrumbExtender>("CrumbExtender", "")
	{
		assert(false);
	}

	virtual bool preExecuteValidation(const char* function, uint32 functionHash, JSObjHandle object, size_t argc, JSObjHandle* argv)
	{
		return (m_pParent != NULL);
	}

	virtual JavaScriptExtenderI* clone()
	{
		return new CrumbExtender(m_pParent);
	}

protected:
	void clearCrumbs()
	{
		m_pParent->onClearCrumbsEvent();
	}

	void addCrumb(gcString name, gcString val)
	{
		Crumb crumb;

		crumb.name = name;
		crumb.url = val;

		m_pParent->onAddCrumbEvent(crumb);
	}

	gcWebControlI* m_pParent;
};


class ObjectExtender : public DesuraJSBase<ObjectExtender>
{
public:
	ObjectExtender() : DesuraJSBase<ObjectExtender>("ObjectExtender", "")
	{
	}

	ObjectExtender(const char* name) : DesuraJSBase<ObjectExtender>(name, "")
	{
	}
};


class ContextMenuExtender : public DesuraJSBase<ContextMenuExtender>
{
public:
	//Dont use
	ContextMenuExtender() : DesuraJSBase<ContextMenuExtender>("ContextMenuExtender", "")
	{
		assert(false);
	}

	ContextMenuExtender(gcWebControlI* parent) : DesuraJSBase<ContextMenuExtender>("ContextMenuExtender", "")
	{
		m_pParent = parent;
		REG_SIMPLE_JS_VOIDFUNCTION( showContextMenu, ContextMenuExtender );
	}

	virtual bool preExecuteValidation(const char* function, uint32 functionHash, JSObjHandle object, size_t argc, JSObjHandle* argv)
	{
		return (m_pParent != NULL);
	}

	virtual JavaScriptExtenderI* clone()
	{
		return new ContextMenuExtender(m_pParent);
	}


protected:
	void showContextMenu(int32 xpos, int32 ypos, std::vector<std::map<gcString, gcString> >, JSObjHandle thisObj, JSObjHandle callBack);

private:
	gcWebControlI* m_pParent;
};

void DisplayContextMenu(gcWebControlI* m_pParent, ContextClientDataI* ccd, gcMenu* menu, int32 xPos, int32 yPos);

void ContextMenuExtender::showContextMenu(int32 xPos, int32 yPos, std::vector<std::map<gcString, gcString> > args, JSObjHandle thisObj, JSObjHandle callBack)
{
	gcMenu* menu = new gcMenu(true);

	std::vector<gcMenu*> menuList;
	menuList.push_back(menu);

	for (size_t x=0; x<args.size(); x++)
	{
		gcString type = args[x]["type"];
		gcString id = args[x]["id"];
		gcWString lable = args[x]["label"];

		gcString enabled = args[x]["enabled"];
		gcString checked = args[x]["checked"];

		bool enabledB = (enabled == "true" || enabled == "1");
		bool checkedB = (checked == "true" || checked == "1");

		if (type == "submenu")
		{
			gcMenu* menu = new gcMenu();
			menuList.push_back(menu);
#ifdef NIX
			ContextResultCallback* crc = new ContextResultCallback(getContext()->clone(), thisObj, callBack);
			menu->SetClientObject(new ContextClientData(crc));
#endif
		}
		else if (type == "endsubmenu")
		{
			gcMenu* sub = menuList.back();
			menuList.pop_back();

			menuList.back()->AppendSubMenu(sub, lable);
		}
		else if (type == "separator")
		{
			menuList.back()->AppendSeparator();
		}
		else
		{
			wxItemKind kind = wxITEM_NORMAL;

			if (type == "checkbox")
				kind = wxITEM_CHECK;

			gcMenuItem* item = new gcMenuItem(menuList.back(), baseId + atoi(id.c_str()), lable, L"", kind);

			menuList.back()->Append(item);

			item->Enable(enabledB);

			if (checkedB)
				item->Check(checkedB);
		}
	}

	menu->layoutMenu();

	ContextResultCallback* crc = new ContextResultCallback(getContext()->clone(), thisObj, callBack);
	DisplayContextMenu(m_pParent, new ContextClientData(crc), menu, xPos, yPos);
}

#ifndef WIN32
void DisplayContextMenu(gcWebControlI* m_pParent, ContextClientDataI* ccd, gcMenu* menu, int32 xPos, int32 yPos)
{
	menu->SetClientObject((ContextClientData*)ccd);
	m_pParent->PopupMenu((wxMenu*)menu);
}
#endif



void EventHandler::HandleJSBinding(ChromiumDLL::JavaScriptObjectI* jsObject, ChromiumDLL::JavaScriptFactoryI* factory)
{
	ChromiumDLL::JSObjHandle dObj = jsObject->getValue("desura");
	
	if (!dObj.get())
	{
		dObj = factory->CreateObject(new ObjectExtender("desura"));
		jsObject->setValue("desura", dObj);
	}

	ChromiumDLL::JSObjHandle cObj = dObj->getValue("crumbs");
		
	if (!cObj.get())
	{
		cObj = factory->CreateObject(new ObjectExtender("crumbs"));
		dObj->setValue("crumbs", cObj);
	}

	cObj->setValue("clearCrumbs", factory->CreateFunction("clearCrumbs", new CrumbExtender(m_pParent)));
	cObj->setValue("addCrumb", factory->CreateFunction("addCrumb", new CrumbExtender(m_pParent)));


	ChromiumDLL::JSObjHandle uObj = dObj->getValue("utils");
		
	if (!uObj.get())
	{
		uObj = factory->CreateObject(new ObjectExtender("utils"));
		dObj->setValue("utils", uObj);
	}

	uObj->setValue("showContextMenu", factory->CreateFunction("showContextMenu", new ContextMenuExtender(m_pParent)));
}
