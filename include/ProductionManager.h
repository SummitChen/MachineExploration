#pragma once

#include <Common.h>
#include "BuildOrderQueue.h"
#include "ConstrucitionManager.h"
//#include "StarcraftBuildOrderSearchManager.h"
//#include "StrategyManager.h"

typedef unsigned char Action;

class CompareWhenStarted 
{

public:

	CompareWhenStarted() {}

	// the sorting operator
	bool operator() (BWAPI::Unit * u1, BWAPI::Unit * u2) 
	{
		int startedU1 = BWAPI::Broodwar->getFrameCount() - (u1->getType().buildTime() - u1->getRemainingBuildTime());
		int startedU2 = BWAPI::Broodwar->getFrameCount() - (u2->getType().buildTime() - u2->getRemainingBuildTime());
		return startedU1 > startedU2;
    }
};

class ProductionManager 
{
	ProductionManager();

    static ProductionManager*   pm_instance;
	
	bool						initial_build_set;

	std::map<char, MetaType>	type_char_map;
	//std::vector< std::pair<MetaType, UnitCountType> > searchGoal;

	bool						assigned_worker_for_this_building;
	bool						have_location_for_this_building;
	int							reserved_minerals, reserved_gas;
	bool						enemy_cloaked_detected;
	bool						rush_detected;

	BWAPI::TilePosition			predicted_tile_position;

	bool						contains(UnitVector & units, BWAPI::Unit * unit);
	void						populate_type_char_map();

	bool						has_resources(BWAPI::UnitType type);
	bool						can_make(BWAPI::UnitType type);
	bool						has_num_completed_unit_type(BWAPI::UnitType type, int num);
	bool						meets_reserved_resources(MetaType type);
	BWAPI::UnitType				get_producer(MetaType t);

//	void						perform_build_order_search(const std::vector< std::pair<MetaType, UnitCountType> > & goal);
	void						set_build_order(const std::vector<MetaType> & buildOrder);
	void						create_meta_type(BWAPI::Unit * producer, MetaType type);
	BWAPI::Unit *				select_unit_of_type(BWAPI::UnitType type, bool leastTrainingTimeRemaining = true, BWAPI::Position closestTo = BWAPI::Position(0,0));
	
	BuildOrderQueue				queue;
	void						manage_build_order_queue();
	void						perform_command(BWAPI::UnitCommandType t);
	bool						can_make_now(BWAPI::Unit * producer, MetaType t);
	void						predict_worker_movement(const Building & b);

	bool						detect_build_order_dead_lock();

	int							get_free_minerals();
	int							get_free_gas();

public:

	static ProductionManager *	get_instance();
	static void                 destroy_instance();

	void						draw_queue_information(std::map<BWAPI::UnitType, int> & numUnits, int x, int y, int index);
	void						update();

	void						on_unit_morph(BWAPI::Unit * unit);
	void						on_unit_destroy(BWAPI::Unit * unit);
	void						on_send_text(std::string text);

	void						draw_production_information(int x, int y);
};
