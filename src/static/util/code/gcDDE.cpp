/*
Desura is the leading indie game distribution platform
Copyright (C) 2011 Mark Chandler (Desura Net Pty Ltd)

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
#include "util/gcDDE.h"

#include "gcDDEManager.h"

// default timeout for DDE operations (5sec)
#define DDE_TIMEOUT     5000

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

gcDDEServer::gcDDEServer()
{
	GetDDEManager()->init();
	GetDDEManager()->addServer(this);
	m_szName = NULL;
}

gcDDEServer::~gcDDEServer()
{
	GetDDEManager()->removeServer(this);
	safe_delete(m_szName);
}

bool gcDDEServer::create(const char* name)
{
    Safe::strcpy(&m_szName, name, 255);
    HSZ hsz = GetDDEManager()->atomFromString(m_szName);

    if ( !hsz )
        return false;

    bool success = (DdeNameService(GetDDEManager()->getDDEInst(), hsz, (HSZ) NULL, DNS_REGISTER) != NULL);

    if (!success)
    {
        printf("Failed to register DDE server '%s'\n", m_szName);
    }

    return success;
}

HDDEDATA gcDDEServer::acceptConnection(const char* topic, HCONV conv)
{
	//printf("Got a connection request on topic: %s\n", topic);

	gcDDEConnection *connection = onAcceptConnection(topic);

	if (connection)
	{
		connection->setInfo( NULL, topic, this);
		m_vConnections.push_back(connection);

		GetDDEManager()->m_pCurConnecting = connection;
		return (HDDEDATA)(DWORD)true;
	}

	return (HDDEDATA)(DWORD)false;
}

gcDDEConnection *gcDDEServer::onAcceptConnection(const char* topic)
{
    return new gcDDEConnection();
}

void gcDDEServer::removeConnection(gcDDEConnection *connection)
{
	if (!connection)
		return;

	for (size_t x=0; x<m_vConnections.size(); x++)
	{
		if (m_vConnections[x] == connection)
		{
			m_vConnections[x] = NULL;
			break;
		}
	}
}

gcDDEConnection* gcDDEServer::findConnection(HCONV conv)
{
	for (size_t x=0; x<m_vConnections.size(); x++)
	{
		if (m_vConnections[x] && m_vConnections[x]->getConv() == conv)
			return m_vConnections[x];
	}

	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


gcDDEClient::gcDDEClient()
{
	GetDDEManager()->init();
	GetDDEManager()->addClient(this);
}

gcDDEClient::~gcDDEClient()
{
	GetDDEManager()->removeClient(this);
}

gcDDEConnection *gcDDEClient::makeConnection(const char* server, const char* topic)
{
    HSZ hszServer = GetDDEManager()->atomFromString(server);

    if ( !hszServer )
        return NULL;

    HSZ hszTopic = GetDDEManager()->atomFromString(topic);

    if ( !hszTopic )
        return NULL;

    HCONV hConv = ::DdeConnect(GetDDEManager()->getDDEInst(), hszServer, hszTopic, (PCONVCONTEXT) NULL);

    if ( !hConv )
    {
        printf("Failed to create connection to server '%s' on topic '%s'\n", server, topic );
    }
    else
    {
        gcDDEConnection *connection = (gcDDEConnection*) onMakeConnection();
        if (connection)
        {	
			connection->setInfo(hConv, topic, this);
            m_vConnections.push_back(connection);
            return connection;
        }
    }

    return NULL;
}

gcDDEConnection *gcDDEClient::onMakeConnection()
{
    return new gcDDEConnection;
}

void gcDDEClient::removeConnection(gcDDEConnection *connection)
{
	if (!connection)
		return;

	for (size_t x=0; x<m_vConnections.size(); x++)
	{
		if (m_vConnections[x] == connection)
		{
			m_vConnections[x] = NULL;
			break;
		}
	}
}

gcDDEConnection* gcDDEClient::findConnection(HCONV conv)
{
	for (size_t x=0; x<m_vConnections.size(); x++)
	{
		if (m_vConnections[x] && m_vConnections[x]->getConv() == conv)
			return m_vConnections[x];
	}

	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

gcDDEConnection::gcDDEConnection()
{
	m_pClient = NULL;
	m_pServer = NULL;

	m_hConv = NULL;
	m_szTopic = NULL;

	m_bConnected = false;
}	

gcDDEConnection::~gcDDEConnection()
{
	safe_delete(m_szTopic);

	if (m_pServer)
		m_pServer->removeConnection(this);

	if (m_pClient)
		m_pClient->removeConnection(this);
}

void gcDDEConnection::setInfo(HCONV con, const char* topic, gcDDEClient *client)
{
	m_hConv = con;
	m_pClient = client;
	Safe::strcpy(&m_szTopic, topic, 255);
}

void gcDDEConnection::setInfo(HCONV con, const char* topic, gcDDEServer *server)
{
	m_hConv = con;
	m_pServer = server;
	Safe::strcpy(&m_szTopic, topic, 255);
}

HDDEDATA gcDDEConnection::disconnect()
{
	m_bConnected = false;

    if (onDisconnect())
	{
		if (DdeDisconnect(m_hConv) == 0)
			 printf("Failed to disconnect from DDE server gracefully");

		delete this;
		return (HDDEDATA)(DWORD)true;
	}

	return (HDDEDATA)(DWORD)false;
}

bool gcDDEConnection::onDisconnect()
{	
	return true;
}

HDDEDATA gcDDEConnection::handlePoke(HSZ hszItem, HDDEDATA hData)
{
	char item_name[255];
	GetDDEManager()->stringFromAtom(hszItem, item_name, 255);

	if (!hData)
	{
		onPoke(item_name, NULL, 0);
		return (HDDEDATA)DDE_FACK;
	}

	DWORD len = DdeGetData(hData, NULL, 0, 0);

	if (len == 0)
	{
		onPoke(item_name, NULL, 0);
		return (HDDEDATA)DDE_FACK;
	}

	char *data = new char[len+1];

	DdeGetData(hData, (LPBYTE)data, len, 0);
	data[len] = '\0';

	DdeFreeDataHandle(hData);

	if (len > 0)
	{
		int x = len-1;
		while ( x+1 )
		{
			if (data[x] == '\r' || data[x] == '\n')
			{
				data[x] = '\0'; 
				len--;
			}
			else
			{
				break;
			}

			x--;
		}
	}

	char f = data[0];
	char l = data[len-1];

	//if it has " " around it remove them
	if (f == 34 && l == 34)
	{
		for (size_t x=0; x<(len-2); x++)
		{
			data[x]=data[x+1];
		}

		data[len-2]='\0';
	}

	size_t size = strlen((char*)data);
	onPoke(item_name, (char*)data, size);

	safe_delete(data);
	return (HDDEDATA)DDE_FACK;
}


void gcDDEConnection::onPoke(const char* item, const char* data, size_t len)
{

}

bool gcDDEConnection::poke(const char* item, const char* data)
{
    DWORD result;

    HSZ item_atom = GetDDEManager()->atomFromString(item);

	char fdata[255];

	Safe::snprintf(fdata, 255, "%s\r\n\0", data);
	size_t size = strlen(fdata);

	HDDEDATA res = DdeClientTransaction((LPBYTE)fdata, size, m_hConv, item_atom, CF_TEXT, XTYP_POKE, DDE_TIMEOUT, &result);
	bool ok =  res?true:false;
    if ( !ok )
    {
        printf("DDE poke request failed");
    }

    return ok;
}

