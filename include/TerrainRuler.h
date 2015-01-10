#pragma once
#include "Common.h"
using namespace BWTA;

#define  PASS_UNKONW      0
#define  PASS_WALKABLE    1
#define  PASS_UNWALKABLE  2

#define RATIO 32
#define WALKTILE_RATE 8
#define BUILDINGTILE_RATE 32

struct TerrainTile{

	BWAPI::Position pos;
	int potential_value;
	//enum Passibility pass_flag;

	double a_potential_value;
	double obstacle_value;
	double resource_value;
	double unexplored_value;

    int pass_flag;
};

class MapData{
public:
	MapData(void){

	}
	~MapData(void){

	}

    ChokeSet detected_chokepoints;
	RegionSet detected_regions;	
	BWTA::Region* enemy_base;

};

class TerrainRuler
{
public:
	static TerrainRuler*                         get_instance();
	static void                                  destroy_instance();

	void                                         initialization();
	bool                                         region_detection(BWAPI::Unit*);
	bool                                         chokepoint_detection(BWAPI::Unit*);
	bool                                         enemy_base_detection(BWAPI::Unit*);
	
	const std::set<BWTA::Region*>&               get_regions();
	const std::set<Chokepoint*>&                 get_choke_points();
	const std::set<BaseLocation*>&               get_base_locations();
	const std::set<BaseLocation*>&               get_start_locations();
	const std::set<BWTA::Polygon*>&              get_unwalkable_polygons();
	
	BWTA::BaseLocation*                          get_start_location(BWAPI::Player* player);
	TerrainTile***                               get_tiles_map();

	size_t                                       get_map_height();
	size_t                                       get_map_width();

	bool                                         get_initialized_flag();

private:
	static TerrainRuler*                         tr_instance;
	MapData                                      detected_map_data;
	TerrainTile***                               ter_tiles_map;
	size_t                                       map_height;
	size_t                                       map_width;
	bool                                         is_initialized;
	TerrainRuler(void);
	~TerrainRuler(void);


};

