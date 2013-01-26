/***
 * Demonstrike Core
 */

#include "LogonStdAfx.h"
#pragma pack(push, 1)
typedef struct
{
	uint16 opcode;
	uint32 size;
}logonpacket;
#pragma pack(pop)

LogonCommServerSocket::LogonCommServerSocket(SOCKET fd, const sockaddr_in * peer) : TcpSocket(fd, 65536, 524288, false, peer)
{
	// do nothing
	last_ping = (uint32)UNIXTIME;
	next_server_ping = last_ping + 20;
	remaining = opcode = 0;
	removed = true;

	latency = 0;
	use_crypto = false;
	authenticated = 0;
}

LogonCommServerSocket::~LogonCommServerSocket()
{

}

void LogonCommServerSocket::OnConnect()
{
	if( !IsServerAllowed(GetRemoteAddress().s_addr) )
	{
		printf("Server connection from %u(%s:%u) DENIED, not an allowed IP.\n", GetRemoteAddress().s_addr, GetIP(), GetPort());
		Disconnect();
		return;
	}

	sInfoCore.AddServerSocket(this);
	removed = false;
}

void LogonCommServerSocket::OnDisconnect()
{
	// if we're registered -> de-register
	if(!removed)
	{
		set<uint32>::iterator itr = server_ids.begin();
		for(; itr != server_ids.end(); ++itr)
			sInfoCore.SetRealmOffline((*itr), this);

		sInfoCore.RemoveServerSocket(this);
	}
}

void LogonCommServerSocket::OnRecvData()
{
	while(true)
	{
		if(!remaining)
		{
			if(GetReadBuffer()->GetSize() < 6)
				return;	 // no header

			// read header
			Read((uint8*)&opcode, 2);
			Read((uint8*)&remaining, 4);

			if(use_crypto)
			{
				// decrypt the packet
				recvCrypto.Process((unsigned char*)&opcode, (unsigned char*)&opcode, 2);
				recvCrypto.Process((unsigned char*)&remaining, (unsigned char*)&remaining, 4);
			}

			EndianConvert(opcode);
			/* reverse byte order */
			EndianConvertReverse(remaining);
		}

		// do we have a full packet?
		if(GetReadBuffer()->GetSize() < remaining)
			return;

		// create the buffer
		WorldPacket buff(opcode, remaining);
		if(remaining)
		{
			buff.resize(remaining);
			Read((uint8*)buff.contents(), remaining);
		}

		if(use_crypto && remaining)
			recvCrypto.Process((unsigned char*)buff.contents(), (unsigned char*)buff.contents(), remaining);

		// handle the packet
		HandlePacket(buff);

		remaining = 0;
		opcode = 0;
	}
}

void LogonCommServerSocket::HandlePacket(WorldPacket & recvData)
{
	if(authenticated == 0 && recvData.GetOpcode() != RCMSG_AUTH_CHALLENGE)
	{
		// invalid
		Disconnect();
		return;
	}

	static logonpacket_handler Handlers[RMSG_COUNT] = {
		NULL,												// RMSG_NULL
		&LogonCommServerSocket::HandleRegister,				// RCMSG_REGISTER_REALM
		NULL,												// RSMSG_REALM_REGISTERED
		&LogonCommServerSocket::HandleSessionRequest,		// RCMSG_REQUEST_SESSION
		NULL,												// RSMSG_SESSION_RESULT
		&LogonCommServerSocket::HandlePing,					// RCMSG_PING
		NULL,												// RSMSG_PONG
		NULL,/*Deprecated*/									// RCMSG_SQL_EXECUTE
		&LogonCommServerSocket::HandleReloadAccounts,		// RCMSG_RELOAD_ACCOUNTS
		&LogonCommServerSocket::HandleAuthChallenge,		// RCMSG_AUTH_CHALLENGE
		NULL,												// RSMSG_AUTH_RESPONSE
		NULL,												// RSMSG_REQUEST_ACCOUNT_CHARACTER_MAPPING
		&LogonCommServerSocket::HandleMappingReply,			// RCMSG_ACCOUNT_CHARACTER_MAPPING_REPLY
		&LogonCommServerSocket::HandleUpdateMapping,		// RCMSG_UPDATE_CHARACTER_MAPPING_COUNT
		NULL,												// RSMSG_DISCONNECT_ACCOUNT
		&LogonCommServerSocket::HandleTestConsoleLogin,		// RCMSG_TEST_CONSOLE_LOGIN
		NULL,												// RSMSG_CONSOLE_LOGIN_RESULT
		&LogonCommServerSocket::HandleDatabaseModify,		// RCMSG_MODIFY_DATABASE
		NULL,												// RSMSG_REALM_POP_REQ
		&LogonCommServerSocket::HandlePopulationRespond,	// RCMSG_REALM_POP_RES
		NULL,												// RSMSG_SERVER_PING
		&LogonCommServerSocket::HandleServerPong,			// RCMSG_SERVER_PONG
	};

	if(recvData.GetOpcode() >= RMSG_COUNT || Handlers[recvData.GetOpcode()] == NULL)
	{
		printf("Got unknown packet from logoncomm: %u\n", recvData.GetOpcode());
		return;
	}

	(this->*(Handlers[recvData.GetOpcode()]))(recvData);
}

void LogonCommServerSocket::HandleRegister(WorldPacket & recvData)
{
	string Name;
	int32 tmp_RealmID;

	recvData >> Name;
	tmp_RealmID = sInfoCore.GetRealmIdByName(Name);

	if (tmp_RealmID == -1)
	{
		tmp_RealmID = sInfoCore.GenerateRealmID();
		Log.Notice("LogonCommServer","Registering realm `%s` under ID %u.", Name.c_str(), tmp_RealmID);
	}
	else
	{
		sInfoCore.TimeoutSockets();
		Realm* oldrealm = sInfoCore.GetRealm(tmp_RealmID);
		if(oldrealm == NULL || oldrealm->Flag == REALM_FLAG_OFFLINE) // The oldrealm should always exist.
		{
			sInfoCore.RemoveRealm(tmp_RealmID);
//			int new_tmp_RealmID = sInfoCore.GenerateRealmID(); //socket timout will DC old id after a while, make sure it's not the one we restarted
			Log.Notice("LogonCommServer","Updating realm `%s` with ID %u to new ID %u.",
				Name.c_str(), tmp_RealmID, (tmp_RealmID = sInfoCore.GenerateRealmID()));
//			tmp_RealmID = new_tmp_RealmID;
		}
		else
		{
			// We already have a realm here, and it's not offline, this may be dangerous, but meh.
			WorldPacket data(RSMSG_REALM_REGISTERED, 4);
			data << uint32(1); // Error
			data << uint32(0);
			data << string("ERROR");
			SendPacket(&data);
			Log.Notice("LogonCommServer", "Realm(%s) addition denied, realm already connected.", Name.c_str());
			return;
		}
	}

	uint16 tester = 0;
	std::string adress;
	recvData >> adress;
	// Check if we have a conflicting realm that is using the same adress.
	if(sInfoCore.FindRealmWithAdress(adress))
	{
		WorldPacket data(RSMSG_REALM_REGISTERED, 4);
		data << uint32(1); // Error
		data << uint32(0); // Error
		data << string("ERROR"); // Error
		SendPacket(&data);
		Log.Notice("LogonCommServer", "Realm(%s) addition denied, adress already used.", Name.c_str());
		Log.Line();
		return;
	}

	recvData >> tester;
	if(tester != 0x042)
	{
		WorldPacket data(RSMSG_REALM_REGISTERED, 4);
		data << uint32(1); // Error
		data << uint32(0);
		data << string("ERROR");
		SendPacket(&data);
		Log.Notice("LogonCommServer", "Realm(%s) addition denied, incorrect world server type.", Name.c_str());
		return;
	}

	Realm * realm = new Realm;
	ZeroMemory(realm, sizeof(Realm*));
	realm->Name = Name;
	realm->Address = adress;
	realm->Flag = REALM_FLAG_RECOMMENDED;
	realm->Population = REALM_POP_NEW_PLAYERS;
	realm->ServerSocket = this;
	recvData >> realm->Icon >> realm->WorldRegion >> realm->RealmCap >> realm->RequiredCV[0] >> realm->RequiredCV[1] >> realm->RequiredCV[2] >> realm->RequiredBuild;

	// Add to the main realm list
	sInfoCore.AddRealm(tmp_RealmID, realm);

	// Send back response packet.
	WorldPacket data(RSMSG_REALM_REGISTERED, 4);
	data << uint32(0);			// Error
	data << tmp_RealmID;		// Realm ID
	data << realm->Name;
	SendPacket(&data);
	server_ids.insert(tmp_RealmID);

	/* request character mapping for this realm */
	data.Initialize(RSMSG_REQUEST_ACCOUNT_CHARACTER_MAPPING);
	data << tmp_RealmID;
	SendPacket(&data);

	Log.Notice("LogonCommServer", "Realm(%s) successfully added.", realm->Name.c_str());
}

void LogonCommServerSocket::HandleSessionRequest(WorldPacket & recvData)
{
	string account_name;
	int32 identifier = 0;
	uint32 serverid = 0, request_id = 0;
	recvData >> identifier;
	if(identifier != -42)
		request_id = identifier;
	else
	{
		recvData >> serverid;
		recvData >> request_id;
	}
	recvData >> account_name;

	// get sessionkey!
	uint32 error = 0;
	Account * acct = sAccountMgr.GetAccount(account_name);
	if(acct == NULL || acct->SessionKey == NULL || acct == NULL )
		error = 1;		  // Unauthorized user.

	if(serverid)
	{
		Realm* realm = sInfoCore.GetRealm(serverid);
		if(realm != NULL && realm->Flag & REALM_FLAG_FULL)
			error = 1; // Unauthorized user.
	}

	// build response packet
	WorldPacket data(RSMSG_SESSION_RESULT, 150);
	data << request_id;
	data << error;
	if(!error)
	{
		// Append account information.
		data << acct->AccountId;
		data << acct->UsernamePtr->c_str();
		if(!acct->GMFlags)
			data << uint8(0);
		else
			data << acct->GMFlags;

		data << acct->AccountFlags;
		data.append(acct->SessionKey, 40);
		data.append(acct->Locale, 4);
		data << acct->Muted;
	}

	SendPacket(&data);
}

void LogonCommServerSocket::HandlePing(WorldPacket & recvData)
{
	uint32 MSTime;
	recvData >> MSTime;
	latency = getMSTime()-MSTime;

	WorldPacket data(RSMSG_PONG, 4);
	data << getMSTime();
	SendPacket(&data);
	last_ping = (uint32)time(NULL);
}

void LogonCommServerSocket::SendPacket(WorldPacket * data)
{
	bool rv;
	LockWriteBuffer();

	logonpacket header;
	header.opcode = data->GetOpcode();
	EndianConvert(header.opcode);
	header.size = (uint32)data->size();
	EndianConvertReverse(header.size);

	if(use_crypto)
		sendCrypto.Process((unsigned char*)&header, (unsigned char*)&header, 6);

	rv = WriteButHold((uint8*)&header, 6);

	if(data->size() > 0 && rv)
	{
		if(use_crypto)
			sendCrypto.Process((unsigned char*)data->contents(), (unsigned char*)data->contents(), (uint32)data->size());

		rv = Write(data->contents(), (uint32)data->size());
	}
	else if(rv)
		ForceSend();

	UnlockWriteBuffer();
}

void LogonCommServerSocket::HandleSQLExecute(WorldPacket & recvData)
{
	/*string Query;
	recvData >> Query;
	sLogonSQL->Execute(Query.c_str());*/
	printf("!! WORLD SERVER IS REQUESTING US TO EXECUTE SQL. THIS IS DEPRECATED AND IS BEING IGNORED. THE SERVER WAS: %s, PLEASE UPDATE IT.\n", GetIP());
}

void LogonCommServerSocket::HandleReloadAccounts(WorldPacket & recvData)
{
	if( !IsServerAllowedMod( GetRemoteAddress().s_addr ) )
	{
		Log.Notice("WORLD", "We received a reload request from %s, but access was denied.", GetIP());
		return;
	}

	uint32 num1;
	recvData >> num1; //uint8(42);

	if(	num1 == 3 )
	{
		Log.Notice("WORLD", "World Server at %s is forcing us to reload accounts.", GetIP());
		sAccountMgr.ReloadAccounts(false);
	}
	else
	{
		Log.Notice("WORLD", "We received a reload request from %s, but bad packet received.", GetIP());
	}
}

void LogonCommServerSocket::HandleAuthChallenge(WorldPacket & recvData)
{
	unsigned char key[20];
	uint32 result = 1;
	recvData.read(key, 20);

	// check if we have the correct password
	if(memcmp(key, LogonServer::getSingleton().sql_hash, 20))
		result = 0;

	Log.Notice("LogonCommServer","Authentication request from %s, result %s.", GetIP(), result ? "OK" : "FAIL");

	printf("Key: ");
	for(int i = 0; i < 20; ++i)
		printf("%.2X", key[i]);
	printf("\n");

	recvCrypto.Setup(key, 20);
	sendCrypto.Setup(key, 20);	

	/* packets are encrypted from now on */
	use_crypto = true;

	/* send the response packet */
	WorldPacket data(RSMSG_AUTH_RESPONSE, 4);
	data << result;
	SendPacket(&data);

	/* set our general var */
	authenticated = result;
}

void LogonCommServerSocket::HandleMappingReply(WorldPacket & recvData)
{
	/* this packet is gzipped, whee! :D */
	uint32 real_size;
	recvData >> real_size;
	uLongf rsize = real_size;

	ByteBuffer buf(real_size);
	buf.resize(real_size);

	if(uncompress((uint8*)buf.contents(), &rsize, recvData.contents() + 4, (u_long)recvData.size() - 4) != Z_OK)
	{
		printf("Uncompress of mapping failed.\n");
		return;
	}

	uint32 account_id;
	uint8 number_of_characters;
	uint32 count;
	uint32 realm_id;
	buf >> realm_id;
	Realm * realm = sInfoCore.GetRealm(realm_id);
	if(!realm)
		return;

	sInfoCore.getRealmLock().Acquire();

	HM_NAMESPACE::hash_map<uint32, uint8>::iterator itr;
	buf >> count;
	Log.Notice("LogonCommServer","Got mapping packet for realm %u, total of %u entries.\n", (unsigned int)realm_id, (unsigned int)count);
	for(uint32 i = 0; i < count; ++i)
	{
		buf >> account_id >> number_of_characters;
		itr = realm->CharacterMap.find(account_id);
		if(itr != realm->CharacterMap.end())
			itr->second = number_of_characters;
		else
			realm->CharacterMap.insert( make_pair( account_id, number_of_characters ) );
	}

	sInfoCore.getRealmLock().Release();
}

void LogonCommServerSocket::HandleUpdateMapping(WorldPacket & recvData)
{
	uint32 realm_id;
	uint32 account_id;
	int8 toadd;
	recvData >> realm_id;

	Realm * realm = sInfoCore.GetRealm(realm_id);
	if(!realm)
		return;

	sInfoCore.getRealmLock().Acquire();
	recvData >> account_id >> toadd;

	HM_NAMESPACE::hash_map<uint32, uint8>::iterator itr = realm->CharacterMap.find(account_id);
	if(itr != realm->CharacterMap.end())
	{
		if(itr->second > 0 || toadd > 0)
			itr->second += toadd; // Crow: Bwahahahahaha....
	}
	else
	{
		if(toadd < 0)
			toadd = 0;
		realm->CharacterMap.insert( make_pair( account_id, toadd ) );
	}

	sInfoCore.getRealmLock().Release();
}

void LogonCommServerSocket::HandleTestConsoleLogin(WorldPacket & recvData)
{
	WorldPacket data(RSMSG_CONSOLE_LOGIN_RESULT, 8);
	uint32 request;
	string accountname;
	uint8 key[20];

	recvData >> request;
	recvData >> accountname;
	recvData.read(key, 20);
	DEBUG_LOG("LogonCommServerSocket","Testing console login: %s\n", accountname.c_str());

	data << request;

	Account * pAccount = sAccountMgr.GetAccount(accountname);
	if(pAccount == NULL)
	{
		data << uint32(0);
		SendPacket(&data);
		return;
	}

	if(pAccount->GMFlags == NULL || strchr(pAccount->GMFlags, 'z') == NULL)
	{
		data << uint32(0);
		SendPacket(&data);
		return;
	}

	if(memcmp(pAccount->SrpHash, key, 20) != 0)
	{
		data << uint32(0);
		SendPacket(&data);
		return;
	}

	data << uint32(1);
	SendPacket(&data);
}

void LogonCommServerSocket::HandleDatabaseModify(WorldPacket& recvData)
{
	uint32 method;
	recvData >> method;

	if( !IsServerAllowedMod(GetRemoteAddress().s_addr) )
	{
		Log.Error("LogonCommServerSocket","Database modify request %u denied for %s.\n", method, GetIP());
		return;
	}

	switch(method)
	{
	case 1:			// set account ban
		{
			string account;
			uint32 duration;
			string reason;
			recvData >> account >> duration >> reason;

			// remember we expect this in uppercase
			HEARTHSTONE_TOUPPER(account);

			Account * pAccount = sAccountMgr.GetAccount(account);
			if( pAccount == NULL )
				return;

			pAccount->Banned = duration;

			// update it in the sql (duh)
			sLogonSQL->Execute("UPDATE accounts SET banned = %u, banReason = \"%s\" WHERE login = \"%s\"", duration, sLogonSQL->EscapeString(reason).c_str(), 
				sLogonSQL->EscapeString(account).c_str());

		}break;

	case 2:		// set gm
		{
			string account;
			string gm;
			recvData >> account >> gm;

			// remember we expect this in uppercase
			HEARTHSTONE_TOUPPER(account);

			Account * pAccount = sAccountMgr.GetAccount(account);
			if( pAccount == NULL )
				return;

			pAccount->SetGMFlags( account.c_str() );

			// update it in the sql (duh)
			sLogonSQL->Execute("UPDATE accounts SET gm = \"%s\" WHERE login = \"%s\"", sLogonSQL->EscapeString(gm).c_str(), sLogonSQL->EscapeString(account).c_str());

		}break;

	case 3:		// set mute
		{
			string account;
			uint32 duration;
			recvData >> account >> duration;

			// remember we expect this in uppercase
			HEARTHSTONE_TOUPPER(account);

			Account * pAccount = sAccountMgr.GetAccount(account);
			if( pAccount == NULL )
				return;

			pAccount->Muted = duration;

			// update it in the sql (duh)
			sLogonSQL->Execute("UPDATE accounts SET muted = %u WHERE login = \"%s\"", duration, sLogonSQL->EscapeString(account).c_str());
		}break;

	case 4:		// ip ban add
		{
			string ip;
			string reason;
			uint32 duration;

			recvData >> ip >> duration >> reason;

			if( sIPBanner.Add( ip.c_str(), duration ) )
				sLogonSQL->Execute("INSERT INTO ipbans (ip, expire, banreason) VALUES(\"%s\", %u, \"%s\")", sLogonSQL->EscapeString(ip).c_str(), duration, sLogonSQL->EscapeString(reason).c_str() );
		}break;

	case 5:		// ip ban reomve
		{
			string ip;
			recvData >> ip;

			if( sIPBanner.Remove( ip.c_str() ) )
				sLogonSQL->Execute("DELETE FROM ipbans WHERE ip = \"%s\"", sLogonSQL->EscapeString(ip).c_str());

		}break;

	}
}

void LogonCommServerSocket::SendPing()
{
	next_server_ping = (uint32)UNIXTIME + 20;
	WorldPacket data(RSMSG_SERVER_PING, 4);
	data << uint32(0);
	SendPacket(&data);
}

void LogonCommServerSocket::HandleServerPong(WorldPacket &recvData)
{
	// nothing
}

void LogonCommServerSocket::HandlePopulationRespond(WorldPacket & recvData)
{
	uint32 realmId, population;
	recvData >> realmId >> population;
	sInfoCore.UpdateRealmPop(realmId, population);
}

void LogonCommServerSocket::RefreshRealmsPop()
{
	if(server_ids.empty())
		return;

	WorldPacket data(RSMSG_REALM_POP_REQ, 4);
	set<uint32>::iterator itr = server_ids.begin();
	for( ; itr != server_ids.end() ; itr++ )
	{
		data.clear();
		data << (*itr);
		SendPacket(&data);
	}
}
