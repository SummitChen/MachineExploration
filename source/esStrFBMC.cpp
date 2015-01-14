//
//  esStrFBMC.cpp
//  ExplorationStrategy
//
//  Created by Chen Si on 29/07/2014.
//
//

#include "esStrFBMC.h"
#include "esMapAnalysis.h"

void esStrFBMC::calCandidatePosition(bool & resetTarget, TilePosition &target, Unit * scout){

    scoutBot = scout;
    
    esMapAnalysis::getInstance()->analyzeExploredArea();
    
	TilePosition scoutLocation = TilePosition(scout->getTilePosition()); // Not sure whether it is correct. Rechecking......
    
    std::vector<TilePosition> globleCanPos;
    std::vector<TilePosition> postGlobleCanPosArray1;
    std::vector<TilePosition> postGlobleCanPosArray2;
    std::map<double,TilePosition> canPosMap;

	int sightRange = scout->getType().sightRange();
	sightRange /= 8;
    
    int searchLevel1 = 2 * sightRange;
    int searchLevel2 = 4 * sightRange;
    int searchLevel3 = 8 * sightRange;
    
    //------------debugging---------------
    level1_radius = (float)searchLevel1;
    level2_radius = (float)searchLevel2;
    level3_radius = (float)searchLevel3;
	circleCenter = scoutLocation;
    //------------------------------------
    
    level1 = level2 = level3 = false;
    
    int iteraterLevel1 = (int)pow((double)searchLevel1, 2.0) * 10;
    int iteraterLevel2 = (int)pow((double)searchLevel2, 2.0) * 10;
    int iteraterLevel3 = (int)pow((double)searchLevel3, 2.0) * 10;
	int iteraterGlobal = Broodwar->mapHeight() * Broodwar->mapWidth() * 10;
    
    std::vector<TilePosition> localCanPosLevel1;
    std::vector<TilePosition> localCanPosLevel2;
    std::vector<TilePosition> localCanPosLevel3;
    std::vector<TilePosition> localCanPosLevel3_2;
    std::vector<TilePosition> localCanPosVec;

    alpha = 0.4;
    beta = 0.6;
    
    double               tileExploredUtility = 0.0;
    double               segExploredUtility = 0.0;
    double               featureExploredUtility = 0.0;
    double               costComponent = 0.0;
    double               utilityComponent = 0.0;
    double               sumEvaluation = 0.0;
    
    bool                 repeatFlag = false;
  
	std::set<esPolygon*> exploredPolygons = esMapAnalysis::getInstance()->getExploredPolygons();

#ifdef DEBUGGING 

    //----------------debugging-------------------
    std::vector<TilePosition> polygonPoints;
    //--------------------------------------------
    
    //-------------------------debugging------------------------------
    std::set<esPolygon*>::iterator it = exploredPolygons.begin();
    std::set<esPolygon*>::const_iterator end = exploredPolygons.end();
    
    for (; it != end; ++it) {
        for (unsigned int i = 0; i < (*it)->pointSet.size(); i ++) {
            globleCanPos.push_back((*it)->pointSet[i]);
        }
        if ((*it)->getHoles().size() != 0) {
            for (unsigned int j = 0; j < (*it)->getHoles().size(); ++j) {
                for (unsigned int k = 0; k < (*it)->getHoles()[j].pointSet.size(); k ++ ) {
                    globleCanPos.push_back((*it)->getHoles()[j].pointSet[k]);
                }
            }
        }
    }
    
    it = exploredPolygons.begin();
    for (; it != end; ++it) {
        for (unsigned int i = 0; i < (*it)->pointSet.size(); i ++) {
            polygonPoints.push_back((*it)->pointSet[i]);
        }
        if ((*it)->getHoles().size() != 0) {
            for (unsigned int j = 0; j < (*it)->getHoles().size(); ++j) {
                for (unsigned int k = 0; k < (*it)->getHoles()[j].pointSet.size(); k += 2) {
                    polygonPoints.push_back((*it)->getHoles()[j].pointSet[k]);
                }
            }
        }
    }
    
    //-----------------------------------------------------------------------------------
#endif

    if (globleCanPos.size() == 0) {
        return;
    }

#ifdef DEBUGGING
    
	candidate_number = globleCanPos.size();

#endif

    for ( unsigned int i = 0; i < globleCanPos.size(); ++ i) {
        
        for (unsigned int j = 0; j < visitNodes.size(); ++j) {
            if ( globleCanPos[i].x() == visitNodes[j].pos.x()
                && globleCanPos[i].y() == visitNodes[j].pos.y()) {
                repeatFlag = true;
                break;
            }
        }
        
        if ( repeatFlag) {
            repeatFlag = false;
            continue;
        }
        
        if ( globleCanPos[i].x() < 0 || globleCanPos[i].x() > Broodwar->mapWidth()
            || globleCanPos[i].y() < 0 || globleCanPos[i].y() > Broodwar->mapHeight()) {
            continue;
        }
        
        if ( i % 2 == 0) {
            postGlobleCanPosArray2.push_back(globleCanPos[i]);
        }else{
            postGlobleCanPosArray1.push_back(globleCanPos[i]);
        }
        
    }
    
    for ( unsigned int i = 0; i < postGlobleCanPosArray1.size(); ++ i) {
        
		float distanceToCenter = (float)sqrt( pow((double)(postGlobleCanPosArray1[i].x() - scout->getTilePosition().x()), 2.0) + pow((double)(postGlobleCanPosArray1[i].y() - scout->getTilePosition().y()), 2.0));
        if ( distanceToCenter < searchLevel3 ) {
            localCanPosLevel3.push_back(postGlobleCanPosArray1[i]);
            if ( distanceToCenter < searchLevel2 ) {
                localCanPosLevel2.push_back(postGlobleCanPosArray1[i]);
                if ( distanceToCenter < searchLevel1) {
                    localCanPosLevel1.push_back(postGlobleCanPosArray1[i]);
                }
            }
        }
    }
    
    for ( unsigned int i = 0; i < postGlobleCanPosArray2.size(); ++ i) {
        
        float distanceToCenter = (float)sqrt( pow((double)(postGlobleCanPosArray2[i].x() - scout->getTilePosition().x()), 2.0) + pow((double)(postGlobleCanPosArray2[i].y() - scout->getTilePosition().y()), 2.0));
        if ( distanceToCenter < searchLevel3 ) {
            localCanPosLevel3_2.push_back(postGlobleCanPosArray2[i]);
        }
    }
    
    if ( localCanPosLevel1.size() != 0) {
        utilityDecision(canPosMap, localCanPosLevel1, exploredPolygons, target, true);
        if ( canPosMap.size() != 0) {
            resetTarget = true;
            level1 = true;
            goto display;
        }
    }
    
    if ( localCanPosLevel2.size() != 0) {
        utilityDecision(canPosMap, localCanPosLevel2, exploredPolygons, target, true);
        if ( canPosMap.size() != 0) {
            resetTarget = true;
            level2 = true;
            goto display;
        }
    }
    
    if ( localCanPosLevel3.size() != 0) {
        utilityDecision(canPosMap, localCanPosLevel3, exploredPolygons, target, true);
        if ( canPosMap.size() != 0) {
            resetTarget = true;
            level3 = true;
            goto display;
        }
    }
    
    if ( localCanPosLevel3_2.size() != 0) {
        utilityDecision(canPosMap, localCanPosLevel3_2, exploredPolygons, target, true);
        if ( canPosMap.size() != 0) {
            resetTarget = true;
            level3 = true;
            goto display;
        }
    }

    
    if ( postGlobleCanPosArray1.size() != 0) {
        utilityDecision(canPosMap, postGlobleCanPosArray1, exploredPolygons, target, false);
        if ( canPosMap.size() != 0) {
            resetTarget = true;
            goto display;
        }
    }
    
    if ( postGlobleCanPosArray2.size() != 0) {
        utilityDecision(canPosMap, postGlobleCanPosArray2, exploredPolygons, target, false);
        if ( canPosMap.size() != 0) {
            resetTarget = true;
            goto display;
        }
    }
    
display:
    ;

}

void esStrFBMC::utilityDecision(std::map<double, TilePosition> &resultMap, std::vector<TilePosition> canPosVec, std::set<esPolygon *> exploredPolygons, TilePosition& target, bool localSearch){
    
    resultMap.clear();
    
    int                  travelDistance = 0;
    double               tileExploredUtility = 0.0;
    double               segExploredUtility = 0.0;
    double               featureExploredUtility = 0.0;
    double               costComponent = 0.0;
    double               utilityComponent = 0.0;
    double               sumEvaluation = 0.0;
	double               unitBoundRadius = 0.0;
    
    //Position scoutLocation = Position( scoutBot->getPosition());
    
    bool repeatFlag = false;
    
    for ( unsigned int i = 0; i < canPosVec.size(); ++i) {
        
        //candidatePos = ofVec2f((float)canPosVec[i].x, (float)canPosVec[i].y);
		//travelDistance = scoutBot->getDistance(canPosVec[i]);

		if (evaluateBuildTile(canPosVec[i]) != WALKABLE || isResourceTaken(canPosVec[i])){
			continue;
		}
		
		travelDistance = (int)sqrt(pow(scoutBot->getTilePosition().x() - canPosVec[i].x(), 2.0) + pow(scoutBot->getTilePosition().y() - canPosVec[i].y(), 2.0));
		unitBoundRadius = sqrt(pow( (scoutBot->getRight() - scoutBot->getLeft())/2.0, 2.0) + pow( (scoutBot->getTop() - scoutBot->getBottom())/2.0, 2.0));
		costComponent = exp( unitBoundRadius - travelDistance);
        
		tileExploredUtility = calculateTilePercentage(scoutBot, canPosVec[i]);
        segExploredUtility = calculateSegPercentage(scoutBot, canPosVec[i]);
        featureExploredUtility = calculateFeaPercentage(scoutBot, canPosVec[i]);
        
       // utilityComponent = 0.4 * tileExploredUtility + 0.4 * segExploredUtility + 0.2 * featureExploredUtility;
        
        if ( fabs((config.getGrid() + config.getSegment() + config.getFeature()) - 1.0) < 0.002) {
        
            utilityComponent = config.getGrid() * tileExploredUtility + config.getSegment() * segExploredUtility + config.getFeature() * featureExploredUtility;
        
        }else{
        
            utilityComponent = 0.4 * tileExploredUtility + 0.4 * segExploredUtility + 0.2 * featureExploredUtility;
        }

        
        if ( localSearch && utilityComponent < 0.2) {
            continue;
        }
        
        if ( fabs((config.getAlpha() + config.getBeta()) - 1.0) < 0.002 ) {
            sumEvaluation = config.getAlpha() * costComponent + config.getBeta() * utilityComponent;
        }else{
            sumEvaluation = 0.4 * costComponent + 0.6 * utilityComponent;
        }
        
        sumEvaluation = -sumEvaluation;
        
		resultMap.insert(std::pair<double, TilePosition>(sumEvaluation, canPosVec[i]));
        
    }
    
    if ( resultMap.size() != 0){
        
        //target = ofVec2f((float)resoultMap.begin()->second.x, (float)resoultMap.begin()->second.y);
		TilePosition walkableTarget = searchWalkableNeighbor(resultMap.begin()->second);
		target = walkableTarget;
       // target = ofVec2f(walkableTarget.x, walkableTarget.y);
        esNode nextTarget;
        nextTarget.pos = resultMap.begin()->second;
        nextTarget.visitFlag = true;
        visitNodes.push_back(nextTarget);
    }
    
}
