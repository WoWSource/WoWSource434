#include "InfoMgr.h"

void InfoMgr::Initialize()
{
    ASSERT(m_charInfos.size() == 0);

    sLog->outInfo(LOG_FILTER_SERVER_LOADING, "\n\nInitializing InfoMgr...");
    uint32 count;
    uint32 allStart = getMSTime();

    // General stuff
    uint32 start = allStart;
    count = 0;
    if (QueryResult result = CharacterDatabase.Query("SELECT guid, name, race, gender, class, account, level, zone FROM characters WHERE account != 0"))
    {
        do
        {
            Field *fields = result->Fetch();
            UpdateCharBase(fields[0].GetUInt32() /* guid */, fields[1].GetString() /* name */, fields[3].GetUInt8() /*gender*/,
                fields[2].GetUInt8() /*race*/, fields[4].GetUInt8() /*class*/, fields[5].GetUInt32() /* account */, fields[6].GetUInt8() /* level */, fields[7].GetUInt16() /* zone */);
            ++count;
        } while (result->NextRow());
    }
    sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded base for %u characters (%ums)", count, GetMSTimeDiffToNow(start));

    // MMR
    start = getMSTime();
    count = 0;
    if (QueryResult mmrResult = CharacterDatabase.Query("SELECT guid, matchMakerRating, slot FROM character_arena_stats WHERE guid != 0"))
    {
        do
        {
            Field *fields = mmrResult->Fetch();
            UpdateCharMMR(fields[0].GetUInt32() /* guid */, fields[1].GetUInt32() /* MMR */, fields[2].GetUInt8() /* slot */);
            ++count;
        } while (mmrResult->NextRow());
    }
    sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded MMR for %u characters (%ums)", count, GetMSTimeDiffToNow(start));

    // Guilds
    start = getMSTime();
    count = 0;
    if (QueryResult guildResult = CharacterDatabase.Query("SELECT guildid, guid FROM guild_member"))
    {
        do
        {
            Field *fields = guildResult->Fetch();
            UpdateCharGuild(fields[1].GetUInt32(), fields[0].GetUInt32());
            ++count;
        } while (guildResult->NextRow());
    }
    sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded guild for %u characters (%ums)", count, GetMSTimeDiffToNow(start));

    // Groups
    start = getMSTime();
    count = 0;
    if (QueryResult groupResult = CharacterDatabase.Query("SELECT guid, memberGuid FROM group_member"))
    {
        do
        {
            Field *fields = groupResult->Fetch();
            UpdateCharGroup(fields[1].GetUInt32(), fields[0].GetUInt32());
            ++count;
        } while (groupResult->NextRow());
    }
    sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded group for %u characters (%ums)", count, GetMSTimeDiffToNow(start));

    // Arena teams
    start = getMSTime();
    count = 0;
    if (QueryResult arenaResult = CharacterDatabase.Query("SELECT arena_team.arenateamid, arena_team.type, arena_team_member.guid FROM arena_team JOIN arena_team_member ON arena_team.arenateamid = arena_team_member.arenateamid"))
    {
        do
        {
            Field *fields = arenaResult->Fetch();
            UpdateCharArenaTeam(fields[2].GetUInt32(), fields[0].GetUInt32(), ArenaTeam::GetSlotByType(fields[1].GetUInt8()));
            ++count;
        } while (arenaResult->NextRow());
    }
    sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded %u arena teams (%ums)", count, GetMSTimeDiffToNow(start));

    // Account char counts
    start = getMSTime();
    count = 0;
    if (QueryResult charCountResult = CharacterDatabase.Query("SELECT account, COUNT(1) FROM characters WHERE account <> 0 GROUP BY account"))
    {
        do
        {
            Field *fields = charCountResult->Fetch();
            SetAccountCharCount(fields[0].GetUInt32(), fields[1].GetUInt64());
            ++count;
        } while (charCountResult->NextRow());
    }
    sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded char count for %u accounts (%ums)", count, GetMSTimeDiffToNow(start));

    // pets
    /*start = getMSTime();
    count = 0;
    if (QueryResult pets = CharacterDatabase.Query("SELECT id, entry, owner, modelid, CreatedBySpell, PetType, Reactstate, name, renamed, slot, curhealth, curmana, curhappiness, savetime, abdata, level, exp FROM character_pet"))
    {
        do
        {
            Field *fields = pets->Fetch();
            UpdatePet(fields[0].GetUInt32(), fields[1].GetUInt32(), fields[2].GetUInt32(), fields[3].GetUInt32(), fields[4].GetUInt32(), fields[5].GetUInt32(), fields[6].GetUInt8(), fields[7].GetString(),
                fields[8].GetUInt8(), fields[9].GetUInt8(), fields[10].GetUInt32(), fields[11].GetUInt32(), fields[12].GetUInt32(), fields[13].GetUInt32(), fields[14].GetString(), fields[15].GetUInt8(),
                fields[16].GetUInt32());
            ++count;
        } while (pets->NextRow());
    }
    sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded base for %u pets (%ums)", count, GetMSTimeDiffToNow(start));

    start = getMSTime();
    count = 0;
    if (QueryResult petAuras = CharacterDatabase.Query("SELECT guid, caster_guid, spell, effect_mask, recalculate_mask, stackcount, amount0, amount1, amount2, base_amount0, base_amount1, base_amount2, maxduration, remaintime, remaincharges FROM pet_aura WHERE guid != 0 AND caster_guid != 0"))
    {
        do
        {
            Field *fields = petAuras->Fetch();
            UpdatePetAuras(fields[0].GetUInt32(), fields[1].GetUInt64(), fields[2].GetUInt32(), fields[3].GetUInt8(), fields[4].GetUInt8(), fields[5].GetUInt8(), fields[6].GetInt32(), fields[7].GetInt32(),
                fields[8].GetInt32(), fields[9].GetInt32(), fields[10].GetInt32(), fields[11].GetInt32(), fields[12].GetInt32(), fields[13].GetInt32(), fields[14].GetUInt8());
            ++count;
        } while (petAuras->NextRow());
    }
    sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded %u petauras (%ums)", count, GetMSTimeDiffToNow(start));

    start = getMSTime();
    count = 0;
    if (QueryResult petSpells = CharacterDatabase.Query("SELECT guid, spell, active FROM pet_spell WHERE guid != 0"))
    {
        do
        {
            Field *fields = petSpells->Fetch();
            UpdatePetSpells(fields[0].GetUInt32(), fields[1].GetUInt32(), fields[2].GetUInt8());
            ++count;
        } while (petSpells->NextRow());
    }
    sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded %u petSpells (%ums)", count, GetMSTimeDiffToNow(start));

    start = getMSTime();
    count = 0;
    if (QueryResult petSpellCooldowns = CharacterDatabase.Query("SELECT guid, spell, time FROM pet_spell_cooldown WHERE guid != 0"))
    {
        do
        {
            Field *fields = petSpellCooldowns->Fetch();
            UpdatePetSpellCooldowns(fields[0].GetUInt32(), fields[1].GetUInt32(), fields[2].GetUInt32());
            ++count;
        } while (petSpellCooldowns->NextRow());
    }
    sLog->outInfo(LOG_FILTER_SERVER_LOADING, ">> Loaded %u petSpellCooldowns (%ums)", count, GetMSTimeDiffToNow(start));

    // remove this if you want to use petsMap, f.i. find SavedPet by Guid
    petsMap.clear();*/

    sLog->outInfo(LOG_FILTER_SERVER_LOADING, "Done initializing InfoMgr. (%ums)", GetMSTimeDiffToNow(allStart));

}

bool InfoMgr::GetCharInfo(uint32 guid, InfoCharEntry &info)
{
    ACE_Read_Guard<ACE_Thread_Mutex> g(m_charMutex);

    CharInfoItr itr = m_charInfos.find(guid);
    if (itr != m_charInfos.end())
    {
        memcpy(&info, itr->second, sizeof(info));
        return true;
    }

    return false;
}

bool InfoMgr::GetCharInfo(std::string name, InfoCharEntry &info)
{
    ACE_Read_Guard<ACE_Thread_Mutex> g(m_charMutex);

    std::string tmpname = name;
    if (!normalizePlayerName(tmpname))
        return false;

    CharInfoNameItr itr = m_charInfosName.find(tmpname);
    if (itr != m_charInfosName.end())
    {
        memcpy(&info, itr->second, sizeof(info));
        return true;
    }

    return false;
}

void InfoMgr::RemoveCharInfo(uint32 guid)
{
    ACE_Guard<ACE_Thread_Mutex> g(m_charMutex);

    CharInfoItr itr = m_charInfos.find(guid);
    if (itr != m_charInfos.end())
    {
        _RemoveName(std::string(itr->second->Name));
        delete itr->second;
        m_charInfos.erase(itr);
    }
}

void InfoMgr::UpdateCharLevel(uint32 guid, uint8 level)
{
    ACE_Guard<ACE_Thread_Mutex> g(m_charMutex);

    CharInfoItr itr = m_charInfos.find(guid);
    if (itr != m_charInfos.end())
        itr->second->Level = level;
}

void InfoMgr::UpdateCharArenaTeam(uint32 guid, uint32 team, uint8 slot)
{
    ACE_Guard<ACE_Thread_Mutex> g(m_charMutex);

    ASSERT(slot < MAX_ARENA_SLOT);

    CharInfoItr itr = m_charInfos.find(guid);
    if (itr != m_charInfos.end())
        itr->second->ArenaTeam[slot] = team;
}

void InfoMgr::UpdateCharMMR(uint32 guid, uint32 mmr, uint8 slot)
{
    ACE_Guard<ACE_Thread_Mutex> g(m_charMutex);

    ASSERT(slot < MAX_ARENA_SLOT);

    CharInfoItr itr = m_charInfos.find(guid);
    if (itr != m_charInfos.end())
        itr->second->MMR[slot] = mmr;
}

void InfoMgr::UpdateCharGroup(uint32 guid, uint32 group)
{
    ACE_Guard<ACE_Thread_Mutex> g(m_charMutex);

    CharInfoItr itr = m_charInfos.find(guid);
    if (itr != m_charInfos.end())
        itr->second->Group = group;
}

void InfoMgr::UpdateCharGuild(uint32 guid, uint32 guild)
{
    ACE_Guard<ACE_Thread_Mutex> g(m_charMutex);

    CharInfoItr itr = m_charInfos.find(guid);
    if (itr != m_charInfos.end())
        itr->second->Guild = guild;
}

void InfoMgr::UpdateCharBase(uint32 guid, std::string name, uint8 gender, uint8 race, uint8 cclass, uint32 account, uint8 level, uint16 zone, uint8 XPfactor)
{
    ACE_Guard<ACE_Thread_Mutex> g(m_charMutex);

    CharInfoItr itr = m_charInfos.find(guid);
    if (itr != m_charInfos.end())
    {
        itr->second->Guid = guid;
        if (gender != GENDER_NONE)
            itr->second->Gender = gender;
        if (race)
            itr->second->Race = race;
        if (cclass)
            itr->second->Class = cclass;
        if (account)
            itr->second->Account = account;
        if (level)
            itr->second->Level = level;
        if (zone)
            itr->second->Zone = zone;

        if (memcmp(itr->second->Name, name.c_str(), name.size()))
        {
            _RemoveName(std::string(itr->second->Name));
            _AddName(name, itr->second);
        }

        memset(itr->second->Name, 0, MAX_INFOCHAR_NAME_LENGTH);
        memcpy(itr->second->Name, name.c_str(), name.size());
    }
    else
    {
        InfoCharEntry *info = new InfoCharEntry();
        info->Guid = guid;
        if (account)
            info->Account = account;
        if (gender != GENDER_NONE)
            info->Gender = gender;
        if (race)
            info->Race = race;
        if (cclass)
            info->Class = cclass;
        if (zone)
            info->Zone = zone;
        if (level)
            info->Level = level;
        
        memset(info->Name, 0, MAX_INFOCHAR_NAME_LENGTH);
        memcpy(info->Name, name.c_str(), name.size());

        _AddName(name, info);

        m_charInfos[guid] = info;
    }
}

void InfoMgr::_AddName(std::string name, InfoCharEntry *info)
{
    m_charInfosName[name] = info;
}

void InfoMgr::_RemoveName(std::string name)
{
    CharInfoNameItr itr = m_charInfosName.find(name);
    if (itr != m_charInfosName.end())
        m_charInfosName.erase(itr);
}

void InfoMgr::IncreaseAccountCharCount(uint32 id)
{
    ACE_Write_Guard<ACE_Thread_Mutex> g(m_accountMutex);

    ++m_accountCharCounts[id];
}

void InfoMgr::DecreaseAccountCharCount(uint32 id)
{
    ACE_Guard<ACE_Thread_Mutex> g(m_accountMutex);

    AccountCharCountMapItr itr = m_accountCharCounts.find(id);
    if (itr != m_accountCharCounts.end() && itr->second > 0)
        --itr->second;
}

uint8 InfoMgr::GetAccountCharCount(uint32 id)
{
    ACE_Read_Guard<ACE_Thread_Mutex> g(m_accountMutex);

    AccountCharCountMapItr itr = m_accountCharCounts.find(id);
    if (itr != m_accountCharCounts.end())
        return itr->second;

    return 0;
}

void InfoMgr::SetAccountCharCount(uint32 id, uint8 count)
{
    ACE_Write_Guard<ACE_Thread_Mutex> g(m_accountMutex);

    m_accountCharCounts[id] = count;
}

/*void InfoMgr::UpdatePet(uint32 id, uint32 entry, uint32 ownerId, uint32 modelId, uint32 createdBySpell, uint32 type, uint8 reacteState, std::string name, uint8 renamed, uint8 slot,
                        uint32 health, uint32 mana, uint32 happiness, uint32 saveTime, std::string actionbars, uint8 level, uint32 experience)
{
    ACE_Guard<ACE_Thread_Mutex> g(petsMutex);

    PetsMapItr itr = petsMap.find(id);
    if (itr == petsMap.end())
    {
        SavedPet* pet = new SavedPet();

        pet->id = id;
        pet->entry = entry;
        pet->owner = ownerId;
        pet->model = modelId;
        pet->createdBySpell = createdBySpell;
        pet->type = type;
        pet->reactState = reacteState;
        pet->name = name;
        pet->renamed = renamed;
        pet->slot = slot;
        pet->health = health;
        pet->mana = mana;
        pet->happiness = happiness;
        pet->saveTime = saveTime;
        pet->actionbars = actionbars;
        pet->level = level;
        pet->experience = experience;

        petsMap.insert(itr, std::pair<uint32, SavedPet*>(id, pet));

        PetsToOwnerMapItr it = petsToOwner.find(ownerId);

        if (it != petsToOwner.end())
        {
            it->second->insert(pet);
        }
        else
        {
            PetsSet* petsSet = new PetsSet();
            petsSet->insert(pet);
            petsToOwner[ownerId] = petsSet;
        }
    }
    else
    {
        // fuer spaeter
    }
}

void InfoMgr::UpdatePetAuras(uint32 guid, uint64 casterGuid, uint32 spellId, uint8 effectMask, uint8 recalcMask, uint8 stacks,
                             int32 amount_0, int32 amount_1, int32 amount_2, int32 baseAmount_0, int32 baseAmount_1, int32 baseAmount_2,
                             int32 maxDuration, int32 remDuration, uint8 charges)
{
    ACE_Guard<ACE_Thread_Mutex> g(petsMutex);

    PetsMapItr itr = petsMap.find(guid);
    if (itr != petsMap.end())
    {
        if (SavedPet* pet = itr->second)
        {
            SavedPetAura *aura = new SavedPetAura();
            aura->caster = casterGuid;
            aura->spell = spellId;
            aura->effectMask = effectMask;
            aura->recalcMask = recalcMask;
            aura->stacks = stacks;
            aura->amount[0] = amount_0;
            aura->amount[1] = amount_1;
            aura->amount[2] = amount_2;
            aura->baseAmount[0] = baseAmount_0;
            aura->baseAmount[1] = baseAmount_1;
            aura->baseAmount[2] = baseAmount_2;
            aura->maxDuration = maxDuration;
            aura->remDuration = remDuration;
            aura->charges = charges;

            pet->m_auras.push_back(aura);
        }
    }
}

void InfoMgr::UpdatePetSpells(uint32 guid, uint32 spellId, uint8 active)
{
    ACE_Guard<ACE_Thread_Mutex> g(petsMutex);

    PetsMapItr itr = petsMap.find(guid);
    if (itr != petsMap.end())
    {
        if (SavedPet* pet = itr->second)
        {
            SavedPetSpell *spell = new SavedPetSpell;
            spell->state = PETSPELL_UNCHANGED;
            spell->active = ActiveStates(active);

            pet->m_spells[spellId] = spell;
        }
    }
}

void InfoMgr::UpdatePetSpellCooldowns(uint32 guid, uint32 spellId, uint32 time)
{
    ACE_Guard<ACE_Thread_Mutex> g(petsMutex);

    PetsMapItr itr = petsMap.find(guid);
    if (itr != petsMap.end())
    {
        if (SavedPet* pet = itr->second)
        {
            pet->m_cooldowns[spellId] = time;
        }
    }
}

void InfoMgr::FillPetList(uint32 ownerId, SavedPetList &petList)
{
    ACE_Guard<ACE_Thread_Mutex> g(petsMutex);

    for (SavedPetList::iterator itr = petList.begin(); itr != petList.end(); ++itr)
    {
        delete *itr;
    }

    petList.clear();

    PetsToOwnerMapItr it = petsToOwner.find(ownerId);

    if (it != petsToOwner.end())
    {
        PetsSet* petsSet = it->second;
        petList.clear();
        for (PetsSetItr itr = petsSet->begin(); itr != petsSet->end(); ++itr)
        {
            SavedPet* pet = new SavedPet(**itr);
            petList.push_back(pet);
        }
    }
}

void InfoMgr::SavePet(SavedPet* pet, Player* owner)
{
    ACE_Guard<ACE_Thread_Mutex> g(petsMutex);

    SavedPet* petCopy = NULL;
    uint32 ownerId = owner->GetGUIDLow();

    PetsToOwnerMapItr lb = petsToOwner.lower_bound(ownerId);

    if (lb != petsToOwner.end() && !(petsToOwner.key_comp()(ownerId, lb->first)))
    {
        PetsSet* petsSet = lb->second;
        PetsSetItr itr = std::find_if(petsSet->begin(), petsSet->end(), PetsCompareByPointer(pet));
        if (itr != petsSet->end())
        {
            delete *itr;
            petsSet->erase(itr);
        }
        petCopy = new SavedPet(*pet);
        petsSet->insert(petCopy);
    }
    else
    {
        petCopy = new SavedPet(*pet);
        PetsSet* petsSet = new PetsSet();
        petsSet->insert(petCopy);
        petsToOwner.insert(lb, PetsToOwnerMap::value_type(ownerId, petsSet));
    }

    // remove comments if you want to use petsMap, f.i. find SavedPet by Guid
    //PetsMapItr lbPM = petsMap.lower_bound(pet->id);

    //if(lbPM != petsMap.end() && !(petsMap.key_comp()(pet->id, lbPM->first)))
    //{
    //    lbPM->second = petCopy;
    //}
    //else
    //{
    //    petsMap.insert(lbPM, PetsMap::value_type(pet->id, petCopy));
    //}
}

void InfoMgr::DeletePet(SavedPet* pet, Player* owner)
{
    ACE_Guard<ACE_Thread_Mutex> g(petsMutex);

    uint32 ownerId = owner->GetGUIDLow();

    PetsToOwnerMapItr lb = petsToOwner.lower_bound(ownerId);

    if (lb != petsToOwner.end() && !(petsToOwner.key_comp()(ownerId, lb->first)))
    {
        PetsSet* petsSet = lb->second;
        PetsSetItr itr = std::find_if(petsSet->begin(), petsSet->end(), PetsCompareByPointer(pet));
        if (itr != petsSet->end())
        {
            delete *itr;
            petsSet->erase(itr);
        }
        else
            sLog->outError(LOG_FILTER_UNITS, "InfoMgr::DeletePet, Try to delete pet %u where pets owner %u has no pet with this id in petsset ", pet->id, ownerId);
    }
    else
    {
       sLog->outError(LOG_FILTER_UNITS, "InfoMgr::DeletePet, Try to delete pet %u where pets owner %u has no petsset ", pet->id, ownerId);
    }

    // remove comments if you want to use petsMap, f.i. find SavedPet by Guid
    //PetsMapItr lbPM = petsMap.lower_bound(pet->id);

    //if(lbPM != petsMap.end() && !(petsMap.key_comp()(pet->id, lbPM->first)))
    //{
    //    petsMap.erase(lbPM);
    //}
    //else
    //{
    //    sLog->outError("InfoMgr::DeletePet %u pet is not in petsMap", pet->id);
    //}
}

void InfoMgr::DeleteAllPetsFromOwner(uint32 ownerGuid)
{
    ACE_Guard<ACE_Thread_Mutex> g(petsMutex);

    PetsToOwnerMapItr lb = petsToOwner.lower_bound(ownerGuid);

    if(lb != petsToOwner.end() && !(petsToOwner.key_comp()(ownerGuid, lb->first)))
    {

        PetsSet* petsSet = lb->second;
        for (PetsSetItr it = petsSet->begin(); it != petsSet->end(); ++it)
        {
            // remove comments if you want to use petsMap, f.i. find SavedPet by Guid
            //SavedPet* pet = *it;
            //PetsMapItr lbPM = petsMap.lower_bound(pet->id);

            //if(lbPM != petsMap.end() && !(petsMap.key_comp()(pet->id, lbPM->first)))
            //{
            //    petsMap.erase(lbPM);
            //}
            //else
            //{
            //    sLog->outError("InfoMgr::DeletePet %u pet is not in petsMap", pet->id);
            //}
            delete *it;
        }

        petsSet->clear();
        delete lb->second;
        petsToOwner.erase(lb);
    }
    //else
    //{
    //    sLog->outError(LOG_FILTER_UNITS, "InfoMgr::DeleteAllPetFromOwner, Try to delete pets where owner %u has no pets ", ownerGuid);
    //}
}

InfoMgr::~InfoMgr()
{
    //sLog->outInfo(LOG_FILTER_SERVER_LOADING, "InfoMgr::~InfoMgr()");
    for (PetsToOwnerMapItr itr = petsToOwner.begin(); itr != petsToOwner.end(); ++itr)
    {
        PetsSet* petsSet = itr->second;
        for (PetsSetItr it = petsSet->begin(); it != petsSet->end(); ++it)
        {
            delete *it;
        }
        petsSet->clear();
        delete itr->second;
    }

    petsToOwner.clear();

    // remove comments if you want to use petsMap, f.i. find SavedPet by Guid
    //for (PetsMapItr itr = petsMap.begin(); itr != petsMap.end(); ++itr)
    //    delete itr->second;

   // petsMap.clear();
}*/

void InfoMgr::UnloadAll()
{
    sLog->outInfo(LOG_FILTER_SERVER_LOADING, "InfoMgr::UnloadAll()");

    for (CharInfoItr charItr = m_charInfos.begin(); charItr != m_charInfos.end(); ++charItr)
    {
        delete charItr->second;
    }
    m_charInfos.clear();
    m_charInfosName.clear();

    /*for (PetsToOwnerMapItr itr = petsToOwner.begin(); itr != petsToOwner.end(); ++itr)
    {
        PetsSet* petsSet = itr->second;
        for (PetsSetItr it = petsSet->begin(); it != petsSet->end(); ++it)
        {
            delete *it;
        }
        petsSet->clear();
        delete itr->second;
    }

    petsToOwner.clear();*/

    // remove comments if you want to use petsMap, f.i. find SavedPet by Guid
    //for (PetsMapItr itr = petsMap.begin(); itr != petsMap.end(); ++itr)
    //    delete itr->second;

    // petsMap.clear();
}
