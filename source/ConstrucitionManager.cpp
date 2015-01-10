#include "..\include\ConstrucitionManager.h"
#include "Common.h"

ConstrucitionManager* ConstrucitionManager::cm_instantce = NULL;

ConstrucitionManager::ConstrucitionManager(void)
{
}


ConstrucitionManager::~ConstrucitionManager(void)
{
}

ConstrucitionManager* ConstrucitionManager::get_instance()
{
	if ( cm_instantce == NULL)
	{
		cm_instantce = new ConstrucitionManager();
	}

	return cm_instantce;
}

void ConstrucitionManager::destroy_instance()
{
	if ( cm_instantce != NULL)
	{
		delete cm_instantce;
		cm_instantce = NULL;
	}
}

void ConstrucitionManager::update()
{
	validate_workers_and_buildings();

	assign_workers_to_unassigned_buildings();

	construct_assigned_buildings();

	check_for_started_construction();
	
	check_for_dead_terran_builders();

	check_for_completed_buildings();

}

//check all the building data to see whether the given type is being built
bool ConstrucitionManager::is_being_built( BWAPI::UnitType type )
{
	// check unassigned buildings
	return building_data.is_being_built(type);
}

// STEP 1: 
void ConstrucitionManager::validate_workers_and_buildings()
{
	building_data.begin( ConstructionData::UnderConstruction );

	while ( building_data.has_next_building( ConstructionData::UnderConstruction))
	{
		Building & b = building_data.get_next_building( ConstructionData::UnderConstruction );

		if ( b.building_unit == NULL || !b.building_unit->getType().isBuilding() || b.building_unit->getHitPoints() <= 0)
		{
			building_data.remove_current_building( ConstructionData::UnderConstruction);
			break;
		}
	}
}

// STEP 2
void ConstrucitionManager::assign_workers_to_unassigned_buildings()
{
    building_data.begin( ConstructionData::Unassigned);
	while ( building_data.has_next_building( ConstructionData::Unassigned))
	{
		Building & b = building_data.get_next_building( ConstructionData::Unassigned);
		
#ifdef DEBUGGING_OUTPUT
		BWAPI::Broodwar->printf("Assigning Worker To: %s", b.type.getName().c_str());
#endif

		// Tested by UalbertaBot refineries after 3, should be removed
		if ( b.type.isRefinery() && BWAPI::Broodwar->self()->allUnitCount(b.type) >= 3)
		{
			building_data.remove_current_building( ConstructionData::Unassigned);
			break;
		}

		BWAPI::TilePosition test_position= get_building_location(b);

		if ( !test_position.isValid())
		{
			continue;
		}

		b.final_position = test_position;

		BWAPI::Unit* worker_to_assign = UnitManager::get_instance()->get_builder(b);

		if ( worker_to_assign)
		{
			b.builder_unit = worker_to_assign;

			test_position = get_building_location(b);

			if ( !test_position.isValid())
			{
				continue;
			}

			b.final_position = test_position;

			BuildingPlacer::get_instance()->reserve_tiles( b.final_position, b.type.tileWidth(), b.type.tileHeight());

			building_data.add_building( ConstructionData::Assigned, b);

			building_data.remove_current_building( ConstructionData::Unassigned);

		}
	}
	
}

void ConstrucitionManager::construct_assigned_buildings()
{
	building_data.begin( ConstructionData::Assigned);

	while( building_data.has_next_building( ConstructionData::Assigned )){

		Building & b = building_data.get_next_building( ConstructionData::Assigned );

		if ( !b.building_unit->isConstructing())
		{
			if ( !is_building_position_explored(b))
			{
				b.builder_unit->move( BWAPI::Position( b.final_position ));

			}else if ( b.build_command_given)  // confused
			{
				UnitManager::get_instance()->finished_with_worker( b.builder_unit);

				BuildingPlacer::get_instance()->free_tiles( b.final_position, b.type.tileWidth(), b.type.tileHeight());

				b.builder_unit = NULL;

				b.build_command_given = false;

				building_data.add_building( ConstructionData::Unassigned, b);

				building_data.remove_current_building( ConstructionData::Assigned);
			} 
			else
			{
				b.builder_unit->build( b.final_position, b.type);

				b.build_command_given = true;
			}
		}
	}
}

// STEP 4
void ConstrucitionManager::check_for_started_construction()
{
	BOOST_FOREACH(BWAPI::Unit * building_started, BWAPI::Broodwar->self()->getUnits()){
		
		if ( !( building_started->getType().isBuilding() && building_started->isBeingConstructed()))
		{
			continue;
		}

		building_data.begin(ConstructionData::Assigned);

	    while ( building_data.has_next_building(ConstructionData::Assigned))
	    {
			Building & b = building_data.get_next_building( ConstructionData::Assigned );

			if ( b.final_position == building_started->getTilePosition())
			{
				reserved_minerals -= building_started->getType().mineralPrice();
				reserved_gas      -= building_started->getType().gasPrice();

				b.under_construction = true;
				b.building_unit = building_started;

				if ( BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg)
				{
					b.builder_unit = NULL;
				}else if ( BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Protoss)
				{
					UnitManager::get_instance()->finished_with_worker( b.builder_unit );
					b.builder_unit = NULL;
				}

				building_data.add_building( ConstructionData::UnderConstruction, b);

				building_data.remove_current_building( ConstructionData::Assigned );

				BuildingPlacer::get_instance()->free_tiles( b.final_position, b.type.tileWidth(), b.type.tileHeight());

				break;

			}
	    }
	}
}

// STEP 5 function for Terrain Race
void ConstrucitionManager::check_for_dead_terran_builders()
{

}
//STEP 6
void ConstrucitionManager::check_for_completed_buildings()
{
	building_data.begin( ConstructionData::UnderConstruction);

	while( building_data.has_next_building( ConstructionData::UnderConstruction)){

		Building & b = building_data.get_next_building( ConstructionData::UnderConstruction );

		if ( b.building_unit->isCompleted())
		{
#ifdef DEBUGGING_OUTPUT
			BWAPI::Broodwar->printf("Construction Completed: %s", b.type.getName().c_str());
#endif
			if ( BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Terran)
			{
				UnitManager::get_instance()->finished_with_worker( b.builder_unit );
			}

			building_data.remove_current_building( ConstructionData::UnderConstruction );
		}
	}
}

void ConstrucitionManager::remove_building_from_vector( BWAPI::Unit * buildingUnit, std::vector<Building> & vec )
{

}

void ConstrucitionManager::remove_building_from_vector( Building & b, std::vector<Building> & vec )
{

}

char ConstrucitionManager::get_building_worker_code( const Building & b ) const
{
	if (b.builder_unit == NULL)	return 'X';
	else						return 'W';
}

void ConstrucitionManager::on_unit_morph( BWAPI::Unit * unit )
{

}

void ConstrucitionManager::add_building_task( BWAPI::UnitType type, BWAPI::TilePosition desiredLocation )
{
#ifdef DEBUGGING_OUTPUT
   BWAPI::Broodwar->printf("Issuing addBuildingTask: %s", type.getName().c_str());
#endif

	total_build_tasks++;

	// reserve the resources for this building
	reserved_minerals += type.mineralPrice();
	reserved_gas	     += type.gasPrice();

	// set it up to receive a worker
	building_data.add_building(ConstructionData::Unassigned, Building(type, desiredLocation));
}

BWAPI::TilePosition ConstrucitionManager::get_building_location( const Building & b )
{
	BWAPI::TilePosition test_location = BWAPI::TilePositions::None;

	int num_pylons = BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Pylon);

	if (b.type.isRefinery())
	{
		return BuildingPlacer::get_instance()->get_refinery_position();
	}

	// special case for early pylons
	if (b.type == BWAPI::UnitTypes::Protoss_Pylon && (num_pylons < 3))
	{
		BWAPI::TilePosition pos_in_region =    BuildingPlacer::get_instance()->get_build_location_near(b, 4, true);
		BWAPI::TilePosition pos_not_in_region = BuildingPlacer::get_instance()->get_build_location_near(b, 4, false);

		test_location = ( pos_in_region != BWAPI::TilePositions::None) ? pos_in_region : pos_not_in_region;
	}
	// every other type of building
	else
	{
		// if it is a protoss building and we have no pylons, quick check
		if (b.type.requiresPsi() && ( num_pylons == 0))
		{
			// nothing
		}
		// if the unit is a resource depot
		else if (b.type.isResourceDepot())
		{
			// get the location 
			//BWAPI::TilePosition tile = MapTools::Instance().getNextExpansion();

			//return tile;
		}
		// any other building
		else
		{
			// set the building padding specifically
			int distance = b.type == BWAPI::UnitTypes::Protoss_Photon_Cannon ? 0 : 1;

			// whether or not we want the distance to be horizontal only
			bool horizontal_only = true;

			// get a position within our region
			BWAPI::TilePosition pos_in_region =    BuildingPlacer::get_instance()->get_build_location_near(b, distance, true,  horizontal_only);

			// get a region anywhere
			BWAPI::TilePosition pos_not_in_region = BuildingPlacer::get_instance()->get_build_location_near(b, distance, false, horizontal_only);

			// set the location with priority on positions in our own region
			test_location = (pos_in_region!= BWAPI::TilePositions::None) ? pos_in_region: pos_not_in_region;
		}
	}

	// send back the location
	return test_location;
}

int ConstrucitionManager::get_reserved_minerals()
{
	return reserved_minerals;
}

int ConstrucitionManager::get_reserved_gas()
{
	return reserved_gas;
}

void ConstrucitionManager::print_building_numbers()
{

}

void ConstrucitionManager::draw_building_information( int x, int y )
{
#if 0
	BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->self()->getUnits())
	{
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextMap(unit->getPosition().x(), unit->getPosition().y()+5, "\x07%d", unit->getID()); 
	}

	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y, "\x04 Building Information:");
	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y+20, "\x04 Name");
	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x+150, y+20, "\x04 State");

	int yspace = 0;

	buildingData.begin(ConstructionData::Unassigned);
	while (buildingData.hasNextBuilding(ConstructionData::Unassigned)) {

		Building & b = buildingData.getNextBuilding(ConstructionData::Unassigned);

		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y+40+((yspace)*10), "\x03 %s", b.type.getName().c_str());
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x+150, y+40+((yspace++)*10), "\x03 Need %c", getBuildingWorkerCode(b));
	}

	buildingData.begin(ConstructionData::Assigned);
	while (buildingData.hasNextBuilding(ConstructionData::Assigned)) {

		Building & b = buildingData.getNextBuilding(ConstructionData::Assigned);

		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y+40+((yspace)*10), "\x03 %s %d", b.type.getName().c_str(), b.builderUnit->getID());
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x+150, y+40+((yspace++)*10), "\x03 A %c (%d,%d)", getBuildingWorkerCode(b), b.finalPosition.x(), b.finalPosition.y());

		int x1 = b.finalPosition.x()*32;
		int y1 = b.finalPosition.y()*32;
		int x2 = (b.finalPosition.x() + b.type.tileWidth())*32;
		int y2 = (b.finalPosition.y() + b.type.tileHeight())*32;

		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawLineMap(b.builderUnit->getPosition().x(), b.builderUnit->getPosition().y(), (x1+x2)/2, (y1+y2)/2, BWAPI::Colors::Orange);
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawBoxMap(x1, y1, x2, y2, BWAPI::Colors::Red, false);
	}

	buildingData.begin(ConstructionData::UnderConstruction);
	while (buildingData.hasNextBuilding(ConstructionData::UnderConstruction)) {

		Building & b = buildingData.getNextBuilding(ConstructionData::UnderConstruction);

		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y+40+((yspace)*10), "\x03 %s %d", b.type.getName().c_str(), b.buildingUnit->getID());
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x+150, y+40+((yspace++)*10), "\x03 Const %c", getBuildingWorkerCode(b));
	}
#endif
}

bool ConstrucitionManager::is_building_position_explored( const Building & b ) const
{
	return true;
}









