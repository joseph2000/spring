/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */

#include "UnitDef.h"
#include "MoveData.h"

/******************************************************************************/

springLegacyAI::UnitDef::UnitDefWeapon::UnitDefWeapon()
: name("NOWEAPON")
, def(NULL)
, slavedTo(0)
, mainDir(0, 0, 1)
, maxAngleDif(-1)
, fuelUsage(0)
, badTargetCat(0)
, onlyTargetCat(0)
{
}

springLegacyAI::UnitDef::UnitDefWeapon::UnitDefWeapon(
	std::string name, const WeaponDef* def, int slavedTo, float3 mainDir, float maxAngleDif,
	unsigned int badTargetCat, unsigned int onlyTargetCat, float fuelUse)
: name(name)
, def(def)
, slavedTo(slavedTo)
, mainDir(mainDir)
, maxAngleDif(maxAngleDif)
, fuelUsage(fuelUse)
, badTargetCat(badTargetCat)
, onlyTargetCat(onlyTargetCat)
{
}


/******************************************************************************/

springLegacyAI::UnitDef::UnitDef()
: valid(true)
, id(-1)
, decoyDef(NULL)
, aihint(0)
, cobID(-1)
, techLevel(0)
, gaia("")
, metalUpkeep(0.0f)
, energyUpkeep(0.0f)
, metalMake(0.0f)
, makesMetal(0.0f)
, energyMake(0.0f)
, metalCost(0.0f)
, energyCost(0.0f)
, buildTime(0.0f)
, extractsMetal(0.0f)
, extractRange(0.0f)
, windGenerator(0.0f)
, tidalGenerator(0.0f)
, metalStorage(0.0f)
, energyStorage(0.0f)
, extractSquare(false)
, autoHeal(0.0f)
, idleAutoHeal(0.0f)
, idleTime(0)
, power(0.0f)
, health(0.0f)
, category(-1)
, speed(0.0f)
, rSpeed(0.0f)
, turnRate(0.0f)
, turnInPlace(false)
, turnInPlaceDistance(0.0f)
, turnInPlaceSpeedLimit(0.0f)
, upright(false)
, collide(false)
, losRadius(0.0f)
, airLosRadius(0.0f)
, losHeight(0.0f)
, radarRadius(0.0f)
, sonarRadius(0.0f)
, jammerRadius(0.0f)
, sonarJamRadius(0.0f)
, seismicRadius(0.0f)
, seismicSignature(0.0f)
, stealth(false)
, sonarStealth(false)
, buildRange3D(false)
, buildDistance(0.0f)
, buildSpeed(0.0f)
, reclaimSpeed(0.0f)
, repairSpeed(0.0f)
, maxRepairSpeed(0.0f)
, resurrectSpeed(0.0f)
, captureSpeed(0.0f)
, terraformSpeed(0.0f)
, mass(0.0f)
, pushResistant(false)
, strafeToAttack(false)
, minCollisionSpeed(0.0f)
, slideTolerance(0.0f)
, maxSlope(0.0f)
, maxHeightDif(0.0f)
, minWaterDepth(0.0f)
, waterline(0.0f)
, maxWaterDepth(0.0f)
, armoredMultiple(0.0f)
, armorType(0)
, flankingBonusMode(0)
, flankingBonusDir(ZeroVector)
, flankingBonusMax(0.0f)
, flankingBonusMin(0.0f)
, flankingBonusMobilityAdd(0.0f)
, modelCenterOffset(ZeroVector)
, usePieceCollisionVolumes(false)
, shieldWeaponDef(NULL)
, stockpileWeaponDef(NULL)
, maxWeaponRange(0.0f)
, maxCoverage(0.0f)
, canSelfD(true)
, selfDCountdown(0)
, canSubmerge(false)
, canfly(false)
, canmove(false)
, canhover(false)
, floater(false)
, builder(false)
, activateWhenBuilt(false)
, onoffable(false)
, fullHealthFactory(false)
, factoryHeadingTakeoff(false)
, reclaimable(false)
, capturable(false)
, repairable(false)
, canRestore(false)
, canRepair(false)
, canSelfRepair(false)
, canReclaim(false)
, canAttack(false)
, canPatrol(false)
, canFight(false)
, canGuard(false)
, canAssist(false)
, canBeAssisted(false)
, canRepeat(false)
, canFireControl(false)
, fireState(0)
, moveState(0)
, wingDrag(0.0f)
, wingAngle(0.0f)
, drag(0.0f)
, frontToSpeed(0.0f)
, speedToFront(0.0f)
, myGravity(0.0f)
, maxBank(0.0f)
, maxPitch(0.0f)
, turnRadius(0.0f)
, wantedHeight(0.0f)
, verticalSpeed(0.0f)
, useSmoothMesh(false)
, canCrash(false)
, hoverAttack(false)
, airStrafe(false)
, dlHoverFactor(0.0f)
, bankingAllowed(false)
, maxAcc(0.0f)
, maxDec(0.0f)
, maxAileron(0.0f)
, maxElevator(0.0f)
, maxRudder(0.0f)
, crashDrag(0.0f)
, movedata(NULL)
, xsize(0)
, zsize(0)
, buildangle(0)
, loadingRadius(0.0f)
, unloadSpread(0.0f)
, transportCapacity(0)
, transportSize(0)
, minTransportSize(0)
, isAirBase(false)
, isFirePlatform(false)
, transportMass(0.0f)
, minTransportMass(0.0f)
, holdSteady(false)
, releaseHeld(false)
, cantBeTransported(false)
, transportByEnemy(false)
, transportUnloadMethod(0)
, fallSpeed(0.0f)
, unitFallSpeed(0.0f)
, canCloak(false)
, startCloaked(false)
, cloakCost(0.0f)
, cloakCostMoving(0.0f)
, decloakDistance(0.0f)
, decloakSpherical(false)
, decloakOnFire(false)
, cloakTimeout(0)
, canKamikaze(false)
, kamikazeDist(0.0f)
, kamikazeUseLOS(false)
, targfac(false)
, canDGun(false)
, needGeo(false)
, isFeature(false)
, hideDamage(false)
, isCommander(false)
, showPlayerName(false)
, canResurrect(false)
, canCapture(false)
, highTrajectoryType(0)
, noChaseCategory(0)
, leaveTracks(false)
, trackWidth(0.0f)
, trackOffset(0.0f)
, trackStrength(0.0f)
, trackStretch(0.0f)
, trackType(0)
, canDropFlare(false)
, flareReloadTime(0.0f)
, flareEfficiency(0.0f)
, flareDelay(0.0f)
, flareDropVector(ZeroVector)
, flareTime(0)
, flareSalvoSize(0)
, flareSalvoDelay(0)
, smoothAnim(false)
, canLoopbackAttack(false)
, levelGround(false)
, useBuildingGroundDecal(false)
, buildingDecalType(0)
, buildingDecalSizeX(0)
, buildingDecalSizeY(0)
, buildingDecalDecaySpeed(0.0f)
, showNanoFrame(false)
, showNanoSpray(false)
, nanoColor(ZeroVector)
, maxFuel(0.0f)
, refuelTime(0.0f)
, minAirBasePower(0.0f)
, pieceTrailCEGRange(-1)
, maxThisUnit(0)
, realMetalCost(0.0f)
, realEnergyCost(0.0f)
, realMetalUpkeep(0.0f)
, realEnergyUpkeep(0.0f)
, realBuildTime(0.0f)
{
	yardmaps[0] = NULL;
	yardmaps[1] = NULL;
	yardmaps[2] = NULL;
	yardmaps[3] = NULL;
}


springLegacyAI::UnitDef::~UnitDef()
{
	delete movedata; movedata = NULL;

	delete[] yardmaps[0]; yardmaps[0] = NULL;
	delete[] yardmaps[1]; yardmaps[1] = NULL;
	delete[] yardmaps[2]; yardmaps[2] = NULL;
	delete[] yardmaps[3]; yardmaps[3] = NULL;
}


/******************************************************************************/
