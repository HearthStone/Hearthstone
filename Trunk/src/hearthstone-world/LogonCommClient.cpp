/***
 * Demonstrike Core
 */

#include "StdAfx.h"
#pragma pack(push, 1)
typedef struct
{
	uint16 opcode;
	uint32 size;
}logonpacket;
#pragma pack(pop)

LogonCommClientSocket::LogonCommClientSocket(SOCKET fd, const sockaddr_in * peer) : TcpSocket(fd, 524288, 65536, false, peer)
{
	// do nothing
	last_ping = last_pong = uint32(time(NULL));
	remaining = opcode = 0;
	_id = 0;
	latency = 0;
	use_crypto = false;
	authenticated = 0;
}

void LogonCommClientSocket::OnRecvData()
{
	while(true)
	{
		if(!remaining)
		{
			if(GetReadBuffer()->GetSize() < 6)
				return;	 // no header

			// read header
			Read(&opcode, 2);
			Read(&remaining, 4);

			if(use_crypto)
			{
				_recvCrypto.Process((unsigned char*)&opcode, (unsigned char*)&opcode, 2);
				_recvCrypto.Process((unsigned char*)&remaining, (unsigned char*)&remaining, 4);
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
			_recvCrypto.Process((unsigned char*)buff.contents(), (unsigned char*)buff.contents(), remaining);

		// handle the packet
		HandlePacket(buff);

		remaining = 0;
		opcode = 0;
	}
}

void LogonCommClientSocket::HandlePacket(WorldPacket & recvData)
{
	static logonpacket_handler Handlers[RMSG_COUNT] = {
		NULL,												// RMSG_NULL
		NULL,												// RCMSG_REGISTER_REALM
		&LogonCommClientSocket::HandleRegister,				// RSMSG_REALM_REGISTERED
		NULL,												// RCMSG_REQUEST_SESSION
		&LogonCommClientSocket::HandleSessionInfo,			// RSMSG_SESSION_RESULT
		NULL,												// RCMSG_PING
		&LogonCommClientSocket::HandlePong,					// RSMSG_PONG
		NULL,												// RCMSG_SQL_EXECUTE
		NULL,												// RCMSG_RELOAD_ACCOUNTS
		NULL,												// RCMSG_AUTH_CHALLENGE
		&LogonCommClientSocket::HandleAuthResponse,			// RSMSG_AUTH_RESPONSE
		&LogonCommClientSocket::HandleRequestAccountMapping,// RSMSG_REQUEST_ACCOUNT_CHARACTER_MAPPING
		NULL,												// RCMSG_ACCOUNT_CHARACTER_MAPPING_REPLY
		NULL,												// RCMSG_UPDATE_CHARACTER_MAPPING_COUNT
		&LogonCommClientSocket::HandleDisconnectAccount,	// RSMSG_DISCONNECT_ACCOUNT
		NULL,												// RCMSG_TEST_CONSOLE_LOGIN
		&LogonCommClientSocket::HandleConsoleAuthResult,	// RSMSG_CONSOLE_LOGIN_RESULT
		NULL,												// RCMSG_MODIFY_DATABASE
		&LogonCommClientSocket::HandlePopulationRequest,	// RSMSG_REALM_POP_REQ
		NULL,												// RCMSG_REALM_POP_RES
		&LogonCommClientSocket::HandleServerPing,			// RCMSG_SERVER_PING
		NULL,												// RSMSG_SERVER_PONG
	};

	if(recvData.GetOpcode() >= RMSG_COUNT || Handlers[recvData.GetOpcode()] == 0)
	{
		printf("Got unknown packet from logoncomm: %u\n", recvData.GetOpcode());
		return;
	}

	(this->*(Handlers[recvData.GetOpcode()]))(recvData);
}

void LogonCommClientSocket::HandleRegister(WorldPacket & recvData)
{
	uint32 error;
	uint32 realmlid;
	string realmname;
	recvData >> error >> realmlid >> realmname;
	if(error || realmlid == 0) // Adress already used, or realm is active on our slot/name
	{
		// FUUUUU
		return;
	}

	Log.Notice("LogonCommClient", "Realm `%s` registered as realm %u.", realmname.c_str(), realmlid);
	sLogonCommHandler.AdditionAck(realmlid);
	realm_ids.insert(realmlid);
}

void LogonCommClientSocket::HandleSessionInfo(WorldPacket & recvData)
{
	uint32 request_id;
	recvData >> request_id;

	// find the socket with this request
	WorldSocket * sock = sLogonCommHandler.GetSocketByRequest(request_id);
	if(sock == 0 || sock->Authed || !sock->IsConnected())		// Expired/Client disconnected
		return;

	// extract sessionkey / account information (done by WS)
	sock->Authed = true;
	sLogonCommHandler.RemoveUnauthedSocket(request_id);
	sock->InformationRetreiveCallback(recvData, request_id);
}

void LogonCommClientSocket::HandlePong(WorldPacket & recvData)
{
	uint32 MSTime;
	recvData >> MSTime;
	latency = getMSTime()-MSTime;

	last_pong = uint32(time(NULL));
}

void LogonCommClientSocket::SendPing(uint32 diff)
{
	uint32 MSTime = getMSTime();
	WorldPacket data(RCMSG_PING, 4);
	data << MSTime-diff;
	SendPacket(&data);

	last_ping = uint32(time(NULL));
}

void LogonCommClientSocket::SendPacket(WorldPacket * data, bool no_crypto)
{
	logonpacket header;
	bool rv;
	if(!m_connected || m_deleted)
		return;

	LockWriteBuffer();

	header.opcode = data->GetOpcode();
	EndianConvert(header.opcode);
	header.size = (uint32)data->size();
	EndianConvertReverse(header.size);

	if(use_crypto && !no_crypto)
		_sendCrypto.Process((unsigned char*)&header, (unsigned char*)&header, 6);

	rv = WriteButHold((const uint8*)&header, 6);

	if(data->size() > 0 && rv)
	{
		if(use_crypto && !no_crypto)
			_sendCrypto.Process((unsigned char*)data->contents(), (unsigned char*)data->contents(), (unsigned int)data->size());

		rv = Write((const uint8*)data->contents(), (uint32)data->size());
	}
	else if(rv)
		rv = ForceSend();

	UnlockWriteBuffer();
}

void LogonCommClientSocket::OnDisconnect()
{
	if(_id != 0)
	{
		DEBUG_LOG("LogonCommClientSocket","Calling ConnectionDropped() due to OnDisconnect().");
		sLogonCommHandler.ConnectionDropped();
	}
}

LogonCommClientSocket::~LogonCommClientSocket()
{

}

void LogonCommClientSocket::SendChallenge()
{
	uint8 * key = sLogonCommHandler.sql_passhash;

	_recvCrypto.Setup(key, 20);
	_sendCrypto.Setup(key, 20);

	/* packets are encrypted from now on */
	use_crypto = true;

	WorldPacket data(RCMSG_AUTH_CHALLENGE, 20);
	data.append(key, 20);
	SendPacket(&data, true);
}

void LogonCommClientSocket::HandleAuthResponse(WorldPacket & recvData)
{
	uint32 result = 0;
	recvData >> result;
	if(result != 1)
	{
		authenticated = 0xFFFFFFFF;
	}
	else
	{
		authenticated = 1;
	}
	use_crypto = true;
}

void LogonCommClientSocket::UpdateAccountCount(uint32 account_id, int8 add)
{
	WorldPacket data(RCMSG_UPDATE_CHARACTER_MAPPING_COUNT, 9);
	set<uint32>::iterator itr = realm_ids.begin();

	for(; itr != realm_ids.end(); itr++)
	{
		data.clear();
		data << (*itr) << account_id << add;
		SendPacket(&data);
	}
}

void LogonCommClientSocket::HandleRequestAccountMapping(WorldPacket & recvData)
{
	uint32 t = getMSTime();
	uint32 realm_id;
	uint32 account_id;
	QueryResult * result;
	map<uint32, uint8> mapping_to_send;
	map<uint32, uint8>::iterator itr;

	// grab the realm id
	recvData >> realm_id;

	// fetch the character mapping
	result = CharacterDatabase.Query("SELECT acct FROM characters");

	if(result)
	{
		do
		{
			account_id = result->Fetch()[0].GetUInt32();
			itr = mapping_to_send.find(account_id);
			if(itr != mapping_to_send.end())
				itr->second++;
			else
				mapping_to_send.insert( make_pair( account_id, 1 ) );
		} while(result->NextRow());
		delete result;
	}

	if(!mapping_to_send.size())
	{
		// no point sending empty shit
		return;
	}

	ByteBuffer uncompressed(40000 * 5 + 8);
	//uint32 Count = 0;
	uint32 Remaining = uint32(mapping_to_send.size());
	itr = mapping_to_send.begin();
	for(;;)
	{
		uncompressed << realm_id;

		// Send no more than 40000 characters at once.
		if(Remaining > 40000)
			uncompressed << uint32(40000);
		else
			uncompressed << Remaining;

		for(uint32 i = 0; i < 40000; i++, itr++)
		{
			uncompressed << uint32(itr->first) << uint8(itr->second);
			if(!--Remaining)
				break;
		}

		CompressAndSend(uncompressed);
		if(!Remaining)
			break;

		uncompressed.clear();
	}
	Log.Notice("LogonCommClient", "Build character mapping in %ums. (%u)", getMSTime()-t,mapping_to_send.size());
}

void LogonCommClientSocket::CompressAndSend(ByteBuffer & uncompressed)
{
	// I still got no idea where this came from :p
	size_t destsize = uncompressed.size() + uncompressed.size()/10 + 16;

	// w000t w000t kat000t for gzipped packets
	WorldPacket data(RCMSG_ACCOUNT_CHARACTER_MAPPING_REPLY, destsize + 4);
	data.resize(destsize + 4);

	z_stream stream;
	stream.zalloc = 0;
	stream.zfree  = 0;
	stream.opaque = 0;

	if(deflateInit(&stream, 1) != Z_OK)
	{
		OUT_DEBUG("deflateInit failed.");
		return;
	}

	// set up stream pointers
	stream.next_out  = (Bytef*)((uint8*)data.contents())+4;
	stream.avail_out = (uInt)destsize;
	stream.next_in   = (Bytef*)uncompressed.contents();
	stream.avail_in  = (uInt)uncompressed.size();

	// call the actual process
	if(deflate(&stream, Z_NO_FLUSH) != Z_OK ||
		stream.avail_in != 0)
	{
		OUT_DEBUG("deflate failed.");
		return;
	}

	// finish the deflate
	if(deflate(&stream, Z_FINISH) != Z_STREAM_END)
	{
		OUT_DEBUG("deflate failed: did not end stream");
		return;
	}

	// finish up
	if(deflateEnd(&stream) != Z_OK)
	{
		OUT_DEBUG("deflateEnd failed.");
		return;
	}

	*(uint32*)data.contents() = (uint32)uncompressed.size();
	data.resize(stream.total_out + 4);
	SendPacket(&data);
}

void LogonCommClientSocket::HandleDisconnectAccount(WorldPacket & recvData)
{
	uint32 id;
	recvData >> id;

	WorldSession * sess = sWorld.FindSession(id);
	if(sess != NULL)
		sess->Disconnect();
}

void ConsoleAuthCallback(uint32 request, uint32 result);
void LogonCommClientSocket::HandleConsoleAuthResult(WorldPacket & recvData)
{
	uint32 requestid, result;
	recvData >> requestid >> result;

	ConsoleAuthCallback(requestid, result);
}

void LogonCommClientSocket::HandleServerPing(WorldPacket &recvData)
{
	uint32 r;
	recvData >> r;

	WorldPacket data(RCMSG_SERVER_PONG, 4);
	data << r;
	SendPacket(&data, false);
}

void LogonCommClientSocket::HandlePopulationRequest(WorldPacket & recvData)
{
	uint32 realmId;
	recvData >> realmId; // Grab the realm id

	// Send the result
	WorldPacket data(RCMSG_REALM_POP_RES, 8);
	data << realmId << uint32(sWorld.AlliancePlayers*sWorld.HordePlayers);
	SendPacket(&data, false);
}
