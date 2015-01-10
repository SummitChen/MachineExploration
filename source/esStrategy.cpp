//
//  esStrategy.cpp
//  ExplorationStrategy
//
//  Created by Chen Si on 20/07/2014.
//
//

#include "esStrategy.h"
#include "esMapAnalysis.h"


#define EPS 0.00001

bool esStrategy::isIncircle(TilePosition center, int radius, TilePosition node){
	return ( node.getDistance(center) <= radius );
}

/*
*  @target represent the walk tile, where the target in
*
*/
float esStrategy::calculateTilePercentage(const Unit* scout, const TilePosition &target){

	int sumUnexplored = 0;
	int sum = 0;
	int scoutSight = scout->getType().sightRange();
	scoutSight /= 32;

	for( int i = - scoutSight; i <= scoutSight; i++){
		for( int j = - scoutSight; j <= scoutSight; j++){
			if (sqrt( pow((double)i, 2.0) + pow((double)j, 2.0)) <= scoutSight
				&& ( i + target.x() ) > 0 && (i + target.x()) < Broodwar->mapWidth()
				&& ( j + target.y() ) > 0 && (j + target.y()) < Broodwar->mapHeight()){
					sum ++;
					if( !Broodwar->isExplored(i, j)){
						sumUnexplored ++;
					}
			}
		}
	}

	if ( sum == 0) {
		return 0;
	}else{
		double sumD = (double)sum;
		return sumUnexplored / sumD;
	}
}
/*
*  @target build tile
*
*/
float esStrategy::calculateSegPercentage(const Unit* scout, const TilePosition& target){

	double sumUnexplored = 0.0;
	double sumExplored = 0.0;
	int sightRange = scout->getType().sightRange();
	sightRange /= 32;
	TilePosition buildPosition = TilePosition(scout->getPosition().x() / 32, scout->getPosition().y() / 32); 

	std::vector<std::vector<edgeNode> > proVertexVec;

	std::set<esPolygon*> exploredPolygon = esMapAnalysis::getInstance()->getExploredPolygons();

	std::set<esPolygon*>::iterator it = exploredPolygon.begin();
	std::set<esPolygon*>::iterator end = exploredPolygon.end();

	for (; it != end; ++ it) {

		std::vector<edgeNode>  vertexVec;

		for ( unsigned int i = 0; i < (*it)->pointSet.size(); i ++) {
			edgeNode node;
			node.pos = TilePosition((*it)->pointSet[i].x()/32, (*it)->pointSet[i].y()/32);
			node.explorationFlag = evaluateEdgePoint(node.pos);
			vertexVec.push_back(node);
		}

		proVertexVec.push_back(vertexVec);

		for ( unsigned int j = 0; j < (*it)->getHoles().size(); ++ j) {

			std::vector<edgeNode> holeVertexVec;

			for ( unsigned int k = 0; k < (*it)->getHoles()[j].pointSet.size(); ++ k) {

				edgeNode hNode;
				hNode.pos = TilePosition((*it)->getHoles()[j].pointSet[k].x(), (*it)->getHoles()[j].pointSet[k].y());
				hNode.explorationFlag = evaluateEdgePoint(hNode.pos);
				holeVertexVec.push_back(hNode);
			}

			proVertexVec.push_back(holeVertexVec);
		}
	}

#ifdef DEBUGGING
	printf("There are %lu ploygons. \n", proVertexVec.size());
	//--------------------debugging-------------------------
	for (unsigned int i = 0; i < proVertexVec.size(); ++ i) {
		printf("There are %lu vertices in vector %u. \n", proVertexVec[i].size(), i);
		for ( unsigned int j = 0; j < proVertexVec[i].size(); ++ j) {
			printf("vectex (%u, %u) located in (%u, %u)", i, j, proVertexVec[i][j].pos.x, proVertexVec[i][j].pos.y);
			switch (proVertexVec[i][j].explorationFlag) {
			case WALKABLE:
				printf("is walkable");
				break;
			case UN_WALKABLE:
				printf("is unwalkable");
				break;
			case UN_KNOWN:
				printf("is unknown");
				break;
			case RESOURCE:
				printf("is resource");
				break;
			default:
				printf("is error");
				break;
			}
			printf("\n");
		}
	}
	//------------------------------------------------------
#endif


	for( unsigned int i = 0; i < proVertexVec.size(); ++ i){

		unsigned int insertAcount = 0;
		edgeNode iter;
		edgeNode trail;


		for ( unsigned int j = 0; j < proVertexVec[i].size(); ++ j) {

			TilePosition ptPoint1;
			TilePosition ptPoint2;

			iter = proVertexVec[i][j];

			if ( j < proVertexVec[i].size() - 1) {
				trail = proVertexVec[i][j+1];
			}else{
				// printf("reach the end of List\n");
				trail = proVertexVec[i][0];

			}
			//------------------------------------

			if ( lineInterCircle( iter.pos, trail.pos, target, scout->getType().sightRange() / 32, ptPoint1, ptPoint2)) {
				//---------------------------------------
				//insertAcount ++;
				//printf("-----%u------iteration---%u-----", insertAcount,j);
				//printf("begin point (%u, %u), end point (%u, %u)\n", iter.pos.x, iter.pos.y, trail.pos.x, trail.pos.y);
				//--------------------------------------

				if ( ptPoint1.x() > 0 && ptPoint1.x() < Broodwar->mapWidth()
					&& ptPoint1.y() > 0 && ptPoint1.y() < Broodwar->mapHeight()) {

						edgeNode hp1Node;
						hp1Node.pos = ptPoint1;
						hp1Node.explorationFlag = evaluateEdgePoint(hp1Node.pos);

						try {
							proVertexVec[i].insert( proVertexVec[i].begin() + j, hp1Node);
						} catch (...) {
							proVertexVec[i].insert( proVertexVec[i].begin() + j, hp1Node);
						}

						++j;
						//----------------------------------------
						//printf("First insert position (%u, %u) \n", ptPoint1.x, ptPoint1.y);
						//----------------------------------------
				}

				if ( ptPoint2.x() > 0 && ptPoint2.x() < Broodwar->mapWidth()
					&& ptPoint2.y() > 0 && ptPoint2.y() < Broodwar->mapHeight()) {

						edgeNode hp2Node;
						hp2Node.pos = ptPoint2;
						hp2Node.explorationFlag = evaluateEdgePoint(hp2Node.pos);

						try {
							proVertexVec[i].insert( proVertexVec[i].begin() + j, hp2Node);
						} catch (...) {
							proVertexVec[i].insert( proVertexVec[i].begin() + j, hp2Node);
						}

						++ j;
						//------------------------------------------
						//printf("Second insert position (%u, %u)\n", ptPoint2.x, ptPoint2.y);
						//------------------------------------------
				}
			}

		}

	}

	for( unsigned int i = 0; i < proVertexVec.size(); ++ i){

		edgeNode iter;
		edgeNode trail;

		for ( unsigned int j = 0; j < proVertexVec[i].size(); ++j) {

			iter = proVertexVec[i][j];
			if ( j < proVertexVec[i].size() ) {
				trail = proVertexVec[i][j+1];
			}else{
				trail = proVertexVec[i][0];
			}

			float distanceP1 = sqrt(pow((double)(iter.pos.x() - buildPosition.x()), 2.0)
				+ pow((double)(iter.pos.y() - buildPosition.y()), 2.0));

			float distanceP2 = sqrt(pow((double)(trail.pos.x() - buildPosition.x()), 2.0)
				+ pow((double)(trail.pos.y() - buildPosition.y()), 2.0));

			if ( ( distanceP1 <= sightRange + 2 ) && ( distanceP2 <= sightRange + 2)) {
				double distance_ = iter.pos.getDistance(trail.pos);
				if ( iter.explorationFlag == UN_WALKABLE
					&& trail.explorationFlag == UN_WALKABLE) {
						sumExplored += distance_;
				}else{
					sumUnexplored += distance_;
				}

			}

		}

	}

	if ( sumUnexplored + sumExplored < EPS) {
		return 0.0;
	}else{
		return sumUnexplored / ( sumUnexplored + sumExplored );
	}

}

/*
void esStrategy::recalSegVertices(Unit* scout, TilePosition& target){

}
*/

/*
*  Note: All the tile-based calculation for exploration is running 
*        on build tiles, because the explored and visiable states are
*        only working for build tiles in Brood War API.
*  @ vertex : vertex on the outline polygon of explored areas.
*/
unsigned char esStrategy::evaluateEdgePoint(const TilePosition &vertex){

	int sumUnknown = 0;
	int sumWalkable = 0;
	int sumObstacle = 0;

	int width = Broodwar->mapWidth();
	int height = Broodwar->mapHeight();

	for (int i = vertex.x() - 1; i <= vertex.x() + 1; ++ i) {
		for ( int j = vertex.y() - 1; j <= vertex.y() + 1; ++j) {

			if ( i < 0 || i > width
				|| j < 0 || j > height) {
					sumObstacle ++;
					continue;
			}

			if ( Broodwar-> isExplored(i, j)){
				if( evaluateBuildTile( TilePosition(i, j)) == WALKABLE){
					sumWalkable ++;
				}else{
					sumObstacle ++;
				}
			}else{
				sumUnknown ++;
			}
		}
	}

	if ( sumObstacle == sumUnknown &&
		sumObstacle == 0) {
			return WALKABLE;
	}else if( sumObstacle != 0){
		return UN_WALKABLE;
	}else{
		return UN_KNOWN;
	}
}

/*
*  Note:Evaluate the walkability of a build tile using 
*       the walkability of the walk tiles in it.
*       4 * 4 walk tiles make up a build tile.
*  @ pos : the target build tile
*/
unsigned char esStrategy::evaluateBuildTile(const TilePosition& pos){

	int sumWalkable = 0;

	for( int i = pos.x() * 4; i < ( pos.x() + 1) * 4; ++ i ){
		for( int j = pos.y() * 4; j < ( pos.y() + 1) * 4; ++ j ){
			if( Broodwar -> isWalkable(i, j)){
				sumWalkable++; 
			}
		}
	}

	if (sumWalkable / 16.0 >= 0.5){
		return WALKABLE;
	}else{
		return UN_WALKABLE;
	}
}

/*
*  Note: Calculate the intersected points for 
*
*/
bool esStrategy::lineInterCircle(const TilePosition &ptStart, const TilePosition &ptEnd, const TilePosition &ptCenter,
								 float Radius, TilePosition &ptInter1, TilePosition &ptInter2){

									 ptInter1.x() = ptInter1.y() = 65535;
									 ptInter2.x() = ptInter2.y() = 65535;

									 if ( ptStart.getDistance(ptCenter) < Radius
										 && ptEnd.getDistance(ptCenter) < Radius) {
											 return false;
									 }

									 float fDis = sqrt(pow((double)(ptEnd.x() - ptStart.x()), 2.0) + pow((double)(ptEnd.y() - ptStart.y()), 2.0));

									 float d_x = ( ptEnd.x() - ptStart.x()) / fDis;
									 float d_y = ( ptEnd.y() - ptStart.y()) / fDis;

									 float e_x = (float)(ptCenter.x() - ptStart.x());
									 float e_y = (float)(ptCenter.y() - ptStart.y());

									 float a = e_x * d_x + e_y * d_y;
									 float a2 = a * a;

									 float e2 = e_x * e_x + e_y * e_y;

									 float r2 = Radius * Radius;

									 if ((r2 - e2 + a2) < 0) {

										 return false;

									 }else{

										 float f =sqrt(r2 - e2 + a2);

										 float t = a - f;

										 if ( ((t - 0.0) > -EPS) && (t - fDis) < EPS) {
											 ptInter1.x() = ptStart.x() + t * d_x;
											 ptInter1.y() = ptStart.y() + t * d_y;
										 }

										 t = a + f;

										 if ( ((t - 0.0) > -EPS) && ( t - fDis) < EPS) {
											 ptInter2.x() = ptStart.x() + t * d_x;
											 ptInter2.y() = ptStart.y() + t * d_y;
										 }

										 return true;
									 }
}

float esStrategy::calculateFeaPercentage(const Unit * scout, const TilePosition &target){
	//calculate
	int sumUnexplored = 0;
	int sum = 0;
	int sumResource = 0;
	int sightRange = scout->getType().sightRange();
	sightRange /= 32;

	for( int i = - sightRange; i <= sightRange; i++){
		for( int j = - sightRange; j <= sightRange; j++){
			if (sqrt( pow((double)i, 2.0) + pow((double)j, 2.0)) <= sightRange
				&& ( i + target.x() ) > 0 && (i + target.x()) < Broodwar->mapWidth()
				&& ( j + target.y() ) > 0 && (j + target.y()) < Broodwar->mapHeight()){
					sum ++;
					if( !Broodwar->isExplored(i + target.x(), j + target.y())){
						sumUnexplored ++;
					}else if( isResourceTaken(target)){
						sumResource ++;
					}
			}
		}
	}

	if ( sum == 0 ) {
		return 0;
	}else{
		double sumD = (double)sum;
		double sumExplored = (double)(sum - sumUnexplored);
		return ( sumResource * sumUnexplored) / ( sumD * sumExplored );
	}

#if 0
	if ( sum == 0 || sum == sumUnexplored) {
		return 0;
	}else{
		double sumD = (double)sum;
		double sumExplored = (double)(sum - sumUnexplored);
		double exploredPercent = sumResource / sumExplored;
		if ( exploredPercent < 0.1) {
			return 4 * ( sumResource * sumUnexplored) / ( sumD * sumExplored );
		}else if( exploredPercent >= 0.1 && exploredPercent <= 0.5){
			return 2 * ( sumResource * sumUnexplored) / ( sumD * sumExplored );
		}else{
			return ( sumResource * sumUnexplored) / ( sumD * sumExplored );
		}

	}
#endif
}

bool esStrategy::isReachable(std::set<esPolygon *> polygons, const TilePosition & p1, const TilePosition & p2){

	std::set<esPolygon*>::iterator it = polygons.begin();
	std::set<esPolygon*>::const_iterator end = polygons.end();

	for ( ; it != end; ++it) {
		if ( (*it)->isInside(p1)
			&& (*it)->isInside(p2)) {
				return true;
		}
	}

	return false;
}

esStrategy::~esStrategy(){
	//printf("call esStrategy destructor!\n");
	visitNodes.clear();
}

esStrategy::esStrategy(){

}

void esStrategy::setAlgorithmConfig(algorithmConfig confi){
	config = confi;
}

TilePosition esStrategy::searchWalkableNeighbor(const TilePosition & original){
	if ( evaluateBuildTile(original) == WALKABLE) {
		return original;
	}else{
		for(int i = original.x() - 2; i <= original.x() + 2; i ++){
			for (int j = original.y() - 2; j <= original.y() + 2; j ++) {
				if ( i < 0 || i > Broodwar->mapWidth() || j < 0 || j > Broodwar->mapHeight()) {
					continue;
				}else{
					TilePosition tp = TilePosition(i, j);
					if (evaluateBuildTile( tp ) == WALKABLE) {
						return tp;
					}
				}
			}
		}
		return original;
	}
}

/*
*  Note: To check whether one tile position is taken by 
*        a resource block. 
*
*/
bool esStrategy::isResourceTaken(const TilePosition & p){
	return isMineralsTaken(p) || isGeysersTaken(p);  
}

/*
* Note: According to the note in Game.h of bwapi 3,
*       the function getminerals() means get the accessible(visible) 
*       minerals. Yet, it is not sure what the visible, here, exactly 
*		mean.
*/
bool esStrategy::isMineralsTaken(const TilePosition & p){

	std::set<Unit*> mineralSet = Broodwar->getStaticMinerals();

	std::set<Unit*>::iterator iter = mineralSet.begin();
	std::set<Unit*>::const_iterator iter_end = mineralSet.end();

	int left = p.x() * 32;
	int right = (p.x() + 1) * 32 - 1;
	int top = p.y() * 32;
	int bottom = (p.y() + 1) * 32 -1;

	for(; iter != iter_end; ++iter){
		// collision detection judgement between the mineral boundary and the build Tile
		int mLeft = (*iter)->getLeft();
		int mRight = (*iter)->getRight();
		int mTop = (*iter)->getTop();
		int mBottom = (*iter)->getBottom();

		if(!( left > mRight || top > mBottom || mLeft > right || mTop > bottom )){
			return true;
		}

	}

	return false;
}

/*
* Note: According to the note in Game.h of bwapi 3,
*       the function getGeyser() means get the accessible(visible) 
*       geyser. Yet, it is not sure what the visible, here, exactly 
*		mean.
*/
bool esStrategy::isGeysersTaken(const TilePosition & p){

	std::set<Unit*> gayserSet = Broodwar->getStaticGeysers();

	std::set<Unit*>::iterator iter = gayserSet.begin();
	std::set<Unit*>::const_iterator iter_end = gayserSet.end();

	int left = p.x() * 32;
	int right = (p.x() + 1) * 32 - 1;
	int top = p.y() * 32;
	int bottom = (p.y() + 1) * 32 -1;

	for(; iter != iter_end; ++iter){
		// collision detection judgement between the geysers boundary and the build Tile
		int mLeft = (*iter)->getLeft();
		int mRight = (*iter)->getRight();
		int mTop = (*iter)->getTop();
		int mBottom = (*iter)->getBottom();

		if(!( left > mRight || top > mBottom || mLeft > right || mTop > bottom )){
			return true;
		}  
	}

	return false;
}
