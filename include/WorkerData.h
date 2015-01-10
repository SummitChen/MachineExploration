#pragma once

#include "Common.h"

class WorkerMoveData
{
public:

	int minerals_needed;
	int gas_needed;
	BWAPI::Position position;

	WorkerMoveData(int m, int g, BWAPI::Position p)
	{
		minerals_needed = m;
		gas_needed = g;
		position = p;
	}

	WorkerMoveData() {}
};

class WorkerData 
{

public:

	enum WorkerJob {Minerals, Gas, Build, Combat, Idle, Repair, Move, Scout, Default};

private:

	std::set<BWAPI::Unit *>							workers;
	std::set<BWAPI::Unit *>							depots;

	std::map<BWAPI::Unit *, enum WorkerJob>			worker_job_map;
	std::map<BWAPI::Unit *, BWAPI::Unit *>			worker_mineral_map;
	std::map<BWAPI::Unit *, BWAPI::Unit *>			worker_depot_map;
	std::map<BWAPI::Unit *, BWAPI::Unit *>			worker_refinery_map;
	std::map<BWAPI::Unit *, BWAPI::Unit *>			worker_repair_map;
	std::map<BWAPI::Unit *, WorkerMoveData>			worker_move_map;
	std::map<BWAPI::Unit *, BWAPI::UnitType>		worker_building_type_map;

	std::map<BWAPI::Unit *, int>					depot_worker_count;
	std::map<BWAPI::Unit *, int>					refinery_worker_count;

	void											clear_previous_job(BWAPI::Unit * unit);

public:

	WorkerData();

	void					worker_destroyed(BWAPI::Unit * unit);
	void					add_depot(BWAPI::Unit * unit);
	void					remove_depot(BWAPI::Unit * unit);
	void					add_worker(BWAPI::Unit * unit);
	void					add_worker(BWAPI::Unit * unit, WorkerJob job, BWAPI::Unit * job_unit);
	void					add_worker(BWAPI::Unit * unit, WorkerJob job, BWAPI::UnitType job_unit_type);
	void					set_worker_job(BWAPI::Unit * unit, WorkerJob job, BWAPI::Unit * job_unit);
	void					set_worker_job(BWAPI::Unit * unit, WorkerJob job, WorkerMoveData wmd);
	void					set_worker_job(BWAPI::Unit * unit, WorkerJob job, BWAPI::UnitType job_unit_type);

	int						get_num_workers();
	int						get_num_mineral_workers();
	int						get_num_gas_workers();
	int						get_num_idle_workers();
	char					get_job_code(BWAPI::Unit * unit);

	void					get_mineral_workers(std::set<BWAPI::Unit *> & mw);
	void					get_gas_workers(std::set<BWAPI::Unit *> & mw);
	void					get_building_workers(std::set<BWAPI::Unit *> & mw);
	void					get_repair_workers(std::set<BWAPI::Unit *> & mw);
	
	bool					depot_is_full(BWAPI::Unit * depot);
	int						get_minerals_near_depot(BWAPI::Unit * depot);

	int						get_num_assigned_workers(BWAPI::Unit * unit);
	BWAPI::Unit *			get_mineral_to_mine(BWAPI::Unit * worker);

	enum WorkerJob			get_worker_job(BWAPI::Unit * unit);
	BWAPI::Unit *			get_worker_resource(BWAPI::Unit * unit);
	BWAPI::Unit *			get_worker_depot(BWAPI::Unit * unit);
	BWAPI::Unit *			get_worker_repair_unit(BWAPI::Unit * unit);
	BWAPI::UnitType			get_worker_building_type(BWAPI::Unit * unit);
	WorkerMoveData			get_worker_move_data(BWAPI::Unit * unit);

	void					draw_depot_debug_info();

	const std::set<BWAPI::Unit *> & get_workers() const { return workers; }
};
