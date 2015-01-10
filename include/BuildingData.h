#pragma once

#include "Common.h"

class Building {
     
public:
      
	BWAPI::TilePosition            desired_position;
	BWAPI::TilePosition            final_position;
	BWAPI::Position                position;
	BWAPI::UnitType                type;
	BWAPI::Unit *                  building_unit;
	BWAPI::Unit *                  builder_unit;

	int                            last_order_frame;
	bool                           build_command_given;
	bool                           under_construction;

	Building() 
		: desired_position(0,0), final_position(BWAPI::TilePositions::None), position(0,0),
			type(BWAPI::UnitTypes::Unknown), building_unit(NULL), 
			builder_unit(NULL), last_order_frame(0), build_command_given(false), under_construction(false) {} 

	// constructor we use most often
	Building(BWAPI::UnitType t, BWAPI::TilePosition desired)
		: desired_position(desired), final_position(0,0), position(0,0),
		type(t), building_unit(NULL), builder_unit(NULL), 
		last_order_frame(0), build_command_given(false), under_construction(false) {}

	// equals operator
	bool operator==(const Building & b) {
		// buildings are equal if their worker unit or building unit are equal
		return (b.building_unit == building_unit) || (b.builder_unit== builder_unit);
	}
};

class ConstructionData {

public:

	typedef enum BuildingState_t {Unassigned = 0, Assigned = 1, UnderConstruction = 2, NumBuildingStates = 3} BuildingState;

private:

	int							reserved_minerals;				// minerals reserved for planned buildings
	int							reserved_gas;					// gas reserved for planned buildings
	int							building_space;					// how much space we want between buildings

	std::vector< size_t >						building_index;
	std::vector< std::vector<Building> >		buildings;			// buildings which do not yet have builders assigned

	std::set<BWAPI::Unit *>		building_units_constructing;		// units which have been recently detected as started construction

public:

	ConstructionData();
	
	Building &					get_next_building(BuildingState bs);
	bool						has_next_building(BuildingState bs);
	void						begin(BuildingState bs);
	void						add_building(BuildingState bs, const Building & b);
	void						remove_current_building(BuildingState bs);
	void						remove_building(BuildingState bs, Building & b);

	int							get_num_buildings(BuildingState bs);

	bool						is_being_built(BWAPI::UnitType type);
};
