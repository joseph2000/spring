/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */


#include "Factory.h"
#include "Game/GameHelper.h"
#include "Game/WaitCommandsAI.h"
#include "Map/Ground.h"
#include "Map/ReadMap.h"
#include "Sim/Misc/GroundBlockingObjectMap.h"
#include "Sim/Misc/QuadField.h"
#include "Sim/Projectiles/ProjectileHandler.h"
#include "Sim/Units/Scripts/UnitScript.h"
#include "Sim/Units/CommandAI/CommandAI.h"
#include "Sim/Units/CommandAI/FactoryCAI.h"
#include "Sim/Units/CommandAI/MobileCAI.h"
#include "Sim/Units/UnitHandler.h"
#include "Sim/Units/UnitLoader.h"
#include "Sim/Units/UnitDefHandler.h"
#include "System/EventHandler.h"
#include "System/Matrix44f.h"
#include "System/myMath.h"
#include "System/Sound/SoundChannels.h"
#include "System/Sync/SyncTracer.h"
#include "System/mmgr.h"

#define PLAY_SOUNDS 1

CR_BIND_DERIVED(CFactory, CBuilding, );

CR_REG_METADATA(CFactory, (
				CR_MEMBER(buildSpeed),
				CR_MEMBER(quedBuild),
				//CR_MEMBER(nextBuild),
				CR_MEMBER(nextBuildName),
				CR_MEMBER(curBuild),
				CR_MEMBER(opening),
				CR_MEMBER(lastBuild),
				CR_RESERVED(16),
				CR_POSTLOAD(PostLoad)
				));

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFactory::CFactory():
	buildSpeed(100),
	quedBuild(false),
	nextBuild(0),
	curBuild(0),
	opening(false),
	lastBuild(-1000)
{
}

CFactory::~CFactory() {
	if (curBuild != NULL) {
		curBuild->KillUnit(false, true, NULL);
		curBuild = NULL;
	}
}



void CFactory::PostLoad()
{
	nextBuild = unitDefHandler->GetUnitDefByName(nextBuildName);
	if (opening) {
		script->Activate();
	}
	if (curBuild) {
		script->StartBuilding();
	}
}

void CFactory::PreInit(const UnitDef* def, int team, int facing, const float3& position, bool build)
{
	buildSpeed = def->buildSpeed / TEAM_SLOWUPDATE_RATE;
	CBuilding::PreInit(def, team, facing, position, build);
}

int CFactory::GetBuildPiece()
{
	return script->QueryBuildInfo();
}

// GetBuildPiece() is called if piece < 0
float3 CFactory::CalcBuildPos(int buildPiece)
{
	float3 relBuildPos = script->GetPiecePos(buildPiece < 0 ? GetBuildPiece() : buildPiece);
	float3 buildPos = pos + frontdir * relBuildPos.z + updir * relBuildPos.y + rightdir * relBuildPos.x;
	return buildPos;
}

void CFactory::Update()
{
	if (beingBuilt) {
		// factory under construction
		CUnit::Update();
		return;
	}

	if (quedBuild && !opening && !stunned) {
		script->Activate();
		groundBlockingObjectMap->OpenBlockingYard(this, curYardMap);
		opening = true;
	}

	if (quedBuild && inBuildStance && !stunned) {
		// start building a unit
		const float3        buildPos = CalcBuildPos();
		const CSolidObject* solidObj = groundBlockingObjectMap->GroundBlocked(buildPos);

		if (solidObj == NULL || (dynamic_cast<const CUnit*>(solidObj) == this)) {
			quedBuild = false;
			CUnit* b = unitLoader->LoadUnit(nextBuild, buildPos + float3(0.01f, 0.01f, 0.01f), team,
											true, buildFacing, this);

			if (!unitDef->canBeAssisted) {
				b->soloBuilder = this;
				b->AddDeathDependence(this, DEPENDENCE_BUILDER);
			}

			AddDeathDependence(b, DEPENDENCE_BUILD);
			curBuild = b;

			script->StartBuilding();

			#if (PLAY_SOUNDS == 1)
			const int soundIdx = unitDef->sounds.build.getRandomIdx();
			if (soundIdx >= 0) {
				Channels::UnitReply.PlaySample(
					unitDef->sounds.build.getID(soundIdx), pos,
					unitDef->sounds.build.getVolume(0));
			}
			#endif
		} else {
			helper->BuggerOff(buildPos - float3(0.01f, 0, 0.02f), radius + 8, true, true, team, NULL);
		}
	}


	if (curBuild && !beingBuilt) {
		if (!stunned) {
			// factory not under construction and
			// nanolathing unit: continue building
			lastBuild = gs->frameNum;

			// buildPiece is the rotating platform
			const int buildPiece = GetBuildPiece();
			const float3& buildPos = CalcBuildPos(buildPiece);
			const CMatrix44f& mat = script->GetPieceMatrix(buildPiece);
			const int h = GetHeadingFromVector(mat[2], mat[10]); //! x.z, z.z

			// rotate unit nanoframe with platform
			curBuild->heading = (h + GetHeadingFromFacing(buildFacing)) & (SPRING_CIRCLE_DIVS - 1);
			curBuild->pos = buildPos;

			if (curBuild->floatOnWater && (buildPos.y <= 0.0f))
				curBuild->pos.y = -curBuild->unitDef->waterline;

			curBuild->UpdateMidPos();

			const CCommandQueue& queue = commandAI->commandQue;

			if (!queue.empty() && (queue.front().GetID() == CMD_WAIT)) {
				curBuild->AddBuildPower(0, this);
			} else {
				if (curBuild->AddBuildPower(buildSpeed, this)) {
					CreateNanoParticle();
				}
			}
		}

		if (!curBuild->beingBuilt &&
				(!unitDef->fullHealthFactory ||
						(curBuild->health >= curBuild->maxHealth)))
		{
			if (group && curBuild->group == 0) {
				curBuild->SetGroup(group);
			}

			bool userOrders = true;
			if (curBuild->commandAI->commandQue.empty() ||
					(dynamic_cast<CMobileCAI*>(curBuild->commandAI) &&
					 ((CMobileCAI*)curBuild->commandAI)->unimportantMove)) {
				userOrders = false;

				AssignBuildeeOrders(curBuild);
				waitCommandsAI.AddLocalUnit(curBuild, this);
			}
			eventHandler.UnitFromFactory(curBuild, this, userOrders);

			StopBuild();
		}
	}

	if (((lastBuild + 200) < gs->frameNum) && !stunned &&
	    !quedBuild && opening && groundBlockingObjectMap->CanCloseYard(this)) {
		// close the factory after inactivity
		groundBlockingObjectMap->CloseBlockingYard(this, curYardMap);
		opening = false;
		script->Deactivate();
	}

	CBuilding::Update();
}

void CFactory::StartBuild(const UnitDef* ud)
{
	if (beingBuilt)
		return;

	if (curBuild)
		StopBuild();

	quedBuild = true;
	nextBuild = ud;
	nextBuildName = ud->name;

#ifdef TRACE_SYNC
	tracefile << "Start build: ";
	tracefile << ud->name.c_str() << "\n";
#endif

	if (!opening && !stunned) {
		script->Activate();
		groundBlockingObjectMap->OpenBlockingYard(this, curYardMap);
		opening = true;
	}
}

void CFactory::StopBuild()
{
	// cancel a build-in-progress
	script->StopBuilding();
	if (curBuild) {
		if (curBuild->beingBuilt) {
			AddMetal(curBuild->metalCost * curBuild->buildProgress, false);
			curBuild->KillUnit(false, true, NULL);
		}
		DeleteDeathDependence(curBuild, DEPENDENCE_BUILD);
	}
	curBuild = 0;
	quedBuild = false;
}

void CFactory::DependentDied(CObject* o)
{
	if (o == curBuild) {
		curBuild = 0;
		StopBuild();
	}
	CUnit::DependentDied(o);
}



void CFactory::SendToEmptySpot(CUnit* unit)
{
	float r = radius * 1.7f + unit->radius * 4;
	float3 foundPos = pos + frontdir * r;

	for (int a = 0; a < 20; ++a) {
		float3 testPos = pos + frontdir * r * cos(a * PI / 10) + rightdir * r * sin(a * PI / 10);
		testPos.y = ground->GetHeightAboveWater(testPos.x, testPos.z);

		if (qf->GetSolidsExact(testPos, unit->radius * 1.5f).empty()) {
			foundPos = testPos;
			break;
		}
	}

	Command c(CMD_MOVE);
	c.params.push_back(foundPos.x);
	c.params.push_back(foundPos.y);
	c.params.push_back(foundPos.z);
	unit->commandAI->GiveCommand(c);
}

void CFactory::AssignBuildeeOrders(CUnit* unit) {
	const CFactoryCAI* facAI = (CFactoryCAI*) commandAI;
	const CCommandQueue& newUnitCmds = facAI->newUnitCommands;

	if (newUnitCmds.empty()) {
		SendToEmptySpot(unit);
		return;
	}

	Command c(CMD_MOVE);

	if (!unit->unitDef->canfly) {

		// HACK: when a factory has a rallypoint set far enough away
		// to trigger the non-admissable path estimators, we want to
		// avoid units getting stuck inside by issuing them an extra
		// move-order. However, this order can *itself* cause the PF
		// system to consider the path blocked if the extra waypoint
		// falls within the factory's confines, so use a wide berth.
		const float xs = unitDef->xsize * SQUARE_SIZE * 0.5f;
		const float zs = unitDef->zsize * SQUARE_SIZE * 0.5f;

		float tmpDst = 2.0f;
		float3 tmpPos = unit->pos + (frontdir * this->radius * tmpDst);

		if (buildFacing == FACING_NORTH || buildFacing == FACING_SOUTH) {
			while ((tmpPos.z >= unit->pos.z - zs && tmpPos.z <= unit->pos.z + zs)) {
				tmpDst += 0.5f;
				tmpPos = unit->pos + (frontdir * this->radius * tmpDst);
			}
		} else {
			while ((tmpPos.x >= unit->pos.x - xs && tmpPos.x <= unit->pos.x + xs)) {
				tmpDst += 0.5f;
				tmpPos = unit->pos + (frontdir * this->radius * tmpDst);
			}
		}

		c.params.push_back(tmpPos.x);
		c.params.push_back(tmpPos.y);
		c.params.push_back(tmpPos.z);
		unit->commandAI->GiveCommand(c);
	}

	for (CCommandQueue::const_iterator ci = newUnitCmds.begin(); ci != newUnitCmds.end(); ++ci) {
		c = *ci;
		c.options |= SHIFT_KEY;
		unit->commandAI->GiveCommand(c);
	}
}



void CFactory::SlowUpdate(void)
{
	if (!transporter)
		helper->BuggerOff(pos - float3(0.01f, 0, 0.02f), radius, true, true, team, NULL);
	CBuilding::SlowUpdate();
}

bool CFactory::ChangeTeam(int newTeam, ChangeType type)
{
	StopBuild();
	return CBuilding::ChangeTeam(newTeam, type);
}


void CFactory::CreateNanoParticle(bool highPriority)
{
	const int piece = script->QueryNanoPiece();

#ifdef USE_GML
	if (gs->frameNum - lastDrawFrame > 20)
		return;
#endif

	const float3 relWeaponFirePos = script->GetPiecePos(piece);
	const float3 weaponPos = pos
		+ (frontdir * relWeaponFirePos.z)
		+ (updir    * relWeaponFirePos.y)
		+ (rightdir * relWeaponFirePos.x);

	// unsynced
	ph->AddNanoParticle(weaponPos, curBuild->midPos, unitDef, team, highPriority);
}
