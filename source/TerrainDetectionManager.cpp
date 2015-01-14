#include "TerrainDetectionManager.h"
#include "Common.h"
#include "TerrainRuler.h"
#include <vector>
#include "time.h"
#include "esStrFBMC.h"


using namespace BWTA;


bool quadratic_solver(float p_a, float p_b, float p_c, float & x_1, float & x_2){
	float p_d = ( p_b * p_b ) - ( 4 * p_a * p_c);
	if ( p_d >= 0)
	{
		x_1 = ( -p_b + sqrt( p_d ))/ (2 * p_a);
		x_2 = ( -p_b - sqrt( p_d ))/ (2 * p_a);

		return true;
	}else
	{
		return false;
	}
}

TerrainDetectionManager* TerrainDetectionManager::tdm_instance = NULL;

TerrainDetectionManager::TerrainDetectionManager(void)
{
	start_location = TerrainRuler::get_instance()->get_start_location( BWAPI::Broodwar->self());

	scout_property.moving_flag = false;
	scout_property.navigation_flag = false;

	initial_flag = false;

	initialization();
}


TerrainDetectionManager::~TerrainDetectionManager(void)
{
#if 1
	if(strategy != NULL){
		delete strategy;
		strategy = NULL;
	}
#endif
}


TerrainDetectionManager* TerrainDetectionManager::get_instance()
{
	if ( tdm_instance == NULL)
	{
		tdm_instance = new TerrainDetectionManager();
	}

	return tdm_instance;
}

void TerrainDetectionManager::destroy_instance()
{
	if ( tdm_instance != NULL)
	{
		delete tdm_instance;
		tdm_instance = NULL;
	}
}

const std::set<BWTA::Region*>& TerrainDetectionManager::get_regions()
{
	return regions;
}

const std::set<BWTA::Chokepoint*>& TerrainDetectionManager::get_choke_points()
{
	return chokepoints;
}

const std::set<BWTA::BaseLocation*>& TerrainDetectionManager::get_base_locations()
{
	return baselocations;
}

const std::set<BWTA::BaseLocation*>& TerrainDetectionManager::get_start_locations()
{
	return startlocations;
}

const std::set<BWTA::Polygon*>& TerrainDetectionManager::get_unwalkable_polygons()
{
	return unwalkablePolygons;
}

// without considering uniqueness
void TerrainDetectionManager::insert_region( BWTA::Region* r )
{
	regions.insert(r);
}

// without considering uniqueness
void TerrainDetectionManager::insert_choke_point( BWTA::Chokepoint* cp )
{
	chokepoints.insert(cp);
}

// without considering uniqueness
void TerrainDetectionManager::insert_base_location( BWTA::BaseLocation* bl )
{
	baselocations.insert(bl);
}

// without considering uniqueness
void TerrainDetectionManager::insert_start_location( BWTA::BaseLocation* sl )
{
	startlocations.insert(sl);
}

// without considering uniqueness
void TerrainDetectionManager::insert_unwalkable_polygons( BWTA::Polygon* up )
{
	unwalkablePolygons.insert(up);
}

void TerrainDetectionManager::destribute_terrain_info()
{
	BOOST_FOREACH( BWTA::Region* r, TerrainRuler::get_instance()->get_regions()){
		regions.insert(r);
	}

	BOOST_FOREACH( BWTA::Chokepoint* r, TerrainRuler::get_instance()->get_choke_points()){
		chokepoints.insert(r);
	}

	BOOST_FOREACH( BWTA::BaseLocation* r, TerrainRuler::get_instance()->get_base_locations()){
		baselocations.insert(r);
	}

	BOOST_FOREACH( BWTA::BaseLocation* r, TerrainRuler::get_instance()->get_start_locations()){
		startlocations.insert(r);
	}

	BOOST_FOREACH( BWTA::Polygon* r, TerrainRuler::get_instance()->get_unwalkable_polygons()){
		unwalkablePolygons.insert(r);
	}

}

BWTA::BaseLocation* TerrainDetectionManager::get_start_location()
{
	return start_location;
}

void TerrainDetectionManager::single_scout_detection( BWAPI::Unit* unit, BWAPI::Position _destination )
{
	assert(unit);
	ScoutProperty* property_ = new ScoutProperty();
	property_->contral_flag = true;
	property_->moving_flag = false;
	property_->navigation_flag = false;
	property_->destination = _destination;
	scouts_map.insert( std::make_pair(unit, property_));
}

void TerrainDetectionManager::low_level_patch_merge()
{
	if ( scouts_map.size() == 0)
	{
		return;
	}

	Patch temp;
	std::pair<BWAPI::Unit*, ScoutProperty*> unit_pair;
	BOOST_FOREACH(unit_pair, scouts_map){
		BWTA::Region* region = getRegion( unit_pair.first->getPosition());
		BOOST_FOREACH(BWAPI::Position pos, region->getPolygon() ){
			if ( (unit_pair.first->getPosition() - pos).getLength() < unit_pair.first->getType().sightRange())
			{
				temp.push_back(pos);
			}
		}
	}

	dynamic_patch_buffer.insert( dynamic_patch_buffer.end(), temp.begin(), temp.end());
	sort( dynamic_patch_buffer.begin(), dynamic_patch_buffer.end());
	dynamic_patch_buffer.erase( unique( dynamic_patch_buffer.begin(), dynamic_patch_buffer.end()), dynamic_patch_buffer.end());
}

void TerrainDetectionManager::high_level_patch_merge()
{
	if ( dynamic_patch_buffer.size() == 0)
	{
		return;
	}

	static_patch_buffer.insert( static_patch_buffer.end(), dynamic_patch_buffer.begin(), dynamic_patch_buffer.end());
	sort( static_patch_buffer.begin(), static_patch_buffer.end());
	static_patch_buffer.erase( unique( static_patch_buffer.begin(), static_patch_buffer.end()), static_patch_buffer.end());

	dynamic_patch_buffer.clear();
}

void TerrainDetectionManager::region_recogonition()
{

	if ( static_patch_buffer.size() == 0)
	{
		return;
	}

	//recogonise region
	std::set<BWTA::Region*> temp_regions; 
	std::set<BWTA::Chokepoint*> temp_chokepoints;

	BOOST_FOREACH( BWTA::Region* r, TerrainRuler::get_instance()->get_regions()){

		int iterator_region = 0;
		int size_region = r->getPolygon().size();

		for ( ; iterator_region < size_region; ++iterator_region)
		{
			BWAPI::Position p = *( r->getPolygon().begin() + iterator_region);
			int iterator_patch = 0;
			int size_patch = static_patch_buffer.size();
			BWAPI::Position pos;
			for (; iterator_patch < size_patch; ++iterator_patch)
			{
				pos = *( static_patch_buffer.begin() + iterator_patch);
				if ( pos == p)
				{
					break;
				}
			}
			if ( iterator_patch == size_patch)
			{
				break;
			}
		}
		if( iterator_region < 0.8 * size_region){
			continue;
		}
		temp_regions.insert(r);
	}

	BOOST_FOREACH( BWTA::Chokepoint* cp, TerrainRuler::get_instance()->get_choke_points()){

		std::vector<BWAPI::Position> choke_positions;
		choke_positions.push_back( cp->getSides().first);
		choke_positions.push_back( cp->getSides().second);

		size_t iterator_chokepoint = 0;
		size_t size_chokepoint = 2;

		for ( ; iterator_chokepoint  < size_chokepoint; ++iterator_chokepoint )
		{
			BWAPI::Position p = *( choke_positions.begin() + iterator_chokepoint);
			size_t iterator_patch = 0;
			size_t size_patch = static_patch_buffer.size();
			BWAPI::Position pos;
			for (; iterator_patch < size_patch; ++iterator_patch)
			{
				pos = *( static_patch_buffer.begin() + iterator_patch);
				if ( pos == p)
				{
					break;
				}
			}
			if ( iterator_patch == size_patch)
			{
				break;
			}
		}
		if( iterator_chokepoint != size_chokepoint){
			continue;
		}
		temp_chokepoints.insert(cp);
	}


	if ( temp_regions.size() != 0)
	{
		BOOST_FOREACH(BWTA::Region* r, temp_regions){
			BOOST_FOREACH( BWAPI::Position p, r->getPolygon()){
				size_t iterator_static_buffer = 0;
				size_t size_static_buffer = static_patch_buffer.size();
				size_t num_deleted = 0;

				for ( ; iterator_static_buffer + num_deleted < size_static_buffer; )
				{
					BWAPI::Position pos = *( static_patch_buffer.begin() + iterator_static_buffer);
					if ( p == pos)
					{
						static_patch_buffer.erase( static_patch_buffer.begin() + iterator_static_buffer);
						num_deleted ++;
						continue;
					}else{
						iterator_static_buffer++;
					}

				}
			}
		}
	}

	if ( temp_chokepoints.size() != 0)
	{
		BOOST_FOREACH(BWTA::Chokepoint* cp, temp_chokepoints){
			std::vector<BWAPI::Position> sides_set;
			sides_set.push_back( cp->getSides().first);
			sides_set.push_back( cp->getSides().second);
			BOOST_FOREACH( BWAPI::Position p, sides_set){
				size_t iterator_static_buffer = 0;
				size_t size_static_buffer = static_patch_buffer.size();
				size_t num_deleted = 0;

				for ( ; iterator_static_buffer + num_deleted < size_static_buffer; )
				{
					BWAPI::Position pos = *( static_patch_buffer.begin() + iterator_static_buffer);
					if ( p == pos)
					{
						static_patch_buffer.erase( static_patch_buffer.begin() + iterator_static_buffer);
						num_deleted ++;
						continue;
					}else{
						iterator_static_buffer++;
					}

				}
			}
		}
	}

	regions.insert( temp_regions.begin(), temp_regions.end());
	chokepoints.insert( temp_chokepoints.begin(), temp_chokepoints.end());

	temp_regions.clear();
	temp_chokepoints.clear();

}


void TerrainDetectionManager::on_time()
{

	if ( initial_flag == false)
	{

		#ifdef DEBUGGING
		   Broodwar->drawTextScreen(200, 0, "Terrain Detection Manager is not initilized...");
        #endif
		return;
	}

#if 1

#ifdef DEBUGGING

	// S: whether the may has been analyzied in exploration loops.
	if(analyzed){
	   Broodwar->drawTextScreen( 5, 16 , "Analyzation : ON");
	}else{
	   Broodwar->drawTextScreen( 5, 16 , "Analyzation : OFF");
	}

	// S: Illustrate the target resetting situation.
	if(last_resetting){
		Broodwar->drawTextScreen( 5, 16 * 2, "Last Reset : ON");
	}else{
	    Broodwar->drawTextScreen( 5, 16 * 2, "Last Reset : OFF");
	}

	// Number of candidate positions 
	Broodwar->drawTextScreen(5, 16 * 3, "Candidate Number : %d", strategy->getCandidateNumbers());

	// Name of scout.
	Broodwar->drawTextScreen(5, 16 * 4, "Scout name : %d", scout->getID());

	// Target
	Broodwar->drawTextScreen(5, 16 * 5, "Target Tile : (%d, %d)", temp_target.x(), temp_target.y());

	// Current Position
	Broodwar->drawTextScreen(5, 16 * 6, "Current Position : (%d, %d)", scout->getTilePosition().x(), scout->getTilePosition().y());
	

#endif
	
#endif

	low_level_navigation();

#if 0
	//-----------------------------------
	update_grid_info();

	//high_level_planning();



	low_level_patch_merge();


	if ( BWAPI::Broodwar->getFrameCount() % 5 == 0)
	{
		high_level_patch_merge();

		if ( BWAPI::Broodwar->getFrameCount() % 2 == 0)
		{
			region_recogonition();
		}
	}
#endif

}


void TerrainDetectionManager::update_grid_info()
{
	//update walkable info
	std::pair<BWAPI::Unit*, ScoutProperty*> scout_pair;
	int width = TerrainRuler::get_instance()->get_map_width();
	int height = TerrainRuler::get_instance()->get_map_height();
	TerrainTile*** tiles_map = TerrainRuler::get_instance()->get_tiles_map();

	BOOST_FOREACH( scout_pair, scouts_map){

		int i = 0;
		int j = 0;

		for (  i = 0 ; i < height; ++i )
		{
			for ( j = 0 ; j < width; ++j)
			{
				int x_difference = abs(tiles_map[i][j]->pos.x() - scout_pair.first->getPosition().x());
				int y_difference = abs(tiles_map[i][j]->pos.y() - scout_pair.first->getPosition().y());

				if ( ( x_difference < 5) && ( y_difference < 5) )
				{
					goto potentical_value_update;
				}
			}
		}



potentical_value_update:


		if ( ( i > 0 ) && (j > 0 ) &&
			( i != height ) && ( j != width ))
		{

			scout_pair.second->tile_index_x = i;
			scout_pair.second->tile_index_y = j;

			int scope_ = int( scout_pair.first->getType().sightRange() * 1.5 / RATIO);
			
			for( int u = -scope_; u <= scope_; u++){
				for ( int v = -scope_; v < scope_; v++)
				{
					if ( ( i + u < 0 ) || (j + v < 0 ) ||
						(i + u >= height) || (j + v >= width))
					{
						break;
					}

					if ( tiles_map[ i + u][j + v]->pass_flag == PASS_UNKONW)
					{
						double2 dir;
						dir.x = tiles_map[ i + u][j + v]->pos.x() - tiles_map[i][j]->pos.x();
						dir.y = tiles_map[ i + u][j + v]->pos.y() - tiles_map[i][j]->pos.y();

						if ( dir.len() <= scout_pair.first->getType().sightRange() )
						{
							if ( BWAPI::Broodwar->isWalkable((i + u) * RATIO / WALKTILE_RATE , ( j + v) *  RATIO / WALKTILE_RATE ) )
							{
								tiles_map[ i + u][j + v]->pass_flag = PASS_WALKABLE;

							}else
							{
								tiles_map[ i + u][j + v]->pass_flag = PASS_UNWALKABLE;
							}
						}
					}
				}
			}

		}
	}

	//update the potential value
	BOOST_FOREACH( scout_pair, scouts_map){


		int i = scout_pair.second->tile_index_x;
		int j = scout_pair.second->tile_index_y;\
	    int sight_range = scout_pair.first->getType().sightRange();
		
		for ( int u = -1; u <= 1; u++)
		{
			for ( int v = -1; v <= 1; v++)
			{
				if ( ((u == 0) && (v == 0)) ||
					( i + u < 0 ) || ( j + v < 0 ) ||
					( i + u >= height ) || ( j + v >= width))
				{
					break;
				}

				if ( tiles_map[ i + u][j + v]->pass_flag == PASS_UNWALKABLE)
				{
					tiles_map[ i + u][j + v]->potential_value = 3.0;

				}else
				{
					/**
					* variables begin with o means obstacle-value calculation related;
					* begin with u means unexplored-value calculation related;
					* begin with r means resource-value calculation related;
					*/
					// update the obstacle value
					double nearest_o_distance = 65535.0;
					double nearest_u_distance = 65535.0;
					double nearest_r_distance = 65535.0;

					
					int   o_scope = int( sight_range * 1.5 / RATIO);
					
					int   u_scope = 1;
					bool  u_sensor_flag = false;
					bool  u_extention_flag = false;
					int   u_extention_threshold = 0;
					int   u_extention_factor = 0;


					

					for( int u_x = -o_scope; u_x <= o_scope; u_x ++){

						for ( int v_y = -o_scope; v_y < o_scope; v_y ++)
						{
							if ( ( i + u + u_x < 0 ) || ( j + v + v_y < 0 ) ||
								( i + u + u_x >= height) || (j + v + v_y >= width))
							{
								double distance_ = RATIO * 0.5 * sqrt( pow(u_x, 2.0) + pow(v_y, 2.0));

								if ( nearest_o_distance >= distance_)
								{
									nearest_o_distance = distance_;
								}
							} else if( tiles_map[i + u + u_x][j + v + v_y]->pass_flag == PASS_UNWALKABLE){

								double distance_ = sqrt( pow( tiles_map[i + u + u_x][j + v + v_y]->pos.x() - tiles_map[i + u][j + v]->pos.x(), 2.0)
									+ pow( tiles_map[i + u + u_x][j + v + v_y]->pos.y() - tiles_map[i + u][j + v]->pos.y(), 2.0));

								if ( nearest_o_distance >= distance_)
								{
									nearest_o_distance = distance_;
								}
							}

						}
					}

					if ( nearest_o_distance > sight_range * 0.5)
					{
						tiles_map[i + u][j + v]->obstacle_value = 0.0;
					}else if ( nearest_o_distance <= sight_range * 0.4 && nearest_o_distance > 1.0)
					{
						tiles_map[i + u][j + v]->obstacle_value = double( 1.0 / nearest_o_distance);
					}else if ( nearest_o_distance > 1.0){
						tiles_map[i + u][j + v]->obstacle_value = double( -1.0 /  nearest_o_distance);
					}else
					{
						tiles_map[i + u][j + v]->obstacle_value = 10.0;
					}					
					
#if 1
					BWAPI::Region* cur_region = BWAPI::Broodwar->getRegionAt(scout_pair.first->getPosition());
					BWAPI::Region* un_region = NULL;

					// update the unexplored area value
					for ( ;  RATIO * (u_scope - 1) < scout_pair.first->getType().sightRange() * sqrt(2.0) * 1.3; ++u_scope )
					{
						int v_x =          u_scope + i + u;
						int v_y =          -u_scope + j + v;
						double distance_ = 0.0;

						for (; v_y <= u_scope + j + v; ++v_y)
						{
							if ( ( v_x < 0 ) || ( v_y < 0 ) ||
								( v_x >= height ) || ( v_y >= width )){
									continue;
							}

							if (tiles_map[v_x][v_y]->pass_flag == PASS_UNKONW)
							{
								un_region = BWAPI::Broodwar->getRegionAt( tiles_map[v_x][v_y]->pos );

								if ( cur_region != NULL && un_region != NULL &&
									cur_region->getID() == un_region->getID())
								{
									u_sensor_flag = true;
									distance_ = sqrt(pow(tiles_map[v_x][v_y]->pos.x() - tiles_map[i+u][j+v]->pos.x(), 2.0) 
										+ pow(tiles_map[v_x][v_y]->pos.y() - tiles_map[i+u][j+v]->pos.y(), 2.0));

									nearest_u_distance = nearest_u_distance < distance_ ? nearest_u_distance : distance_;
								}
							}

						}

						v_x = -u_scope + i + u;
						v_y = -u_scope + j + v;

						for (; v_y <= u_scope + j + v; ++v_y)
						{
							if ( ( v_x < 0 ) ||  ( v_y < 0 ) ||
								( v_x >= height ) || ( v_y >= width )){
									continue;
							} 
							
							if(tiles_map[v_x][v_y]->pass_flag == PASS_UNKONW)
							{
								un_region = BWAPI::Broodwar->getRegionAt( tiles_map[v_x][v_y]->pos );

								if ( cur_region != NULL && un_region != NULL &&
									cur_region->getID() == un_region->getID())
								{
									u_sensor_flag = true;
									distance_ = sqrt(pow(tiles_map[v_x][v_y]->pos.x() - tiles_map[i+u][j+v]->pos.x(), 2.0) 
										+ pow(tiles_map[v_x][v_y]->pos.y() - tiles_map[i+u][j+v]->pos.y(), 2.0));

									nearest_u_distance = nearest_u_distance < distance_ ? nearest_u_distance : distance_;
								}
		
							}
						}

						v_x = -u_scope + i + u;
						v_y = -u_scope + j + v;

						for (; v_x <= u_scope + i + u; ++v_x)
						{
							if ( ( v_x < 0 ) || ( v_y < 0 ) ||
								( v_x >= height ) || ( v_y >= width )){
						       continue;
							}

							if(tiles_map[v_x][v_y]->pass_flag == PASS_UNKONW)
							{
								un_region = BWAPI::Broodwar->getRegionAt( tiles_map[v_x][v_y]->pos );

								if ( cur_region != NULL && un_region != NULL &&
									cur_region->getID() == un_region->getID())
								{
									u_sensor_flag = true;
									distance_ = sqrt(pow(tiles_map[v_x][v_y]->pos.x() - tiles_map[i+u][j+v]->pos.x(), 2.0) 
										+ pow(tiles_map[v_x][v_y]->pos.y() - tiles_map[i+u][j+v]->pos.y(), 2.0));

									nearest_u_distance = nearest_u_distance < distance_ ? nearest_u_distance : distance_;
								}

							}
						}

						v_x = -u_scope + i + u;
						v_y = u_scope + j + v;

						for (; v_x <= u_scope + i + u; ++v_x)
						{
							if ( ( v_x < 0 ) || ( v_y < 0 ) ||
								( v_x >= height ) || ( v_y >= width )){
									continue;
							} 
							
							if(tiles_map[v_x][v_y]->pass_flag == PASS_UNKONW)
							{

								un_region = BWAPI::Broodwar->getRegionAt( tiles_map[v_x][v_y]->pos );

								if ( cur_region != NULL && un_region != NULL &&
									cur_region->getID() == un_region->getID())
								{
									u_sensor_flag = true;
									distance_ = sqrt(pow(tiles_map[v_x][v_y]->pos.x() - tiles_map[i+u][j+v]->pos.x(), 2.0) 
										+ pow(tiles_map[v_x][v_y]->pos.y() - tiles_map[i+u][j+v]->pos.y(), 2.0));

									nearest_u_distance = nearest_u_distance < distance_ ? nearest_u_distance : distance_;
								}
							}
						}

						// handle extensive search condition
						if ( u_sensor_flag && !u_extention_flag )
						{
							u_extention_flag = true;

							if ( nearest_u_distance > u_scope * RATIO)
							{
								u_extention_threshold = int( nearest_u_distance/RATIO - u_scope);
							}else
							{
								break;
							}
						}

						if ( u_extention_flag)
						{
							if ( u_extention_factor < u_extention_threshold)
							{
								u_extention_factor ++;
							}else
							{
								break;
							}
						}
					}

					if ( nearest_u_distance > 1.0)
					{
						tiles_map[i + u][j + v]->unexplored_value = double( -1.0 / nearest_u_distance);
					}else
					{
						tiles_map[i + u][j + v]->unexplored_value = 10.0;
					}

#endif

#if 0
					//update resource value
					double distance_;

					BOOST_FOREACH(BWAPI::Unit* mu, BWAPI::Broodwar->getMinerals()){

						BWAPI::Position q= mu->getInitialPosition();

						distance_ = sqrt(pow(q.x() - tiles_map[i+u][j+v]->pos.x(), 2.0) 
							+ pow(q.y() - tiles_map[i+u][j+v]->pos.y(), 2.0));

						distance_ -= 30;

						nearest_r_distance = distance_ <= nearest_r_distance ? distance_ : nearest_r_distance;
					}

					BOOST_FOREACH(BWAPI::Unit* gu, BWAPI::Broodwar->getGeysers()){

						BWAPI::TilePosition c = gu->getInitialTilePosition();
						BWAPI::Position p = gu->getInitialPosition();

						double distance_1;

						double p1 = (c.x()*32 - p.x()) * (tiles_map[i+u][j+v]->pos.y() - p.y()) / (tiles_map[i+u][j+v]->pos.x() - p.x());
						
						if ( p1 >= c.y() * 32 && p1 <= c.y() * 32 + 2 * 32 )
						{
							distance_ = sqrt( pow( c.x() * 32 - tiles_map[i+u][j+v]->pos.x(), 2.0) + pow( p1 - tiles_map[i+u][j+v]->pos.y(), 2.0));
						}

						double p2 = (c.x()*32 + 4 * 32 - p.x()) * (tiles_map[i+u][j+v]->pos.y() - p.y()) / (tiles_map[i+u][j+v]->pos.x() - p.x());

						if ( p2 >= c.y() * 32 && p1 <= c.y() * 32 + 2 * 32 )
						{
							distance_1 = sqrt( pow( c.x() * 32 - tiles_map[i+u][j+v]->pos.x(), 2.0) + pow( p2 - tiles_map[i+u][j+v]->pos.y(), 2.0));
							distance_ = distance_ < distance_1 ? distance_ : distance_1;
						}

						double p3 = (c.y()*32 - p.y()) * (tiles_map[i+u][j+v]->pos.x() - p.x()) / (tiles_map[i+u][j+v]->pos.y() - p.y());

						if ( p3 >= c.x() * 32 && p3 <= c.x() * 32 + 4 * 32 )
						{
							distance_1 = sqrt( pow( c.y() * 32 - tiles_map[i+u][j+v]->pos.x(), 2.0) + pow( p3 - tiles_map[i+u][j+v]->pos.x(), 2.0));
							distance_ = distance_ < distance_1 ? distance_ : distance_1;
						}

						double p4 = (c.y()*32 + 2 * 32 - p.y()) * (tiles_map[i+u][j+v]->pos.x() - p.x()) / (tiles_map[i+u][j+v]->pos.y() - p.y());

						if ( p4 >= c.x() * 32 && p4 <= c.x() * 32 + 4 * 32 )
						{
							distance_1 = sqrt( pow( c.y() * 32 - tiles_map[i+u][j+v]->pos.x(), 2.0) + pow( p3 - tiles_map[i+u][j+v]->pos.x(), 2.0));
							distance_ = distance_ < distance_1 ? distance_ : distance_1;
						}

						nearest_r_distance = distance_ <= nearest_r_distance ? distance_ : nearest_r_distance;

					}

					if ( nearest_r_distance > sight_range * 0.6)
					{
						tiles_map[i + u][j + v]->resource_value = 0.0;
					}else if ( nearest_r_distance <= sight_range * 0.2 && nearest_r_distance > 1.0)
					{
						tiles_map[i + u][j + v]->resource_value = double( 0.9 / nearest_r_distance );
					}else if ( nearest_r_distance > 1.0){
						tiles_map[i + u][j + v]->resource_value = double( - 0.9 /  nearest_r_distance);
					}else
					{
						tiles_map[i + u][j + v]->resource_value = 10.0;
					}

					tiles_map[i + u][j + v]->a_potential_value = tiles_map[i + u][j + v]->obstacle_value + tiles_map[i + u][j + v]->unexplored_value + tiles_map[i + u][j + v]->resource_value;

#endif
					tiles_map[i + u][j + v]->a_potential_value = tiles_map[i + u][j + v]->obstacle_value + tiles_map[i + u][j + v]->unexplored_value ;
					//tiles_map[i + u][j + v]->a_potential_value = tiles_map[i + u][j + v]->obstacle_value;
				}
			}
		}
	
	}


}

void TerrainDetectionManager::high_level_planning()
{

}

void TerrainDetectionManager::low_level_navigation()
{

	// detection code 

	//-1. Arrived in target 
	//-2. Case two the scout gets sucked.
	if ( temp_target.x() == scout->getTilePosition().x() 
		&& temp_target.y() == scout->getTilePosition().y()){
        strategy->calCandidatePosition(reset_flag, temp_target, scout);
#ifdef DEBUGGING
		last_resetting = false;
#endif
	}

	if (reset_flag){
		move_to_tile(scout, temp_target);

#ifdef DEBUGGING
		last_resetting = true;
#endif
		reset_flag = false;
	}

}


void TerrainDetectionManager::calculate_possible_startlocations()
{

}

void TerrainDetectionManager::initialization()
{

	size_t width = TerrainRuler::get_instance()->get_map_width();
	size_t height = TerrainRuler::get_instance()->get_map_height();

#ifdef DEBUGGING
	analyzed = false;
	potential_num = 0;
	last_resetting = false;
#endif

	width *=  RATIO;
	height *= RATIO;
	
	BOOST_FOREACH(BWAPI::Unit* unit, BWAPI::Broodwar->self()->getUnits()){
		if ( unit->getType().isWorker() &&
			!unit->isTraining())
		{
			scout = unit;
			break;
		}
	}

	strategy = new esStrFBMC();
	strategy->calCandidatePosition(reset_flag, temp_target, scout);
	if (reset_flag){
		move_to_tile(scout, temp_target);
#ifdef DEBUGGING
       last_resetting = true;
#endif
		reset_flag = false;
	}

	initial_flag = true;
	debug_flag = false;
	file_name = std::string("E:\\TerrainDetectionLog.txt");
}


void TerrainDetectionManager::debugging_info_assign()
{
	TerrainTile*** tiles_map = TerrainRuler::get_instance()->get_tiles_map();
	int height_ = TerrainRuler::get_instance()->get_map_height();
	int width_ = TerrainRuler::get_instance()->get_map_width();

	for ( int i = 0; i < height_; i++)
	{
		for (int j = 0; j < width_; j++){

			if ( BWAPI::Broodwar->isWalkable(i * RATIO / WALKTILE_RATE, j * RATIO / WALKTILE_RATE))
			{
				tiles_map[i][j]->pass_flag = PASS_WALKABLE;
			}else
			{
				tiles_map[i][j]->pass_flag = PASS_UNWALKABLE;
			}			
		}
	}
}

std::map<BWAPI::Unit*, ScoutProperty*> TerrainDetectionManager::get_scouts_map()
{
	return scouts_map;
}

Patch TerrainDetectionManager::getStaticPatchBuffer()
{
   return static_patch_buffer;
}

void TerrainDetectionManager::move_to_tile(Unit* unit, TilePosition tile){
   Position tile_center( tile.x() * 32 + 16, tile.y() * 32 + 16);
   unit->move( tile_center );
}
