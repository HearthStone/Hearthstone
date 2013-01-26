/***
 * Demonstrike Core
 */
// Class WorldSocket - Main network code functions, handles
// reading/writing of all packets.

#include "StdAfx.h"

/* echo send/received packets to console */
//#define ECHO_PACKET_LOG_TO_CONSOLE 1

extern bool bServerShutdown;

#pragma pack(push, 1)

struct ClientPktHeader
{
	uint16 size;
	uint32 cmd;
};

struct ServerPktHeader
{
	uint16 size;
	uint16 cmd;
};

#pragma pack(pop)

WorldSocket::WorldSocket(SOCKET fd, const sockaddr_in * peer) : TcpSocket(fd, WORLDSOCKET_SENDBUF_SIZE, WORLDSOCKET_RECVBUF_SIZE, false, peer)
{
	Authed = false;
	mSize = mOpcode = mRemaining = 0;
	_latency = 0;
	mSession = NULL;
	mSeed = RandomUInt();
	pAuthenticationPacket = NULL;
	mQueued = false;
	mRequestID = 0;
	m_nagleEanbled = false;
	m_fullAccountName = NULL;
}

WorldSocket::~WorldSocket()
{
	WorldPacket * pck;
	queueLock.Acquire();
	while((pck = _queue.Pop()))
		delete pck;
	queueLock.Release();

	if(pAuthenticationPacket)
		delete pAuthenticationPacket;

	if(mSession)
	{
		mSession->SetSocket(NULL);
		mSession=NULL;
	}

	if( m_fullAccountName != NULL )
	{
		delete m_fullAccountName;
		m_fullAccountName = NULL;
	}
}

void WorldSocket::OnDisconnect()
{
	if(mSession)
	{
		mSession->SetSocket(0);
		mSession = NULL;
	}

	if(mRequestID != 0)
	{
		sLogonCommHandler.UnauthedSocketClose(mRequestID);
		mRequestID = 0;
	}

	if(mQueued)
	{
		sWorld.RemoveQueuedSocket(this);	// Remove from queued sockets.
		mQueued = false;
	}

	// clear buffer
	queueLock.Acquire();
	WorldPacket *pck;
	while((pck = _queue.Pop()))
		delete pck;
	queueLock.Release();
}

void WorldSocket::OutPacket(uint16 opcode, size_t len, const void* data, bool InWorld)
{
	OUTPACKET_RESULT res;
	if( (len + 10) > WORLDSOCKET_SENDBUF_SIZE )
	{
		printf("WARNING: Tried to send a packet of %u bytes (which is too large) to a socket. Opcode was: %u (0x%03X)\n", uint(len), uint(opcode), uint(opcode));
		return;
	}

	res = _OutPacket(opcode, len, data, InWorld);
	if(res == OUTPACKET_RESULT_SUCCESS)
		return;

	if(res == OUTPACKET_RESULT_NO_ROOM_IN_BUFFER)
	{
		/* queue the packet */
		queueLock.Acquire();
		WorldPacket *pck = new WorldPacket(opcode, len);
		pck->SetOpcode(opcode);
		if(len)
			pck->append((const uint8*)data, len);
		_queue.Push(pck);
		queueLock.Release();
	}
}

void WorldSocket::UpdateQueuedPackets()
{
	queueLock.Acquire();
	if(!_queue.HasItems())
	{
		queueLock.Release();
		return;
	}

	WorldPacket * pck;
	while((pck = _queue.front()))
	{
		/* try to push out as many as you can */
		switch(_OutPacket(pck->GetOpcode(), pck->size(), pck->size() ? pck->contents() : NULL))
		{
		case OUTPACKET_RESULT_SUCCESS:
			{
				delete pck;
				_queue.pop_front();
			}break;

		case OUTPACKET_RESULT_NO_ROOM_IN_BUFFER:
			{
				/* still connected */
				queueLock.Release();
				return;
			}break;

		default:
			{
				/* kill everything in the buffer */
				while((pck = _queue.Pop()))
					delete pck;
				queueLock.Release();
				return;
			}break;
		}
	}
	queueLock.Release();
}

//set<uint32> Uniques;

OUTPACKET_RESULT WorldSocket::_OutPacket(uint16 opcode, size_t len, const void* data, bool InWorld)
{
	bool rv;
	if(!IsConnected())
		return OUTPACKET_RESULT_NOT_CONNECTED;

	if( GetWriteBuffer()->GetSpace() < (len+4) )
		return OUTPACKET_RESULT_NO_ROOM_IN_BUFFER;

/*	if(InWorld)
	{
		if(Uniques.find(opcode) == Uniques.end())
		{
			Log.Notice("", "Sent packet %s (0x%03X)", LookupOpcodeName(opcode), uint(opcode), uint(opcode));
			Uniques.insert(opcode);
		}
	}
	else if(opcode != SMSG_UPDATE_OBJECT && opcode != SMSG_PONG && opcode != SMSG_WORLD_STATE_UI_TIMER_UPDATE && opcode != SMSG_WEATHER)
		printf("Sent packet %s (0x%03X)\n", LookupOpcodeName(opcode), uint(opcode), uint(opcode));*/

	LockWriteBuffer();
	// Encrypt the packet
	// First, create the header.
	ServerPktHeader Header;
	Header.cmd = opcode;
	Header.size = ntohs((uint16)len + 2);
	_crypt.EncryptSend((uint8*)&Header, sizeof (ServerPktHeader));

	// Pass the header to our send buffer
	rv = WriteButHold((const uint8*)&Header, 4);

	// Pass the rest of the packet to our send buffer (if there is any)
	if(len > 0 && rv)
		rv = Write((const uint8*)data, (uint32)len);
	else if(rv)
		rv = ForceSend();

	UnlockWriteBuffer();
	if(len > 0 && rv && !bServerShutdown)
		sWorld.NetworkStressOut += float(float(len+4)/1024);
	return rv ? OUTPACKET_RESULT_SUCCESS : OUTPACKET_RESULT_SOCKET_ERROR;
}

void WorldSocket::OnConnect()
{
	sWorld.mAcceptedConnections++;
	_latency = getMSTime();
	WorldPacket data (SMSG_AUTH_CHALLENGE, 25);
	data << uint32(1);			// Unk
	data << mSeed;
	data << uint32(0xF3539DA3);	// Generated Random.
	data << uint32(0x6E8547B9);	// 3.2.2
	data << uint32(0x9A6AA2F8);	// 3.2.2
	data << uint32(0xA4F170F4);	// 3.2.2
	SendPacket(&data);
}

void WorldSocket::_HandleAuthSession(WorldPacket* recvPacket)
{
	std::string account;
	uint32 unk;
	uint64 unk3; // 3.2.2 Unk
	_latency = getMSTime() - _latency;

	try
	{
		*recvPacket >> mClientBuild;
		*recvPacket >> unk;
		*recvPacket >> account;
		*recvPacket >> unk;
		*recvPacket >> mClientSeed;
		// 3.2.2
		*recvPacket >> unk3;
		// 3.3.5
		*recvPacket >> unk;
		*recvPacket >> unk;
		*recvPacket >> unk;
	}
	catch(ByteBufferException &)
	{
		OUT_DEBUG("Incomplete copy of AUTH_SESSION Received.");
		return;
	}

	if(mClientBuild != CL_BUILD_SUPPORT)
	{
		OutPacket(SMSG_AUTH_RESPONSE, 1, "\x14");
		return;
	}

	// Send out a request for this account.
	mRequestID = sLogonCommHandler.ClientConnected(account, this);
	if(mRequestID == 0xFFFFFFFF)
	{
		Disconnect();
		return;
	}

	// shitty hash !
	m_fullAccountName = new string( account );

	// Set the authentication packet
	pAuthenticationPacket = recvPacket;
}

void WorldSocket::InformationRetreiveCallback(WorldPacket & recvData, uint32 requestid)
{
	if(requestid != mRequestID)
		return;

	uint32 error;
	recvData >> error;

	if(error != 0 || pAuthenticationPacket == NULL)
	{
		// something happened wrong @ the logon server
		OutPacket(SMSG_AUTH_RESPONSE, 1, "\x0D");
		return;
	}

	// Extract account information from the packet.
	string AccountName;
	const string * ForcedPermissions;
	uint32 AccountID;
	string GMFlags;
	uint8 AccountFlags;
	string lang = "enUS";
	uint32 i;

	recvData >> AccountID >> AccountName >> GMFlags >> AccountFlags;
	ForcedPermissions = sLogonCommHandler.GetForcedPermissions(AccountName);
	if( ForcedPermissions != NULL )
		GMFlags.assign(ForcedPermissions->c_str());

	DEBUG_LOG( "WorldSocket","Received information packet from logon: `%s` ID %u (request %u)", AccountName.c_str(), AccountID, mRequestID);
//	sLog.outColor(TNORMAL, "\n");

	mRequestID = 0;
	//Pull the session key.

	BigNumber BNK;
	recvData.read(K, 40);
	_crypt.Init(K);
	BNK.SetBinary(K, 40);

	//checking if player is already connected
	//disconnect current player and login this one(blizzlike)

	if(recvData.rpos() != recvData.wpos())
		recvData.read((uint8*)lang.data(), 4);

	WorldSession *session = NULL;
	session = sWorld.FindSession( AccountID );
	if( session != NULL )
	{
		if(session->_player != NULL && session->_player->GetMapMgr() == NULL)
		{
			DEBUG_LOG("WorldSocket","_player found without m_mapmgr during logon, trying to remove him [player %s, map %d, instance %d].", session->_player->GetName(), session->_player->GetMapId(), session->_player->GetInstanceID() );
			if(objmgr.GetPlayer(session->_player->GetLowGUID()))
				objmgr.RemovePlayer(session->_player);
			session->LogoutPlayer(false);
		}
		// AUTH_FAILED = 0x0D
		session->Disconnect();

		// clear the logout timer so he times out straight away
		session->SetLogoutTimer(1);

		// we must send authentication failed here.
		// the stupid newb can relog his client.
		// otherwise accounts dupe up and disasters happen.
		OutPacket(SMSG_AUTH_RESPONSE, 1, "\x15");
		return;
	}

	Sha1Hash sha;

	uint8 digest[20];
	pAuthenticationPacket->read(digest, 20);

	uint32 t = 0;
	if( m_fullAccountName == NULL )				// should never happen !
		sha.UpdateData(AccountName);
	else
	{
		sha.UpdateData(*m_fullAccountName);

		// this is unused now. we may as well free up the memory.
		delete m_fullAccountName;
		m_fullAccountName = NULL;
	}

	sha.UpdateData((uint8 *)&t, 4);
	sha.UpdateData((uint8 *)&mClientSeed, 4);
	sha.UpdateData((uint8 *)&mSeed, 4);
	sha.UpdateBigNumbers(&BNK, NULL);
	sha.Finalize();

	if (memcmp(sha.GetDigest(), digest, 20))
	{
		// AUTH_UNKNOWN_ACCOUNT = 21
		OutPacket(SMSG_AUTH_RESPONSE, 1, "\x15");
		return;
	}

	// Allocate session
	WorldSession * pSession = new WorldSession(AccountID, AccountName, this);
	mSession = pSession;
	ASSERT(mSession);
	pSession->deleteMutex.Acquire();

	// Set session properties
	pSession->permissioncount = 0;//just to make sure it's 0
	pSession->SetClientBuild(mClientBuild);
	pSession->LoadSecurity(GMFlags);
	pSession->SetAccountFlags(AccountFlags);
	pSession->m_lastPing = (uint32)UNIXTIME;

	if(recvData.rpos() != recvData.wpos())
		recvData >> pSession->m_muted;

	for(uint32 i = 0; i < 8; i++)
		pSession->SetAccountData(i, NULL, true, 0);

	if(sWorld.m_useAccountData)
	{
		QueryResult * pResult = CharacterDatabase.Query("SELECT * FROM account_data WHERE acct = %u", AccountID);
		if( pResult == NULL )
			CharacterDatabase.Execute("INSERT INTO account_data VALUES(%u, '', '', '', '', '', '', '', '', '')", AccountID);
		else
		{
			char * d;
			size_t len;
			const char * data;
			for(i = 0; i < 8; i++)
			{
				data = pResult->Fetch()[1+i].GetString();
				len = data ? strlen(data) : 0;
				if(len > 1)
				{
					d = new char[len+1];
					memcpy(d, data, len+1);
					pSession->SetAccountData(i, d, true, (uint32)len);
				}
			}

			delete pResult;
		}
	}

	DEBUG_LOG("Auth", "%s from %s:%u [%ums]", AccountName.c_str(), GetIP(), GetPort(), _latency);

	// Check for queue.
	if( (sWorld.GetSessionCount() < sWorld.GetPlayerLimit()) || pSession->HasGMPermissions() )
		Authenticate();
	else
	{
		// Queued, sucker.
		uint32 Position = sWorld.AddQueuedSocket(this);
		mQueued = true;
		DEBUG_LOG("Queue", "%s added to queue in position %u", AccountName.c_str(), Position);

		// Send packet so we know what we're doing
		UpdateQueuePosition(Position);
	}

	pSession->deleteMutex.Release();
}

void WorldSocket::Authenticate()
{
	WorldSession * pSession = mSession;
	ASSERT(pAuthenticationPacket);
	mQueued = false;

	if(!pSession)
	{
		DEBUG_LOG( "WorldSocket","Lost Session");
		return;
	}

	if(pSession->HasFlag(ACCOUNT_FLAG_XPACK_02))
		OutPacket(SMSG_AUTH_RESPONSE, 11, "\x0C\x30\x78\x00\x00\x00\x00\x00\x00\x00\x02");
	else if(pSession->HasFlag(ACCOUNT_FLAG_XPACK_01))
		OutPacket(SMSG_AUTH_RESPONSE, 11, "\x0C\x30\x78\x00\x00\x00\x00\x00\x00\x00\x01");
	else
		OutPacket(SMSG_AUTH_RESPONSE, 11, "\x0C\x30\x78\x00\x00\x00\x00\x00\x00\x00\x00");

	sAddonMgr.SendAddonInfoPacket(pAuthenticationPacket, (uint32)pAuthenticationPacket->rpos(), pSession);
	pSession->_latency = _latency;

	delete pAuthenticationPacket;
	pAuthenticationPacket = NULL;

	sWorld.AddSession(pSession);
	sWorld.AddGlobalSession(pSession);
}

void WorldSocket::UpdateQueuePosition(uint32 Position)
{
	WorldPacket QueuePacket(SMSG_AUTH_RESPONSE, 15);
	QueuePacket << uint8(0x1B) << uint8(0x2C) << uint8(0x73) << uint8(0) << uint8(0);
	QueuePacket << uint32(0) << uint8(0);
	QueuePacket << Position;
	SendPacket(&QueuePacket);
}

void WorldSocket::_HandlePing(WorldPacket* recvPacket)
{
	uint32 ping;

	*recvPacket >> ping;
	*recvPacket >> _latency;

	if(mSession)
	{
		mSession->_latency = _latency;
		mSession->m_lastPing = (uint32)UNIXTIME;

		// reset the move time diff calculator, don't worry it will be re-calculated next movement packet.
		mSession->m_clientTimeDelay = 0;
	}

	OutPacket(SMSG_PONG, 4, &ping);

#ifdef WIN32
	// Dynamically change nagle buffering status based on latency.
	if(_latency >= 250)
	{
		if(!m_nagleEanbled)
		{
			u_long arg = 0;
			setsockopt(GetFd(), 0x6, 0x1, (const char*)&arg, sizeof(arg));
			m_nagleEanbled = true;
		}
	}
	else
	{
		if(m_nagleEanbled)
		{
			u_long arg = 1;
			setsockopt(GetFd(), 0x6, 0x1, (const char*)&arg, sizeof(arg));
			m_nagleEanbled = false;
		}
	}
#endif
}

void WorldSocket::OnRecvData()
{
	for(;;)
	{
		// Check for the header if we don't have any bytes to wait for.
		if(mRemaining == 0)
		{
			if(GetReadBuffer()->GetSize() < 6)
			{
				// No header in the packet, let's wait.
				return;
			}

			// Copy from packet buffer into header local var
			ClientPktHeader Header;
			Read(&Header, 6);

			// Decrypt the header
			_crypt.DecryptRecv((uint8*)&Header, sizeof (ClientPktHeader));
			mRemaining = mSize = ntohs(Header.size) - 4;
			mOpcode = Header.cmd;
		}

		if(mRemaining > 0)
		{
			if( GetReadBuffer()->GetSize() < mRemaining )
			{
				// We have a fragmented packet. Wait for the complete one before proceeding.
				return;
			}
		}

		WorldPacket *Packet = new WorldPacket(mOpcode, mSize);
		if(mRemaining > 0)
		{
			Packet->resize(mRemaining);
			Read((uint8*)Packet->contents(), mRemaining);

			if(!bServerShutdown)
				sWorld.NetworkStressIn += float(float(mSize+6)/1024);
		}
		mRemaining = mSize = mOpcode = 0;

		// Check for packets that we handle
		switch(Packet->GetOpcode())
		{
		case CMSG_PING:
			{
				_HandlePing(Packet);
				delete Packet;
			}break;
		case CMSG_AUTH_SESSION:
			{
				_HandleAuthSession(Packet);
			}break;
		default:
			{
				if(mSession)
					mSession->QueuePacket(Packet);
				else
				{
					delete Packet;
					Packet = NULL;
				}
			}break;
		}
	}
}

void FastGUIDPack(ByteBuffer & buf, const uint64 & oldguid)
{
	if( &oldguid == NULL )
		return;

	// hehe speed freaks
	uint8 guidmask = 0;
	uint8 guidfields[9] = {0,0,0,0,0,0,0,0};

	int j = 1;
	uint8 * test = (uint8*)&oldguid;

	if (*test) //7*8
	{
		guidfields[j] = *test;
		guidmask |= 1;
		j++;
	}
	if (*(test+1)) //6*8
	{
		guidfields[j] = *(test+1);
		guidmask |= 2;
		j++;
	}
	if (*(test+2)) //5*8
	{
		guidfields[j] = *(test+2);
		guidmask |= 4;
		j++;
	}
	if (*(test+3)) //4*8
	{
		guidfields[j] = *(test+3);
		guidmask |= 8;
		j++;
	}
	if (*(test+4)) //3*8
	{
		guidfields[j] = *(test+4);
		guidmask |= 16;
		j++;
	}
	if (*(test+5))//2*8
	{
		guidfields[j] = *(test+5);
		guidmask |= 32;
		j++;
	}
	if (*(test+6))//1*8
	{
		guidfields[j] = *(test+6);
		guidmask |= 64;
		j++;
	}
	if (*(test+7)) //0*8
	{
		guidfields[j] = *(test+7);
		guidmask |= 128;
		j++;
	}
	guidfields[0] = guidmask;

	buf.append(guidfields,j);
}

unsigned int FastGUIDPack(const uint64 & oldguid, unsigned char * buffer, uint32 pos)
{
	// hehe speed freaks
	uint8 guidmask = 0;

	int j = 1 + pos;
	uint8 * test = (uint8*)&oldguid;

	if (*test) //7*8
	{
		buffer[j] = *test;
		guidmask |= 1;
		j++;
	}
	if (*(test+1)) //6*8
	{
		buffer[j] = *(test+1);
		guidmask |= 2;
		j++;
	}
	if (*(test+2)) //5*8
	{
		buffer[j] = *(test+2);
		guidmask |= 4;
		j++;
	}
	if (*(test+3)) //4*8
	{
		buffer[j] = *(test+3);
		guidmask |= 8;
		j++;
	}
	if (*(test+4)) //3*8
	{
		buffer[j] = *(test+4);
		guidmask |= 16;
		j++;
	}
	if (*(test+5))//2*8
	{
		buffer[j] = *(test+5);
		guidmask |= 32;
		j++;
	}
	if (*(test+6))//1*8
	{
		buffer[j] = *(test+6);
		guidmask |= 64;
		j++;
	}
	if (*(test+7)) //0*8
	{
		buffer[j] = *(test+7);
		guidmask |= 128;
		j++;
	}
	buffer[pos] = guidmask;
	return (j - pos);
}
