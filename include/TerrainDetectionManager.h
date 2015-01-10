#pragma once
#include "BWTA.h"
#include "Common.h"
#include <fstream>
#include "BWAPI/TilePosition.h"
#include "esStrategy.h"

using namespace BWAPI;

typedef std::vector<BWAPI::Position> Patch;

//enum  {unkonw, walkable, unwalkable};

//enum Plan_Status { };
struct tile_coordiante{
	int x;
	int y;
};

struct ScoutProperty
{
	bool                          moving_flag;
	bool                          navigation_flag;
	bool                          contral_flag;                        //TRUE high level control FALSE low level control
	Position                      destination;
	double2                       direction;
	size_t                        tile_index_x;
	size_t                        tile_index_y;
	int                           pre_tile_x;
	int                           pre_tile_y;
	Position                      pre_pos;
	std::vector<tile_coordiante>  pre_visited_tiles;	
};



class TerrainDetectionManager
{
public:
	static TerrainDetectionManager*                           get_instance();
	static void                                               destroy_instance();

	const std::set<BWTA::Region*>&                            get_regions();
	const std::set<BWTA::Chokepoint*>&                        get_choke_points();
	const std::set<BWTA::BaseLocation*>&                      get_base_locations();
	const std::set<BWTA::BaseLocation*>&                      get_start_locations();
	const std::set<BWTA::Polygon*>&                           get_unwalkable_polygons();
	BWTA::BaseLocation*                                       get_start_location();

	void                                                      single_scout_detection(BWAPI::Unit* unit, BWAPI::Position _distance);

	void                                                      insert_region(BWTA::Region* r);
	void                                                      insert_choke_point(BWTA::Chokepoint* cp);
	void                                                      insert_base_location(BWTA::BaseLocation* bl);
	void                                                      insert_start_location(BWTA::BaseLocation* sl);
	void                                                      insert_unwalkable_polygons(BWTA::Polygon* up);
	void                                                      destribute_terrain_info();

	void                                                      initialization();
	void                                                      on_time();

	void                                                      low_level_patch_merge();
	void                                                      high_level_patch_merge();
	void                                                      region_recogonition();
	void                                                      update_grid_info();

	void                                                      high_level_planning();
	void                                                      low_level_navigation();
	void                                                      calculate_possible_startlocations();
	std::map<BWAPI::Unit*, ScoutProperty*>                    get_scouts_map();

	//debugging information
	void                                                      debugging_info_assign();
	Patch                                                     getStaticPatchBuffer();

private:
	static TerrainDetectionManager*                           tdm_instance;
	TerrainDetectionManager(void);
	~TerrainDetectionManager(void);
		
	/*
	*  In the exploration application, computation is running on the level of build tile.
	*  So, a specific position should be selected as the target for a unit according to BWAPI, 
	*  when the target tile is ready.
 	*  @ unit - scout
	*  @ tile - the target tile
	*/
    void                                                      move_to_tile(Unit* unit, TilePosition tile);

	std::set<BWTA::Region*>                                   regions;
	std::set<BWTA::Chokepoint*>                               chokepoints;
	std::set<BWTA::BaseLocation*>                             baselocations;
	std::set<BWTA::BaseLocation*>                             startlocations;
	std::set<BWTA::Polygon*>                                  unwalkablePolygons; 
	std::map<BWAPI::Unit*, ScoutProperty*>                    scouts_map;
	
	
	BWTA::BaseLocation*                                       start_location;
	ScoutProperty                                             scout_property;

	Patch                                                     static_patch_buffer;
	Patch                                                     dynamic_patch_buffer;

	std::vector<BWAPI::Position>                              poss_startlocations; 
	bool                                                      initial_flag;

	TilePosition                                              temp_target;
	bool                                                      update_flag;
	bool                                                      reset_flag;
	esStrategy                                                *strategy;


	// debug 
	bool                                                      debug_flag; 
	std::fstream                                              out;
	std::fstream                                              middle_level_control;
	std::fstream                                              recognition_debug_log;
	std::string                                               file_name;

};

