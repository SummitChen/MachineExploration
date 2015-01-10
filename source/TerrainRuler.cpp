#include "TerrainRuler.h"
#include "Common.h"

TerrainRuler* TerrainRuler::tr_instance = NULL;

TerrainRuler::~TerrainRuler(void)
{
}


TerrainRuler* TerrainRuler::get_instance()
{
	if ( tr_instance == NULL)
	{
		tr_instance = new TerrainRuler();
	}

	return tr_instance;
}

void TerrainRuler::destroy_instance()
{
	if ( tr_instance != NULL)
	{
		delete tr_instance;
		tr_instance = NULL;
	}
}

void TerrainRuler::initialization()
{
	BWTA::readMap();
	BWTA::analyze();

	map_width = BWAPI::Broodwar->mapWidth();                       // building tiles
	map_width *= ( 32 / RATIO );                                                    // convert to walk tiles
	map_height = BWAPI::Broodwar->mapHeight();                     // building tiles
	map_height *= ( 32 / RATIO);                                                   // convert to walk tiles

	ter_tiles_map = new TerrainTile**[map_height];
	for (int i = 0; i < map_height; ++i)
	{
		ter_tiles_map[i] = new TerrainTile*[map_width];
		for (int j = 0; j < map_width; ++j)
		{
			ter_tiles_map[i][j] = new TerrainTile;
			ter_tiles_map[i][j]->pass_flag = PASS_UNKONW;
			ter_tiles_map[i][j]->pos = BWAPI::Position( RATIO / 2 + i * RATIO , RATIO / 2 + j * RATIO);
			ter_tiles_map[i][j]->potential_value = -2;
		}
	}
	
	is_initialized = true;

}

// to be continue...
bool TerrainRuler::region_detection( BWAPI::Unit* unit )
{
	if ( unit == NULL)
	{
		assert(unit);
	}
	
	double2 unit_pos( unit->getPosition() );
	double2 region_center;
	double2 direct_vector;
	
	BOOST_FOREACH( BWTA::Region* region, BWTA::getRegions()){
		
		region_center.x = region->getCenter().x();
		region_center.y = region->getCenter().y();
		
		direct_vector = unit_pos - region_center;

		if ( direct_vector.len() <= 32)
		{
			
		}

	}

	return false;
}

const std::set<Region*>& TerrainRuler::get_regions()
{
	return BWTA::getRegions();
}

const std::set<Chokepoint*>& TerrainRuler::get_choke_points()
{
	return BWTA::getChokepoints();
}

const std::set<BaseLocation*>& TerrainRuler::get_base_locations()
{
	return BWTA::getBaseLocations();
}

const std::set<BaseLocation*>& TerrainRuler::get_start_locations()
{
	return BWTA::getStartLocations();
}

const std::set<Polygon*>& TerrainRuler::get_unwalkable_polygons()
{
	return BWTA::getUnwalkablePolygons();
}

TerrainRuler::TerrainRuler( void )
{
	ter_tiles_map = NULL;
	is_initialized = false;
}

BWTA::BaseLocation* TerrainRuler::get_start_location( BWAPI::Player* player )
{
	return BWTA::getStartLocation( player );
}

bool TerrainRuler::get_initialized_flag()
{
	return is_initialized;
}

size_t TerrainRuler::get_map_height()
{
	return map_height;
}

size_t TerrainRuler::get_map_width()
{
	return map_width;
}

TerrainTile*** TerrainRuler::get_tiles_map()
{
	return ter_tiles_map;
}


