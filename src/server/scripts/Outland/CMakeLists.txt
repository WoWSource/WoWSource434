# Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

file(GLOB_RECURSE scripts_Auchindoun Outland/Auchindoun/*.cpp Outland/Auchindoun/*.h)
file(GLOB_RECURSE scripts_BlackTemple Outland/BlackTemple/*.cpp Outland/BlackTemple/*.h)
file(GLOB_RECURSE scripts_CoilfangReservoir Outland/CoilfangReservoir/*.cpp Outland/CoilfangReservoir/*.h)
file(GLOB_RECURSE scripts_GruulsLair Outland/GruulsLair/*.cpp Outland/GruulsLair/*.h)
file(GLOB_RECURSE scripts_HellfireCitadel Outland/HellfireCitadel/*.cpp Outland/HellfireCitadel/*.h)
file(GLOB_RECURSE scripts_TempestKeep Outland/TempestKeep/*.cpp Outland/TempestKeep/*.h)
file(GLOB_RECURSE scripts_Zones Outland/*.cpp Outland/*.h)

source_group(Outland\\Auchindoun FILES ${scripts_Auchindoun})
source_group(Outland\\BlackTemple FILES ${scripts_BlackTemple})
source_group(Outland\\CoilfangReservoir FILES ${scripts_CoilfangReservoir})
source_group(Outland\\GruulsLair FILES ${scripts_GruulsLair})
source_group(Outland\\HellfireCitadel FILES ${scripts_HellfireCitadel})
source_group(Outland\\TempestKeep FILES ${scripts_TempestKeep})
source_group(Outland\\Zones FILES ${scripts_Zones})


set(scripts_STAT_SRCS
  ${scripts_STAT_SRCS}
  ${scripts_Auchindoun}
  ${scripts_BlackTemple}
  ${scripts_CoilfangReservoir}
  ${scripts_GruulsLair}
  ${scripts_HellfireCitadel}
  ${scripts_TempestKeep}
  ${scripts_Zones}
)

message("  -> Prepared: Outland")