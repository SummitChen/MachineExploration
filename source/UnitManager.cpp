#include "..\include\UnitManager.h"
#include "Common.h"

UnitManager* UnitManager::um_instance = NULL;

UnitManager::UnitManager(void)
{
}


UnitManager::~UnitManager(void)
{
}

UnitManager* UnitManager::get_instance()
{
	if ( um_instance == NULL)
	{
		um_instance = new UnitManager();
	}

	return um_instance;
}

void UnitManager::destroy_instance(){
	
	if ( um_instance != NULL)
	{
		delete um_instance;
		um_instance = NULL;
	}

}

void UnitManager::update()
{
	// worker bookkeeping
	update_worker_status();

	// set the gas workers
	handle_gas_workers();

	// handle idle workers
	handle_idle_workers();

	// handle move workers
	handle_move_workers();

	// handle combat workers
	handle_combat_workers();
}

void UnitManager::onUnitDestroy( BWAPI::Unit * unit )
{
	if (unit == NULL) 
	{
		assert(false);
	}

	// remove the depot if it exists
	if (unit->getType().isResourceDepot() && unit->getPlayer() == BWAPI::Broodwar->self())
	{
		worker_data.remove_depot(unit);
	}

	// if the unit that was destroyed is a worker
	if (unit->getType().isWorker() && unit->getPlayer() == BWAPI::Broodwar->self()) 
	{
		// tell the worker data it was destroyed
		worker_data.worker_destroyed(unit);
	}
}

void UnitManager::onUnitShow( BWAPI::Unit * unit )
{
	if ( unit == NULL )
	{
		assert(false);
	}
	
	// if the unit is new depot
	if ( unit->getType().isResourceDepot() && unit->getPlayer() ==  BWAPI::Broodwar->self())
	{
		worker_data.add_depot( unit );
	}

	// if the unit that is a worker
	if ( unit->getType().isWorker() && unit->getPlayer() == BWAPI::Broodwar->self())
	{
		worker_data.add_worker(unit);
	}
}

void UnitManager::onUnitMorph( BWAPI::Unit * unit )
{
	if ( unit == NULL)
	{
		assert(false);
	}

	// if the unit that is a worker
	if ( unit->getType().isWorker() && unit->getPlayer() == BWAPI::Broodwar->self())
	{
		worker_data.add_worker( unit);
	}

	// if something morphs into a building, it was a worker?
	if (unit->getType().isBuilding() && unit->getPlayer() == BWAPI::Broodwar->self() && unit->getPlayer()->getRace() == BWAPI::Races::Zerg)
	{
		//BWAPI::Broodwar->printf("A Drone started building");
		worker_data.worker_destroyed(unit);
	}
}

void UnitManager::onUnitRenegade( BWAPI::Unit * unit )
{

}

// give back the units
void UnitManager::finished_with_worker( BWAPI::Unit * unit )
{
	if (unit == NULL)
	{
		BWAPI::Broodwar->printf("finishedWithWorker() called with NULL unit");
		return; 
	}

	//BWAPI::Broodwar->printf("BuildingManager finished with worker %d", unit->getID());
	if (worker_data.get_worker_job(unit) != WorkerData::Scout)
	{
		worker_data.set_worker_job(unit, WorkerData::Idle, NULL);
	}
}

// Simply assign idle workers to mine
void UnitManager::handle_idle_workers()
{
	BOOST_FOREACH( BWAPI::Unit* unit, worker_data.get_workers()){
		if ( worker_data.get_worker_job(unit) == WorkerData::Idle)
		{
			set_mineral_worker(unit);
		}
	}
}

void UnitManager::set_mineral_worker( BWAPI::Unit * unit )
{
	if ( unit == NULL)
	{
		assert(false);
	}

	// check if there is a mineral available to send the worker to
	BWAPI::Unit* depot = get_closest_depot(unit);

	// if there is a valid mineral
	if ( depot )
	{
		// update workerData with the new job
		worker_data.set_worker_job( unit,  WorkerData::Minerals, depot);
	}else{

	}
}

/** 
*  Normally, assigning three workers in each refinery would be  
*
*/
void UnitManager::handle_gas_workers()
{
	BOOST_FOREACH( BWAPI::Unit* unit, BWAPI::Broodwar->self()->getUnits()){

		// if that unit is a refinery
		if ( unit->getType().isRefinery() && unit->isCompleted())
		{
			// get the number of workers currently assigned to it
			int num_assigned = worker_data.get_num_assigned_workers(unit);
			
			for( int i = 0; i < (WORKER_PER_REFINERY - num_assigned); i++){

				BWAPI::Unit* gasworker = get_gas_worker( unit);
				if ( gasworker )
				{
					worker_data.set_worker_job( gasworker, WorkerData::Gas, unit);
				}
			}
		}

	}
}

BWAPI::Unit * UnitManager::get_gas_worker( BWAPI::Unit * refinery )
{
	if ( refinery == NULL)
	{
		assert(false);
	}

	BWAPI::Unit* closest_worker = NULL;
	double closest_distance = 0;

	BOOST_FOREACH( BWAPI::Unit* unit, worker_data.get_workers()){

		if ( worker_data.get_worker_job( unit) == WorkerData::Minerals)
		{
			double _distance = unit->getDistance( refinery );
            
			if ( !closest_worker || _distance < closest_distance)
			{
				closest_worker = unit;
				closest_distance = _distance;
			}
		}
	}

	return closest_worker;
}


BWAPI::Unit * UnitManager::get_closest_depot( BWAPI::Unit * worker )
{
	if ( worker == NULL)
	{
		assert(false);
	}

	BWAPI::Unit* closest_depot = NULL;
	int closest_distance = 0;

	BOOST_FOREACH( BWAPI::Unit* unit, BWAPI::Broodwar->self()->getUnits()){

		if ( unit->getType().isResourceDepot() && unit->isCompleted())
		{
			int _distance = worker->getDistance(unit);

			if ( !closest_depot || _distance < closest_distance)
			{
				closest_depot = unit;
				closest_distance = _distance;
			}
		}
	}

	return closest_depot;
}

/**
*  Command workers to move to a certain location
*/

void UnitManager::handle_move_workers()
{
	BOOST_FOREACH( BWAPI::Unit * worker, worker_data.get_workers()){

		if ( worker_data.get_worker_job( worker) == WorkerData::Move)
		{
			WorkerMoveData data = worker_data.get_worker_move_data( worker );

			worker->move(data.position);
		}
	}
}

void UnitManager::handle_combat_workers()
{

}

void UnitManager::update_worker_status()
{
	BOOST_FOREACH( BWAPI::Unit* worker, worker_data.get_workers()){

		if ( !worker->isCompleted())
		{
			continue;
		}

		// handle idle workers
		if ( worker->isIdle()&&
			 (worker_data.get_worker_job( worker) != WorkerData::Build) &&
			 (worker_data.get_worker_job( worker) != WorkerData::Move) &&
			 (worker_data.get_worker_job(worker) != WorkerData::Scout))
		{
			worker_data.set_worker_job( worker, WorkerData::Idle, NULL);
		}

		// handle gas workers
		if ( worker_data.get_worker_job( worker ) == WorkerData::Gas)
		{
			BWAPI::Unit* refinery = worker_data.get_worker_resource( worker );

			if ( !refinery || !refinery->exists() || refinery->getHitPoints() <= 0)
			{
				set_mineral_worker(worker);
			}
		}
		
	}
}

void UnitManager::draw_worker_information( int x, int y )
{

}

bool UnitManager::is_free(BWAPI::Unit * worker)
{
	return worker_data.get_worker_job(worker) == WorkerData::Minerals || worker_data.get_worker_job(worker) == WorkerData::Idle;
}

bool UnitManager::is_worker_scout(BWAPI::Unit * worker)
{
	return (worker_data.get_worker_job(worker) == WorkerData::Scout);
}

bool UnitManager::is_builder(BWAPI::Unit * worker)
{
	return (worker_data.get_worker_job(worker) == WorkerData::Build);
}

int UnitManager::get_num_mineral_workers()
{
	return worker_data.get_num_mineral_workers();
}

int UnitManager::get_num_gas_workers()
{
	return worker_data.get_num_gas_workers();
}

int UnitManager::get_num_idle_workers()
{
	return worker_data.get_num_idle_workers();
}

void UnitManager::set_scout_worker( BWAPI::Unit * worker )
{
	if ( worker == NULL)
	{
		assert(false);
	}

	worker_data.set_worker_job( worker, WorkerData::Scout, NULL);

}

// Random interface
BWAPI::Unit * UnitManager::get_move_worker( BWAPI::Position p )
{
	return NULL;
}

// Random interface
BWAPI::Unit * UnitManager::get_closest_enemy_unit( BWAPI::Unit * worker )
{
	return NULL;
}

/**
*  Get a builder which is very near to the position where we tend to locate a building.
*  Unfinished
*/
BWAPI::Unit * UnitManager::get_builder( Building & b, bool set_job_as_builder /*= true*/ )
{
    BWAPI::Unit * closest_moving_worker = NULL;
	BWAPI::Unit * closest_mining_worker = NULL;
	double closest_moving_worker_distance = 0;
	double closest_mining_worker_distance = 0;

	BOOST_FOREACH(BWAPI::Unit * unit, worker_data.get_workers()){

		// check mining workers
		if ( unit->isCompleted() && worker_data.get_worker_job( unit ) == WorkerData::Minerals)
		{
			double _distance = unit->getDistance( BWAPI::Position(b.final_position));

			if ( !closest_mining_worker || _distance < closest_mining_worker_distance )
			{
				closest_mining_worker = unit;
				closest_mining_worker_distance = _distance;
			}
		}

		// check moving workers
		if ( unit->isCompleted() && worker_data.get_worker_job( unit ) == WorkerData::Move)
		{
			double _distance = unit->getDistance( BWAPI::Position(b.final_position));

			if ( !closest_mining_worker || _distance < closest_mining_worker_distance )
			{
				closest_mining_worker = unit;
				closest_mining_worker_distance = _distance;
			}
		}

		BWAPI::Unit* chosen_worker = closest_moving_worker ? closest_moving_worker : closest_mining_worker;

		if ( chosen_worker && set_job_as_builder)
		{
			worker_data.set_worker_job( chosen_worker, WorkerData::Build, b.type);
		}
	}

	return false;
}

bool UnitManager::will_have_resources( int minerals_required, int gas_required, double distance )
{
	// if we don't require anything, we will have it
	if (minerals_required <= 0 && gas_required <= 0)
	{
		return true;
	}

	// the speed of the worker unit
	double speed = BWAPI::Broodwar->self()->getRace().getWorker().topSpeed();

	// how many frames it will take us to move to the building location
	// add a second to account for worker getting stuck. better early than late
	double frames_to_move = (distance / speed) + 50;

	// magic numbers to predict income rates
	double mineral_rate = get_num_mineral_workers() * 0.045;
	double gas_rate     = get_num_gas_workers() * 0.07;

	// calculate if we will have enough by the time the worker gets there
	if (mineral_rate * frames_to_move >= minerals_required &&
		gas_rate * frames_to_move >= gas_required)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void UnitManager::set_move_worker( int minerals_needed, int gas_needed, BWAPI::Position p )
{
	// set up the pointer
	BWAPI::Unit * closest_worker = NULL;
	double closest_distance = 0;

	// for each worker we currently have
	BOOST_FOREACH (BWAPI::Unit * unit, worker_data.get_workers())
	{
		// only consider it if it's a mineral worker
		if (unit->isCompleted() && worker_data.get_worker_job(unit) == WorkerData::Minerals)
		{
			// if it is a new closest distance, set the pointer
			double distance = unit->getDistance(p);
			if (!closest_worker|| distance < closest_distance)
			{
				closest_worker = unit;
				closest_distance = distance;
			}
		}
	}

	if (closest_worker)
	{
		//BWAPI::Broodwar->printf("Setting worker job Move for worker %d", closestWorker->getID());
		worker_data.set_worker_job(closest_worker, WorkerData::Move, WorkerMoveData( minerals_needed, gas_needed, p));
	}
	else
	{
		//BWAPI::Broodwar->printf("Error, no worker found");
	}
}

void UnitManager::onUnitCreate( BWAPI::Unit* unit )
{
	if ( unit == NULL)
	{
		assert(false);
	}

	// if the unit that is a worker
	if ( unit->getType().isWorker() && unit->getPlayer() == BWAPI::Broodwar->self())
	{
		worker_data.add_worker( unit);
	}

}

void UnitManager::insert_worker( BWAPI::Unit * unit )
{
	if ( unit == NULL)
	{
		assert(false);
	}

	if ( unit->getType().isWorker() && unit->getPlayer() == BWAPI::Broodwar->self())
	{
		worker_data.add_worker( unit );
	}
}






