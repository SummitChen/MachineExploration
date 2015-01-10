//
//  esStrFBMC.h
//  ExplorationStrategy
//
//  Created by Chen Si on 29/07/2014.
//
//

#ifndef ExplorationStrategy_esStrFBMC_h
#define ExplorationStrategy_esStrFBMC_h

#include "esStrategy.h"
#include "BWAPI/Unit.h"

class esStrFBMC : public esStrategy{
    
public:
    
	virtual void calCandidatePosition(bool & resetTarget, TilePosition &Target, Unit * scout);

#ifdef DEBUGGING
	virtual void debugDraw();
#endif

private:
    
    void utilityDecision(std::map<double, TilePosition>& resultMap,
                         std::vector<TilePosition> canPosVec,
                         std::set<esPolygon*> exploredPolygon,
                         TilePosition& target,
                         bool  localSearch);
    
    float            alpha, beta;
    Unit             *scoutBot;
    
    bool level1, level2, level3;
    TilePosition circleCenter;
    float level1_radius;
    float level2_radius;
    float level3_radius;
    
};


#endif
