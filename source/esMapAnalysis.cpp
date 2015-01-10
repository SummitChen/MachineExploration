//
//  esMapAnalysis.cpp
//  ExplorationStrategy
//
//  Created by Chen Si on 14/07/2014.
//
//


#include <stddef.h>
#include "bwapi.h"
#include "esMapAnalysis.h"
#include "RectangleArray.h"
#include "esExtractPolygons.h"
#include "esConnectedComponent.h"
//#include "TerrainDetectorModule.h"

using namespace BWAPI;

esMapAnalysis* esMapAnalysis::instance = NULL;

esMapAnalysis::esMapAnalysis(){

}

esMapAnalysis::~esMapAnalysis(){

}

esMapAnalysis* esMapAnalysis::getInstance(){
    if ( instance == NULL) {
        instance = new esMapAnalysis();
    }
    
    return instance;
}

void esMapAnalysis::deleteInstance(){
    
    instance->clearData();
    
    if ( instance != NULL) {
        delete instance;
        instance = NULL;
    }
}


void esMapAnalysis::draw(const esPolygon &polygon){
	/*
    ofSetPolyMode(OF_POLY_WINDING_NONZERO);
    ofBeginShape();
    for (unsigned int i = 0; i < polygon.pointSet.size(); i ++) {
        ofVertex(polygon.pointSet[i].x, polygon.pointSet[i].y);
    }
    for (unsigned int j = 0; j < polygon.holes.size(); j++) {
        draw(polygon.holes[j]);
    }
    ofEndShape();
	*/
}

void esMapAnalysis::draw(const std::set<esPolygon *> &polygons){
	/*
    ofPushStyle();
    ofSetColor(255, 0, 0);
    ofSetLineWidth(5.0);
    ofNoFill();
    std::set<esPolygon *>::iterator cursor = polygons.begin();
    std::set<esPolygon *>::const_iterator end = polygons.end();
    
    for (; cursor != end;  ++ cursor) {
        draw(*(*cursor));
        //break;
    }
    
    ofPopStyle();
	*/
}

void esMapAnalysis::draw(){
    //draw(unwalkablePolygons);
    //draw(exploredPolygons);
    //debugDraw();
}

void esMapAnalysis::debugDraw(){
 //To test the correction of walkablity
	/*
    ofPushStyle();
    ofSetColor(255, 255, 0);
    ofSetLineWidth(5.0);
    debugmesh.draw();
    ofPopStyle();
	*/
}

void esMapAnalysis::analyzeExploredArea(/*ofxTileMap *dynamicMap*/){
    clearPolygonSet(exploredPolygons);
    
    //-------------------debugging----------------------
    unsigned int size = exploredPolygons.size();
    
    KERNEL::RectangleArray<esConnectedComponent*> getComponent;
    std::list<esConnectedComponent> components;
    

	bool* unexploredData = new bool[ Broodwar->mapWidth() * Broodwar->mapHeight()];

	for ( unsigned int i = 0; i < Broodwar->mapWidth(); i ++) {
		for (unsigned int j = 0; j < Broodwar->mapHeight(); j ++) {
			if ( Broodwar->isExplored(i, j)) {
				unexploredData[i * Broodwar->mapHeight() + j] = false;
			}else{
				unexploredData[i * Broodwar->mapHeight() + j] = true;
			}
		}
	}

	KERNEL::RectangleArray<bool> unexploredArray(Broodwar->mapWidth(), Broodwar->mapHeight(), unexploredData);

	findConnectedComponents(unexploredArray, getComponent, components);

	std::vector<esPolygon> polygons;

	extractPolygons(unexploredArray, components, polygons);

	// Discard polygons that are too small
	for ( unsigned int p = 0; p < polygons.size(); ) {
		/*
		if (abs(polygons[p].getArea()) <= 2)
		{
			polygons.erase(polygons.begin() + p);
		}
		else
		{
			p++;
		}
		*/
		p++;
	}

	// Save the remaining polygons
	for( unsigned int i = 0; i < polygons.size(); i++){
		exploredPolygons.insert(new esPolygon(polygons[i]));
	}

	//-clear-data--------------
	delete [] unexploredData;
        
#if 0
        //------------debugging---------------------------------
        printf("Analyzed explored polygon size %lu \n", exploredPolygons.size());
        
        std::set<esPolygon*>::iterator pit = exploredPolygons.begin();
        std::set<esPolygon*>::iterator pend = exploredPolygons.end();
        
        for ( unsigned int i = 0; pit != pend; ++ pit, ++ i) {
            printf("Analyzed polygon %u size %lu \n", i, (*pit)->pointSet.size());
            if ( (*pit)->getHoles().size() != 0) {
                printf("Analyzed polygon %u holes size %lu \n", i, (*pit)->getHoles().size());
                for ( unsigned int j = 0; j < (*pit)->getHoles().size(); ++ j) {
                    printf("Analyzed polygon %u hole %u size %lu \n", i, j, (*pit)->getHoles()[j].pointSet.size());
                }
            }
        }
#endif
}

void esMapAnalysis::clearPolygonSet(std::set<esPolygon *> &polygonSet){
    std::set<esPolygon*>::iterator it = polygonSet.begin();
    std::set<esPolygon*>::const_iterator end = polygonSet.end();
    for (; it != end; ++it) {
        if ((*it) != NULL) {
            delete (*it);
        }
    }
    
    polygonSet.clear();
}


std::set<esPolygon*>& esMapAnalysis::getExploredPolygons(){
    return exploredPolygons;
}

void esMapAnalysis::clearExploredPolygons(){
    clearPolygonSet(exploredPolygons);
    //debugMesh2.clear();
}

void esMapAnalysis::clearUnwalkablePolygons(){
    clearPolygonSet(unwalkablePolygons);
}

std::set<esPolygon*>& esMapAnalysis::getUnwalkablePolygons(){
    return unwalkablePolygons;
}

void esMapAnalysis::clearData(){
    clearExploredPolygons();
    clearUnwalkablePolygons();
    //debugmesh.clear();
}
