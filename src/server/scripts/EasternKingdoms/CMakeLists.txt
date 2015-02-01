# Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

file(GLOB_RECURSE scripts_AlteracValley EasternKingdoms/AlteracValley/*.cpp EasternKingdoms/AlteracValley/*.h)
file(GLOB_RECURSE scripts_BaradinHold EasternKingdoms/BaradinHold/*.cpp EasternKingdoms/BaradinHold/*.h)
file(GLOB_RECURSE scripts_BastionOfTwilight EasternKingdoms/BastionOfTwilight/*.cpp EasternKingdoms/BastionOfTwilight/*.h)
file(GLOB_RECURSE scripts_BlackrockCaverns EasternKingdoms/BlackrockCaverns/*.cpp EasternKingdoms/BlackrockCaverns/*.h)
file(GLOB_RECURSE scripts_BlackrockDepths EasternKingdoms/BlackrockDepths/*.cpp EasternKingdoms/BlackrockDepths/*.h)
file(GLOB_RECURSE scripts_BlackrockSpire EasternKingdoms/BlackrockSpire/*.cpp EasternKingdoms/BlackrockSpire/*.h)
file(GLOB_RECURSE scripts_BlackwingDescent EasternKingdoms/BlackwingDescent/*.cpp EasternKingdoms/BlackwingDescent/*.h)
file(GLOB_RECURSE scripts_BlackwingLair EasternKingdoms/BlackwingLair/*.cpp EasternKingdoms/BlackwingLair/*.h)
file(GLOB_RECURSE scripts_Deadmines EasternKingdoms/Deadmines/*.cpp EasternKingdoms/Deadmines/*.h)
file(GLOB_RECURSE scripts_Gilneas EasternKingdoms/Gilneas/*.cpp EasternKingdoms/Gilneas/*.h)
file(GLOB_RECURSE scripts_Gnomeregan EasternKingdoms/Gnomeregan/*.cpp EasternKingdoms/Gnomeregan/*.h)
file(GLOB_RECURSE scripts_GrimBatol EasternKingdoms/GrimBatol/*.cpp EasternKingdoms/GrimBatol/*.h)
file(GLOB_RECURSE scripts_Karazhan EasternKingdoms/Karazhan/*.cpp EasternKingdoms/Karazhan/*.h)
file(GLOB_RECURSE scripts_MagistersTerrace EasternKingdoms/MagistersTerrace/*.cpp EasternKingdoms/MagistersTerrace/*.h)
file(GLOB_RECURSE scripts_MoltenCore EasternKingdoms/MoltenCore/*.cpp EasternKingdoms/MoltenCore/*.h)
file(GLOB_RECURSE scripts_ScarletEnclave EasternKingdoms/ScarletEnclave/*.cpp EasternKingdoms/ScarletEnclave/*.h)
file(GLOB_RECURSE scripts_ScarletMonastery EasternKingdoms/ScarletMonastery/*.cpp EasternKingdoms/ScarletMonastery/*.h)
file(GLOB_RECURSE scripts_Scholomance EasternKingdoms/Scholomance/*.cpp EasternKingdoms/Scholomance/*.h)
file(GLOB_RECURSE scripts_ShadowfangKeep EasternKingdoms/ShadowfangKeep/*.cpp EasternKingdoms/ShadowfangKeep/*.h)
file(GLOB_RECURSE scripts_Stratholme EasternKingdoms/Stratholme/*.cpp EasternKingdoms/Stratholme/*.h)
file(GLOB_RECURSE scripts_SunkenTemple EasternKingdoms/SunkenTemple/*.cpp EasternKingdoms/SunkenTemple/*.h)
file(GLOB_RECURSE scripts_SunwellPlateau EasternKingdoms/SunwellPlateau/*.cpp EasternKingdoms/SunwellPlateau/*.h)
file(GLOB_RECURSE scripts_ThroneOfTheTides EasternKingdoms/ThroneOfTheTides/*.cpp EasternKingdoms/ThroneOfTheTides/*.h)
file(GLOB_RECURSE scripts_TolBarad EasternKingdoms/TolBarad/*.cpp EasternKingdoms/TolBarad/*.h)
file(GLOB_RECURSE scripts_Uldaman EasternKingdoms/Uldaman/*.cpp EasternKingdoms/Uldaman/*.h)
file(GLOB_RECURSE scripts_ZulAman EasternKingdoms/ZulAman/*.cpp EasternKingdoms/ZulAman/*.h)
file(GLOB_RECURSE scripts_ZulGurub EasternKingdoms/ZulGurub/*.cpp EasternKingdoms/ZulGurub/*.h)

file(GLOB_RECURSE scripts_Zones EasternKingdoms/*.cpp EasternKingdoms/*.h)

source_group(EasternKingdoms\\AlteracValley FILES ${scripts_AlteracValley})
source_group(EasternKingdoms\\BaradinHold FILES ${scripts_BaradinHold})
source_group(EasternKingdoms\\BastionOfTwilight FILES ${scripts_BastionOfTwilight})
source_group(EasternKingdoms\\BlackrockCaverns FILES ${scripts_BlackrockCaverns})
source_group(EasternKingdoms\\BlackrockDepths FILES ${scripts_BlackrockDepths})
source_group(EasternKingdoms\\BlackrockSpire FILES ${scripts_BlackrockSpire})
source_group(EasternKingdoms\\BlackwingDescent FILES ${scripts_BlackwingDescent})
source_group(EasternKingdoms\\BlackwingLair FILES ${scripts_BlackwingLair})
source_group(EasternKingdoms\\Deadmines FILES ${scripts_Deadmines})
source_group(EasternKingdoms\\Gilneas FILES ${scripts_Gilneas})
source_group(EasternKingdoms\\Gnomeregan FILES ${scripts_Gnomeregan})
source_group(EasternKingdoms\\GrimBatol FILES ${scripts_GrimBatol})
source_group(EasternKingdoms\\Karazhan FILES ${scripts_Karazhan})
source_group(EasternKingdoms\\MagistersTerrace FILES ${scripts_MagistersTerrace})
source_group(EasternKingdoms\\MoltenCore FILES ${scripts_MoltenCore})
source_group(EasternKingdoms\\ScarletEnclave FILES ${scripts_ScarletEnclave})
source_group(EasternKingdoms\\ScarletMonastery FILES ${scripts_ScarletMonastery})
source_group(EasternKingdoms\\Scholomance FILES ${scripts_Scholomance})
source_group(EasternKingdoms\\ShadowfangKeep FILES ${scripts_ShadowfangKeep})
source_group(EasternKingdoms\\Stratholme FILES ${scripts_Stratholme})
source_group(EasternKingdoms\\SunkenTemple FILES ${scripts_SunkenTemple})
source_group(EasternKingdoms\\SunwellPlateau FILES ${scripts_SunwellPlateau})
source_group(EasternKingdoms\\ThroneOfTheTides FILES ${scripts_ThroneOfTheTides})
source_group(EasternKingdoms\\TolBarad FILES ${scripts_TolBarad})
source_group(EasternKingdoms\\Uldaman FILES ${scripts_Uldaman})
source_group(EasternKingdoms\\ZulAman FILES ${scripts_ZulAman})
source_group(EasternKingdoms\\ZulGurub FILES ${scripts_ZulGurub})
source_group(EasternKingdoms\\Zones FILES ${scripts_Zones})


set(scripts_STAT_SRCS
  ${scripts_STAT_SRCS}
  ${scripts_AlteracValley}
  ${scripts_BaradinHold}
  ${scripts_BastionOfTwilight}
  ${scripts_BlackrockCaverns}
  ${scripts_BlackrockDepths}
  ${scripts_BlackrockSpire}
  ${scripts_BlackwingDescent}
  ${scripts_BlackwingLair}
  ${scripts_Deadmines}
  ${scripts_Gilneas}
  ${scripts_Gnomeregan}
  ${scripts_GrimBatol}
  ${scripts_Karazhan}
  ${scripts_MagistersTerrace}
  ${scripts_MoltenCore}
  ${scripts_ScarletEnclave}
  ${scripts_ScarletMonastery}
  ${scripts_Scholomance}
  ${scripts_ShadowfangKeep}
  ${scripts_Stratholme}
  ${scripts_SunkenTemple}
  ${scripts_SunwellPlateau}
  ${scripts_ThroneOfTheTides}
  ${scripts_TolBarad}
  ${scripts_Uldaman}
  ${scripts_ZulAman}
  ${scripts_ZulGurub}
  ${scripts_Zones}
  
)

message("  -> Prepared: Eastern Kingdoms")
