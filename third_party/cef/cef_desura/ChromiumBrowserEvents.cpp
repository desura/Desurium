///////////// Copyright © 2010 DesuraNet. All rights reserved. /////////////
//
//   Project     : desura_libcef_dll_wrapper
//   File        : ChromiumBrowserEvents.cpp
//   Description :
//      [TODO: Write the purpose of ChromiumBrowserEvents.cpp.]
//
//   Created On: 7/1/2010 11:36:15 PM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////


#include "ChromiumBrowserEvents.h"
#include "ChromiumBrowser.h"
#include "MenuInfo.h"

#include <sstream>
#include "JavaScriptObject.h"
#include "JavaScriptFactory.h"

std::map<int, std::string> g_mErrorMsgMap;

class FillMap
{
public:
	FillMap()
	{
		g_mErrorMsgMap[ERR_FAILED] = "A generic failure occured";
		g_mErrorMsgMap[ERR_ABORTED] = "An operation was aborted (due to user action)";
		g_mErrorMsgMap[ERR_INVALID_ARGUMENT] = "An argument to the function is incorrect";
		g_mErrorMsgMap[ERR_INVALID_HANDLE] = "The handle or file descriptor is invalid";
		g_mErrorMsgMap[ERR_FILE_NOT_FOUND] = "The file or directory cannot be found";
		g_mErrorMsgMap[ERR_TIMED_OUT] = "An operation timed out";
		g_mErrorMsgMap[ERR_FILE_TOO_BIG] = "The file is too large";
		g_mErrorMsgMap[ERR_UNEXPECTED] = "An unexpected error.  This may be caused by a programming mistake or an invalid assumption";
		g_mErrorMsgMap[ERR_ACCESS_DENIED] = "Permission to access a resource was denied";
		g_mErrorMsgMap[ERR_NOT_IMPLEMENTED] = "The operation failed because of unimplemented functionality";
		g_mErrorMsgMap[ERR_CONNECTION_CLOSED] = "A connection was closed (corresponding to a TCP FIN)";
		g_mErrorMsgMap[ERR_CONNECTION_RESET] = "A connection was reset (corresponding to a TCP RST)";
		g_mErrorMsgMap[ERR_CONNECTION_REFUSED] = "A connection attempt was refused";
		g_mErrorMsgMap[ERR_CONNECTION_ABORTED] = "A connection timed out as a result of not receiving an ACK for data sent";
		g_mErrorMsgMap[ERR_CONNECTION_FAILED] = "A connection attempt failed";
		g_mErrorMsgMap[ERR_NAME_NOT_RESOLVED] = "The host name could not be resolved";
		g_mErrorMsgMap[ERR_INTERNET_DISCONNECTED] = "The Internet connection has been lost";
		g_mErrorMsgMap[ERR_SSL_PROTOCOL_ERROR] = "An SSL protocol error occurred";
		g_mErrorMsgMap[ERR_ADDRESS_INVALID] = "The IP address or port number is invalid";
		g_mErrorMsgMap[ERR_ADDRESS_UNREACHABLE] = "The IP address is unreachable";
		g_mErrorMsgMap[ERR_SSL_CLIENT_AUTH_CERT_NEEDED] = "The server requested a client certificate for SSL client authentication";
		g_mErrorMsgMap[ERR_TUNNEL_CONNECTION_FAILED] = "A tunnel connection through the proxy could not be established";
		g_mErrorMsgMap[ERR_NO_SSL_VERSIONS_ENABLED] = "No SSL protocol versions are enabled";
		g_mErrorMsgMap[ERR_SSL_VERSION_OR_CIPHER_MISMATCH] = "The client and server don't support a common SSL protocol version or cipher suite";
		g_mErrorMsgMap[ERR_SSL_RENEGOTIATION_REQUESTED] = "The server requested a renegotiation (rehandshake)";
		g_mErrorMsgMap[ERR_CERT_COMMON_NAME_INVALID] = "The server responded with a certificate whose common name did not match the host name";
		g_mErrorMsgMap[ERR_CERT_DATE_INVALID] = "The server responded with a certificate that, by our clock, appears to either not yet be valid or to have expired";
		g_mErrorMsgMap[ERR_CERT_AUTHORITY_INVALID] = "The server responded with a certificate that is signed by an authority we don't trust";
		g_mErrorMsgMap[ERR_CERT_CONTAINS_ERRORS] = "The server responded with a certificate that contains errors";
		g_mErrorMsgMap[ERR_CERT_NO_REVOCATION_MECHANISM] = "The certificate has no mechanism for determining if it is revoked";
		g_mErrorMsgMap[ERR_CERT_UNABLE_TO_CHECK_REVOCATION] = "Revocation information for the security certificate for this site is not avaliable";
		g_mErrorMsgMap[ERR_CERT_REVOKED] = "The server responded with a certificate has been revoked";
		g_mErrorMsgMap[ERR_CERT_INVALID] = "The server responded with a certificate that is invalid";
		g_mErrorMsgMap[ERR_CERT_END] = "The value immediately past the last certificate error code";
		g_mErrorMsgMap[ERR_INVALID_URL] = "The URL is invalid";
		g_mErrorMsgMap[ERR_DISALLOWED_URL_SCHEME] = "The scheme of the URL is disallowed";
		g_mErrorMsgMap[ERR_UNKNOWN_URL_SCHEME] = "The scheme of the URL is unknown";
		g_mErrorMsgMap[ERR_TOO_MANY_REDIRECTS] = "Attempting to load an URL resulted in too many redirects";
		g_mErrorMsgMap[ERR_UNSAFE_REDIRECT] = "Attempting to load an URL resulted in an unsafe redirect";
		g_mErrorMsgMap[ERR_UNSAFE_PORT] = "Attempting to load an URL with an unsafe port number";
		g_mErrorMsgMap[ERR_INVALID_RESPONSE] = "The server's response was invalid";
		g_mErrorMsgMap[ERR_INVALID_CHUNKED_ENCODING] = "Error in chunked transfer encoding";
		g_mErrorMsgMap[ERR_METHOD_NOT_SUPPORTED] = "The server did not support the request method";
		g_mErrorMsgMap[ERR_UNEXPECTED_PROXY_AUTH] = "The response was 407 (Proxy Authentication Required), yet we did not send the request to a proxy";
		g_mErrorMsgMap[ERR_EMPTY_RESPONSE] = "The server closed the connection without sending any data";
		g_mErrorMsgMap[ERR_RESPONSE_HEADERS_TOO_BIG] = "The headers section of the response is too large";
		g_mErrorMsgMap[ERR_CACHE_MISS] = "The cache does not have the requested entry";
		g_mErrorMsgMap[ERR_INSECURE_RESPONSE] = "The server's response was insecure (e.g. there was a cert error)";
	}
};

FillMap fm;


/////////////////////////////////////////////////////////////////////////////////////////////
/// LifeSpanHandler
/////////////////////////////////////////////////////////////////////////////////////////////

void LifeSpanHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
	if (browser->IsPopup())
		return;

	SetBrowser(browser);
}

void LifeSpanHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
	if (GetBrowser() && GetBrowser()->GetWindowHandle() == browser->GetWindowHandle())
		SetBrowser(NULL);
}

bool LifeSpanHandler::OnBeforePopup(CefRefPtr<CefBrowser> parentBrowser, const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo, const CefString& url, CefRefPtr<CefClient>& client, CefBrowserSettings& settings)
{
	//dont show popups unless its the inspector
	const char* u = url.c_str();
	return (!u || std::string(u).find("resources/inspector/devtools.") == std::string::npos);
}

/////////////////////////////////////////////////////////////////////////////////////////////
/// LifeSpanHandler
/////////////////////////////////////////////////////////////////////////////////////////////

void LoadHandler::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
{
	if (GetCallback() && frame->IsMain())
		GetCallback()->onPageLoadStart();
}

void LoadHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
	if (GetCallback() && frame->IsMain())
		GetCallback()->onPageLoadEnd();
}

bool LoadHandler::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString& failedUrl, CefString& errorText)
{
	//if no frame its the whole page
	if (GetCallback())
	{
		const size_t size = 100*1024;
		char buff[size];
		buff[0] = 0;

		std::string errorMsg;
		std::map<int,std::string>::iterator it = g_mErrorMsgMap.find(errorCode);

		if (it != g_mErrorMsgMap.end())
		{
			std::stringstream stream;
			stream << g_mErrorMsgMap[errorCode] << " [" << errorCode << "]";
			errorMsg = stream.str();
		}
		else
		{
			std::stringstream stream;
			stream << "Error Code " << errorCode;
			errorMsg = stream.str();
		}

		if (GetCallback()->onLoadError(errorMsg.c_str(), failedUrl.c_str(), buff, size))
		{
			errorText = buff;
			return true;
		}
	}
		
	// All other messages.
	std::stringstream ss;
	ss <<       "<html><head><title>Load Failed</title></head>"
				"<body><h1>Load Failed</h1>"
				"<h2>Load of URL " << failedUrl.c_str() <<
				" failed with error code " << static_cast<int>(errorCode) <<
				".</h2></body>"
				"</html>";
	errorText = ss.str();
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
/// RequestHandler
/////////////////////////////////////////////////////////////////////////////////////////////

bool RequestHandler::OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, NavType navType, bool isRedirect)
{
	if (!GetCallback())
		return false;

	std::string url = request->GetURL();

	if (url.find("resources/inspector/devtools.") != std::string::npos)
		return false;

	return !GetCallback()->onNavigateUrl(url.c_str(), frame->IsMain());
}


/////////////////////////////////////////////////////////////////////////////////////////////
/// DisplayHandler
/////////////////////////////////////////////////////////////////////////////////////////////

bool DisplayHandler::OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line)
{
	if (GetCallback())
		GetCallback()->onLogConsoleMsg(message.c_str(), source.c_str(), line);

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////
/// KeyboardHandler
/////////////////////////////////////////////////////////////////////////////////////////////

bool KeyboardHandler::OnKeyEvent(CefRefPtr<CefBrowser> browser, KeyEventType type, int code, int modifiers, bool isSystemKey)
{
	if (!GetCallback())
		return false;

	return GetCallback()->onKeyEvent((ChromiumDLL::KeyEventType)type, code, modifiers, isSystemKey);
}


/////////////////////////////////////////////////////////////////////////////////////////////
/// MenuHandler
/////////////////////////////////////////////////////////////////////////////////////////////

bool MenuHandler::OnBeforeMenu(CefRefPtr<CefBrowser> browser, const MenuInfo& menuInfo)
{
	if (!GetCallback())
		return false;

	ChromiumMenuInfo cmi(menuInfo, GetBrowser()->GetWindowHandle());
	return GetCallback()->HandlePopupMenu(&cmi);
}


/////////////////////////////////////////////////////////////////////////////////////////////
/// JSDialogHandler
/////////////////////////////////////////////////////////////////////////////////////////////

bool JSDialogHandler::OnJSAlert(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& message)
{
	if (!GetCallback())
		return false;

	return GetCallback()->onJScriptAlert(message.c_str());
}

bool JSDialogHandler::OnJSConfirm(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& message, bool& retval)
{
	if (!GetCallback())
		return false;

	return (GetCallback()->onJScriptConfirm(message.c_str(), &retval));
}

bool JSDialogHandler::OnJSPrompt(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& message, const CefString& defaultValue, bool& retval, CefString& result)
{
	if (!GetCallback())
		return false;

	char resultBuff[255] = {0};
	bool res = GetCallback()->onJScriptPrompt(message.c_str(), defaultValue.c_str(), &retval, resultBuff);

	if (res)
		result = resultBuff;

	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////////
/// JSBindingHandler
/////////////////////////////////////////////////////////////////////////////////////////////

void JSBindingHandler::OnJSBinding(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Value> object)
{
	setContext(CefV8Context::GetCurrentContext());

	JavaScriptObject obj(object);

	if (GetCallback())
		GetCallback()->HandleJSBinding(&obj, GetJSFactory());
}



/////////////////////////////////////////////////////////////////////////////////////////////
/// WinEventHandler
/////////////////////////////////////////////////////////////////////////////////////////////

void WinEventHandler::OnWndProc(CefRefPtr<CefBrowser> browser, int message, int wparam, int lparam)
{
#ifdef OS_WIN
	if (GetCallback())
		GetCallback()->HandleWndProc(message, wparam, lparam);

	if (message != WM_XBUTTONDOWN)
		return;

	int fwButton = GET_XBUTTON_WPARAM(wparam);

	if (fwButton == XBUTTON1)
		browser->GoBack();
	else if (fwButton == XBUTTON2)
		browser->GoForward();
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////
/// ChromiumBrowserEvents
/////////////////////////////////////////////////////////////////////////////////////////////

ChromiumBrowserEvents::ChromiumBrowserEvents(ChromiumBrowser* pParent)
{
	m_pParent = pParent;
	m_pEventCallBack = NULL;
}

void ChromiumBrowserEvents::setCallBack(ChromiumDLL::ChromiumBrowserEventI* cbe)
{
	m_pEventCallBack = cbe;
}

void ChromiumBrowserEvents::setParent(ChromiumBrowser* parent)
{
	m_pParent = parent;
}

ChromiumDLL::ChromiumBrowserEventI* ChromiumBrowserEvents::GetCallback()
{
	return m_pEventCallBack;
}

void ChromiumBrowserEvents::SetBrowser(CefRefPtr<CefBrowser> browser)
{
	m_Browser = browser;

	if (m_pParent)
		m_pParent->setBrowser(browser);
}

CefRefPtr<CefBrowser> ChromiumBrowserEvents::GetBrowser()
{
	return m_Browser;
}

void ChromiumBrowserEvents::setContext(CefRefPtr<CefV8Context> context)
{
	m_pParent->setContext(context);
}


