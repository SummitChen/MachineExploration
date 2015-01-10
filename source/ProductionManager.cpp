#include "Common.h"
#include "ProductionManager.h"

//#include "..\..\StarcraftBuildOrderSearch\Source\starcraftsearch\ActionSet.hpp"
//#include "..\..\StarcraftBuildOrderSearch\Source\starcraftsearch\DFBBStarcraftSearch.hpp"
//#include "..\..\StarcraftBuildOrderSearch\Source\starcraftsearch\StarcraftData.hpp"

#define BOADD(N, T, B) for (int i=0; i<N; ++i) { queue.queueAsLowestPriority(MetaType(T), B); }

#define GOAL_ADD(G, M, N) G.push_back(std::pair<MetaType, int>(M, N))

ProductionManager* ProductionManager::pm_instance = NULL;

ProductionManager::ProductionManager() 
	: initial_build_set(false)
	, reserved_minerals(0)
	, reserved_gas(0)
	, assigned_worker_for_this_building(false)
	, have_location_for_this_building(false)
	, enemy_cloaked_detected(false)
	, rush_detected(false)
{
	populate_type_char_map();

	// Fundamental experiment for produce workers only
    std::vector<MetaType> build_order;
	for ( int i = 0; i < 20; ++i)
	{
		//build_order.push_back( MetaType(BWAPI::Broodwar->self()->getRace()));
	}

	set_build_order( build_order );
}

void ProductionManager::set_build_order(const std::vector<MetaType> & buildOrder)
{
	// clear the current build order
	queue.clear_all();

	// for each item in the results build order, add it
	for (size_t i(0); i<buildOrder.size(); ++i)
	{
		// queue the item
		queue.queue_as_lowest_priority(buildOrder[i], true);
	}
}

/*
void ProductionManager::performBuildOrderSearch(const std::vector< std::pair<MetaType, UnitCountType> > & goal)
{	
	std::vector<MetaType> buildOrder = StarcraftBuildOrderSearchManager::Instance().findBuildOrder(goal);

	// set the build order
	setBuildOrder(buildOrder);
}
*/

void ProductionManager::update() 
{
	// check the queue for stuff we can build
	manage_build_order_queue();

	/*
	// if nothing is currently building, get a new goal from the strategy manager
	if (queue.size() == 0 && BWAPI::Broodwar->getFrameCount() > 100)
	{
		BWAPI::Broodwar->drawTextScreen(150, 10, "Nothing left to build, new search!");
		const std::vector< std::pair<MetaType, UnitCountType> > newGoal = StrategyManager::Instance().getBuildOrderGoal();
		performBuildOrderSearch(newGoal);
	}
	*/

	// detect if there's a build order deadlock once per second
	if ((BWAPI::Broodwar->getFrameCount() % 24 == 0) && detect_build_order_dead_lock())
	{
		BWAPI::Broodwar->printf("Supply deadlock detected, building pylon!");
		queue.queue_as_highest_priority(MetaType(BWAPI::Broodwar->self()->getRace().getSupplyProvider()), true);
	}

	/*
	// if they have cloaked units get a new goal asap
	if (!enemyCloakedDetected && InformationManager::Instance().enemyHasCloakedUnits())
	{
		if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Photon_Cannon) < 2)
		{
			queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Protoss_Photon_Cannon), true);
			queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Protoss_Photon_Cannon), true);
		}

		if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Forge) == 0)
		{
			queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Protoss_Forge), true);
		}

		BWAPI::Broodwar->printf("Enemy Cloaked Unit Detected!");
		enemyCloakedDetected = true;
	}
	*/


//	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(447, 17, "\x07 %d", BuildingManager::Instance().getReservedMinerals());
}

// on unit destroy
void ProductionManager::on_unit_destroy(BWAPI::Unit * unit)
{
	// we don't care if it's not our unit
	if (!unit || unit->getPlayer() != BWAPI::Broodwar->self())
	{
		return;
	}
		
	// if it's a worker or a building, we need to re-search for the current goal
	if ((unit->getType().isWorker() && !UnitManager::get_instance()->is_worker_scout(unit)) || unit->getType().isBuilding())
	{
		BWAPI::Broodwar->printf("Critical unit died, re-searching build order");

		/*
		if (unit->getType() != BWAPI::UnitTypes::Zerg_Drone)
		{
			performBuildOrderSearch(StrategyManager::Instance().getBuildOrderGoal());
		}
		*/
	}
}

void ProductionManager::manage_build_order_queue() 
{
	// if there is nothing in the queue, oh well
	if (queue.is_empty()) 
	{
		return;
	}

	// the current item to be used
	BuildOrderItem<PRIORITY_TYPE> & currentItem = queue.get_highest_priority_item();

	// while there is still something left in the queue
	while (!queue.is_empty()) 
	{
		// this is the unit which can produce the currentItem
		BWAPI::Unit * producer = select_unit_of_type(currentItem.meta_type.what_builds());

		// check to see if we can make it right now
		bool _can_make = can_make_now(producer, currentItem.meta_type);

		// if we try to build too many refineries manually remove it
		if (currentItem.meta_type.is_refinery() && (BWAPI::Broodwar->self()->allUnitCount(BWAPI::Broodwar->self()->getRace().getRefinery() >= 3)))
		{
			queue.remove_current_highest_priority_item();
			break;
		}

		// if the next item in the list is a building and we can't yet make it
		if (currentItem.meta_type.is_building() && !(producer && _can_make))
		{
			// construct a temporary building object
			Building b(currentItem.meta_type.unit_type, BWAPI::Broodwar->self()->getStartLocation());

			// set the producer as the closest worker, but do not set its job yet
			producer = UnitManager::get_instance()->get_builder(b, false);

			// predict the worker movement to that building location
			predict_worker_movement(b);
		}

		// if we can make the current item
		if (producer && _can_make) 
		{
			// create it
			create_meta_type(producer, currentItem.meta_type);
			assigned_worker_for_this_building = false;
			have_location_for_this_building = false;

			// and remove it from the queue
			queue.remove_current_highest_priority_item();

			// don't actually loop around in here
			break;
		}
		// otherwise, if we can skip the current item
		else if (queue.can_skip_item())
		{
			// skip it
			queue.skip_item();

			// and get the next one
			currentItem = queue.get_next_highest_priority_item();				
		}
		else 
		{
			// so break out
			break;
		}
	}
}


bool ProductionManager::can_make_now(BWAPI::Unit * producer, MetaType t)
{
	bool _can_make = meets_reserved_resources(t);
	if (_can_make)
	{
		if (t.is_unit())
		{
			_can_make = BWAPI::Broodwar->canMake(producer, t.unit_type);
		}
		else if (t.is_tech())
		{
			_can_make = BWAPI::Broodwar->canResearch(producer, t.tech_type);
		}
		else if (t.is_upgrade())
		{
			_can_make = BWAPI::Broodwar->canUpgrade(producer, t.upgrade_type);
		}
		else
		{	
			assert(false);
		}
	}

	return _can_make;
}

bool ProductionManager::detect_build_order_dead_lock()
{
	// if the queue is empty there is no deadlock
	if (queue.size() == 0 || BWAPI::Broodwar->self()->supplyTotal() >= 390)
	{
		return false;
	}

	// are any supply providers being built currently
	bool supply_in_progress =		ConstrucitionManager::get_instance()->is_being_built(BWAPI::Broodwar->self()->getRace().getCenter()) || 
								ConstrucitionManager::get_instance()->is_being_built(BWAPI::Broodwar->self()->getRace().getSupplyProvider());

	// does the current item being built require more supply
	int supply_cost			    = queue.get_highest_priority_item().meta_type.supply_required();
	int supply_available		= std::max(0, BWAPI::Broodwar->self()->supplyTotal() - BWAPI::Broodwar->self()->supplyUsed());

	// if we don't have enough supply and none is being built, there's a deadlock
	if ((supply_available < supply_cost) && !supply_in_progress)
	{
		return true;
	}

	return false;
}

// When the next item in the queue is a building, this checks to see if we should move to it
// This function is here as it needs to access prodction manager's reserved resources info
void ProductionManager::predict_worker_movement(const Building & b)
{
	// get a possible building location for the building
	if (!have_location_for_this_building)
	{
		predicted_tile_position			= ConstrucitionManager::get_instance()->get_building_location(b);
	}

	if (predicted_tile_position != BWAPI::TilePositions::None)
	{
		have_location_for_this_building		= true;
	}
	else
	{
		return;
	}
	
	// draw a box where the building will be placed
	int x1 = predicted_tile_position.x() * 32;
	int x2 = x1 + (b.type.tileWidth()) * 32;
	int y1 = predicted_tile_position.y() * 32;
	int y2 = y1 + (b.type.tileHeight()) * 32;
	//if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawBoxMap(x1, y1, x2, y2, BWAPI::Colors::Blue, false);

#ifdef DEBUGGING_OUTPUT
	BWAPI::Broodwar->drawBoxMap(x1, y1, x2, y2, BWAPI::Colors::Blue, false);
#endif

	// where we want the worker to walk to
	BWAPI::Position walk_to_position		= BWAPI::Position(x1 + (b.type.tileWidth()/2)*32, y1 + (b.type.tileHeight()/2)*32);

	// compute how many resources we need to construct this building
	int minerals_required				= std::max(0, b.type.mineralPrice() - get_free_minerals());
	int gas_required					= std::max(0, b.type.gasPrice() - get_free_gas());

	// get a candidate worker to move to this location
	BWAPI::Unit * move_worker			= UnitManager::get_instance()->get_move_worker( walk_to_position );

	// Conditions under which to move the worker: 
	//		- there's a valid worker to move
	//		- we haven't yet assigned a worker to move to this location
	//		- the build position is valid
	//		- we will have the required resources by the time the worker gets there
	if (move_worker && have_location_for_this_building && !assigned_worker_for_this_building&& (predicted_tile_position!= BWAPI::TilePositions::None) &&
		UnitManager::get_instance()->will_have_resources(minerals_required, gas_required, move_worker->getDistance(walk_to_position)) )
	{
		// we have assigned a worker
		assigned_worker_for_this_building = true;

		// tell the worker manager to move this worker
		UnitManager::get_instance()->set_move_worker(minerals_required, gas_required, walk_to_position);
	}
}

void ProductionManager::perform_command(BWAPI::UnitCommandType t) {

	// if it is a cancel construction, it is probably the extractor trick
	if (t == BWAPI::UnitCommandTypes::Cancel_Construction)
	{
		BWAPI::Unit * extractor = NULL;
		BOOST_FOREACH(BWAPI::Unit * unit, BWAPI::Broodwar->self()->getUnits())
		{
			if (unit->getType() == BWAPI::UnitTypes::Zerg_Extractor)
			{
				extractor = unit;
			}
		}

		if (extractor)
		{
			extractor->cancelConstruction();
		}
	}
}

int ProductionManager::get_free_minerals()
{
	return BWAPI::Broodwar->self()->minerals() - ConstrucitionManager::get_instance()->get_reserved_minerals();
}

int ProductionManager::get_free_gas()
{
	return BWAPI::Broodwar->self()->gas() - ConstrucitionManager::get_instance()->get_reserved_gas();
}

// return whether or not we meet resources, including building reserves
bool ProductionManager::meets_reserved_resources(MetaType type) 
{
	// return whether or not we meet the resources
	return (type.mineral_price() <= get_free_minerals()) && (type.gas_price() <= get_free_gas());
}

// this function will check to see if all preconditions are met and then create a unit
void ProductionManager::create_meta_type(BWAPI::Unit * producer, MetaType t) 
{
	if (!producer)
	{
		return;
	}

	// TODO: special case of evolved zerg buildings needs to be handled

	// if we're dealing with a building
	if (t.is_unit() && t.unit_type.isBuilding() 
		&& t.unit_type != BWAPI::UnitTypes::Zerg_Lair 
		&& t.unit_type != BWAPI::UnitTypes::Zerg_Hive
		&& t.unit_type != BWAPI::UnitTypes::Zerg_Greater_Spire)
	{
		// send the building task to the building manager
		ConstrucitionManager::get_instance()->add_building_task(t.unit_type, BWAPI::Broodwar->self()->getStartLocation());
	}
	// if we're dealing with a non-building unit
	else if (t.is_unit()) 
	{
		// if the race is zerg, morph the unit
		if (t.unit_type.getRace() == BWAPI::Races::Zerg) {
			producer->morph(t.unit_type);

		// if not, train the unit
		} else {
			producer->train(t.unit_type);
		}
	}
	// if we're dealing with a tech research
	else if (t.is_tech())
	{
		producer->research(t.tech_type);
	}
	else if (t.is_upgrade())
	{
		//Logger::Instance().log("Produce Upgrade: " + t.getName() + "\n");
		producer->upgrade(t.upgrade_type);
	}
	else
	{	
		// critical error check
//		assert(false);

		//Logger::Instance().log("createMetaType error: " + t.getName() + "\n");
	}
	
}

// selects a unit of a given type
BWAPI::Unit * ProductionManager::select_unit_of_type(BWAPI::UnitType type, bool leastTrainingTimeRemaining, BWAPI::Position closestTo) {

	// if we have none of the unit type, return NULL right away
	if (BWAPI::Broodwar->self()->completedUnitCount(type) == 0) 
	{
		return NULL;
	}

	BWAPI::Unit * unit = NULL;

	// if we are concerned about the position of the unit, that takes priority
	if (closestTo != BWAPI::Position(0,0)) {

		double minDist(1000000);

		BOOST_FOREACH (BWAPI::Unit * u, BWAPI::Broodwar->self()->getUnits()) {

			if (u->getType() == type) {

				double distance = u->getDistance(closestTo);
				if (!unit || distance < minDist) {
					unit = u;
					minDist = distance;
				}
			}
		}

		// if it is a building and we are worried about selecting the unit with the least
		// amount of training time remaining
	} else if (type.isBuilding() && leastTrainingTimeRemaining) {

		BOOST_FOREACH (BWAPI::Unit * u, BWAPI::Broodwar->self()->getUnits()) {

			if (u->getType() == type && u->isCompleted() && !u->isTraining() && !u->isLifted() &&!u->isUnpowered()) {

				return u;
			}
		}
		// otherwise just return the first unit we come across
	} else {

		BOOST_FOREACH(BWAPI::Unit * u, BWAPI::Broodwar->self()->getUnits()) 
		{
			if (u->getType() == type && u->isCompleted() && u->getHitPoints() > 0 && !u->isLifted() &&!u->isUnpowered()) 
			{
				return u;
			}
		}
	}

	// return what we've found so far
	return NULL;
}

void ProductionManager::on_send_text(std::string text)
{
#if 0
	MetaType typeWanted(BWAPI::UnitTypes::None);
	int numWanted = 0;

	if (text.compare("clear") == 0)
	{
		searchGoal.clear();
	}
	else if (text.compare("search") == 0)
	{
		performBuildOrderSearch(searchGoal);
		searchGoal.clear();
	}
	else if (text[0] >= 'a' && text[0] <= 'z')
	{
		MetaType typeWanted = typeCharMap[text[0]];
		text = text.substr(2,text.size());
		numWanted = atoi(text.c_str());

		searchGoal.push_back(std::pair<MetaType, int>(typeWanted, numWanted));
	}
#endif
}

void ProductionManager::populate_type_char_map()
{
#if 0
	typeCharMap['p'] = MetaType(BWAPI::UnitTypes::Protoss_Probe);
	typeCharMap['z'] = MetaType(BWAPI::UnitTypes::Protoss_Zealot);
	typeCharMap['d'] = MetaType(BWAPI::UnitTypes::Protoss_Dragoon);
	typeCharMap['t'] = MetaType(BWAPI::UnitTypes::Protoss_Dark_Templar);
	typeCharMap['c'] = MetaType(BWAPI::UnitTypes::Protoss_Corsair);
	typeCharMap['e'] = MetaType(BWAPI::UnitTypes::Protoss_Carrier);
	typeCharMap['h'] = MetaType(BWAPI::UnitTypes::Protoss_High_Templar);
	typeCharMap['n'] = MetaType(BWAPI::UnitTypes::Protoss_Photon_Cannon);
	typeCharMap['a'] = MetaType(BWAPI::UnitTypes::Protoss_Arbiter);
	typeCharMap['r'] = MetaType(BWAPI::UnitTypes::Protoss_Reaver);
	typeCharMap['o'] = MetaType(BWAPI::UnitTypes::Protoss_Observer);
	typeCharMap['s'] = MetaType(BWAPI::UnitTypes::Protoss_Scout);
	typeCharMap['l'] = MetaType(BWAPI::UpgradeTypes::Leg_Enhancements);
	typeCharMap['v'] = MetaType(BWAPI::UpgradeTypes::Singularity_Charge);
#endif
}

void ProductionManager::draw_production_information(int x, int y)
{
#if 0
	// fill prod with each unit which is under construction
	std::vector<BWAPI::Unit *> prod;
	BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->self()->getUnits())
	{
		if (unit->isBeingConstructed())
		{
			prod.push_back(unit);
		}
	}
	
	// sort it based on the time it was started
	std::sort(prod.begin(), prod.end(), CompareWhenStarted());

	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y, "\x04 Build Order Info:");
	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y+20, "\x04UNIT NAME");

	size_t reps = prod.size() < 10 ? prod.size() : 10;

	y += 40;
	int yy = y;

	// for each unit in the queue
	for (size_t i(0); i<reps; i++) {

		std::string prefix = "\x07";

		yy += 10;

		BWAPI::UnitType t = prod[i]->getType();

		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, yy, "%s%s", prefix.c_str(), t.getName().c_str());
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x+150, yy, "%s%6d", prefix.c_str(), prod[i]->getRemainingBuildTime());
	}

	queue.drawQueueInformation(x, yy+10);
#endif
}


ProductionManager * ProductionManager::get_instance()
{
	if ( pm_instance == NULL)
	{
		pm_instance = new ProductionManager;
	}

	return pm_instance;
}

void ProductionManager::destroy_instance()
{
	if ( pm_instance != NULL)
	{
		delete pm_instance;
		pm_instance = NULL;
	}
}



