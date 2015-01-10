#pragma once
#include "Common.h"
#include "UnitManager.h"
#include "BuildingPlacer.h"
#include "BuildingData.h"


class ConstrucitionManager
{
public:
	
	static ConstrucitionManager*       get_instance();
	static void                        destroy_instance();

	void						       update();
	void						       on_unit_morph(BWAPI::Unit * unit);
	void						       on_unit_destroy(BWAPI::Unit * unit);
	void						       add_building_task(BWAPI::UnitType type, BWAPI::TilePosition desiredLocation);
	BWAPI::TilePosition			       get_building_location(const Building & b);

	int							       get_reserved_minerals();
	int							       get_reserved_gas();

	void						       print_building_numbers();

	bool						       is_being_built(BWAPI::UnitType type);

	void						       draw_building_information(int x, int y);

private:
	ConstructionData			       building_data;

	bool						       debug_mode;
	int							       total_build_tasks;

	int							       reserved_minerals;				// minerals reserved for planned buildings
	int							       reserved_gas;					// gas reserved for planned buildings
	int							       building_space;					// how much space we want between buildings

	std::vector<BWAPI::Unit *>	       builders;						// workers which have been assigned to buildings
	std::vector<Building>		       buildings_needing_builders;		// buildings which do not yet have builders assigned
	std::vector<Building>		       buildings_assigned;				// buildings which have workers but not yet under construction
	std::vector<Building>		       buildings_under_construction;		// buildings which are under construction
	std::vector<BWAPI::Unit *>	       building_units_constructing;		// units which have been recently detected as started construction

	// functions
	bool						       is_evolved_building(BWAPI::UnitType type);
	bool						       is_building_position_explored(const Building & b) const;

	// the update() functions
	void						       validate_workers_and_buildings();		// STEP 1
	void						       assign_workers_to_unassigned_buildings();	// STEP 2
	void						       construct_assigned_buildings();			// STEP 3
	void						       check_for_started_construction();			// STEP 4
	void						       check_for_dead_terran_builders();			// STEP 5
	void						       check_for_completed_buildings();			// STEP 6

	// functions for performing tedious vector tasks
	void						       remove_building_from_vector(BWAPI::Unit * buildingUnit, std::vector<Building> & vec);
	void						       remove_building_from_vector(Building & b, std::vector<Building> & vec);

	char						       get_building_worker_code(const Building & b) const;


	static ConstrucitionManager*       cm_instantce;
	ConstrucitionManager(void);
	~ConstrucitionManager(void);
};

