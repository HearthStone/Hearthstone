/***
 * Demonstrike Core
 */

// Crow: If it has more than 40 columns, break em up.
#include "DBCStores.h"
#include "DataStore.h"
#include "NGLog.h"

SERVER_DECL DBCStorage<AchievementEntry> dbcAchievement;
SERVER_DECL DBCStorage<AchievementCriteriaEntry> dbcAchievementCriteria;
SERVER_DECL DBCStorage<AreaGroup> dbcAreaGroup;
SERVER_DECL DBCStorage<BattleMasterListEntry> dbcBattleMasterList;
SERVER_DECL DBCStorage<CurrencyTypesEntry> dbcCurrencyTypes;
SERVER_DECL DBCStorage<GemPropertyEntry> dbcGemProperty;
SERVER_DECL DBCStorage<GlyphPropertyEntry> dbcGlyphProperty;
SERVER_DECL DBCStorage<ItemSetEntry> dbcItemSet;
SERVER_DECL DBCStorage<Lock> dbcLock;
SERVER_DECL DBCStorage<LookingForGroup> dbcLookingForGroup;
SERVER_DECL DBCStorage<SpellEntry> dbcSpell;
SERVER_DECL DBCStorage<SpellDuration> dbcSpellDuration;
SERVER_DECL DBCStorage<SpellRange> dbcSpellRange;
SERVER_DECL DBCStorage<SpellRuneCostEntry> dbcSpellRuneCost;
SERVER_DECL DBCStorage<emoteentry> dbcEmoteEntry;
SERVER_DECL DBCStorage<SpellRadius> dbcSpellRadius;
SERVER_DECL DBCStorage<SpellCastTime> dbcSpellCastTime;
SERVER_DECL DBCStorage<SpellDifficultyEntry> dbcSpellDifficulty;
SERVER_DECL DBCStorage<AreaTable> dbcArea;
SERVER_DECL DBCStorage<AreaTriggerEntry> dbcAreaTrigger;
SERVER_DECL DBCStorage<FactionTemplateDBC> dbcFactionTemplate;
SERVER_DECL DBCStorage<FactionDBC> dbcFaction;
SERVER_DECL DBCStorage<EnchantEntry> dbcEnchant;
SERVER_DECL DBCStorage<RandomProps> dbcRandomProps;
SERVER_DECL DBCStorage<skilllinespell> dbcSkillLineSpell;
SERVER_DECL DBCStorage<skilllineentry> dbcSkillLine;
SERVER_DECL DBCStorage<DBCTaxiNode> dbcTaxiNode;
SERVER_DECL DBCStorage<DBCTaxiPath> dbcTaxiPath;
SERVER_DECL DBCStorage<DBCTaxiPathNode> dbcTaxiPathNode;
SERVER_DECL DBCStorage<AuctionHouseDBC> dbcAuctionHouse;
SERVER_DECL DBCStorage<TalentEntry> dbcTalent;
SERVER_DECL DBCStorage<TalentTabEntry> dbcTalentTab;
SERVER_DECL DBCStorage<CreatureBoundData> dbcCreatureBoundData;
SERVER_DECL DBCStorage<CreatureDisplayInfo> dbcCreatureDisplayInfo;
SERVER_DECL DBCStorage<CreatureSpellDataEntry> dbcCreatureSpellData;
SERVER_DECL DBCStorage<CreatureFamilyEntry> dbcCreatureFamily;
SERVER_DECL DBCStorage<CharClassEntry> dbcCharClass;
SERVER_DECL DBCStorage<CharRaceEntry> dbcCharRace;
SERVER_DECL DBCStorage<MapEntry> dbcMap;
SERVER_DECL DBCStorage<ItemEntry> dbcItem;
SERVER_DECL DBCStorage<ItemExtendedCostEntry> dbcItemExtendedCost;
SERVER_DECL DBCStorage<ItemRandomSuffixEntry> dbcItemRandomSuffix;
SERVER_DECL DBCStorage<ItemLimitCategoryEntry> dbcItemLimitCategory;
SERVER_DECL DBCStorage<CombatRatingDBC> dbcCombatRating;
SERVER_DECL DBCStorage<ChatChannelDBC> dbcChatChannels;
SERVER_DECL DBCStorage<DurabilityQualityEntry> dbcDurabilityQuality;
SERVER_DECL DBCStorage<DurabilityCostsEntry> dbcDurabilityCosts;
SERVER_DECL DBCStorage<BankSlotPrice> dbcBankSlotPrices;
SERVER_DECL DBCStorage<BankSlotPrice> dbcStableSlotPrices;
SERVER_DECL DBCStorage<BarberShopStyleEntry> dbcBarberShopStyle;
SERVER_DECL DBCStorage<gtFloat> dbcBarberShopPrices;
SERVER_DECL DBCStorage<gtFloat> dbcMeleeCrit;
SERVER_DECL DBCStorage<gtFloat> dbcMeleeCritBase;
SERVER_DECL DBCStorage<gtFloat> dbcSpellCrit;
SERVER_DECL DBCStorage<gtFloat> dbcSpellCritBase;
SERVER_DECL DBCStorage<gtFloat> dbcManaRegen;
SERVER_DECL DBCStorage<gtFloat> dbcManaRegenBase;
SERVER_DECL DBCStorage<gtFloat> dbcHPRegen;
SERVER_DECL DBCStorage<gtFloat> dbcHPRegenBase;
SERVER_DECL DBCStorage<QuestXP> dbcQuestXP;
SERVER_DECL DBCStorage<SpellShapeshiftForm> dbcSpellShapeshiftForm;
SERVER_DECL DBCStorage<VehicleEntry> dbcVehicle;
SERVER_DECL DBCStorage<VehicleSeatEntry> dbcVehicleSeat;
SERVER_DECL DBCStorage<WorldMapOverlayEntry> dbcWorldMapOverlay;
SERVER_DECL DBCStorage<WMOAreaTableEntry> dbcWMOAreaTable;
SERVER_DECL DBCStorage<SummonPropertiesEntry> dbcSummonProps;
SERVER_DECL DBCStorage<ScalingStatDistributionEntry> dbcScalingStatDistribution;
SERVER_DECL DBCStorage<ScalingStatValuesEntry> dbcScalingStatValues;
SERVER_DECL DBCStorage<DestructibleModelDataEntry> dbcDestructibleModelDataEntry;

const char* SummonPropertiesfmt = "uuuuuu";
const char* AreaGroupFormat="niiiiiii";
const char* questxpformat = "uxuuuuuuuux";
const char* CurrencyTypesEntryFormat = "xnxu";
const char* ItemSetFormat = "uxxxxxxxxxxxxxxxxxuuuuuuuuuuxxxxxxxuuuuuuuuuuuuuuuuuu";
const char* LockFormat = "uuuuuuuuuuuuuuuuuuuuuuuuuxxxxxxxx";
const char* LFGDungeonsFormat = "uxxxxxxxxxxxxxxxxxuuuuuiuxuxxuxuxxxxxxxxxxxxxxxxx";
const char* EmoteEntryFormat = "uxuxxxxxxxxxxxxxxxx";
const char* skilllinespellFormat = "uuuuuuuuuuuuxx";
const char* EnchantEntrYFormat = "uxuuuuuuuuuuuusxxxxxxxxxxxxxxxxuuuuxxx";
const char* GemPropertyEntryFormat = "uuuuu";
const char* GlyphPropertyEntryFormat = "uuuu";
const char* skilllineentrYFormat = "uuusxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
const char* spellentryFormat =
	"u" // Id
	"u" // Category
	"u" // DispelType
	"u" // MechanicsType
	"uuuuuuuu" // 8x Flags
	"u" // RequiredShapeShift
	"x" // unk00
	"u" // NotAllowedShapeShift
	"x" // unk01
	"u" // Targets
	"u" // TargetCreatureType
	"u" // RequiresSpellFocus
	"u" // FacingCasterFlags
	"uuuuuuuu" // 8x Aura crap
	"u" // CastingTimeIndex
	"u" // RecoveryTime
	"u" // CategoryRecoveryTime
	"u" // InterruptFlags
	"u" // AuraInterruptFlags
	"u" // ChannelInterruptFlags
	"u" // procFlags
	"u" // procChance
	"u" // procCharges
	"u" // maxLevel
	"u" // baseLevel
	"u" // spellLevel
	"u" // DurationIndex
	"u" // powerType
	"u" // manaCost
	"u" // manaCostPerlevel
	"u" // manaPerSecond
	"u" // manaPerSecondPerLevel
	"u" // rangeIndex
	"f" // Speed
	"u" // modalNextSpell
	"u" // maxstack
	"uu" // Totem
	"uuuuuuuu" // 8x Reagent
	"uuuuuuuu" // 8x ReagentCount
	"i" // EquippedItemClass
	"u" // EquippedItemSubClass
	"u" // RequiredItemFlags
	"uuu" // 3x Spell Effect
	"uuu" // 3x Spell Effect Die Sides
	"fff" // 3x Spell Effect Real Points per Level
	"iii" // 3x Spell Effect Base Points
	"iii" // 3x Spell Effect Mechanic
	"uuu" // 3x Spell Effect Target A
	"uuu" // 3x Spell Effect Target B
	"uuu" // 3x Spell Effect Radius index
	"uuu" // 3x Spell Effect Apply Aura Name
	"uuu" // 3x Spell Effect Amplitude
	"fff" // 3x Spell Effect Multiple Value
	"uuu" // 3x Spell Effect Chain Target
	"uuu" // 3x Spell Effect Item Type
	"uuu" // 3x Spell Effect Misc Value
	"uuu" // 3x Spell Effect Misc Value B
	"uuu" // 3x Spell Effect Trigger Spell
	"fff" // 3x Spell Effect Points per Combo Point
	"uuuuuuuuu" // 3x Spell Effect Class Mask[3]
	"uu" // 2x SpellVisual
	"u" // SpellIconID
	"u" // activeIconID
	"u" // spellPriority
	"sxxxxxxxxxxxxxxxx" // Name
	"sxxxxxxxxxxxxxxxx" // Rank
	"sxxxxxxxxxxxxxxxx" // Spell Description
	"sxxxxxxxxxxxxxxxx" // Buff Description
	"u" // ManaCostPercentage
	"u" // StartRecoveryCategory
	"u" // StartRecoveryTime
	"u" // MaxTargetLevel
	"u" // SpellFamilyName
	"uuu" // 3x SpellGroupType
	"u" // MaxTargets
	"u" // Spell_Dmg_Type
	"u" // PreventionType
	"i" // StanceBarOrder
	"fff" // 3x dmg_multiplier
	"u" // MinFactionID
	"u" // MinReputation
	"u" // RequiredAuraVision
	"uu" // 2x TotemCategory
	"i" // AreaGroupId
	"u" // School
	"ux" // runeCostID and spellMissileID
	"u" // PowerDisplayId
	"xxxx"// Unk
	"u"; // SpellDifficultyID

const char* spelldifficultyFormat = "uuuuu";
const char* itemFormat = "uuuiiuuu";
const char* itemextendedcostFormat = "uuuuuuuuuuuuuuux";
const char* talententryFormat = "uuuuuuuuuxxxxuxxuxxxuxx";
const char* talenttabentryFormat = "uxxxxxxxxxxxxxxxxxxxuxux";
const char* spellcasttimeFormat = "uuxx";
const char* spellradiusFormat = "ufxf";
const char* spellrangeFormat =
	"u"					// Id
	"f"					// minRangeHostile
	"f"					// minRangeFriend
	"f"					// maxRangeHostile
	"f"					// maxRangeFriend
	"u"					// type
	"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

const char* WorldMapOverlayfmt="uxuxxxxxxxxxxxxxx";
const char* WMOAreaEntryfmt="niiixxxxxiixxxxxxxxxxxxxxxxx";
const char* SpellRuneCostfmt="uuuuu";
const char* spelldurationFormat = "uiii";
const char* randompropsFormat = "usuuuxxxxxxxxxxxxxxxxxxx";
const char* areatableFormat = "uuuuuxxxxxusxxxxxxxxxxxxxxxxuxxxxxxx";
const char* areatriggerFormat = "uuffffffff"; // ffffffffuu
const char* factiontemplatedbcFormat = "uuuuuuuuuuuuuu";
const char* auctionhousedbcFormat = "uuuuxxxxxxxxxxxxxxxxx";
const char* BattleMasterEntryFormat = "uiiiiiiiiuusxxxxxxxxxxxxxxxuuxuu";
const char* factiondbcFormat = "uiuuuuuuuuiiiiuuuuusxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
const char* dbctaxinodeFormat = "uufffxxxxxxxxxxxxxxxxxuu";
const char* dbctaxipathFormat = "uuuu";
const char* dbctaxipathnodeFormat = "uuuufffuuxx";
const char* creaturedisplayFormat = "uxxxfxxxxxxxxxxx";
const char* creatureboundFormat = "ufffffff";
const char* creaturespelldataFormat = "uuuuuuuuu";
const char* charraceFormat = "uxxxxxxxxxxxuusxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
const char* charclassFormat = "uxuxsxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxuxux";
const char* creaturefamilyFormat = "ufufuuuuuxsxxxxxxxxxxxxxxxxx";
const char* mapentryFormat =
	"u"					// 0 id
	"x"					// 1 name_internal
	"u"					// 2 map_type
	"x"					// 3 is_pvp_zone
	"x"					// 4 0 or 1 for battlegrounds (not arenas)
	"xxxxxxxxxxxxxxxxx"	// 5-21 real_name
	"x"					// 22 linked_zone
	"xxxxxxxxxxxxxxxxx" // 23-39 hordeIntro
	"xxxxxxxxxxxxxxxxx" // 40-56 allianceIntro
	"u"					// 57 multimap_id
	"x"					// 58 unk_float (all 1 but arathi 1.25)
	"x"					// 59 parent_map
	"x"					// 60 start_x
	"x"					// 61 start_y
	"x"					// 62 unk
	"x"					// 63 addon
	"x"					// 64 normalReqText
	"u";				// 65 Max players

const char* itemrandomsuffixformat = "usxxxxxxxxxxxxxxxxxuuuuuuuuuu";
const char* chatchannelformat = "uuxsxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
const char* durabilityqualityFormat = "uf";
const char* durabilitycostsFormat = "uuuuuuuuuuuuuuuuuuuuuuuuuuuuuu";
const char* bankslotpriceformat = "uu";
const char* barbershopstyleFormat="nusxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxuuu";
const char* gtfloatformat = "f";
const char* spellshapeshiftformformat = "uxxxxxxxxxxxxxxxxxxuixuxxxxuuuuuuuu";

const char* vehicleseatentryFormat = "uuxfffxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxuuxxxxxxxxxxxx";
const char* vehicleentryFormat = "uuffxxuuuuuuuuxxxxxxxxxxxxxxxxxxxxxxxuxx";

const char* achievementfmt=
	"n" // Index
	"i" // Faction
	"i" // Mapid
	"u" // Previous Achievement
	"s" // Name
	"xxxxxxxxxxxxxxxx"
	"s" // Description
	"xxxxxxxxxxxxxxxx"
	"i" // Category Id
	"x" // points
	"x" // Order In Category
	"i" // Flags
	"xxxxxxxxxxxxxxxxxxxx";

const char* achievementCriteriafmt="niiiiiiiisxxxxxxxxxxxxxxxxiixix";

const char* scalingstatdistributionformat =
	"u" // ID
	"iiiiiiiiii" // Stat Mod
	"uuuuuuuuuu" // Modifier
	"u"; // Max Level

const char* scalingstatvaluesformat =
	"x" // Id
	"u" // Level
	"uuuu" // ScalingStatD modifier
	"uuuu" // Armor Mod
	"uuuuuu" // DPS mod
	"u" // Spell Power
	"uux" // Multipliers
	"uuuu"; // Armor Type[level]

const char* DestructibleModelDataFormat = "uxxuxxxuxxxuxxxuxxx";
const char* itemlimitcategoryformat = "uxxxxxxxxxxxxxxxxxux";

template<class T>
bool loader_stub(const char * filename, const char * format, bool ind, T& l, bool loadstrs)
{
	Log.Notice("DBC", "Loading %s.", filename);
	return l.Load(filename, format, ind, loadstrs);
}

#define LOAD_DBC(filename, format, ind, stor, strings) if(!loader_stub(filename, format, ind, stor, strings)) { return false; } 

bool LoadRSDBCs(const char* datapath)
{
	/* Needed for: */
	LOAD_DBC(format("%s/AreaTable.dbc", datapath).c_str(), areatableFormat, true, dbcArea, true);
	/* Needed for: */
	LOAD_DBC(format("%s/ChatChannels.dbc", datapath).c_str(), chatchannelformat, true, dbcChatChannels, true);
	/* Needed for: */
	LOAD_DBC(format("%s/ChrClasses.dbc", datapath).c_str(), charclassFormat, true, dbcCharClass, true);
	/* Needed for: */
	LOAD_DBC(format("%s/ChrRaces.dbc", datapath).c_str(), charraceFormat, true, dbcCharRace, true);
	return true;
}

bool LoadDBCs(const char* datapath)
{
	/* Needed for: Used in loading of achievements and finding saving information and grabbing criteria
	info to see if player deserves achievement. */
	LOAD_DBC(format("%s/Achievement.dbc", datapath).c_str(), achievementfmt,true, dbcAchievement,true);
	/* Needed for: */
	LOAD_DBC(format("%s/Achievement_Criteria.dbc", datapath).c_str(), achievementCriteriafmt,true,dbcAchievementCriteria,true);
	/* Needed for: */
	LOAD_DBC(format("%s/AreaGroup.dbc", datapath).c_str(), AreaGroupFormat, true, dbcAreaGroup, true);
	/* Needed for: */
	LOAD_DBC(format("%s/AreaTable.dbc", datapath).c_str(), areatableFormat, true, dbcArea, true);
	/* Needed for: */
	LOAD_DBC(format("%s/AreaTrigger.dbc", datapath).c_str(), areatriggerFormat, true, dbcAreaTrigger, true);
	/* Needed for: */
	LOAD_DBC(format("%s/AuctionHouse.dbc", datapath).c_str(), auctionhousedbcFormat, true, dbcAuctionHouse, false);
	/* Needed for: */
	LOAD_DBC(format("%s/BattlemasterList.dbc", datapath).c_str(), BattleMasterEntryFormat, true, dbcBattleMasterList, true);
	/* Needed for: */
	LOAD_DBC(format("%s/BankBagSlotPrices.dbc", datapath).c_str(), bankslotpriceformat, true, dbcBankSlotPrices, false);
	/* Needed for: */
	LOAD_DBC(format("%s/BarberShopStyle.dbc", datapath).c_str(), barbershopstyleFormat, true, dbcBarberShopStyle, true);
	/* Needed for: */
	LOAD_DBC(format("%s/ChatChannels.dbc", datapath).c_str(), chatchannelformat, true, dbcChatChannels, true);
	/* Needed for: */
	LOAD_DBC(format("%s/CurrencyTypes.dbc", datapath).c_str(), CurrencyTypesEntryFormat, true, dbcCurrencyTypes, true);
	/* Needed for: */
	LOAD_DBC(format("%s/ChrClasses.dbc", datapath).c_str(), charclassFormat, true, dbcCharClass, true);
	/* Needed for: */
	LOAD_DBC(format("%s/ChrRaces.dbc", datapath).c_str(), charraceFormat, true, dbcCharRace, true);
	/* Needed for: */
	LOAD_DBC(format("%s/CreatureBoundInformation.dbc", datapath).c_str(), creatureboundFormat, true, dbcCreatureBoundData, true);
	/* Needed for: */
	LOAD_DBC(format("%s/CreatureDisplayInfo.dbc", datapath).c_str(), creaturedisplayFormat, true, dbcCreatureDisplayInfo, true);
	/* Needed for: */
	LOAD_DBC(format("%s/CreatureFamily.dbc", datapath).c_str(), creaturefamilyFormat, true, dbcCreatureFamily, true);
	/* Needed for: */
	LOAD_DBC(format("%s/CreatureSpellData.dbc", datapath).c_str(), creaturespelldataFormat, true, dbcCreatureSpellData, false);
	/* Needed for: */
	LOAD_DBC(format("%s/DurabilityQuality.dbc", datapath).c_str(), durabilityqualityFormat, true, dbcDurabilityQuality, false);
	/* Needed for: */
	LOAD_DBC(format("%s/DurabilityCosts.dbc", datapath).c_str(), durabilitycostsFormat, true, dbcDurabilityCosts, false);
	/* Needed for: */
	LOAD_DBC(format("%s/EmotesText.dbc", datapath).c_str(), EmoteEntryFormat, true, dbcEmoteEntry, false);
	/* Needed for: */
	LOAD_DBC(format("%s/Faction.dbc", datapath).c_str(), factiondbcFormat, true, dbcFaction, true);
	/* Needed for: */
	LOAD_DBC(format("%s/FactionTemplate.dbc", datapath).c_str(), factiontemplatedbcFormat, true, dbcFactionTemplate, false);
	/* Needed for: */
	LOAD_DBC(format("%s/GemProperties.dbc", datapath).c_str(), GemPropertyEntryFormat, true, dbcGemProperty, false);
	/* Needed for: */
	LOAD_DBC(format("%s/GlyphProperties.dbc", datapath).c_str(), GlyphPropertyEntryFormat, true, dbcGlyphProperty, false);
	/* Needed for: */
	LOAD_DBC(format("%s/gtBarberShopCostBase.dbc", datapath).c_str(), gtfloatformat, false, dbcBarberShopPrices, false);
	/* Needed for: */
	LOAD_DBC(format("%s/gtChanceToMeleeCrit.dbc", datapath).c_str(), gtfloatformat, false, dbcMeleeCrit, false);
	/* Needed for: */
	LOAD_DBC(format("%s/gtChanceToMeleeCritBase.dbc", datapath).c_str(), gtfloatformat, false, dbcMeleeCritBase, false);
	/* Needed for: */
	LOAD_DBC(format("%s/gtChanceToSpellCrit.dbc", datapath).c_str(), gtfloatformat, false, dbcSpellCrit, false);
	/* Needed for: */
	LOAD_DBC(format("%s/gtChanceToSpellCritBase.dbc", datapath).c_str(), gtfloatformat, false, dbcSpellCritBase, false);
	/* Needed for: */
	LOAD_DBC(format("%s/gtCombatRatings.dbc", datapath).c_str(), gtfloatformat, false, dbcCombatRating, false);
	/* Needed for: */
	LOAD_DBC(format("%s/gtOCTRegenHP.dbc", datapath).c_str(), gtfloatformat, false, dbcHPRegen, false);
	/* Needed for: */
	LOAD_DBC(format("%s/gtOCTRegenMP.dbc", datapath).c_str(), gtfloatformat, false, dbcManaRegen, false);
	/* Needed for: */
	LOAD_DBC(format("%s/gtRegenHPPerSpt.dbc", datapath).c_str(), gtfloatformat, false, dbcHPRegenBase, false);
	/* Needed for: */
	LOAD_DBC(format("%s/gtRegenMPPerSpt.dbc", datapath).c_str(), gtfloatformat, false, dbcManaRegenBase, false);
	/* Needed for: */
	LOAD_DBC(format("%s/Item.dbc", datapath).c_str(), itemFormat, true, dbcItem, false);
	/* Needed for: */
	LOAD_DBC(format("%s/ItemExtendedCost.dbc", datapath).c_str(), itemextendedcostFormat, true, dbcItemExtendedCost, false);
	/* Needed for: */
	LOAD_DBC(format("%s/ItemSet.dbc", datapath).c_str(), ItemSetFormat, true, dbcItemSet, true);
	/* Needed for: */
	LOAD_DBC(format("%s/ItemRandomProperties.dbc", datapath).c_str(), randompropsFormat, true, dbcRandomProps, false);
	/* Needed for: */
	LOAD_DBC(format("%s/ItemRandomSuffix.dbc", datapath).c_str(), itemrandomsuffixformat, true, dbcItemRandomSuffix, false);
	/* Needed for: */
	LOAD_DBC(format("%s/Lock.dbc", datapath).c_str(), LockFormat, true, dbcLock, false);
	/* Needed for: LFG and Random dungeon calculations */
	LOAD_DBC(format("%s/LFGDungeons.dbc", datapath).c_str(), LFGDungeonsFormat, true, dbcLookingForGroup, false);
	/* Needed for: */
	LOAD_DBC(format("%s/Map.dbc", datapath).c_str(), mapentryFormat, true, dbcMap, true);
	/* Needed for: */
	LOAD_DBC(format("%s/QuestXP.dbc", datapath).c_str(), questxpformat, true, dbcQuestXP, false);
	/* Needed for: */
	LOAD_DBC(format("%s/ScalingStatDistribution.dbc", datapath).c_str(), scalingstatdistributionformat, true, dbcScalingStatDistribution, false);
	/* Needed for: */
	LOAD_DBC(format("%s/ScalingStatValues.dbc", datapath).c_str(), scalingstatvaluesformat, true, dbcScalingStatValues, false);
	/* Needed for: */
	LOAD_DBC(format("%s/StableSlotPrices.dbc", datapath).c_str(), bankslotpriceformat, true, dbcStableSlotPrices, false);
	/* Needed for: */
	LOAD_DBC(format("%s/SkillLine.dbc", datapath).c_str(), skilllineentrYFormat, true, dbcSkillLine, true);
	/* Needed for: */
	LOAD_DBC(format("%s/SkillLineAbility.dbc", datapath).c_str(), skilllinespellFormat, false, dbcSkillLineSpell, false);
	/* Needed for: */
	LOAD_DBC(format("%s/Spell.dbc", datapath).c_str(), spellentryFormat, true, dbcSpell, true);
	/* Needed for: */
	LOAD_DBC(format("%s/SpellCastTimes.dbc", datapath).c_str(), spellcasttimeFormat, true, dbcSpellCastTime, false);
	/* Needed for: */
	LOAD_DBC(format("%s/SpellDifficulty.dbc", datapath).c_str(), spelldifficultyFormat, true, dbcSpellDifficulty, false);
	/* Needed for: */
	LOAD_DBC(format("%s/SpellDuration.dbc", datapath).c_str(), spelldurationFormat, true, dbcSpellDuration, false);
	/* Needed for: */
	LOAD_DBC(format("%s/SpellItemEnchantment.dbc", datapath).c_str(), EnchantEntrYFormat, true, dbcEnchant, true);
	/* Needed for: */
	LOAD_DBC(format("%s/SpellRadius.dbc", datapath).c_str(), spellradiusFormat, true, dbcSpellRadius, false);
	/* Needed for: */
	LOAD_DBC(format("%s/SpellRange.dbc", datapath).c_str(), spellrangeFormat, true, dbcSpellRange, false);
	/* Needed for: Spell costs and calculations for dummy scripts or scripted spells for DK's. */
	LOAD_DBC(format("%s/SpellRuneCost.dbc", datapath).c_str(), SpellRuneCostfmt, true, dbcSpellRuneCost, false);
	/* Needed for: */
	LOAD_DBC(format("%s/SpellShapeshiftForm.dbc", datapath).c_str(), spellshapeshiftformformat, true, dbcSpellShapeshiftForm, false);
	/* Needed for: */
	LOAD_DBC(format("%s/SummonProperties.dbc", datapath).c_str(), SummonPropertiesfmt, true, dbcSummonProps, false);
	/* Needed for: */
	LOAD_DBC(format("%s/Talent.dbc", datapath).c_str(), talententryFormat, true, dbcTalent, false);
	/* Needed for: */
	LOAD_DBC(format("%s/TalentTab.dbc", datapath).c_str(), talenttabentryFormat, true, dbcTalentTab, false);
	/* Needed for: */
	LOAD_DBC(format("%s/TaxiNodes.dbc", datapath).c_str(), dbctaxinodeFormat, false, dbcTaxiNode, false);
	/* Needed for: */
	LOAD_DBC(format("%s/TaxiPath.dbc", datapath).c_str(), dbctaxipathFormat, false, dbcTaxiPath, false);
	/* Needed for: */
	LOAD_DBC(format("%s/TaxiPathNode.dbc", datapath).c_str(), dbctaxipathnodeFormat, false, dbcTaxiPathNode, false);
	/* Needed for: */
	LOAD_DBC(format("%s/Vehicle.dbc", datapath).c_str(), vehicleentryFormat, true, dbcVehicle, true);
	/* Needed for: */
	LOAD_DBC(format("%s/VehicleSeat.dbc", datapath).c_str(), vehicleseatentryFormat, true, dbcVehicleSeat, true);
	/* Needed for: */
	LOAD_DBC(format("%s/WorldMapOverlay.dbc", datapath).c_str(), WorldMapOverlayfmt, true, dbcWorldMapOverlay, true);
	/* Needed for: */
	LOAD_DBC(format("%s/WMOAreaTable.dbc", datapath).c_str(), WMOAreaEntryfmt, true, dbcWMOAreaTable, true);
	/* Needed for: */
	LOAD_DBC(format("%s/DestructibleModelData.dbc", datapath).c_str(), DestructibleModelDataFormat, true, dbcDestructibleModelDataEntry, false);
	return true;
}
