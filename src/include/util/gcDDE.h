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

#ifndef DESURA_GCDDE_H
#define DESURA_GCDDE_H
#ifdef _WIN32
#pragma once
#endif

#include <ddeml.h>
#include <vector>
#include <set>

class gcDDEClient;
class gcDDEServer;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class gcDDEConnection
{
public:
	gcDDEConnection();
	~gcDDEConnection();

	void setInfo(HCONV con, const char* topic, gcDDEClient *client);
	void setInfo(HCONV con, const char* topic, gcDDEServer *server);

	void setConv(HCONV con){m_hConv = con;}
	HCONV getConv(){return m_hConv;}

	bool poke(const char* item, const char* data);

protected:
	HDDEDATA disconnect();
	HDDEDATA handlePoke(HSZ hszItem, HDDEDATA hData);

	virtual void onPoke(const char* item, const char* data, size_t len);
	virtual bool onDisconnect();

private:
	gcDDEClient *m_pClient;
	gcDDEServer *m_pServer;
	HCONV m_hConv;

	char* m_szTopic;

	bool m_bConnected;

	friend class gcDDEManager;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////



class gcDDEServer
{
public:
	gcDDEServer();
	~gcDDEServer();

	bool create(const char* name);
	const char* getName(){return m_szName;}

	void removeConnection(gcDDEConnection *connection);
	gcDDEConnection* findConnection(HCONV conv);

protected:
	HDDEDATA acceptConnection(const char* topic, HCONV conv);
	virtual gcDDEConnection* onAcceptConnection(const char* topic);

private:
	char* m_szName;
	std::vector<gcDDEConnection*> m_vConnections;
	friend class gcDDEManager;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


class gcDDEClient
{
public:
	gcDDEClient();
	~gcDDEClient();

	gcDDEConnection* makeConnection(const char* server, const char* topic);

	void removeConnection(gcDDEConnection *connection);
	gcDDEConnection* findConnection(HCONV conv);

protected:
	virtual gcDDEConnection* onMakeConnection();

private:
	std::vector<gcDDEConnection*> m_vConnections;
};




#endif //DESURA_GCDDE_H
