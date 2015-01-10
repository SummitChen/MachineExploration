#pragma once
#include "BWAPI.h"
#include "Common.h"
#include "BuildingData.h"
#include "WorkerData.h"

class UnitManager
{

private:
	static UnitManager*               um_instance;
	WorkerData					      worker_data;

	void						      set_mineral_worker(BWAPI::Unit * unit);

	UnitManager(void);
	~UnitManager(void);


public:
		
	static UnitManager*               get_instance();
	static void                       destroy_instance();

	void						      update();
	void						      onUnitDestroy(BWAPI::Unit * unit);
	void						      onUnitMorph(BWAPI::Unit * unit);
	void						      onUnitShow(BWAPI::Unit * unit);
	void						      onUnitRenegade(BWAPI::Unit * unit);
	void                              onUnitCreate(BWAPI::Unit* unit);
	void						      finished_with_worker(BWAPI::Unit * unit);

	void						      handle_idle_workers();
	void						      handle_gas_workers();
	void						      handle_move_workers();
	void						      handle_combat_workers();

	void						      update_worker_status();
	void						      draw_worker_information(int x, int y);

	int							      get_num_mineral_workers();
	int							      get_num_gas_workers();
	int							      get_num_idle_workers();
	void						      set_scout_worker(BWAPI::Unit * worker);

	bool						      is_worker_scout(BWAPI::Unit * worker);
	bool						      is_free(BWAPI::Unit * worker);
	bool						      is_builder(BWAPI::Unit * worker);

	BWAPI::Unit *				      get_builder(Building & b, bool setJobAsBuilder = true);
	BWAPI::Unit *				      get_move_worker(BWAPI::Position p);
	BWAPI::Unit *				      get_closest_depot(BWAPI::Unit * worker);
	BWAPI::Unit *				      get_gas_worker(BWAPI::Unit * refinery);
	BWAPI::Unit *				      get_closest_enemy_unit(BWAPI::Unit * worker);

	bool						      will_have_resources(int minerals_required, int gas_required, double distance);
	void						      set_move_worker(int m, int g, BWAPI::Position p);
	void                              insert_worker(BWAPI::Unit * unit);
   
};

