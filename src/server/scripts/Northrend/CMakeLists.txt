# Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

file(GLOB_RECURSE scripts_AzjolNerub_Ahnkahet Northrend/AzjolNerub/Ahnkahet/*.cpp Northrend/AzjolNerub/Ahnkahet/*.h)
file(GLOB_RECURSE scripts_AzjolNerub_AzjolNerub Northrend/AzjolNerub/AzjolNerub/*.cpp Northrend/AzjolNerub/AzjolNerub/*.h)
file(GLOB_RECURSE scripts_ChamberOfAspects_ObsidianSanctum Northrend/ChamberOfAspects/ObsidianSanctum/*.cpp Northrend/ChamberOfAspects/ObsidianSanctum/*.h)
file(GLOB_RECURSE scripts_ChamberOfAspects_RubySanctum Northrend/ChamberOfAspects/RubySanctum/*.cpp Northrend/ChamberOfAspects/RubySanctum/*.h)
file(GLOB_RECURSE scripts_CrusadersColiseum_TrialOfTheChampion Northrend/CrusadersColiseum/TrialOfTheChampion/*.cpp Northrend/CrusadersColiseum/TrialOfTheChampion/*.h)
file(GLOB_RECURSE scripts_CrusadersColiseum_TrialOfTheCrusader Northrend/CrusadersColiseum/TrialOfTheCrusader/*.cpp Northrend/CrusadersColiseum/TrialOfTheCrusader/*.h)
file(GLOB_RECURSE scripts_DraktharonKeep Northrend/DraktharonKeep/*.cpp Northrend/DraktharonKeep/*.h)
file(GLOB_RECURSE scripts_FrozenHalls_ForgeOfSouls Northrend/FrozenHalls/ForgeOfSouls/*.cpp Northrend/FrozenHalls/ForgeOfSouls/*.h)
file(GLOB_RECURSE scripts_FrozenHalls_HallsOfReflection Northrend/FrozenHalls/HallsOfReflection/*.cpp Northrend/FrozenHalls/HallsOfReflection/*.h)
file(GLOB_RECURSE scripts_FrozenHalls_PitOfSaron Northrend/FrozenHalls/PitOfSaron/*.cpp Northrend/FrozenHalls/PitOfSaron/*.h)
file(GLOB_RECURSE scripts_Gundrak Northrend/Gundrak/*.cpp Northrend/Gundrak/*.h)
file(GLOB_RECURSE scripts_IcecrownCitadel Northrend/IcecrownCitadel/*.cpp Northrend/IcecrownCitadel/*.h)
file(GLOB_RECURSE scripts_Naxxramas Northrend/Naxxramas/*.cpp Northrend/Naxxramas/*.h)
file(GLOB_RECURSE scripts_Nexus_EyeOfEternity Northrend/Nexus/EyeOfEternity/*.cpp Northrend/Nexus/EyeOfEternity/*.h)
file(GLOB_RECURSE scripts_Nexus_Nexus Northrend/Nexus/Nexus/*.cpp Northrend/Nexus/Nexus/*.h)
file(GLOB_RECURSE scripts_Nexus_Oculus Northrend/Nexus/Oculus/*.cpp Northrend/Nexus/Oculus/*.h)
file(GLOB_RECURSE scripts_Ulduar_HallsOfLightning Northrend/Ulduar/HallsOfLightning/*.cpp Northrend/Ulduar/HallsOfLightning/*.h)
file(GLOB_RECURSE scripts_Ulduar_HallsOfStone Northrend/Ulduar/HallsOfStone/*.cpp Northrend/Ulduar/HallsOfStone/*.h)
file(GLOB_RECURSE scripts_Ulduar_Ulduar Northrend/Ulduar/Ulduar/*.cpp Northrend/Ulduar/Ulduar/*.h)
file(GLOB_RECURSE scripts_UtgardeKeep_UtgardeKeep Northrend/UtgardeKeep/UtgardeKeep/*.cpp Northrend/UtgardeKeep/UtgardeKeep/*.h)
file(GLOB_RECURSE scripts_UtgardeKeep_UtgardePinnacle Northrend/UtgardeKeep/UtgardePinnacle/*.cpp Northrend/UtgardeKeep/UtgardePinnacle/*.h)
file(GLOB_RECURSE scripts_VaultOfArchavon Northrend/VaultOfArchavon/*.cpp Northrend/VaultOfArchavon/*.h)
file(GLOB_RECURSE scripts_VioletHold Northrend/VioletHold/*.cpp Northrend/VioletHold/*.h)
file(GLOB_RECURSE scripts_Zones Northrend/*.cpp Northrend/*.h)

source_group(Northrend\\AzjolNerub\\Ahnkahet FILES ${scripts_AzjolNerub_Ahnkahet})
source_group(Northrend\\AzjolNerub\\AzjolNerub FILES ${scripts_AzjolNerub_AzjolNerub})
source_group(Northrend\\ChamberOfAspects\\ObsidianSanctum FILES ${scripts_ChamberOfAspects_ObsidianSanctum})
source_group(Northrend\\ChamberOfAspects\\RubySanctum FILES ${scripts_ChamberOfAspects_RubySanctum})
source_group(Northrend\\CrusadersColiseum\\TrialOfTheChampion FILES ${scripts_CrusadersColiseum_TrialOfTheChampion})
source_group(Northrend\\CrusadersColiseum\\TrialOfTheCrusader FILES ${scripts_CrusadersColiseum_TrialOfTheCrusader})
source_group(Northrend\\DraktharonKeep FILES ${scripts_DraktharonKeep})
source_group(Northrend\\FrozenHalls\\ForgeOfSouls FILES ${scripts_FrozenHalls_ForgeOfSouls})
source_group(Northrend\\FrozenHalls\\HallsOfReflection FILES ${scripts_FrozenHalls_HallsOfReflection})
source_group(Northrend\\FrozenHalls\\PitOfSaron FILES ${scripts_FrozenHalls_PitOfSaron})
source_group(Northrend\\Gundrak FILES ${scripts_Gundrak})
source_group(Northrend\\IcecrownCitadel FILES ${scripts_IcecrownCitadel})
source_group(Northrend\\Naxxramas FILES ${scripts_Naxxramas})
source_group(Northrend\\Nexus\\EyeOfEternity FILES ${scripts_Nexus_EyeOfEternity})
source_group(Northrend\\Nexus\\Nexus FILES ${scripts_Nexus_Nexus})
source_group(Northrend\\Nexus\\Oculus FILES ${scripts_Nexus_Oculus})
source_group(Northrend\\Ulduar\\HallsOfLightning FILES ${scripts_Ulduar_HallsOfLightning})
source_group(Northrend\\Ulduar\\HallsOfStone FILES ${scripts_Ulduar_HallsOfStone})
source_group(Northrend\\Ulduar\\Ulduar FILES ${scripts_Ulduar_Ulduar})
source_group(Northrend\\UtgardeKeep\\UtgardeKeep FILES ${scripts_UtgardeKeep_UtgardeKeep})
source_group(Northrend\\UtgardeKeep\\UtgardePinnacle FILES ${scripts_UtgardeKeep_UtgardePinnacle})
source_group(Northrend\\VaultOfArchavon FILES ${scripts_VaultOfArchavon})
source_group(Northrend\\VioletHold FILES ${scripts_VioletHold})
source_group(Northrend\\Northrend FILES ${scripts_Northrend})
source_group(Northrend\\Zones FILES ${scripts_Zones})


set(scripts_STAT_SRCS
  ${scripts_STAT_SRCS}
  ${scripts_AzjolNerub_Ahnkahet}
  ${scripts_AzjolNerub_AzjolNerub}
  ${scripts_ChamberOfAspects_ObsidianSanctum}
  ${scripts_ChamberOfAspects_RubySanctum}
  ${scripts_CrusadersColiseum_TrialOfTheChampion}
  ${scripts_CrusadersColiseum_TrialOfTheCrusader}
  ${scripts_DraktharonKeep}
  ${scripts_FrozenHalls_ForgeOfSouls}
  ${scripts_FrozenHalls_HallsOfReflection}
  ${scripts_FrozenHalls_PitOfSaron}
  ${scripts_Gundrak}
  ${scripts_IcecrownCitadel}
  ${scripts_Naxxramas}
  ${scripts_Nexus_EyeOfEternity}
  ${scripts_Nexus_Nexus}
  ${scripts_Nexus_Oculus}
  ${scripts_Ulduar_HallsOfLightning}
  ${scripts_Ulduar_HallsOfStone}
  ${scripts_Ulduar_Ulduar}
  ${scripts_UtgardeKeep_UtgardeKeep}
  ${scripts_UtgardeKeep_UtgardePinnacle}
  ${scripts_VaultOfArchavon}
  ${scripts_VioletHold}
  ${scripts_Wintergrasp}
  ${scripts_Zones}
)

message("  -> Prepared: Northrend")
