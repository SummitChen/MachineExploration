#pragma once

#include "Common.h"
#include "BuildingData.h"
#include "MetaType.h"
//#include "../InformationManager.h"

class BuildingPlacer
{
private:
	BuildingPlacer();

	std::vector< std::vector<bool> > reserve_map;
	int build_distance;

	int					box_top, 
						box_bottom, 
						box_left, 
						box_right;

	static BuildingPlacer * bp_instance;

 public:

	static BuildingPlacer * get_instance();

	static void destroy_instance();

	// queries for various BuildingPlacer data
	bool					buildable(int x, int y) const;
	bool					is_reserved(int x, int y) const;
	bool					is_in_resource_box(int x, int y) const;
	bool					tile_overlaps_base_location(BWAPI::TilePosition tile, BWAPI::UnitType type) const;

	// determines whether we can build at a given location
	bool					can_build_here(BWAPI::TilePosition position, const Building & b) const;
	bool					can_build_here_with_space(BWAPI::TilePosition position, const Building & b, int buildDist, bool horizontalOnly = false) const;

	// returns a build location near a building's desired location
	BWAPI::TilePosition		get_build_location_near(const Building & b, int buildDist, bool inRegion = false, bool horizontalOnly = false) const;
	
	void					reserve_tiles(BWAPI::TilePosition position, int width, int height);
	void					free_tiles(BWAPI::TilePosition position, int width, int height);
	void					set_build_distance(int distance);
	int						get_build_distance() const;
	
	void					draw_reserved_tiles();
	void					compute_resource_box();
	
	BWAPI::TilePosition		get_refinery_position();
	
};

