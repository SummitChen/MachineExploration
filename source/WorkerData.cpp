#include "Common.h"
#include "WorkerData.h"

WorkerData::WorkerData() {}

void WorkerData::worker_destroyed(BWAPI::Unit * unit)
{
	if (!unit) { return; }

	clear_previous_job(unit);
	workers.erase(unit);
}

void WorkerData::add_worker(BWAPI::Unit * unit)
{
	if (!unit) { return; }

	workers.insert(unit);
	worker_job_map[unit] = Default;
}

void WorkerData::add_worker(BWAPI::Unit * unit, WorkerJob job, BWAPI::Unit * job_unit)
{
	if (!unit || !job_unit) { return; }

	assert(workers.find(unit) == workers.end());

	workers.insert(unit);
	set_worker_job(unit, job, job_unit);
}

void WorkerData::add_worker(BWAPI::Unit * unit, enum WorkerJob job, BWAPI::UnitType job_unit_type)
{
	if (!unit) { return; }

	assert(workers.find(unit) == workers.end());
	workers.insert(unit);
	set_worker_job(unit, job, job_unit_type);
}

void WorkerData::add_depot(BWAPI::Unit * unit)
{
	if (!unit) { return; }

	assert(depots.find(unit) == depots.end());
	depots.insert(unit);
	depot_worker_count[unit] = 0;
}

void WorkerData::remove_depot(BWAPI::Unit * unit)
{	
	if (!unit) { return; }

	depots.erase(unit);
	depot_worker_count.erase(unit);

	// re-balance workers in here
	BOOST_FOREACH (BWAPI::Unit * worker, workers)
	{
		// if a worker was working at this depot
		if (worker_depot_map[worker] == unit)
		{
			set_worker_job(worker, Idle, NULL);
		}
	}
}

void WorkerData::set_worker_job(BWAPI::Unit * unit, enum WorkerJob job, BWAPI::Unit * job_unit)
{
	if (!unit) { return; }

	clear_previous_job(unit);
	worker_job_map[unit] = job;

	if (job == Minerals)
	{
		// increase the number of workers assigned to this nexus
		depot_worker_count[job_unit] += 1;

		// set the mineral the worker is working on
		worker_depot_map[unit] = job_unit;

		// right click the mineral to start mining
		unit->rightClick(get_mineral_to_mine(unit));
	}
	else if (job == Gas)
	{
		// increase the count of workers assigned to this refinery
		refinery_worker_count[job_unit] += 1;

		// set the refinery the worker is working on
		worker_refinery_map[unit] = job_unit;

		// right click the refinery to start harvesting
		unit->rightClick(job_unit);
	}
	else if (job == Repair)
	{
		// only SCVs can repair
		assert(unit->getType() == BWAPI::UnitTypes::Terran_SCV);

		// set the building the worker is to repair
		worker_repair_map[unit] = job_unit;

		// start repairing it
		unit->repair(job_unit);
	}
	else if (job == Scout)
	{

	}
}

void WorkerData::set_worker_job(BWAPI::Unit * unit, enum WorkerJob job, BWAPI::UnitType job_unit_type)
{
	if (!unit) { return; }

	clear_previous_job(unit);
	worker_job_map[unit] = job;

	if (job == Build)
	{
		worker_building_type_map[unit] = job_unit_type;
	}
}

void WorkerData::set_worker_job(BWAPI::Unit * unit, enum WorkerJob job, WorkerMoveData wmd)
{
	if (!unit) { return; }

	clear_previous_job(unit);
	worker_job_map[unit] = job;

	if (job == Move)
	{
		worker_move_map[unit] = wmd;
	}

	if (worker_job_map[unit] != Move)
	{
		BWAPI::Broodwar->printf("Something went horribly wrong");
	}
}


void WorkerData::clear_previous_job(BWAPI::Unit * unit)
{
	if (!unit) { return; }

	WorkerJob previousJob = get_worker_job(unit);

	if (previousJob == Minerals)
	{
		depot_worker_count[worker_depot_map[unit]] -= 1;

		worker_depot_map.erase(unit);
	}
	else if (previousJob == Gas)
	{
		refinery_worker_count[worker_refinery_map[unit]] -= 1;
		worker_refinery_map.erase(unit);
	}
	else if (previousJob == Build)
	{
		worker_building_type_map.erase(unit);
	}
	else if (previousJob == Repair)
	{
		worker_refinery_map.erase(unit);
	}
	else if (previousJob == Move)
	{
		worker_move_map.erase(unit);
	}

	worker_job_map.erase(unit);
}

int WorkerData::get_num_workers()
{
	return workers.size();
}

int WorkerData::get_num_mineral_workers()
{
	size_t num = 0;
	BOOST_FOREACH (BWAPI::Unit * unit, workers)
	{
		if (worker_job_map[unit] == WorkerData::Minerals)
		{
			num++;
		}
	}
	return num;
}

int WorkerData::get_num_gas_workers()
{
	size_t num = 0;
	BOOST_FOREACH (BWAPI::Unit * unit, workers)
	{
		if (worker_job_map[unit] == WorkerData::Gas)
		{
			num++;
		}
	}
	return num;
}

int WorkerData::get_num_idle_workers()
{
	size_t num = 0;
	BOOST_FOREACH (BWAPI::Unit * unit, workers)
	{
		if (worker_job_map[unit] == WorkerData::Idle)
		{
			num++;
		}
	}
	return num;
}


enum WorkerData::WorkerJob WorkerData::get_worker_job(BWAPI::Unit * unit)
{
	if (!unit) { return Default; }

	std::map<BWAPI::Unit *, enum WorkerJob>::iterator it = worker_job_map.find(unit);

	if (it != worker_job_map.end())
	{
		return it->second;
	}

	return Default;
}

bool WorkerData::depot_is_full(BWAPI::Unit * depot)
{
	if (!depot) { return false; }

	int assigned_workers = get_num_assigned_workers(depot);
	int minerals_near_depot = get_minerals_near_depot(depot);

	if (assigned_workers >= minerals_near_depot * 3)
	{
		return true;
	}
	else
	{
		return false;
	}
}

int WorkerData::get_minerals_near_depot(BWAPI::Unit * depot)
{
	if (!depot) { return 0; }

	int minerals_near_depot = 0;

	BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->getAllUnits())
	{
		if ((unit->getType() == BWAPI::UnitTypes::Resource_Mineral_Field) && unit->getDistance(depot) < 200)
		{
			minerals_near_depot++;
		}
	}

	return minerals_near_depot;
}

BWAPI::Unit * WorkerData::get_worker_resource(BWAPI::Unit * unit)
{
	if (!unit) { return NULL; }

	// create the iterator
	std::map<BWAPI::Unit *, BWAPI::Unit *>::iterator it;
	
	// if the worker is mining, set the iterator to the mineral map
	if (get_worker_job(unit) == Minerals)
	{
		it = worker_mineral_map.find(unit);
		if (it != worker_mineral_map.end())
		{
			return it->second;
		}	
	}
	else if (get_worker_job(unit) == Gas)
	{
		it = worker_refinery_map.find(unit);
		if (it != worker_refinery_map.end())
		{
			return it->second;
		}	
	}

	return NULL;
}

BWAPI::Unit * WorkerData::get_mineral_to_mine(BWAPI::Unit * worker)
{
	if (!worker) { return NULL; }

	// get the depot associated with this unit
	BWAPI::Unit * depot = get_worker_depot(worker);
	BWAPI::Unit * mineral = NULL;
	double closestDist = 10000;

	if (depot)
	{
		BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->getAllUnits())
		{
			if (unit->getType() == BWAPI::UnitTypes::Resource_Mineral_Field && unit->getResources() > 0)
			{
				double dist = unit->getDistance(depot);

				if (!mineral || dist < closestDist)
				{
					mineral = unit;
					closestDist = dist;
				}
			}
		}
	}

	return mineral;
}

BWAPI::Unit * WorkerData::get_worker_repair_unit(BWAPI::Unit * unit)
{
	if (!unit) { return NULL; }

	std::map<BWAPI::Unit *, BWAPI::Unit *>::iterator it = worker_repair_map.find(unit);

	if (it != worker_repair_map.end())
	{
		return it->second;
	}	

	return NULL;
}

BWAPI::Unit * WorkerData::get_worker_depot(BWAPI::Unit * unit)
{
	if (!unit) { return NULL; }

	std::map<BWAPI::Unit *, BWAPI::Unit *>::iterator it = worker_depot_map.find(unit);

	if (it != worker_depot_map.end())
	{
		return it->second;
	}	

	return NULL;
}

BWAPI::UnitType	WorkerData::get_worker_building_type(BWAPI::Unit * unit)
{
	if (!unit) { return BWAPI::UnitTypes::None; }

	std::map<BWAPI::Unit *, BWAPI::UnitType>::iterator it = worker_building_type_map.find(unit);

	if (it != worker_building_type_map.end())
	{
		return it->second;
	}	

	return BWAPI::UnitTypes::None;
}

WorkerMoveData WorkerData::get_worker_move_data(BWAPI::Unit * unit)
{
	std::map<BWAPI::Unit *, WorkerMoveData>::iterator it = worker_move_map.find(unit);

	//assert(it != workerMoveMap.end());
	
	return (it->second);
}

int WorkerData::get_num_assigned_workers(BWAPI::Unit * unit)
{
	if (!unit) { return 0; }

	std::map<BWAPI::Unit *, int>::iterator it;
	
	// if the worker is mining, set the iterator to the mineral map
	if (unit->getType().isResourceDepot())
	{
		it = depot_worker_count.find(unit);

		// if there is an entry, return it
		if (it != depot_worker_count.end())
		{
			return it->second;
		}
	}
	else if (unit->getType().isRefinery())
	{
		it = refinery_worker_count.find(unit);

		// if there is an entry, return it
		if (it != refinery_worker_count.end())
		{
			return it->second;
		}
		// otherwise, we are only calling this on completed refineries, so set it
		else
		{
			refinery_worker_count[unit] = 0;
		}
	}

	// when all else fails, return 0
	return 0;
}

char WorkerData::get_job_code(BWAPI::Unit * unit)
{
	if (!unit) { return 'X'; }

	WorkerData::WorkerJob j = get_worker_job(unit);

	if (j == WorkerData::Build) return 'B';
	if (j == WorkerData::Combat) return 'C';
	if (j == WorkerData::Default) return 'D';
	if (j == WorkerData::Gas) return 'G';
	if (j == WorkerData::Idle) return 'I';
	if (j == WorkerData::Minerals) return 'M';
	if (j == WorkerData::Repair) return 'R';
	if (j == WorkerData::Move) return 'O';
	if (j == WorkerData::Scout) return 'S';
	return 'X';
}

void WorkerData::draw_depot_debug_info()
{
	BOOST_FOREACH(BWAPI::Unit * depot, depots)
	{
		int x = depot->getPosition().x() - 64;
		int y = depot->getPosition().y() - 32;

		//if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawBoxMap(x-2, y-1, x+75, y+14, BWAPI::Colors::Black, true);
		//if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextMap(x, y, "\x04 Workers: %d", getNumAssignedWorkers(depot));
	}
}