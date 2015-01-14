#pragma once

#include <math.h>
#include "Common.h"
#include "algorithmConfig.h"
#include "esPolygon.h"
#include "TerrainDetectorModule.h"

#include "BWAPI/Position.h"
#include "BWAPI/TilePosition.h"
#include "BWAPI/Unit.h"

#ifdef DEBUGGING
#include <fstream>
#endif

#define UN_WALKABLE 0
#define WALKABLE 1
#define UN_KNOWN 2


using namespace BWAPI;

struct esNode{
	TilePosition pos;
	bool     visitFlag;
};

struct edgeNode{
	TilePosition     pos;
	unsigned int explorationFlag;
};

typedef std::vector<esNode> NodeVec;

class esStrategy
{
public:

	virtual void calCandidatePosition(bool & resetTarget, TilePosition &Target, Unit * scout){}

#ifdef DEBUGGING
	//-----------------------------
	virtual void debugDraw(){}
	unsigned int getCandidateNumbers();
	/*-----------------------------*/
#endif

	void setAlgorithmConfig(algorithmConfig confi);
	esStrategy(void);
	~esStrategy(void);

protected:
	 
	float calculateTilePercentage(const Unit* scout, const TilePosition &target);
	float calculateSegPercentage(const Unit* scout, const TilePosition& target);
	float calculateFeaPercentage(const Unit * scout, const TilePosition &target);

	TilePosition searchWalkableNeighbor( const TilePosition & original);

	void calculateExploredPolygons();
	/* void recalSegVertices(); */
    bool isIncircle(TilePosition center, int radius, TilePosition node);


	unsigned char evaluateEdgePoint(const TilePosition& pos);
	unsigned char evaluateBuildTile(const TilePosition& pos);
	bool lineInterCircle(const TilePosition &ptStart, 
		const TilePosition &ptEnd, 
		const TilePosition &ptCenter,
        const float Radius, 
		TilePosition &ptInter1, 
		TilePosition &ptInter2);
	bool isReachable(std::set<esPolygon *> polygons, const TilePosition & p1, const TilePosition & p2);
    
	bool isResourceTaken(const TilePosition & p);
	bool isMineralsTaken(const TilePosition & p);
	bool isGeysersTaken(const TilePosition & p);

	NodeVec visitNodes;
    
#ifdef DEBUGGING
    //-----------------debugging Log---------------
    std::fstream                            logFile;
	unsigned int                            candidate_number;
#endif
	algorithmConfig config; 
};

