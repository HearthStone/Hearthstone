/***
 * Demonstrike Core
 */

#include "StdAfx.h"
#define SWIMMING_TOLERANCE_LEVEL -0.08f
#define MOVEMENT_PACKET_TIME_DELAY 500

#ifdef WIN32

uint32 TimeStamp()
{
	//return timeGetTime();

	FILETIME ft;
	uint64 t;
	GetSystemTimeAsFileTime(&ft);

	t = (uint64)ft.dwHighDateTime << 32;
	t |= ft.dwLowDateTime;
	t /= 10;
	t -= DELTA_EPOCH_IN_USEC;

	return uint32(((t / 1000000L) * 1000) + ((t % 1000000L) / 1000));
}

HEARTHSTONE_INLINE uint32 mTimeStamp()
{
	return timeGetTime();
}

#else

uint32 TimeStamp()
{
	struct timeval tp;
	gettimeofday(&tp, NULL);
	return (tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}

HEARTHSTONE_INLINE uint32 mTimeStamp()
{
	struct timeval tp;
	gettimeofday(&tp, NULL);
	return (tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}

#endif

void WorldSession::HandleMoveWorldportAckOpcode( WorldPacket & recv_data )
{
	DEBUG_LOG( "WORLD"," got MSG_MOVE_WORLDPORT_ACK." );

	GetPlayer()->SetPlayerStatus(NONE);
	sEventMgr.RemoveEvents(GetPlayer(), EVENT_PLAYER_CHECK_STATUS_Transfer);
	if(_player->IsInWorld())
	{
		// get outta here
		return;
	}

	if(_player->m_CurrentTransporter && _player->GetMapId() != _player->m_CurrentTransporter->GetMapId())
	{
		/* wow, our pc must really suck. */
		Transporter* pTrans = _player->m_CurrentTransporter;
		float c_tposx = pTrans->GetPositionX() + _player->m_transportPosition->x;
		float c_tposy = pTrans->GetPositionY() + _player->m_transportPosition->y;
		float c_tposz = pTrans->GetPositionZ() + _player->m_transportPosition->z;

		WorldPacket dataw(SMSG_NEW_WORLD, 20);
		dataw << pTrans->GetMapId() << c_tposx << c_tposy << c_tposz << _player->GetOrientation();
		SendPacket(&dataw);

		_player->SetMapId(_player->m_CurrentTransporter->GetMapId());
		_player->SetPosition(c_tposx, c_tposy, c_tposz, 0, false);
	}
	else
	{
		// don't overwrite the loading flag here.
		// reason: talents/passive spells don't get cast on an invalid instance login
		if( _player->m_TeleportState != 1 )
			_player->m_TeleportState = 2;

		_player->AddToWorld();
	}
}

void WorldSession::HandleMoveTeleportAckOpcode( WorldPacket & recv_data )
{
	WoWGuid guid;
	uint32 flags, time;
	recv_data >> guid;
	recv_data >> flags >> time;
	if(guid == _player->GetGUID())
	{
		if(!_player->GetVehicle() && sWorld.antihack_teleport && !(HasGMPermissions() && sWorld.no_antihack_on_gm) && _player->GetPlayerStatus() != TRANSFER_PENDING)
		{
			/* we're obviously cheating */
			sWorld.LogCheater(this, "Used teleport hack, disconnecting.");
			Disconnect();
			return;
		}

		if(sWorld.antihack_teleport && !(HasGMPermissions() && sWorld.no_antihack_on_gm) && _player->m_position.Distance2DSq(_player->m_sentTeleportPosition) > 625.0f)	/* 25.0f*25.0f */
		{
			/* cheating.... :( */
			sWorld.LogCheater(this, "Used teleport hack {2}, disconnecting.");
			Disconnect();
			return;
		}

		DEBUG_LOG( "WORLD"," got MSG_MOVE_TELEPORT_ACK." );
		GetPlayer()->SetPlayerStatus(NONE);
		sEventMgr.RemoveEvents(GetPlayer(), EVENT_PLAYER_CHECK_STATUS_Transfer);
		GetPlayer()->SetMovement(MOVE_UNROOT,5);
		_player->ResetHeartbeatCoords();

		if(GetPlayer()->GetSummon() != NULL)		// move pet too
			GetPlayer()->GetSummon()->SetPosition((GetPlayer()->GetPositionX() + 2), (GetPlayer()->GetPositionY() + 2), GetPlayer()->GetPositionZ(), float(M_PI));

		if(_player->m_sentTeleportPosition.x != 999999.0f)
		{
			_player->m_position = _player->m_sentTeleportPosition;
			_player->m_sentTeleportPosition.ChangeCoords(999999.0f,999999.0f,999999.0f);
		}
	}
}

void _HandleBreathing(MovementInfo &movement_info, Player* _player, WorldSession * pSession)
{
	if(!_player->IsInWorld())
		return;

	uint16 WaterType = _player->GetMapMgr()->GetWaterType(_player->GetPositionX(), _player->GetPositionY());
	float WaterHeight = _player->GetMapMgr()->GetWaterHeight(_player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ());
	if (!WaterType || WaterHeight == NO_WATER_HEIGHT)
	{
		_player->m_UnderwaterState &= ~(UNDERWATERSTATE_UNDERWATER|UNDERWATERSTATE_FATIGUE|UNDERWATERSTATE_LAVA|UNDERWATERSTATE_SLIME);
		return;
	}
	int64 HeightDelta = (WaterHeight-_player->GetPositionZ())*10;

	// All liquids type - check under water position
	if(WaterType & (0x01|0x02|0x04|0x08))
	{
		if (HeightDelta > 20)
			_player->m_UnderwaterState |= UNDERWATERSTATE_UNDERWATER;
		else
			_player->m_UnderwaterState &= ~UNDERWATERSTATE_UNDERWATER;
	}

	// Allow travel in dark water on taxi or transport
	if ((WaterType & 0x10) && !_player->GetTaxiPath() && !_player->m_TransporterGUID)
		_player->m_UnderwaterState |= UNDERWATERSTATE_FATIGUE;
	else
		_player->m_UnderwaterState &= ~UNDERWATERSTATE_FATIGUE;

	// in lava check, anywhere in lava level
	if (WaterType & 0x04)
	{
		if (HeightDelta > 0)
			_player->m_UnderwaterState |= UNDERWATERSTATE_LAVA;
		else
			_player->m_UnderwaterState &= ~UNDERWATERSTATE_LAVA;
	}

	// in slime check, anywhere in slime level
	if (WaterType & 0x08)
	{
		if (HeightDelta > 0 || movement_info.flags & MOVEFLAG_WATER_WALK)
			_player->m_UnderwaterState |= UNDERWATERSTATE_SLIME;
		else
			_player->m_UnderwaterState &= ~UNDERWATERSTATE_SLIME;
	}
}

void WorldSession::HandleMovementOpcodes( WorldPacket & recv_data )
{
	if(!_player->IsInWorld() || _player->m_uint32Values[UNIT_FIELD_CHARMEDBY] || _player->GetPlayerStatus() == TRANSFER_PENDING && !_player->GetVehicle() || _player->GetTaxiState())
		return;

	// spell cancel on movement, for now only fishing is added
	Object* t_go = _player->m_SummonedObject;
	if (t_go)
	{
		if (t_go->GetEntry() == GO_FISHING_BOBBER)
			TO_GAMEOBJECT(t_go)->EndFishing(GetPlayer(),true);
	}

	uint32 mstime = mTimeStamp();

	/************************************************************************/
	/* Make sure the packet is the correct size range. 77 is real number    */
	/************************************************************************/
	if(recv_data.size() > 90)
	{
		Disconnect();
		return;
	}

	if(sEventMgr.HasEvent(_player, EVENT_PLAYER_FORCE_LOGOUT))
	{
		if(_player->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_AFK))
			sEventMgr.ModifyEventTimeAndTimeLeft(_player, EVENT_PLAYER_FORCE_LOGOUT, 1800000);
		else
			sEventMgr.RemoveEvents(_player, EVENT_PLAYER_FORCE_LOGOUT);
	}

	/************************************************************************/
	/* Read Movement Data Packet                                            */
	/************************************************************************/
	WoWGuid wguid;
	recv_data >> wguid; // active mover guid?
	uint64 guid = wguid.GetOldGuid();
	_player->movement_info.init(recv_data);
	m_MoverWoWGuid = guid;

	/************************************************************************/
	/* Update player movement state                                         */
	/************************************************************************/
	if( sWorld.antihack_cheatengine && _player->m_lastMovementPacketTimestamp != 0 && (int32)mstime - (int32)_player->m_lastMovementPacketTimestamp != 0)
	{
		int32 server_delta = (int32)mstime - (int32)_player->m_lastMovementPacketTimestamp;
		int32 client_delta = (int32)_player->movement_info.time - (int32)_player->m_lastMoveTime;
		int32 diff = client_delta - server_delta;
		//DEBUG_LOG("WorldSession","HandleMovementOpcodes: server delta=%u, client delta=%u", server_delta, client_delta);
		int32 threshold = int32( sWorld.m_CEThreshold ) + int32( _player->GetSession()->GetLatency() );
		if( diff >= threshold )		// replace with threshold var
		{
			// client cheating with process speedup
			if( _player->m_cheatEngineChances == 1 )
			{
				_player->SetMovement( MOVE_ROOT, 1 );
				_player->BroadcastMessage( "Cheat engine detected. Please contact an admin with the below information if you believe this is a false detection." );
				_player->BroadcastMessage( "You will be disconnected in 10 seconds." );
				_player->BroadcastMessage( MSG_COLOR_WHITE"diff: %d server delta: %u client delta: %u\n", diff, server_delta, client_delta );
				sEventMgr.AddEvent( _player, &Player::_Disconnect, EVENT_PLAYER_KICK, 10000, 1, 0 );
				_player->m_cheatEngineChances = 0;
				sWorld.LogCheater(this, "Cheat Engine detected. Diff: %d, Server Delta: %u, Client Delta: %u", diff, server_delta, client_delta );
			}
			else if (_player->m_cheatEngineChances > 0 )
				_player->m_cheatEngineChances--;
		}
	}

	_player->m_lastMovementPacketTimestamp = mstime;
	_player->m_lastMoveTime = _player->movement_info.time;

	/************************************************************************/
	/* Remove Emote State                                                   */
	/************************************************************************/
	if(_player->m_uint32Values[UNIT_NPC_EMOTESTATE])
		_player->SetUInt32Value(UNIT_NPC_EMOTESTATE, 0);

	/************************************************************************/
	/* Make sure the co-ordinates are valid.                                */
	/************************************************************************/
	if( !((_player->movement_info.y >= _minY) && (_player->movement_info.y <= _maxY)) || !((_player->movement_info.x >= _minX) && (_player->movement_info.x <= _maxX)) )
	{
		Disconnect();
		return;
	}

	/************************************************************************/
	/* Dump movement flags - Wheee!                                         */
	/************************************************************************/
#if 0
	printf("=========================================================\n");
	printf("Full movement flags: 0x%.8X\n", _player->movement_info.flags);
	uint32 z, b;
	for(z = 1, b = 1; b < 32;)
	{
		if(_player->movement_info.flags & z)
			printf("   Bit %u (0x%.8X or %u) is set!\n", b, z, z);

		z <<= 1;
		b+=1;
	}
	printf("=========================================================\n");
#endif

	/************************************************************************/
	/* Orientation dumping                                                  */
	/************************************************************************/
#if 0
	printf("Packet: 0x%03X (%s)\n", recv_data.GetOpcode(), LookupName( recv_data.GetOpcode(), g_worldOpcodeNames ) );
	printf("Orientation: %.10f\n", _player->movement_info.orientation);
#endif

	/************************************************************************/
	/* Anti-Hack Checks                                                     */
	/************************************************************************/
	if((!HasGMPermissions() || !sWorld.no_antihack_on_gm) && !_player->m_uint32Values[UNIT_FIELD_CHARM] && !_player->m_heartbeatDisable)
	{
		/************************************************************************/
		/* Anti-Teleport                                                        */
		/************************************************************************/
		if(sWorld.antihack_teleport && _player->m_position.Distance2DSq(_player->movement_info.x, _player->movement_info.y) > 5625.0f
			&& _player->m_runSpeed < 50.0f && !_player->m_TransporterGUID)
		{
			sWorld.LogCheater(this, "Used teleport hack {3}, speed was %f", _player->m_runSpeed);
			Disconnect();
			return;
		}
	}

	// Water walk hack
	if (_player->movement_info.flags & MOVEFLAG_WATER_WALK)
	{
		if(!_player->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_DEATH_WORLD_ENABLE) && !_player->m_isWaterWalking && !_player->m_setwaterwalk)
		{
			if(!HasGMPermissions() || !sWorld.no_antihack_on_gm)
			{
				WorldPacket data( SMSG_MOVE_LAND_WALK );
				data << _player->GetNewGUID();
				data << uint32( 4 );
				SendPacket( &data );

				if(getMSTime() >= _player->m_WaterWalkTimer)
				{
					sWorld.LogCheater(this, "Used water walk hack");
					Disconnect();
					return;
				}
			}
		}
	}

	/************************************************************************/
	/* Calculate the timestamp of the packet we have to send out            */
	/************************************************************************/
	size_t pos = (size_t)m_MoverWoWGuid.GetNewGuidLen() + 1;
	int32 move_time;
	if(m_clientTimeDelay == 0)
		m_clientTimeDelay = mstime - _player->movement_info.time;

	/************************************************************************/
	/* Copy into the output buffer.                                         */
	/************************************************************************/
	if(_player->m_inRangePlayers.size())
	{
		move_time = (_player->movement_info.time - (mstime - m_clientTimeDelay)) + MOVEMENT_PACKET_TIME_DELAY + mstime;
		memcpy(&_player->movement_packet[0], recv_data.contents(), recv_data.size());
		_player->movement_packet[pos+6] = 0;

		/************************************************************************/
		/* Distribute to all inrange players.                                   */
		/************************************************************************/
		for(unordered_set<Player*  >::iterator itr = _player->m_inRangePlayers.begin(); itr != _player->m_inRangePlayers.end(); itr++)
		{
			if( (*itr)->GetSession() && (*itr)->IsInWorld() )
			{
				*(uint32*)&_player->movement_packet[pos+6] = uint32(move_time + (*itr)->GetSession()->m_moveDelayTime);
				(*itr)->GetSession()->OutPacket(recv_data.GetOpcode(), uint16(recv_data.size() + pos), _player->movement_packet);
			}
		}
	}

	/************************************************************************/
	/* Hack Detection by Classic, some changes by Crow						*/
	/************************************************************************/
	if((!HasGMPermissions() || !sWorld.no_antihack_on_gm) && !_player->m_TransporterGUID)
	{
		if(sWorld.antihack_flight)
		{
			if(!(_player->movement_info.flags & MOVEFLAG_SWIMMING || _player->movement_info.flags & MOVEFLAG_FALLING))
			{
				if(recv_data.GetOpcode() != MSG_MOVE_JUMP)
				{
					if(!_player->movement_info.transGuid.GetOldGuid() && !_player->FlyCheat && !_player->m_FlyingAura
						&& _player->movement_info.z > _player->GetPositionZ()+3.0f && _player->movement_info.x == _player->GetPositionX()
						&& _player->movement_info.y == _player->GetPositionY() )
					{
						WorldPacket data (SMSG_MOVE_UNSET_CAN_FLY, 13);
						data << _player->GetNewGUID();
						data << uint32(5);
						SendPacket(&data);

						_player->m_flyHackChances--;
						if(_player->m_flyHackChances == 0)
						{
							sWorld.LogCheater(this, "Disconnected for fly cheat. (1)");
							Disconnect();
							return;
						}
					}
					else if((_player->movement_info.flags & MOVEFLAG_AIR_SWIMMING) && !(_player->m_FlyingAura || _player->FlyCheat))
					{
						WorldPacket data (SMSG_MOVE_UNSET_CAN_FLY, 13);
						data << _player->GetNewGUID();
						data << uint32(5);
						SendPacket(&data);

						_player->m_flyHackChances--;
						if(_player->m_flyHackChances == 0)
						{
							sWorld.LogCheater(this, "Disconnected for fly cheat. (2)");
							Disconnect();
							return;
						}
					}
				}
			}
		}

		if( _player->movement_info.z > -0.001 && _player->movement_info.z < 0.001 && !(_player->movement_info.flags & MOVEFLAG_FALLING_FAR)
			&& (_player->GetPositionZ() > 3.0 || _player->GetPositionZ() < -3.0)/*3 meter tolerance to prevent false triggers*/)
		{
			sWorld.LogCheater(this, "Detected using teleport to plane.");
			Disconnect();
			return;
		}

		if( recv_data.GetOpcode() == MSG_MOVE_START_FORWARD && _player->movement_info.flags & MOVEFLAG_TAXI && !_player->GetTaxiState() )
		{
			sWorld.LogCheater(this, "Detected taxi-flag/speed hacking (Maelstrom Hack Program).");
			Disconnect();
			return;
		}
	}

	/************************************************************************/
	/* Falling damage checks                                                */
	/************************************************************************/

	if( _player->blinked )
	{
		if(_player->blinktimer < getMSTime())
		{
			_player->blinked = false;
			_player->m_fallDisabledUntil = getMSTime() + 5;
			_player->DelaySpeedHack( 5000 );
		}
	}
	else
	{
		if (_player->movement_info.flags & MOVEFLAG_FALLING)
			m_isFalling = true;
		else if(!HasGMPermissions() || !sWorld.no_antihack_on_gm)
		{	// Crow: Base credit for jump hack detection goes to ArcEmu, and whoever coded it.
			if(recv_data.GetOpcode() == MSG_MOVE_JUMP)
			{
				if(m_isJumping)
				{
					SystemMessage("Jump cheat detected. If this is wrong, please report it to an administrator.");
					m_jumpHackChances--;
					if(!m_jumpHackChances)
					{
						sWorld.LogCheater(this, "Disconnected for jump cheat. (1)");
						Disconnect();
						return;
					}
				}
				else if (m_isFalling)
				{
					SystemMessage("Jump cheat detected. If this is wrong, please report it to an administrator.");
					m_jumpHackChances--;
					if(!m_jumpHackChances)
					{
						sWorld.LogCheater(this, "Disconnected for jump cheat. (2)");
						Disconnect();
						return;
					}
				}
				else if(m_isKnockedback)
				{
					SystemMessage("Jump cheat detected. If this is wrong, please report it to an administrator.");
					m_jumpHackChances--;
					if(!m_jumpHackChances)
					{
						sWorld.LogCheater(this, "Disconnected for jump cheat. (3)");
						Disconnect();
						return;
					}
				}

				m_isFalling = true;
				m_isJumping = true;
			}
		}

		if( recv_data.GetOpcode() == MSG_MOVE_FALL_LAND )
		{
			// player has finished falling
			//if _player->z_axisposition contains no data then set to current position
			if( !_player->z_axisposition )
				_player->z_axisposition = _player->movement_info.z;

			// calculate distance fallen
			int32 falldistance = float2int32( _player->z_axisposition - _player->movement_info.z );
			if(_player->movement_info.z > _player->z_axisposition)
				falldistance = 0;

			/*if player is a rogue or druid(in cat form), then apply -17 modifier to fall distance.*/
			if( ( _player->getClass() == ROGUE ) || ( _player->GetShapeShift() == FORM_CAT ) )
				if(_player->safefall)
					falldistance -= _player->m_safeFall;

			//checks that player has fallen more than 12 units, otherwise no damage will be dealt
			//falltime check is also needed here, otherwise sudden changes in Z axis position, such as using !recall, may result in death
			if( _player->isAlive() && !_player->bInvincible && !_player->bGMTagOn && falldistance > 12 && ( getMSTime() >= _player->m_fallDisabledUntil ) &&
				!_player->HasAura(130) && !_player->GetTaxiState()) // Slow Fall or Taxi Path.
			{
				// 1.7% damage for each unit fallen on Z axis over 13
				Unit* toDamage = TO_UNIT(_player);
				if( _player->GetVehicle() )
					toDamage = _player->GetVehicle();
				else if(_player->m_CurrentCharm)
					toDamage = _player->m_CurrentCharm;

				if(!(_player->GetVehicle() && _player->GetVehicle()->GetControllingUnit() != _player))
				{
					uint32 health_loss = float2int32( float( toDamage->GetUInt32Value( UNIT_FIELD_MAXHEALTH ) * ( ( falldistance - 12 ) * 0.017 ) ) );

					if (_player->HasAura(43621))
						health_loss = _player->GetMaxHealth()/2;

					if( health_loss >= toDamage->GetUInt32Value( UNIT_FIELD_HEALTH ) )
						health_loss = toDamage->GetUInt32Value( UNIT_FIELD_HEALTH );

					if( toDamage == _player )
						_player->SendEnvironmentalDamageLog( toDamage->GetGUID(), DAMAGE_FALL, health_loss );
					toDamage->DealDamage( toDamage, health_loss, 0, 0, 0 );

					toDamage->RemoveStealth(); // Fall Damage will cause stealthed units to lose stealth.
				}
			}
			_player->z_axisposition = 0.0f;
		}
		//Opcodes that remove falling flag
		switch (recv_data.GetOpcode())
		{
		case MSG_MOVE_START_ASCEND:
		case MSG_MOVE_START_SWIM:
		case MSG_MOVE_FALL_LAND:
			m_isFalling = false;
			m_isJumping = false;
			m_isKnockedback = false;
		}

		if(!m_isFalling)
			_player->z_axisposition = _player->movement_info.z;
	}

	/************************************************************************/
	/* Transporter Setup                                                    */
	/************************************************************************/
	if(!_player->m_lockTransportVariables)
	{
		if(_player->m_TransporterGUID && !_player->movement_info.transGuid.GetOldGuid())
		{
			/* we left the transporter we were on */
			if(_player->m_CurrentTransporter)
			{
				_player->m_CurrentTransporter->RemovePlayer(_player);
				_player->m_CurrentTransporter = NULLTRANSPORT;
			}

			_player->m_TransporterGUID = 0;
			_player->ResetHeartbeatCoords();
			_player->DelaySpeedHack(5000);
		}
		else if(_player->movement_info.transGuid.GetOldGuid())
		{
			if(!_player->m_TransporterGUID)
			{
				/* just walked into a transport */
				if(_player->IsMounted())
					TO_UNIT(_player)->Dismount();

				// vehicles, meh
				if( _player->GetVehicle() )
					_player->GetVehicle()->RemovePassenger( _player );

				uint64 transporterGUID = _player->movement_info.transGuid.GetOldGuid();
				_player->m_CurrentTransporter = objmgr.GetTransporter(GUID_LOPART(transporterGUID));
				if(_player->m_CurrentTransporter)
					_player->m_CurrentTransporter->AddPlayer(_player);

				/* set variables */
				_player->m_TransporterGUID = _player->movement_info.transGuid.GetOldGuid();
				_player->DelaySpeedHack(5000);
			}
			_player->m_TransporterUnk = _player->movement_info.transTime;
			if( _player->m_transportPosition )
				delete _player->m_transportPosition;
			_player->m_transportPosition = new LocationVector( _player->movement_info.transX, _player->movement_info.transY, _player->movement_info.transZ, _player->movement_info.transO);
		}
		/*float x = _player->movement_info.x - _player->movement_info.transX;
		float y = _player->movement_info.y - _player->movement_info.transY;
		float z = _player->movement_info.z - _player->movement_info.transZ;
		TransporterPointer trans = _player->m_CurrentTransporter;
		if(trans) sChatHandler.SystemMessageToPlr(_player, "Client t pos: %f %f\nServer t pos: %f %f   Diff: %f %f", x,y, trans->GetPositionX(), trans->GetPositionY(), trans->CalcDistance(x,y,z), trans->CalcDistance(_player->movement_info.x, _player->movement_info.y, _player->movement_info.z));*/
	}

	/************************************************************************/
	/* Anti-Speed Hack Checks                                               */
	/************************************************************************/



	/************************************************************************/
	/* Breathing System                                                     */
	/************************************************************************/
	_HandleBreathing(_player->movement_info, _player, this);

	/************************************************************************/
	/* Remove Spells                                                        */
	/************************************************************************/
	uint32 flags = AURA_INTERRUPT_ON_MOVEMENT;
	if( !( _player->movement_info.flags & MOVEFLAG_SWIMMING || _player->movement_info.flags & MOVEFLAG_FALLING ) && !m_bIsWLevelSet )
		flags |= AURA_INTERRUPT_ON_LEAVE_WATER;
	if( _player->movement_info.flags & MOVEFLAG_SWIMMING )
		flags |= AURA_INTERRUPT_ON_ENTER_WATER;
	if( _player->movement_info.flags & ( MOVEFLAG_TURN_LEFT | MOVEFLAG_TURN_RIGHT ) )
		flags |= AURA_INTERRUPT_ON_TURNING;
	_player->m_AuraInterface.RemoveAllAurasByInterruptFlag( flags );

	/************************************************************************/
	/* Update our position in the server.                                   */
	/************************************************************************/
	if( _player->m_CurrentCharm )
		_player->m_CurrentCharm->SetPosition(_player->movement_info.x, _player->movement_info.y, _player->movement_info.z, _player->movement_info.orientation);
	else if( _player->GetVehicle() )
		_player->GetVehicle()->MoveVehicle(_player->movement_info.x, _player->movement_info.y, _player->movement_info.z, _player->movement_info.orientation);
	else
	{
		if(!_player->m_CurrentTransporter)
		{
			if( !_player->SetPosition(_player->movement_info.x, _player->movement_info.y, _player->movement_info.z, _player->movement_info.orientation) )
			{
				_player->SetUInt32Value(UNIT_FIELD_HEALTH, 0);
				_player->KillPlayer();
			}
			if(_player->GetCurrentSpell())
				_player->GetCurrentSpell()->updatePosition(_player->movement_info.x, _player->movement_info.y, _player->movement_info.z);
		}
		else
		{
			_player->SetPosition(_player->movement_info.x, _player->movement_info.y, _player->movement_info.z,
				_player->movement_info.orientation + _player->movement_info.transO, false);
		}
	}

	if(  !(_player->movement_info.flags & MOVEFLAG_MOTION_MASK) )
	{
		if( _player->m_isMoving )
		{
			//printf("MOVING: FALSE (Packet %s)\n", LookupName( recv_data.GetOpcode(), g_worldOpcodeNames ) );
			_player->_SpeedhackCheck();
			_player->m_isMoving = false;
			_player->m_startMoveTime = 0;
		}
	}
	else
	{
		if( !_player->m_isMoving )
		{
			//printf("MOVING: TRUE (Packet %s)\n", LookupName( recv_data.GetOpcode(), g_worldOpcodeNames ) );
			_player->m_isMoving = true;
			_player->m_startMoveTime = _player->movement_info.time;
			_player->m_lastHeartbeatPosition.ChangeCoords(_player->movement_info.x, _player->movement_info.y, _player->movement_info.z);
		}
	}

	// reset the period every 5 seconds, for a little more accuracy
	if( _player->m_isMoving && (_player->m_lastMoveTime - _player->m_startMoveTime) >= 5000 )
	{
		_player->m_lastHeartbeatPosition.ChangeCoords(_player->movement_info.x, _player->movement_info.y, _player->movement_info.z);
		_player->m_startMoveTime = _player->m_lastMoveTime;
		_player->m_cheatEngineChances = 2;
	}

#if defined(_DEBUG)
	//CollideInterface.setDebugPoint(_player->movement_info.x, _player->movement_info.y, _player->movement_info.z, _player->movement_info.orientation);
#endif
}

void WorldSession::HandleMoveTimeSkippedOpcode( WorldPacket & recv_data )
{
	WoWGuid wguid;
	uint32 time_dif;
	recv_data >> wguid;
	recv_data >> time_dif;
	uint64 guid = wguid.GetOldGuid();

	// Ignore updates for not us
	if( guid != _player->GetGUID() )
		return;

	WorldPacket data(MSG_MOVE_TIME_SKIPPED, 16);
	// send to other players
	data << _player->GetNewGUID();
	data << time_dif;
	_player->SendMessageToSet(&data, false);
}

void WorldSession::HandleSetActiveMoverOpcode( WorldPacket & recv_data )
{
	// set current movement object
	uint64 guid;
	recv_data >> guid;

	if(guid != m_MoverWoWGuid.GetOldGuid())
	{
		// make sure the guid is valid and we aren't cheating
		if( !(_player->m_CurrentCharm && _player->m_CurrentCharm->GetGUID() == guid) &&
			!(_player->GetGUID() == guid) )
		{
			// cheater!
			return;
		}

		// generate wowguid
		if(guid != 0)
			m_MoverWoWGuid.Init(guid);
		else
			m_MoverWoWGuid.Init(_player->GetGUID());

		// set up to the movement packet
		_player->movement_packet[0] = m_MoverWoWGuid.GetNewGuidMask();
		memcpy(&_player->movement_packet[1], m_MoverWoWGuid.GetNewGuid(), m_MoverWoWGuid.GetNewGuidLen());
	}
}

void WorldSession::HandleMoveSplineCompleteOpcode(WorldPacket &recvPacket)
{
	SKIP_READ_PACKET(recvPacket);
}

void WorldSession::HandleMountSpecialAnimOpcode(WorldPacket &recvdata)
{
	WorldPacket data(SMSG_MOUNTSPECIAL_ANIM,8);
	data << _player->GetGUID();
	_player->SendMessageToSet(&data, true);
}

void WorldSession::HandleWorldportOpcode(WorldPacket & recv_data)
{
	uint32 mapid;
	float x,y,z,o;
	recv_data >> mapid >> x >> y >> z >> o;

	CHECK_INWORLD_RETURN();

	if(!HasGMPermissions())
	{
		SendNotification("You do not have permission to use this function.");
		return;
	}

	LocationVector vec(x,y,z,o);
	_player->SafeTeleport(mapid,0,vec);
}

void WorldSession::HandleTeleportToUnitOpcode(WorldPacket & recv_data)
{
	CHECK_INWORLD_RETURN();

	uint8 unk;
	Unit* target;
	recv_data >> unk;

	if(!HasGMPermissions())
	{
		SendNotification("You do not have permission to use this function.");
		return;
	}

	if( (target = _player->GetMapMgr()->GetUnit(_player->GetSelection())) == NULL )
		return;

	_player->SafeTeleport(_player->GetMapId(), _player->GetInstanceID(), target->GetPosition());
}

void WorldSession::HandleTeleportCheatOpcode(WorldPacket & recv_data)
{
	float x,y,z,o;
	LocationVector vec;

	if(!HasGMPermissions())
	{
		SendNotification("You do not have permission to use this function.");
		return;
	}

	recv_data >> x >> y >> z >> o;
	vec.ChangeCoords(x,y,z,o);
	_player->SafeTeleport(_player->GetMapId(),_player->GetInstanceID(),vec);
}

void WorldSession::HandleMoveFallResetOpcode(WorldPacket & recvPacket)
{
	_player->z_axisposition = 0.0f;
}

void WorldSession::HandleMoveHoverWaterFlyAckOpcode( WorldPacket & recv_data )
{
	CHECK_INWORLD_RETURN();
	WoWGuid guid;
	uint32 unk, unk2;
	recv_data >> guid >> unk;
	if(guid.GetOldGuid() != _player->GetGUID())
	{
		SKIP_READ_PACKET(recv_data);
		return;
	}

	_player->movement_info.init(recv_data);
	recv_data >> unk2;

	switch(recv_data.GetOpcode())
	{
	case CMSG_MOVE_SET_CAN_FLY_ACK:
		_player->FlyCheat = _player->m_setflycheat; // TODO: Send unfly if wrong.
		break;

	default:
		break;
	}
}

void WorldSession::HandleMoveKnockbackAckOpcode( WorldPacket & recv_data )
{
	CHECK_INWORLD_RETURN();
	WoWGuid guid;
	uint32 unk2;
	recv_data >> guid >> unk2;
	if(guid.GetOldGuid() != _player->GetGUID())
	{
		SKIP_READ_PACKET(recv_data);
		return;
	}

	_player->movement_info.init(recv_data);
}

void WorldSession::HandleAcknowledgementOpcodes( WorldPacket & recv_data )
{
	recv_data.rpos(recv_data.wpos());
}

void WorldSession::HandleForceSpeedChangeOpcodes( WorldPacket & recv_data )
{
	CHECK_INWORLD_RETURN();
	WoWGuid guid;
	uint32 unk;
	recv_data >> guid >> unk;

	if(guid.GetOldGuid() != _player->GetGUID())
	{
		SKIP_READ_PACKET(recv_data);
		return;
	}

	_player->movement_info.init(recv_data);
	SKIP_READ_PACKET(recv_data); // Don't care.

	// TODO: We need more than this I guess...
	if(_player->m_speedChangeInProgress)
	{
		_player->ResetHeartbeatCoords();
		_player->DelaySpeedHack( 5000 );			// give the client a chance to fall/catch up
		_player->m_speedChangeInProgress = false;
	}
}

void MovementInfo::init(WorldPacket & data)
{
	data >> flags >> flags16 >> time;
	data >> x >> y >> z >> orientation;

	if((flags & MOVEFLAG_TAXI) && (flags & MOVEFLAG_FLYING) && (data.size() == 52))
	{
		uint32 ignore1;
		uint8 ignore2;
		data >> transGuid;
		data >> transX;
		data >> transY;
		data >> transZ;
		data >> transO;
		/* Ignore the Transportation Time and the Transportation Seat because we are on a
		moving transport, most likely a zeppelin or a flying battleship.*/
		data >> ignore1;
		data >> ignore2;
	}
	else if (flags & MOVEFLAG_TAXI)
	{
		data >> transGuid >> transX >> transY >> transZ >> transO >> transTime >> transSeat;
	}

	if (flags & (MOVEFLAG_SWIMMING | MOVEFLAG_AIR_SWIMMING) || flags16 & 0x20)
	{
		data >> pitch;
	}

	data >> FallTime;

	if (flags & MOVEFLAG_FALLING || flags & MOVEFLAG_REDIRECTED)
	{
		data >> jump_velocity >> jump_sinAngle >> jump_cosAngle >> jump_xySpeed;
	}
	if (flags & MOVEFLAG_SPLINE_MOVER)
	{
		data >> spline;
	}
}

void MovementInfo::write(WorldPacket & data)
{
	data << flags << flags16 << getMSTime();

	data << x << y << z << orientation;

	if(flags & MOVEFLAG_TAXI)
	{
		data << transGuid << transX << transY << transZ << transO << transTime << transSeat;
	}

	if(flags & (MOVEFLAG_SWIMMING | MOVEFLAG_AIR_SWIMMING) || flags16 & 0x20)
	{
		data << pitch;
	}

	data << FallTime;

	if(flags & MOVEFLAG_FALLING || flags & MOVEFLAG_REDIRECTED)
	{
		data << jump_velocity << jump_sinAngle << jump_cosAngle << jump_xySpeed;
	}
	if(flags & MOVEFLAG_SPLINE_MOVER)
	{
		data << spline;
	}
}
