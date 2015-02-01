# Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

file(GLOB_RECURSE scripts_TheStonecore Maelstrom/TheStonecore/*.cpp Maelstrom/TheStonecore/*.h)
file(GLOB_RECURSE scripts_Zones Maelstrom/*.cpp Maelstrom/*.h)

source_group(Maelstrom\\TheStonecore FILES ${scripts_TheStonecore})
source_group(Maelstrom\\Zones FILES ${scripts_Zones})

set(scripts_STAT_SRCS
  ${scripts_STAT_SRCS}
  ${scripts_TheStonecore}
  ${scripts_Zones}
)

message("  -> Prepared: The Maelstrom")