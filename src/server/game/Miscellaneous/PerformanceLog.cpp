#include "PerformanceLog.h"
#include "World.h"

PerformanceEntry::PerformanceEntry(const char *name, uint8 lengthID)
{
    strcpy(m_name, name);
    m_id = lengthID;
    
    m_totalTime = 0;
    m_lastTime = getMSTime();
}

void PerformanceEntry::LogTime(const char* name)
{
    uint32 time = getMSTime();
    uint32 diff = getMSTimeDiff(m_lastTime, time);
    m_times << name << "=" << diff << " ";
    m_totalTime += diff;
    m_lastTime = time;
}

void PerformanceEntry::AddInfo(const char* format, ...)
{
    char tmp[255];
    va_list ap;
    va_start(ap, format);
    vsprintf(tmp, format, ap);
    va_end(ap);
    m_info << "[" << tmp << "] ";
}

void PerformanceLog::Add(PerformanceEntry *pe)
{
    if (pe->GetTypeId() < MAX_PERFORMANCELOG_TYPEID)
    {
        if (m_config[pe->GetTypeId()] <= pe->m_totalTime)
        {
            WorldDatabase.PExecute(
                "INSERT INTO performance_all (time, type, totalLength, times, info) VALUES (UNIX_TIMESTAMP(), '%s', %u, '%s', '%s')",
                pe->GetType(), pe->m_totalTime, pe->m_times.str().c_str(), pe->m_info.str().c_str()
                );
        }
    }
    else
        sLog->outError(LOG_FILTER_UNITS, "PERFORMANCELOG: Invalid ID for class (%s)", pe->GetType());
}

void PerformanceLog::LoadConfig()
{
    memset(m_config, 0xFF, sizeof(m_config)); // value = 0xFFFFFFFF by default (never log)

    if (QueryResult result = WorldDatabase.Query("SELECT id, minLength FROM performance_config"))
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 id = fields[0].GetUInt32();
            if (id < MAX_PERFORMANCELOG_TYPEID)
                m_config[id] = fields[1].GetUInt32();
        } while (result->NextRow());
    }
}

void PerformanceLog::Initialize()
{
    m_worldSum = m_worldCount = m_worldMax = 0;
    LoadConfig();
}

void PerformanceLog::Update(uint32 diff)
{
    if (diff > m_worldMax)
        m_worldMax = diff;

    ++m_worldCount;
    m_worldSum += diff;

    if (m_worldSum >= MINUTE * IN_MILLISECONDS)
    {
        WorldDatabase.PExecute(
            "INSERT INTO performance_world (time, average, max, players) VALUES (UNIX_TIMESTAMP(), %u, %u, %u)",
            m_worldSum / m_worldCount, m_worldMax, sWorld->GetPlayerCount()
            );

        m_worldMax = m_worldSum = m_worldCount = 0;
    }
}
