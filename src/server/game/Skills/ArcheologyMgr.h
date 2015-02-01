#ifndef __TRINITY_ARCHEOLOGY_MGR_H
#define __TRINITY_ARCHEOLOGY_MGR_H

#include <map>
#include <string>
#include <vector>
#include <ace/Singleton.h>
#include "Common.h"
#include "DatabaseEnv.h"
#include "Player.h"
#include "DBCEnums.h"
#include "DBCStores.h"
#include "WorldSession.h"
#include "SharedDefines.h"
#include "Language.h"
#include "DBCStructure.h"
#include "QueryResult.h"
#include "GameObject.h"




enum DigSiteDistance
{
    ARCHAEOLOGY_DIG_SITE_RADIUS            = 60,
    ARCHAEOLOGY_DIG_SITE_FAR_DIST          = 25,
    ARCHAEOLOGY_DIG_SITE_MED_DIST          = 10,
    ARCHAEOLOGY_DIG_SITE_CLOSE_DIST        = 5,
    ARCHAEOLOGY_DIG_SITE_REWARD_PLAYER     = 0,
    ARCHAEOLOGY_DIG_SITE_REWARD_AMMT_MAX   = 7
};

enum DigSiteSurveyBot
{
    ARCHAEOLOGY_DIG_SITE_FAR_SURVEYBOT     = 206590,
    ARCHAEOLOGY_DIG_SITE_MEDIUM_SURVEYBOT  = 206589,
    ARCHAEOLOGY_DIG_SITE_CLOSE_SURVEYBOT   = 204272
};

enum TreasuresEntry
{
    ARCHAEOLOGY_TROLL_TREASURE             = 202655,
    ARCHAEOLOGY_DWARF_TREASURE             = 204282,
    ARCHAEOLOGY_NIGHTELF_TREASURE          = 203071,
    ARCHAEOLOGY_NERUBIAN_TREASURE          = 203078,
    ARCHAEOLOGY_FOSSIL_TREASURE            = 206836,
    ARCHAEOLOGY_ORC_TREASURE               = 207187,
    ARCHAEOLOGY_DRAENEI_TREASURE           = 207188,
    ARCHAEOLOGY_VRYKUL_TREASURE            = 207189,
    ARCHAEOLOGY_TOLVIR_TREASURE            = 207190
};

enum DigSiteZones
{
    ARCHEOLOGY_ZONE_SOUTHER_BARRENS        = 4709,
    ARCHEOLOGY_ZONE_TWILIGHT_HIGHLAND      = 4922,
    ARCHEOLOGY_ZONE_BADLANDS               = 3,
    ARCHEOLOGY_ZONE_SEARING_GORGE          = 51,
    ARCHEOLOGY_ZONE_LOCH_MODAN             = 38,
    ARCHEOLOGY_ZONE_HILLSBARD_FOOTHILLS    = 267,
    ARCHEOLOGY_ZONE_BURNING_STEPPES        = 46,
    ARCHEOLOGY_ZONE_WETLANDS               = 11,
    ARCHEOLOGY_ZONE_ARATHI_HIGHLAND        = 45,
    ARCHEOLOGY_ZONE_HINTERLANDS            = 47,
    ARCHEOLOGY_ZONE_WEASTERN_PLAGUELANDS   = 28,
    ARCHEOLOGY_ZONE_BLASTED_LANDS          = 4,
    ARCHEOLOGY_ZONE_REDRIGE_MOUNTAINS      = 44,
    ARCHEOLOGY_ZONE_DUSTWALLOW_MARSH       = 15,
    ARCHEOLOGY_ZONE_DUSKWOOD               = 10,
    ARCHEOLOGY_ZONE_DESOLACE               = 405,
    ARCHEOLOGY_ZONE_STONETALON_MOUNTAINS   = 406,
    ARCHEOLOGY_ZONE_EASTERN_PLAGUELANDS    = 139,
};

#define MAX_RESEARCH_SITES                 4


class Player;
class GameObject;

class ArcheologyMgr
{
    public:
        explicit ArcheologyMgr(Player* owner) : m_player(owner){}
        ~ArcheologyMgr() {}

        void GenerateResearchDigSites();
        void GenerateResearchProject(const uint32 & branchId, bool force, const uint32 & excludeId);
        void GenerateResearchDigSitesInMap(const uint32 & map, uint32 & slot);
        void SetActualDigSitePosition();
        void SpawnArchaeologyScope();
        void SetDigSiteInSlot(const uint32 & slot, const uint32 & site1, const uint32 & site2);
        void GenerateSavedArtifacts();
        void CompleteArtifact(const uint32 & artId, const uint32 & spellId, ByteBuffer & data);
        uint32 GetNewRandomSite(const uint32 & map);
        uint8 HasSavedDigSites();

        void LoadArcheologyDigSites(const uint32 & guid);
        void SaveArcheology();
        void SaveArcheologyDigSites();
        void SaveArcheologyArtifacts();

        void TrackRessources(const uint32 &auraId, bool apply);

        void CreateTreasureItem(const uint32 &spellId, int32& itemCount);

        uint8 getMaxDigsites();

    private:

        Player*    m_player;
        uint32     m_digSites[16];
        uint32     m_researchProject[28];
        Position   m_actualDigPos;
        uint8      m_doneDigSites;
};

#endif //__TRINITY_ARCHEOLOGY_MGR_H
