
#include "LacrimiStdAfx.h"

////////////////////////
//// Spell Scripts	////
////////////////////////
bool Cannibalize(uint32 i, Spell* pSpell)
{
	if(pSpell->p_caster != NULL)
	{
		pSpell->p_caster->CastSpell(pSpell->p_caster, 20578, true);
		pSpell->p_caster->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_CANNIBALIZE);
	}
	return true;
}

bool CalloftheAshbringer(uint32 i, Spell* pSpell)
{
	Unit* unitTarget = pSpell->GetUnitTarget();
	if(pSpell->p_caster != NULL && unitTarget != NULL)
	{
		uint32 ashcall = RandomUInt(12);

		WorldPacket data(SMSG_PLAY_OBJECT_SOUND, 12), *crap;
		std::stringstream Reply;
		switch(ashcall)
		{
		case 1:
			{
				data << uint32(8906) << unitTarget->GetGUID();
				pSpell->p_caster->SendMessageToSet(&data, true);
				Reply << "I... was... pure... once.";
				crap = sChatHandler.FillMessageData( CHAT_MSG_WHISPER, LANG_UNIVERSAL, Reply.str().c_str(), pSpell->p_caster->GetGUID(), 0);
			}break;
		case 2:
			{
				data << uint32(8907) << unitTarget->GetGUID();
				pSpell->p_caster->SendMessageToSet(&data, true);
				Reply << "Fought... for... righteousness.";
				crap = sChatHandler.FillMessageData( CHAT_MSG_WHISPER, LANG_UNIVERSAL, Reply.str().c_str(), pSpell->p_caster->GetGUID(), 0);
			}break;
		case 3:
			{
				data << uint32(8908) << unitTarget->GetGUID();
				pSpell->p_caster->SendMessageToSet(&data, true);
				Reply << "I... was... once... called... Ashbringer.";
				crap = sChatHandler.FillMessageData( CHAT_MSG_WHISPER, LANG_UNIVERSAL, Reply.str().c_str(), pSpell->p_caster->GetGUID(), 0);
			}break;
		case 4:
			{
				data << uint32(8920) << unitTarget->GetGUID();
				pSpell->p_caster->SendMessageToSet(&data, true);
				Reply << "Betrayed... by... my... order.";
				crap = sChatHandler.FillMessageData( CHAT_MSG_WHISPER, LANG_UNIVERSAL, Reply.str().c_str(), pSpell->p_caster->GetGUID(), 0);
			}break;
		case 5:
			{
				data << uint32(8921) << unitTarget->GetGUID();
				pSpell->p_caster->SendMessageToSet(&data, true);
				Reply << "Destroyed... by... Kel'Thuzad.";
				crap = sChatHandler.FillMessageData( CHAT_MSG_WHISPER, LANG_UNIVERSAL, Reply.str().c_str(), pSpell->p_caster->GetGUID(), 0);
			}break;
		case 6:
			{
				data << uint32(8922) << unitTarget->GetGUID();
				pSpell->p_caster->SendMessageToSet(&data, true);
				Reply << "Made... to serve.";
				crap = sChatHandler.FillMessageData( CHAT_MSG_WHISPER, LANG_UNIVERSAL, Reply.str().c_str(), pSpell->p_caster->GetGUID(), 0);
			}break;
		case 7:
			{
				data << uint32(8923) << unitTarget->GetGUID();
				pSpell->p_caster->SendMessageToSet(&data, true);
				Reply << "My... son... watched... me... die.";
				crap = sChatHandler.FillMessageData( CHAT_MSG_WHISPER, LANG_UNIVERSAL, Reply.str().c_str(), pSpell->p_caster->GetGUID(), 0);
			}break;
		case 8:
			{
				data << uint32(8924) << unitTarget->GetGUID();
				pSpell->p_caster->SendMessageToSet(&data, true);
				Reply << "Crusades... fed his rage.";
				crap = sChatHandler.FillMessageData( CHAT_MSG_WHISPER, LANG_UNIVERSAL, Reply.str().c_str(), pSpell->p_caster->GetGUID(), 0);
			}break;
		case 9:
			{
				data << uint32(8925) << unitTarget->GetGUID();
				pSpell->p_caster->SendMessageToSet(&data, true);
				Reply << "Truth... is... unknown... to him.";
				crap = sChatHandler.FillMessageData( CHAT_MSG_WHISPER, LANG_UNIVERSAL, Reply.str().c_str(), pSpell->p_caster->GetGUID(), 0);
			}break;
		case 10:
			{
				data << uint32(8926) << unitTarget->GetGUID();
				pSpell->p_caster->SendMessageToSet(&data, true);
				Reply << "Scarlet... Crusade... is pure... no longer.";
				crap = sChatHandler.FillMessageData( CHAT_MSG_WHISPER, LANG_UNIVERSAL, Reply.str().c_str(), pSpell->p_caster->GetGUID(), 0);
			}break;
		case 11:
			{
				data << uint32(8927) << unitTarget->GetGUID();
				pSpell->p_caster->SendMessageToSet(&data, true);
				Reply << "Balnazzar's... crusade... corrupted... my son.";
				crap = sChatHandler.FillMessageData( CHAT_MSG_WHISPER, LANG_UNIVERSAL, Reply.str().c_str(), pSpell->p_caster->GetGUID(), 0);
			}break;
		case 12:
			{
				data << uint32(8928) << unitTarget->GetGUID();
				pSpell->p_caster->SendMessageToSet(&data, true);
				Reply << "Kill... them... all!";
				crap = sChatHandler.FillMessageData( CHAT_MSG_WHISPER, LANG_UNIVERSAL, Reply.str().c_str(), pSpell->p_caster->GetGUID(), 0);
			}break;
		}
		pSpell->p_caster->GetSession()->SendPacket(crap);
	}
	return true;
}

bool CleansingVialDND(uint32 i, Spell* pSpell)
{
	if(pSpell->p_caster != NULL)
	{
		QuestLogEntry *en = pSpell->p_caster->GetQuestLogForEntry(9427);
		if(en != NULL)
		{
			en->Quest_Status = QUEST_STATUS__COMPLETE;
			en->SendQuestComplete();
			en->UpdatePlayerFields();
		}
	}
	return true;
}

bool ForceCastPortalEffectSunwellIsle(uint32 i, Spell* pSpell)
{
	if(pSpell->u_caster != NULL)
	{
		if(pSpell->p_caster != NULL && pSpell->p_caster->getLevel() < 70)
		{
			sChatHandler.RedSystemMessage(pSpell->p_caster->GetSession(),"You must be level 70 to use this!");
			return true;
		}

		pSpell->u_caster->CastSpell(pSpell->GetUnitTarget(), pSpell->damage, true);
	}
	return true;
}

////////////////////////
//// Damage Scripts	////
////////////////////////
void DistanceDamage(uint32 i, Spell* pSpell, uint32 effect)
{
	if( effect == SPELL_EFFECT_SCHOOL_DAMAGE )
	{
		if( pSpell->u_caster != NULL)
		{
			float dist = pSpell->u_caster->CalcDistance( TO_OBJECT( pSpell->GetUnitTarget() ) );
			if( dist <= 20.0f && dist >= 0.0f )
				pSpell->damage = float2int32( -450 * dist + pSpell->damage );
		}
	}
}

void CataclysmicBolt(uint32 i, Spell* pSpell, uint32 effect)
{
	if( effect == SPELL_EFFECT_SCHOOL_DAMAGE )
	{
		pSpell->damage = pSpell->GetUnitTarget()->GetMaxHealth() / 2;
	}
}

void Thundercrash(uint32 i, Spell* pSpell, uint32 effect)
{
	if( effect == SPELL_EFFECT_SCHOOL_DAMAGE )
	{
		pSpell->damage = pSpell->GetUnitTarget()->GetHealth() / 2;
		if(pSpell->damage < 200)
			pSpell->damage = 200;
	}
}

void FatalAttraction(uint32 i, Spell* pSpell, uint32 effect)
{
	if( effect == SPELL_EFFECT_SCHOOL_DAMAGE )
	{
		if(pSpell->GetUnitTarget()->HasAura(43690))// Saber Lash
			pSpell->damage = 0;
	}
}

void TympanicTantrum(uint32 i, Spell* pSpell, uint32 effect)
{
	if( effect == SPELL_EFFECT_SCHOOL_DAMAGE )
	{
		pSpell->damage = pSpell->GetUnitTarget()->GetMaxHealth() / 10;
	}
}

void ChimeraShotSerpant(uint32 i, Spell* pSpell, uint32 effect)
{
	if( effect == SPELL_EFFECT_SCHOOL_DAMAGE )
	{
		pSpell->damage += pSpell->forced_basepoints[0];
	}
}

void Lacrimi::SetupMiscSpells()
{
	////////////////////////
	//// Spell Scripts	////
	////////////////////////
	RegisterSpellScriptEffect(20577, Cannibalize);

	RegisterSpellScriptEffect(28414, CalloftheAshbringer);

	RegisterSpellScriptEffect(29297, CleansingVialDND);

	RegisterSpellScriptEffect(44876, ForceCastPortalEffectSunwellIsle);

	////////////////////////
	//// Damage Scripts	////
	////////////////////////
	RegisterSpellEffectModifier(33654, DistanceDamage);
	RegisterSpellEffectModifier(33671, DistanceDamage);
	RegisterSpellEffectModifier(50810, DistanceDamage);
	RegisterSpellEffectModifier(50811, DistanceDamage);
	RegisterSpellEffectModifier(61547, DistanceDamage);
	RegisterSpellEffectModifier(61546, DistanceDamage);

	RegisterSpellEffectModifier(38441, CataclysmicBolt);

	RegisterSpellEffectModifier(25599, Thundercrash);

	RegisterSpellEffectModifier(41001, FatalAttraction);

	RegisterSpellEffectModifier(62775, TympanicTantrum);

	RegisterSpellEffectModifier(53353, ChimeraShotSerpant);
}


void Todo()
{
	switch(0)
	{
	/*************************
		Non-Class spells
		- ToDo
	 *************************/
	case 6668:// Red Firework
		{
			// Shoots a firework into the air that bursts into a thousand red stars
		}break;
	case 8344:// Universal Remote
		{
			//FIXME:Allows control of a mechanical target for a short time.  It may not always work and may just root the machine or make it very very angry.  Gnomish engineering at its finest.
		}break;
	case 9976:// Polly Eats the E.C.A.C.
		{
			//FIXME: Don't know what this does
		}break;
	case 10137:// Fizzule's Whistle
		{
			//FIXME:Blow on the whistle to let Fizzule know you're an ally
			//This item comes after a finish of quest at venture co.
			//You must whistle this every time you reach there to make Fizzule
			//ally to you.
		}break;
	case 11540:// Blue Firework
		{
			//Shoots a firework into the air that bursts into a thousand blue stars
		}break;
	case 11541:// Green Firework
		{
			//Shoots a firework into the air that bursts into a thousand green stars
		}break;
	case 11542:// Red Streaks Firework
		{
			//Shoots a firework into the air that bursts into a thousand red streaks
		}break;
	case 11543:// Red, White and Blue Firework
		{
			//Shoots a firework into the air that bursts into red, white and blue stars
		}break;
	case 11544:// Yellow Rose Firework
		{
			//Shoots a firework into the air that bursts in a yellow pattern
		}break;
	case 12151:// Summon Atal'ai Skeleton
		{
			//8324	Atal'ai Skeleton

			//FIXME:Add here remove in time event
		}break;
	case 13535:// Tame Beast
		{

		}break;
	case 13006:// Shrink Ray
		{
			//FIXME:Schematic is learned from the gnomish engineering trainer. The gnomish/gobblin engineering decision is made when you are lvl40+ and your engineering is 200+. Interestingly, however, when this item fails to shrink the target, it can do a variety of things, such as...
			//-Make you bigger (attack power +250)
			//-Make you smaller (attack power -250)
			//-Make them bigger (same effect as above)
			//-Make your entire party bigger
			//-Make your entire party smaller
			//-Make every attacking enemy bigger
			//-Make ever attacking enemy smaller
			//Works to your advantage for the most part (about 70% of the time), but don't use in high-pressure situations, unless you're going to die if you don't. Could tip the scales the wrong way.
			//Search for spells of this


			//13004 - grow <- this one
			//13010 - shrink <-this one
			//
		}break;
	case 13180:// Gnomish Mind Control Cap
		{
			// FIXME:Take control of humanoid target by chance(it can be player)
		}break;
	case 13278:// Gnomish Death Ray
		{
			// FIXME:The devices charges over time using your life force and then directs a burst of energy at your opponent
			//Drops life
		}break;
	case 13280:// Gnomish Death Ray
		{
			//FIXME: Drop life
		}break;
	case 17816:// Sharp Dresser
		{
			//Impress others with your fashion sense
		}break;
	case 21343:// Snowball
		{
		}break;
	case 23645:// Hourglass Sand
		{
			//Indeed used at the Chromo fight in BWL. Chromo has a stunning debuff, uncleansable, unless you have hourglass sand. This debuff will stun you every 4 seconds, for 4 seconds. It is resisted a lot though. Mage's and other casters usually have to do this fight with the debuff on, healers, tanks and hunters will get some to cure themselves from the debuff
		}break;
	case 24325:// Pagle's Point Cast - Create Mudskunk Lure
		{
			//FIXME:Load with 5 Zulian Mudskunks, and then cast from Pagle's Point in Zul'Gurub
		}
	case 24392:// Frosty Zap
		{
			//FIXME:Your Frostbolt spells have a 6% chance to restore 50 mana when cast.
			//damage == 50
		}break;
	case 25822:// Firecrackers
		{
			//FIXME:Find firecrackers
		}break;
	case 26373:// Lunar Invititation
		{
			//FIXME: Teleports the caster from within Greater Moonlight
		}break;
	case 26889:// Give Friendship Bracelet
		{
			//Give to a Heartbroken player to cheer them up
			//laugh emote
		}break;
	case 27662:// Throw Cupid's Dart
		{
			//FIXME:Shoot a player, and Kwee Q. Peddlefeet will find them! (Only works on players with no current critter pets.)
		}break;
	case 28806:// Toss Fuel on Bonfire
		{
			//FIXME:Dont know what this dummy does
		}break;
	case 7669:// Bethor's Potion
		{
			// related to Hex of Ravenclaw,
			// its a dispell spell.
			//FIXME:Dont know whats the usage of this dummy
		}break;
	case 8283:// Snufflenose Command
		{
			//FIXME:Quest Blueleaf Tubers
			//For use on a Snufflenose Gopher
		}break;
	case 8913:// Sacred Cleansing
		{
			//FIXME:Removes the protective enchantments around Morbent Fel
			//Quest Morbent Fel
		}break;
	case 9962://Capture Treant
		{
			//Quest Treant Muisek
		}break;
	case 10113:// Flare Gun's flare
		{
			//FIXME:Quest Deep Cover
			//1543 may need to cast this
			//2 flares and the /salute
		}break;
	case 10617:// Release Rageclaw
		{
			//Quest Druid of the Claw
			//Use on the fallen body of Rageclaw
		}break;
	case 11402:// Shay's Bell
		{
			//FIXME:Quest Wandering Shay
			//Ring to call Shay back to you
		}break;
	case 11610:// Gammerita Turtle Camera
		{
			//Quest The Super Snapper FX
		}break;
	case 11886:// Capture Wildkin
		{
			//Quest Testing the Vessel
			//Shrink and Capture a Fallen Wildkin
		}break;
	case 11887:// Capture Hippogryph
		{
			//FIXME:Same with 11888
			//Quest Hippogryph Muisek
		}break;
	case 11888:// Capture Faerie Dragon
		{
			//FIXME:Check Faerie Dragon Muisek is killed or not if its killed update quest
			//And allow create of fearie Dragon which is effect 1
			//Quest: Faerie Dragon Muisek
		}break;
	case 11889:// Capture Mountain Giant
		{
			//FIXME:Same with 11888
			//Quest: Mountain Giant Muisek
		}break;
	case 12189:// Summon Echeyakee
		{
			//3475	Echeyakee

			//FIXME:Quest Echeyakee
		}break;
	case 12283:// Xiggs Signal Flare
		{
			//Quest Signal for Pickup
			//To be used at the makeshift helipad in Azshara. It will summon Pilot Xiggs Fuselighter to pick up the tablet rubbings
		}break;
	case 12938:// Fel Curse
		{
			//FIXME:Makes near target killable(servants of Razelikh the Defiler)
		}break;
	case 14247:// Blazerunner Dispel
		{
			//FIXME:Quest Aquementas and some more
		}break;
	case 14250:// Capture Grark
		{
			//Quest Precarious Predicament
		}break;
	case 14813:// Rocknot's Ale
		{
			//you throw the mug
			//and the guy gets pissed well everyone gets pissed and he crushes the door so you can get past
			//maybe after like 30 seconds so you can get past.  but lke I said I have never done it myself
			//so i am not 100% sure what happens.
		}break;
	case 15991://Revive Ringo
		{
			//Quest A Little Help From My Friends
			//Revive Ringo with water
		}break;
	case 15998:// Capture Worg Pup
		{
			//FIXME:Ends Kibler's Exotic Pets  (Dungeon) quest
		}break;
	case 16031:// Releasing Corrupt Ooze
		{
			//FIXME:Released ooze moves to master ooze and "Merged Ooze Sample"
			//occurs after some time.This item helps to finish quest
		}break;
	case 16378:// Temperature Reading
		{
			//FIXME:Quest Finding the Source
			//Take a reading of the temperature at a hot spot.
		}break;
	case 17166:// Release Umi's Yeti
		{
			//Quest Are We There, Yeti?
			//Select Umi's friend and click to release the Mechanical Yeti
		}break;
	case 17271:// Test Fetid Skull
		{
			//FIXME:Marauders of Darrowshire
			//Wave over a Fetid skull to test its resonance
		}break;
	case 18153:// Kodo Kombobulator
		{
			//FIXME:Kodo Roundup Quest
			//Kodo Kombobulator on any Ancient, Aged, or Dying Kodo to lure the Kodo to follow (one at a time)
		}break;
	case 19250:// Placing Smokey's Explosives
		{
			//This is something related to quest i think
		}break;
	case 20804:// Triage
		{
			//Quest Triage
			//Use on Injured, Badly Injured, and Critically Injured Soldiers
		}break;
	case 21050:// Melodious Rapture
		{
			//Quest Deeprun Rat Roundup
		}break;
	case 21332:// Aspect of Neptulon
		{
			//FIXME:Used on plagued water elementals in Eastern Plaguelands
			//Quest:Poisoned Water
		}break;
	case 21960:// Manifest Spirit
		{
			//FIXME:Forces the spirits of the first centaur Kahns to manifest in the physical world
			//thats a quest
			//its for maraudon i think
			//u use that on the spirit mobs
			//to release them
		}break;
	case 23359:// Transmogrify!
		{
			//Quest Zapped Giants
			//Zap a Feralas giant into a more manageable form
		}break;
	case 27184:// Summon Mor Grayhoof
		{
			//16044	Mor Grayhoof Trigger
			//16080	Mor Grayhoof

			//Related to quests The Left Piece of Lord Valthalak's Amulet  (Dungeon)
			//and The Right Piece of Lord Valthalak's Amulet  (Dungeon)
		}break;
	case 27190:// Summon Isalien
		{
			//16045	Isalien Trigger
			//16097	Isalien

			//Related to quests The Left Piece of Lord Valthalak's Amulet  (Dungeon)
			//and The Right Piece of Lord Valthalak's Amulet  (Dungeon)
		}break;
	case 27191:// Summon the remains of Jarien and Sothos
		{
			/*
			16046	Jarien and Sothos Trigger
			16101	Jarien
			16103	Spirit of Jarien

			16102	Sothos
			16104	Spirit of Sothos
			*/

			//Related to quests The Left Piece of Lord Valthalak's Amulet  (Dungeon)
			//and The Right Piece of Lord Valthalak's Amulet  (Dungeon)
		}break;
	case 27201:// Summon the spirit of Kormok
		{
			/*16047	Kormok Trigger
			16118	Kormok
			*/
			//Related to quests The Left Piece of Lord Valthalak's Amulet  (Dungeon)
			//and The Right Piece of Lord Valthalak's Amulet  (Dungeon)
		}break;
	case 27202:// Summon Lord Valthalak
		{
			/*
			16042	Lord Valthalak
			16048	Lord Valthalak Trigger
			16073	Spirit of Lord Valthalak

			*/
			//Related to quests The Left Piece of Lord Valthalak's Amulet  (Dungeon)
			//and The Right Piece of Lord Valthalak's Amulet  (Dungeon)
		}break;
	case 27203:// Summon the spirits of the dead at haunted locations
		{
			//Related to quests The Left Piece of Lord Valthalak's Amulet  (Dungeon)
			//and The Right Piece of Lord Valthalak's Amulet  (Dungeon)
		}break;
	case 27517:// Use this banner at the Arena in Blackrock Depths to challenge Theldren
		{
			//This is used to make Theldren spawn at the place where it used
			//I couldnt find theldrin, and his men in creature names database
			//Someone has to write this and this is related to The Challange quest
			/*By moving to the center grate, you trigger the arena event.
			A random group of mobs (spiders, worms, bats, raptors) spawns,
			and you have to kill them. After the last one dies, and a small
			break, a boss mob spawns. Successfully completing this event
			turns the arena spectators from red to yellow*/
		}break;
	case 60103: { }break;
	}
}