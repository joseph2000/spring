/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */

#include "DemoReader.h"

#include "System/mmgr.h"

#include "System/Net/RawPacket.h"
#include "Game/GameVersion.h"

#include <limits.h>
#include <stdexcept>
#include <cassert>
#include <cstring>

CDemoReader::CDemoReader(const std::string& filename, float curTime)
{
	playbackDemo.open(filename.c_str(), std::ios::binary);

	if (!playbackDemo.is_open()) {
		// file not found -> exception
		throw std::runtime_error(std::string("Demofile not found: ")+filename);
	}

	playbackDemo.read((char*)&fileHeader, sizeof(fileHeader));
	fileHeader.swab();

	if (memcmp(fileHeader.magic, DEMOFILE_MAGIC, sizeof(fileHeader.magic))
		|| fileHeader.version != DEMOFILE_VERSION
		|| fileHeader.headerSize != sizeof(fileHeader)
		|| fileHeader.playerStatElemSize != sizeof(PlayerStatistics)
		|| fileHeader.teamStatElemSize != sizeof(TeamStatistics)
		// Don't compare spring version in debug mode: we don't want to make
		// debugging dev-version demos impossible (because the version is different
		// each build.)
#ifndef _DEBUG
		|| (!SpringVersion::IsRelease() && strcmp(fileHeader.versionString, SpringVersion::GetSync().c_str()))
#endif
	) {
		throw std::runtime_error(std::string("Demofile corrupt or created by a different version of Spring: ")+filename);
	}

	if (fileHeader.scriptSize != 0) {
		char* buf = new char[fileHeader.scriptSize];
		playbackDemo.read(buf, fileHeader.scriptSize);
		setupScript = std::string(buf, fileHeader.scriptSize);
		delete[] buf;
	}

	playbackDemo.read((char*)&chunkHeader, sizeof(chunkHeader));
	chunkHeader.swab();

	demoTimeOffset = curTime - chunkHeader.modGameTime - 0.1f;
	nextDemoReadTime = curTime - 0.01f;

	if (fileHeader.demoStreamSize != 0) {
		bytesRemaining = fileHeader.demoStreamSize;
	}
	else {
		// Spring crashed while recording the demo: replay until EOF,
		// but at most filesize bytes to block watching demo of running game.
		// For this we must determine the file size.
		// (if this had still used CFileHandler that would have been easier ;-))
		long curPos = playbackDemo.tellg();
		playbackDemo.seekg(0, std::ios::end);
 		bytesRemaining = (long) playbackDemo.tellg() - curPos;
 		playbackDemo.seekg(curPos);
	}
}

netcode::RawPacket* CDemoReader::GetData(float readTime)
{
	if (ReachedEnd())
		return NULL;

	// when paused, modGameTime does not increase (ie. we
	// always pass the same readTime value) so no seperate
	// check needed
	if (readTime > nextDemoReadTime) {
		netcode::RawPacket* buf = new netcode::RawPacket(chunkHeader.length);
		playbackDemo.read((char*)(buf->data), chunkHeader.length);
		bytesRemaining -= chunkHeader.length;

		if (!ReachedEnd()) {
			// read next chunk header
			playbackDemo.read((char*)&chunkHeader, sizeof(chunkHeader));
			chunkHeader.swab();
			nextDemoReadTime = chunkHeader.modGameTime + demoTimeOffset;
			bytesRemaining -= sizeof(chunkHeader);
		}

		return buf;
	} else {
		return NULL;
	}
}

bool CDemoReader::ReachedEnd() const
{
	if (bytesRemaining <= 0 || playbackDemo.eof())
		return true;
	else
		return false;
}


void CDemoReader::LoadStats()
{
	// Stats are not available if Spring crashed while writing the demo.
	if (fileHeader.demoStreamSize == 0) {
		return;
	}

	const int curPos = playbackDemo.tellg();
	playbackDemo.seekg(fileHeader.headerSize + fileHeader.scriptSize + fileHeader.demoStreamSize);

	winningAllyTeams.clear();
	playerStats.clear();
	teamStats.clear();

	for (int allyTeamNum = 0; allyTeamNum < fileHeader.winningAllyTeamsSize; ++allyTeamNum) {
		unsigned char winnerAllyTeam;
		playbackDemo.read((char*) &winnerAllyTeam, sizeof(unsigned char));
		winningAllyTeams.push_back(winnerAllyTeam);
	}

	for (int playerNum = 0; playerNum < fileHeader.numPlayers; ++playerNum) {
		PlayerStatistics buf;
		playbackDemo.read((char*) &buf, sizeof(buf));
		buf.swab();
		playerStats.push_back(buf);
	}

	{ // Team statistics follow player statistics.
		teamStats.resize(fileHeader.numTeams);
		// Read the array containing the number of team stats for each team.
		std::vector<int> numStatsPerTeam(fileHeader.numTeams, 0);
		playbackDemo.read((char*) (&numStatsPerTeam[0]), numStatsPerTeam.size());

		for (int teamNum = 0; teamNum < fileHeader.numTeams; ++teamNum) {
			for (int i = 0; i < numStatsPerTeam[teamNum]; ++i) {
				TeamStatistics buf;
				playbackDemo.read((char*) &buf, sizeof(buf));
				buf.swab();
				teamStats[teamNum].push_back(buf);
			}
		}
	}

	playbackDemo.seekg(curPos);
}
