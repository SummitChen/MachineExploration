//
//  esConnectedComponent.h
//  ExplorationStrategy
//
//  Created by Chen Si on 4/07/2014.
//
//

#ifndef ExplorationStrategy_esConnectedComponent_h
#define ExplorationStrategy_esConnectedComponent_h

#include "BWAPI/position.h"

using namespace BWAPI;

class esConnectedComponent{

public:
	esConnectedComponent(int id, bool walkable);

	bool                 isWalkable(void) const;
	void                 setWalkable(bool walkability);

	int                  getID(void) const;

	Position             topLeft()   const;
	void                 setTopLeft(Position topLeftTile);

private:
	Position             _topLeftTile;
	bool                 _walkability;
	int                  _id;

};



#endif
