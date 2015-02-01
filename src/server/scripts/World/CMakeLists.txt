# Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

file(GLOB_RECURSE scripts_World World/*.cpp World/*.h)
source_group(World FILES ${scripts_World})

set(scripts_STAT_SRCS
  ${scripts_STAT_SRCS}
  ${scripts_World}
   World/boss_garr_hyjal.cpp
   World/boss_akmahat.cpp
   World/boss_mobus.cpp
   World/boss_julak_doom.cpp
)

message("  -> Prepared: World")