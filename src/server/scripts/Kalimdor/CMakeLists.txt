# Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

file(GLOB_RECURSE scripts_BlackfathomDeeps Kalimdor/BlackfathomDeeps/*.cpp Kalimdor/BlackfathomDeeps/*.h)
file(GLOB_RECURSE scripts_CavernsOfTime_BattleForMountHyjal Kalimdor/CavernsOfTime/BattleForMountHyjal/*.cpp Kalimdor/CavernsOfTime/BattleForMountHyjal/*.h)
file(GLOB_RECURSE scripts_CavernsOfTime_CullingOfStratholme Kalimdor/CavernsOfTime/CullingOfStratholme/*.cpp Kalimdor/CavernsOfTime/CullingOfStratholme/*.h)
file(GLOB_RECURSE scripts_CavernsOfTime_DarkPortal Kalimdor/CavernsOfTime/DarkPortal/*.cpp Kalimdor/CavernsOfTime/DarkPortal/*.h)
file(GLOB_RECURSE scripts_CavernsOfTime_DragonSoul Kalimdor/CavernsOfTime/DragonSoul/*.cpp Kalimdor/CavernsOfTime/DragonSoul/*.h)
file(GLOB_RECURSE scripts_CavernsOfTime_EndTime Kalimdor/CavernsOfTime/EndTime/*.cpp Kalimdor/CavernsOfTime/EndTime/*.h)
file(GLOB_RECURSE scripts_CavernsOfTime_EscapeFromDurnholdeKeep Kalimdor/CavernsOfTime/EscapeFromDurnholdeKeep/*.cpp Kalimdor/CavernsOfTime/EscapeFromDurnholdeKeep/*.h)
file(GLOB_RECURSE scripts_CavernsOfTime_HourOfTwilight Kalimdor/CavernsOfTime/HourOfTwilight/*.cpp Kalimdor/CavernsOfTime/HourOfTwilight/*.h)
file(GLOB_RECURSE scripts_CavernsOfTime_WellOfEternity Kalimdor/CavernsOfTime/WellOfEternity/*.cpp Kalimdor/CavernsOfTime/WellOfEternity/*.h)
file(GLOB_RECURSE scripts_Firelands Kalimdor/Firelands/*.cpp Kalimdor/Firelands/*.h)
file(GLOB_RECURSE scripts_HallsOfOrigination Kalimdor/HallsOfOrigination/*.cpp Kalimdor/HallsOfOrigination/*.h)
file(GLOB_RECURSE scripts_LostCityOfTheTolvir Kalimdor/LostCityOfTheTolvir/*.cpp Kalimdor/LostCityOfTheTolvir/*.h)
file(GLOB_RECURSE scripts_Maraudon Kalimdor/Maraudon/*.cpp Kalimdor/Maraudon/*.h)
file(GLOB_RECURSE scripts_OnyxiasLair Kalimdor/OnyxiasLair/*.cpp Kalimdor/OnyxiasLair/*.h)
file(GLOB_RECURSE scripts_RazorfenDowns Kalimdor/RazorfenDowns/*.cpp Kalimdor/RazorfenDowns/*.h)
file(GLOB_RECURSE scripts_RazorfenKraul Kalimdor/RazorfenKraul/*.cpp Kalimdor/RazorfenKraul/*.h)
file(GLOB_RECURSE scripts_RuinsOfAhnQiraj Kalimdor/RuinsOfAhnQiraj/*.cpp Kalimdor/RuinsOfAhnQiraj/*.h)
file(GLOB_RECURSE scripts_TempleOfAhnQiraj Kalimdor/TempleOfAhnQiraj/*.cpp Kalimdor/TempleOfAhnQiraj/*.h)
file(GLOB_RECURSE scripts_ThroneOfTheFourWinds Kalimdor/ThroneOfTheFourWinds/*.cpp Kalimdor/ThroneOfTheFourWinds/*.h)
file(GLOB_RECURSE scripts_TheVortexPinnacle Kalimdor/TheVortexPinnacle/*.cpp Kalimdor/TheVortexPinnacle/*.h)
file(GLOB_RECURSE scripts_WailingCaverns Kalimdor/WailingCaverns/*.cpp Kalimdor/WailingCaverns/*.h)
file(GLOB_RECURSE scripts_ZulFarrak Kalimdor/ZulFarrak/*.cpp Kalimdor/ZulFarrak/*.h)
file(GLOB_RECURSE scripts_Zones Kalimdor/*.cpp Kalimdor/*.h)

source_group(Kalimdor\\BlackfathomDeeps FILES ${scripts_BlackfathomDeeps})
source_group(CavernsOfTime\\BattleForMountHyjal FILES ${scripts_CavernsOfTime_BattleForMountHyjal})
source_group(CavernsOfTime\\CullingOfStratholme FILES ${scripts_CavernsOfTime_CullingOfStratholme})
source_group(CavernsOfTime\\DarkPortal FILES ${scripts_CavernsOfTime_DarkPortal})
source_group(CavernsOfTime\\DragonSoul FILES ${scripts_CavernsOfTime_DragonSoul})
source_group(CavernsOfTime\\EndTime FILES ${scripts_CavernsOfTime_EndTime})
source_group(CavernsOfTime\\EscapeFromDurnholdeKeep FILES ${scripts_CavernsOfTime_EscapeFromDurnholdeKeep})
source_group(CavernsOfTime\\HourOfTwilight FILES ${scripts_CavernsOfTime_HourOfTwilight})
source_group(CavernsOfTime\\WellOfEternity FILES ${scripts_CavernsOfTime_WellOfEternity})
source_group(Kalimdor\\Firelands FILES ${scripts_Firelands})
source_group(Kalimdor\\HallsOfOrigination FILES ${scripts_HallsOfOrigination})
source_group(Kalimdor\\LostCityOfTheTolvir FILES ${scripts_LostCityOfTheTolvir})
source_group(Kalimdor\\Maraudon FILES ${scripts_Maraudon})
source_group(Kalimdor\\OnyxiasLair FILES ${scripts_OnyxiasLair})
source_group(Kalimdor\\RazorfenDowns FILES ${scripts_RazorfenDowns})
source_group(Kalimdor\\RazorfenKraul FILES ${scripts_RazorfenKraul})
source_group(Kalimdor\\RuinsOfAhnQiraj FILES ${scripts_RuinsOfAhnQiraj})
source_group(Kalimdor\\TempleOfAhnQiraj FILES ${scripts_TempleOfAhnQiraj})
source_group(Kalimdor\\ThroneOfTheFourWinds FILES ${scripts_ThroneOfTheFourWinds})
source_group(Kalimdor\\TheVortexPinnacle FILES ${scripts_VortexPinnacle})
source_group(Kalimdor\\WailingCaverns FILES ${scripts_WailingCaverns})
source_group(Kalimdor\\ZulFarrak FILES ${scripts_ZulFarrak})
source_group(Kalimdor\\Zones FILES ${scripts_Zones})

set(scripts_STAT_SRCS
  ${scripts_STAT_SRCS}
  ${scripts_BlackfathomDeeps}
  ${scripts_CavernsOfTime_BattleForMountHyjal}
  ${scripts_CavernsOfTime_CullingOfStratholme}
  ${scripts_CavernsOfTime_DarkPortal}
  ${scripts_CavernsOfTime_EscapeFromDurnholdeKeep}
  ${scripts_Firelands}
  ${scripts_HallsOfOrigination}
  ${scripts_LostCityOfTheTolvir}
  ${scripts_Maraudon}
  ${scripts_OnyxiasLair}
  ${scripts_RazorfenDowns}
  ${scripts_RazorfenKraul}
  ${scripts_RuinsOfAhnQiraj}
  ${scripts_TempleOfAhnQiraj}
  ${scripts_ThroneOfTheFourWinds}
  ${scripts_VortexPinnacle}
  ${scripts_WailingCaverns}
  ${scripts_ZulFarrak}
  ${scripts_Zones}
)

message("  -> Prepared: Kalimdor")
