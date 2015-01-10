//
//  esPolygon.h
//  ExplorationStrategy
//
//  Created by Chen Si on 5/07/2014.
//  
//  Note: To be extended with Template  
//

#ifndef ExplorationStrategy_esPolygon_h
#define ExplorationStrategy_esPolygon_h

#include <vector>
#include "BWAPI/Position.h"
#include "BWAPI/TilePosition.h"

using namespace BWAPI;

class esPolygon{
public:
    esPolygon();
    esPolygon(const esPolygon & b);
    
    
    double           getArea() const;
    double           getPerimeter() const;
    
    TilePosition          getCenter() const;
    TilePosition          getNearestPoint(const TilePosition & p) const;
    
    std::vector<esPolygon> getHoles() const;
    
    bool             isInside( const TilePosition & p) const;
    
    std::vector<TilePosition> pointSet;
    
    std::vector<esPolygon> holes;
};

#endif
