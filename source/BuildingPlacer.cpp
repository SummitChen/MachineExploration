#include "Common.h"
#include "BuildingPlacer.h"

BuildingPlacer * BuildingPlacer::bp_instance = NULL;

BuildingPlacer::BuildingPlacer() : box_top(100000), box_bottom(-1), box_left(100000), box_right(-1)
{
	reserve_map = std::vector< std::vector<bool> >(BWAPI::Broodwar->mapWidth(), std::vector<bool>(BWAPI::Broodwar->mapHeight(), false));
	build_distance = 0;
	compute_resource_box();
}

bool BuildingPlacer::is_in_resource_box(int x, int y) const
{
	int posX(x*32);
	int posY(y*32);

	if (posX >= box_left && posX < box_right && posY >= box_top && posY < box_bottom)
	{
		return true;
	}

	return false;
}

void BuildingPlacer::compute_resource_box()
{
	BWAPI::Position start(BWAPI::Broodwar->self()->getStartLocation());
	std::vector<BWAPI::Unit *> units_around_nexus;

	BOOST_FOREACH(BWAPI::Unit * unit, BWAPI::Broodwar->getAllUnits())
	{
		// if the units are less than 400 away add them if they are resources
		if (unit->getDistance(start) < 400 && unit->getType().isResourceContainer())
		{
			units_around_nexus.push_back(unit);
		}
	}

	BOOST_FOREACH(BWAPI::Unit * unit, units_around_nexus)
	{
		int x = unit->getPosition().x();
		int y = unit->getPosition().y();

		int left = x - unit->getType().dimensionLeft();
		int right = x + unit->getType().dimensionRight() + 1;
		int top = y - unit->getType().dimensionUp();
		int bottom = y + unit->getType().dimensionDown() + 1;

		box_top = top < box_top ? top : box_top;
		box_bottom = bottom > box_bottom ? bottom : box_bottom;
		box_left = left < box_left ? left : box_left;
		box_right = right > box_right ? right : box_right;
	}

	//BWAPI::Broodwar->printf("%d %d %d %d", boxTop, boxBottom, boxLeft, boxRight);
}

// makes final checks to see if a building can be built at a certain location
bool BuildingPlacer::can_build_here(BWAPI::TilePosition position, const Building & b) const
{

#if 0
	if (!b.type.isRefinery() && !InformationManager::Instance().tileContainsUnit(position))
	{
		return false;
	}
#endif

	//returns true if we can build this type of unit here. Takes into account reserved tiles.
	if (!BWAPI::Broodwar->canBuildHere(b.builder_unit, position, b.type))
	{
		return false;
	}

	// check the reserve map
	for(int x = position.x(); x < position.x() + b.type.tileWidth(); x++)
	{
		for(int y = position.y(); y < position.y() + b.type.tileHeight(); y++)
		{
			if (reserve_map[x][y])
			{
				return false;
			}
		}
	}

	// if it overlaps a base location return false
	if (tile_overlaps_base_location(position, b.type))
	{
		return false;
	}

	return true;
}

//returns true if we can build this type of unit here with the specified amount of space.
//space value is stored in this->buildDistance.
bool BuildingPlacer::can_build_here_with_space(BWAPI::TilePosition position, const Building & b, int build_dist, bool horizontal_only) const
{
	//if we can't build here, we of course can't build here with space
	if (!this->can_build_here(position, b))
	{
		return false;
	}

	// height and width of the building
	int width(b.type.tileWidth());
	int height(b.type.tileHeight());

	//make sure we leave space for add-ons. These types of units can have addons:
	if (b.type==BWAPI::UnitTypes::Terran_Command_Center ||
		b.type==BWAPI::UnitTypes::Terran_Factory || 
		b.type==BWAPI::UnitTypes::Terran_Starport ||
		b.type==BWAPI::UnitTypes::Terran_Science_Facility)
	{
		width += 2;
	}

	// define the rectangle of the building spot
	int startx = position.x() - build_dist;
	int starty = position.y() - build_dist;
	int endx   = position.x() + width + build_dist;
	int endy   = position.y() + height + build_dist;

	if (horizontal_only)
	{
		starty += build_dist;
		endy -= build_dist;
	}

	// if this rectangle doesn't fit on the map we can't build here
	if (startx < 0 || starty < 0 || endx > BWAPI::Broodwar->mapWidth() || endx < position.x() + width || endy > BWAPI::Broodwar->mapHeight()) 
	{
		return false;
	}

	// if we can't build here, or space is reserved, or it's in the resource box, we can't build here
	for(int x = startx; x < endx; x++)
	{
		for(int y = starty; y < endy; y++)
		{
			if (!b.type.isRefinery())
			{
				if (!buildable(x, y) || reserve_map[x][y] || ((b.type != BWAPI::UnitTypes::Protoss_Photon_Cannon) && is_in_resource_box(x,y)))
				{
					return false;
				}
			}
		}
	}

	// special cases for terran buildings that can land into addons?
	if (position.x() > 3 && b.type.isFlyingBuilding())
	{
		int startx2 = startx - 2;
		if (startx2 < 0) 
		{
			startx2 = 0;
		}

		for(int x = startx2; x < startx; x++)
		{
			for(int y = starty; y < endy; y++)
			{
				BOOST_FOREACH(BWAPI::Unit * unit, BWAPI::Broodwar->getUnitsOnTile(x, y))
				{
					if (!unit->isLifted())
					{
						BWAPI::UnitType type(unit->getType());

						if (b.type==BWAPI::UnitTypes::Terran_Command_Center || b.type==BWAPI::UnitTypes::Terran_Factory || 
							b.type==BWAPI::UnitTypes::Terran_Starport || b.type==BWAPI::UnitTypes::Terran_Science_Facility)
						{
							return false;
						}
					}
				}
			}
		}
	}

	return true;
}

BWAPI::TilePosition BuildingPlacer::get_build_location_near(const Building & b, int buildDist, bool in_region_priority, bool horizontal_only) const
{
	//returns a valid build location near the specified tile position.
	//searches outward in a spiral.
	int x      = b.desired_position.x();
	int y      = b.desired_position.y();
	int length = 1;
	int j      = 0;
	bool first = true;
	int dx     = 0;
	int dy     = 1;

	while (length < BWAPI::Broodwar->mapWidth()) //We'll ride the spiral to the end
	{
		//if we can build here, return this tile position
		if (x >= 0 && x < BWAPI::Broodwar->mapWidth() && y >= 0 && y < BWAPI::Broodwar->mapHeight())
		{
			// can we build this building at this location
			bool canBuild					= this->can_build_here_with_space(BWAPI::TilePosition(x, y), b, buildDist, horizontal_only);

#ifdef SHUTDOWN_BWTA
			// my starting region
			BWTA::Region * myRegion			= NULL; 

			// the region the build tile is in
			BWTA::Region * tileRegion		= NULL;
#else
			// my starting region
			BWTA::Region * myRegion			= BWTA::getRegion(BWTA::getStartLocation(BWAPI::Broodwar->self())->getTilePosition()); 

			// the region the build tile is in
			BWTA::Region * tileRegion		= BWTA::getRegion(BWAPI::TilePosition(x,y));
#endif


			// is the proposed tile in our region?
			bool tileInRegion				= (tileRegion == myRegion);

			// if this location has priority to be built within our own region
			if (in_region_priority)
			{
				// if the tile is in region and we can build it there
				if (tileInRegion && canBuild)
				{
					// return that position
					return BWAPI::TilePosition(x, y);
				}
			}
			// otherwise priority is not set for this building
			else
			{
				if (canBuild)
				{
					 return BWAPI::TilePosition(x, y);
				}
			}
		}

		//otherwise, move to another position
		x = x + dx;
		y = y + dy;

		//count how many steps we take in this direction
		j++;
		if (j == length) //if we've reached the end, its time to turn
		{
			//reset step counter
			j = 0;

			//Spiral out. Keep going.
			if (!first)
				length++; //increment step counter if needed

			//first=true for every other turn so we spiral out at the right rate
			first =! first;

			//turn counter clockwise 90 degrees:
			if (dx == 0)
			{
				dx = dy;
				dy = 0;
			}
			else
			{
				dy = -dx;
				dx = 0;
			}
		}
		//Spiral out. Keep going.
	}

	return  BWAPI::TilePositions::None;
}

bool BuildingPlacer::tile_overlaps_base_location(BWAPI::TilePosition tile, BWAPI::UnitType type) const
{
	// if it's a resource depot we don't care if it overlaps
	if (type.isResourceDepot())
	{
		return false;
	}

	// dimensions of the proposed location
	int tx1 = tile.x();
	int ty1 = tile.y();
	int tx2 = tx1 + type.tileWidth();
	int ty2 = ty1 + type.tileHeight();

	// for each base location
	BOOST_FOREACH (BWTA::BaseLocation * base, BWTA::getBaseLocations())
	{
		// dimensions of the base location
		int bx1 = base->getTilePosition().x();
		int by1 = base->getTilePosition().y();
		int bx2 = bx1 + BWAPI::Broodwar->self()->getRace().getCenter().tileWidth();
		int by2 = by1 + BWAPI::Broodwar->self()->getRace().getCenter().tileHeight();

		// conditions for non-overlap are easy
		bool noOverlap = (tx2 < bx1) || (tx1 > bx2) || (ty2 < by1) || (ty1 > by2);

		// if the reverse is true, return true
		if (!noOverlap)
		{
			return true;
		}
	}

	// otherwise there is no overlap
	return false;
}

bool BuildingPlacer::buildable(int x, int y) const
{
	//returns true if this tile is currently buildable, takes into account units on tile
	if (!BWAPI::Broodwar->isBuildable(x,y)) 
	{
		return false;
	}

	BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->getUnitsOnTile(x, y))
	{
		if (unit->getType().isBuilding() && !unit->isLifted()) 
		{
			return false;
		}
	}

	for (int i=x-1; i <= x+1; ++i)
	{
		for (int j=y-1; j <= y+1; ++j)
		{
			BWAPI::TilePosition tile(i,j);

			if (!tile.isValid())
			{
				continue;
			}

			BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->getUnitsOnTile(i, j))
			{
				if (unit->getType() == BWAPI::UnitTypes::Protoss_Gateway) 
				{
					return false;
				}
			}
		}	
	}

	return true;
}

void BuildingPlacer::reserve_tiles(BWAPI::TilePosition position, int width, int height)
{
	int rwidth = reserve_map.size();
	int rheight = reserve_map[0].size();
	for(int x = position.x(); x < position.x() + width && x < rwidth; x++) 
	{
		for(int y = position.y(); y < position.y() + height && y < rheight; y++) 
		{
			reserve_map[x][y] = true;
		}
	}
}

void BuildingPlacer::draw_reserved_tiles()
{
	int rwidth = reserve_map.size();
	int rheight = reserve_map[0].size();

	for(int x = 0; x < rwidth; ++x) 
	{
		for(int y = 0; y < rheight; ++y) 
		{
			if (reserve_map[x][y] || is_in_resource_box(x,y))
			{
				int x1 = x*32 + 8;
				int y1 = y*32 + 8;
				int x2 = (x+1)*32 - 8;
				int y2 = (y+1)*32 - 8;

				//if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawBoxMap(x1, y1, x2, y2, BWAPI::Colors::Yellow, false);
			}
		}
	}
}

void BuildingPlacer::free_tiles(BWAPI::TilePosition position, int width, int height)
{
	int rwidth = reserve_map.size();
	int rheight = reserve_map[0].size();

	for(int x = position.x(); x < position.x() + width && x < rwidth; x++) 
	{
		for(int y = position.y(); y < position.y() + height && y < rheight; y++) 
		{
			reserve_map[x][y] = false;
		}
	}
}

void BuildingPlacer::set_build_distance(int distance)
{
	this->build_distance=distance;
}

int BuildingPlacer::get_build_distance() const
{
	return this->build_distance;
}

BWAPI::TilePosition BuildingPlacer::get_refinery_position()
{
	// for each of our units
	BOOST_FOREACH (BWAPI::Unit * depot, BWAPI::Broodwar->self()->getUnits())
	{
		// if it's a resource depot
		if (depot->getType().isResourceDepot())
		{
			// for all units around it
			BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->getAllUnits())
			{
				// if it's a geyser around it
				if (unit->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser && unit->getDistance(depot) < 300)
				{
					return unit->getTilePosition();
				}
			}
		}
	}

	return BWAPI::TilePositions::None;
}

bool BuildingPlacer::is_reserved(int x, int y) const
{
	int rwidth = reserve_map.size();
	int rheight = reserve_map[0].size();
	if (x < 0 || y < 0 || x >= rwidth || y >= rheight) 
	{
		return false;
	}

	return reserve_map[x][y];
}

BuildingPlacer * BuildingPlacer::get_instance() {

	if ( bp_instance == NULL)
	{
		bp_instance = new BuildingPlacer();
	}
	return bp_instance;
}

void BuildingPlacer::destroy_instance(){

	if ( bp_instance != NULL)
	{
		delete bp_instance;
		bp_instance = NULL;
	}
}



