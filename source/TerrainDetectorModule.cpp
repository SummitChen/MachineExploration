#include "TerrainDetectorModule.h"
#include "Common.h"
#include "TerrainRuler.h"
#include "TerrainDetectionManager.h"
#include "UnitManager.h"
#include "BWAPI/Color.h"
using namespace BWAPI;

bool analyzed;
bool analysis_just_finished;


void TerrainDetector::onStart()
{
	Broodwar->sendText("Hello world!");

	//BWAPI::Broodwar->setLocalSpeed(0);

	Broodwar->printf("The map is %s, a %d player map",Broodwar->mapName().c_str(),Broodwar->getStartLocations().size());
	// Enable some cheat flags
	Broodwar->enableFlag(Flag::UserInput);
	// Uncomment to enable complete map information
	//Broodwar->enableFlag(Flag::CompleteMapInformation);

	//read map information into BWTA so terrain analysis can be done in another thread

	analyzed=false;
	analysis_just_finished=false;

	show_bullets=false;
	show_visibility_data=false;

	if (Broodwar->isReplay())
	{
		Broodwar->printf("The following players are in this replay:");
		for(std::set<Player*>::iterator p=Broodwar->getPlayers().begin();p!=Broodwar->getPlayers().end();p++)
		{
			if (!(*p)->getUnits().empty() && !(*p)->isNeutral())
			{
				Broodwar->printf("%s, playing as a %s",(*p)->getName().c_str(),(*p)->getRace().getName().c_str());
			}
		}
	}
	else
	{
		Broodwar->printf("The match up is %s v %s",
			Broodwar->self()->getRace().getName().c_str(),
			Broodwar->enemy()->getRace().getName().c_str());

		//send each worker to the mineral field that is closest to it
		for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
		{
			if ((*i)->getType().isWorker())
			{
				/*
				Unit* closestMineral=NULL;
				for(std::set<Unit*>::iterator m=Broodwar->getMinerals().begin();m!=Broodwar->getMinerals().end();m++)
				{
					if (closestMineral==NULL || (*i)->getDistance(*m)<(*i)->getDistance(closestMineral))
						closestMineral=*m;
				}
				if (closestMineral!=NULL){
					(*i)->rightClick(closestMineral);
					mineral_gathering_workers.push_back(*i);
				}
				*/
				UnitManager::get_instance()->insert_worker( *i );

			}
			else if ((*i)->getType().isResourceDepot())
			{
				//if this is a center, tell it to build the appropiate type of worker
				if ((*i)->getType().getRace()!=Races::Zerg)
				{
					//(*i)->train(Broodwar->self()->getRace().getWorker());
				}
				else //if we are Zerg, we need to select a larva and morph it into a drone
				{
					std::set<Unit*> myLarva=(*i)->getLarva();
					if (myLarva.size()>0)
					{
						Unit* larva=*myLarva.begin();
						larva->morph(UnitTypes::Zerg_Drone);
					}
				}
			}
		}
	}

	UnitManager::get_instance()->handle_idle_workers();

	map_height_tile = Broodwar->mapWidth() * 32;
	map_width_tile = Broodwar->mapHeight() * 32;

	seed_interval = int(sqrt( double( ( map_height_tile * map_width_tile) / 32 ) ));

	map_height_count = map_height_tile / seed_interval;
	map_width_count = map_width_tile / seed_interval;

	construting_flag = false;

	if (analyzed == false)
	{
		Broodwar->printf("Analyzing map... this may take a minute");
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnalyzeThread, NULL, 0, NULL);
	}

}

void TerrainDetector::onEnd(bool isWinner)
{
	if (isWinner)
	{
		//log win to file
	}
}

void TerrainDetector::onFrame()
{

	if (show_visibility_data)
		drawVisibilityData();

	if (show_bullets)
		drawBullets();

	if (Broodwar->isReplay())
		return;

	drawStats();

#ifdef USED_BWTA
	if (analyzed && Broodwar->getFrameCount()%30==0)
	{
#if 0
		//order one of our workers to guard our chokepoint.
		for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
		{
			if ((*i)->getType().isWorker())
			{
				//get the chokepoints linked to our home region
				std::set<BWTA::Chokepoint*> chokepoints= home->getChokepoints();
				double min_length=10000;
				BWTA::Chokepoint* choke=NULL;

				//iterate through all chokepoints and look for the one with the smallest gap (least width)
				for(std::set<BWTA::Chokepoint*>::iterator c=chokepoints.begin();c!=chokepoints.end();c++)
				{
					double length=(*c)->getWidth();
					if (length<min_length || choke==NULL)
					{
						min_length=length;
						choke=*c;
					}
				}

				//order the worker to move to the center of the gap
				(*i)->rightClick(choke->getCenter());
				break;
			}
		}
#endif
	}
#endif // USED_BWTA


	if (analyzed)
	{
		drawTerrainData();
	    drawDebuggingData();
	}

	if (analysis_just_finished)
	{
		Broodwar->printf("Finished analyzing map.");
		analysis_just_finished=false;
	}

	/*
	*  Terrain Detection Process
	**/

    TerrainDetectionManager::get_instance()->on_time();

	/*----------------------------------------------------*/
}

void TerrainDetector::onSendText(std::string text)
{
	if (text=="/show bullets")
	{
		show_bullets = !show_bullets;
	} else if (text=="/show players")
	{
		showPlayers();
	} else if (text=="/show forces")
	{
		showForces();
	} else if (text=="/show visibility")
	{
		show_visibility_data=!show_visibility_data;
	} else if (text=="/analyze")
	{
		if (analyzed == false)
		{
			Broodwar->printf("Analyzing map... this may take a minute");
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnalyzeThread, NULL, 0, NULL);
		}
	} else
	{
		Broodwar->printf("You typed '%s'!",text.c_str());
		Broodwar->sendText("%s",text.c_str());
	}
}

void TerrainDetector::onReceiveText(BWAPI::Player* player, std::string text)
{
	Broodwar->printf("%s said '%s'", player->getName().c_str(), text.c_str());
}

void TerrainDetector::onPlayerLeft(BWAPI::Player* player)
{
	Broodwar->sendText("%s left the game.",player->getName().c_str());
}

void TerrainDetector::onNukeDetect(BWAPI::Position target)
{
	if (target!=Positions::Unknown)
		Broodwar->printf("Nuclear Launch Detected at (%d,%d)",target.x(),target.y());
	else
		Broodwar->printf("Nuclear Launch Detected");
}

void TerrainDetector::onUnitDiscover(BWAPI::Unit* unit)
{
	if (!Broodwar->isReplay() && Broodwar->getFrameCount()>1){

	}
	//Broodwar->sendText("A %s [%x] has been discovered at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
}

void TerrainDetector::onUnitEvade(BWAPI::Unit* unit)
{
	if (!Broodwar->isReplay() && Broodwar->getFrameCount()>1){

	}
	//Broodwar->sendText("A %s [%x] was last accessible at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
}

void TerrainDetector::onUnitShow(BWAPI::Unit* unit)
{
	if (!Broodwar->isReplay() && Broodwar->getFrameCount()>1){

	}
	//Broodwar->sendText("A %s [%x] has been spotted at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
}

void TerrainDetector::onUnitHide(BWAPI::Unit* unit)
{
	if (!Broodwar->isReplay() && Broodwar->getFrameCount()>1){

	}
	//Broodwar->sendText("A %s [%x] was last seen at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
}

void TerrainDetector::onUnitCreate(BWAPI::Unit* unit)
{
	if (Broodwar->getFrameCount()>1)
	{

		if (!Broodwar->isReplay()){	  
			//Broodwar->sendText("A %s [%x] has been created at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
			// handle worker creating stuff
			//on_creat_worker(unit);

			UnitManager::get_instance()->insert_worker( unit );

		}
		else
		{
			/*if we are in a replay, then we will print out the build order
			(just of the buildings, not the units).*/
			if (unit->getType().isBuilding() && unit->getPlayer()->isNeutral()==false)
			{
				int seconds=Broodwar->getFrameCount()/24;
				int minutes=seconds/60;
				seconds%=60;
				// Broodwar->sendText("%.2d:%.2d: %s creates a %s",minutes,seconds,unit->getPlayer()->getName().c_str(),unit->getType().getName().c_str());
			}
		}
	}
}

void TerrainDetector::onUnitDestroy(BWAPI::Unit* unit)
{
	if (!Broodwar->isReplay() && Broodwar->getFrameCount()>1){

	}
	//Broodwar->sendText("A %s [%x] has been destroyed at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
}

void TerrainDetector::onUnitMorph(BWAPI::Unit* unit)
{
	if (!Broodwar->isReplay()){

	}
	//Broodwar->sendText("A %s [%x] has been morphed at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
	else
	{
		/*if we are in a replay, then we will print out the build order
		(just of the buildings, not the units).*/
		if (unit->getType().isBuilding() && unit->getPlayer()->isNeutral()==false)
		{
			int seconds=Broodwar->getFrameCount()/24;
			int minutes=seconds/60;
			seconds%=60;
			//  Broodwar->sendText("%.2d:%.2d: %s morphs a %s",minutes,seconds,unit->getPlayer()->getName().c_str(),unit->getType().getName().c_str());
		}
	}
}

void TerrainDetector::onUnitRenegade(BWAPI::Unit* unit)
{
	if (!Broodwar->isReplay()){

	}
	//Broodwar->sendText("A %s [%x] is now owned by %s",unit->getType().getName().c_str(),unit,unit->getPlayer()->getName().c_str());
}

void TerrainDetector::onSaveGame(std::string gameName)
{
	//Broodwar->printf("The game was saved to \"%s\".", gameName.c_str());
}

DWORD WINAPI AnalyzeThread()
{

	TerrainRuler::get_instance()->initialization();
	//TerrainDetectionManager::get_instance()->destribute_terrain_info();
	analyzed   = true;
	analysis_just_finished = true;

	TerrainDetectionManager::get_instance()->initialization();
#if 0

	BWTA::analyze();

	//self start location only available if the map has base locations
	if (BWTA::getStartLocation(BWAPI::Broodwar->self())!=NULL)
	{
		//home       = BWTA::getStartLocation(BWAPI::Broodwar->self())->getRegion();
	}
	//enemy start location only available if Complete Map Information is enabled.
	if (BWTA::getStartLocation(BWAPI::Broodwar->enemy())!=NULL)
	{
		//enemy_base = BWTA::getStartLocation(BWAPI::Broodwar->enemy())->getRegion();
	}
	analyzed   = true;
	analysis_just_finished = true;
#endif

	return 0;
}

void TerrainDetector::drawStats()
{
	std::set<Unit*> enemy_units = Broodwar->enemy()->getUnits();

	Broodwar->drawTextScreen(5,0,"Enemy holds %d units:",enemy_units.size());

	std::map<BWAPI::UnitType, int> unit_type_counts;

	BOOST_FOREACH( BWAPI::Unit* unit, enemy_units){

		if ( unit_type_counts.find(unit->getType()) == unit_type_counts.end() )
		{
			unit_type_counts.insert( std::make_pair( unit->getType(), 0));
		}

		unit_type_counts.find( unit->getType())->second++;
	}

	int line=1;

	std::pair<BWAPI::UnitType, int> p_type;

	BOOST_FOREACH( p_type, unit_type_counts){

		Broodwar->drawTextScreen(5,16*line,"- %d %ss",p_type.second, p_type.first.getName().c_str());
		line++;
	}

}

void TerrainDetector::drawBullets()
{
	std::set<Bullet*> bullets = Broodwar->getBullets();
	for(std::set<Bullet*>::iterator i=bullets.begin();i!=bullets.end();i++)
	{
		Position p=(*i)->getPosition();
		double velocityX = (*i)->getVelocityX();
		double velocityY = (*i)->getVelocityY();
		if ((*i)->getPlayer()==Broodwar->self())
		{
			Broodwar->drawLineMap(p.x(),p.y(),p.x()+(int)velocityX,p.y()+(int)velocityY,Colors::Green);
			Broodwar->drawTextMap(p.x(),p.y(),"\x07%s",(*i)->getType().getName().c_str());
		}
		else
		{
			Broodwar->drawLineMap(p.x(),p.y(),p.x()+(int)velocityX,p.y()+(int)velocityY,Colors::Red);
			Broodwar->drawTextMap(p.x(),p.y(),"\x06%s",(*i)->getType().getName().c_str());
		}
	}
}

void TerrainDetector::drawVisibilityData()
{
	for(int x=0;x<Broodwar->mapWidth();x++)
	{
		for(int y=0;y<Broodwar->mapHeight();y++)
		{
			if (Broodwar->isExplored(x,y))
			{
				if (Broodwar->isVisible(x,y))
					Broodwar->drawDotMap(x*32+16,y*32+16,Colors::Green);
				else
					Broodwar->drawDotMap(x*32+16,y*32+16,Colors::Blue);
			}
			else
				Broodwar->drawDotMap(x*32+16,y*32+16,Colors::Red);
		}
	}
}

void TerrainDetector::drawTerrainData()
{
#if 0
	Patch detected_edge_point_buffer = TerrainDetectionManager::get_instance()->getStaticPatchBuffer();

    //Draw Patches to see how it organized
	int patch_size = detected_edge_point_buffer.size();
	for ( int i = 0; i < patch_size - 1; i++)
	{
		Position point1 = detected_edge_point_buffer[i];
		Position point2 = detected_edge_point_buffer[i+1];
		Broodwar->drawLine(CoordinateType::Map,point1.x(),point1.y(),point2.x(),point2.y(),Colors::Red);
	}
#endif

	//draw a circle at each mineral patch
	BOOST_FOREACH(BWAPI::Unit* mu, BWAPI::Broodwar->getMinerals()){
		Position q= mu->getInitialPosition();
		Broodwar->drawCircle(CoordinateType::Map,q.x(),q.y(),30,Colors::Cyan,false);
	}

	//draw the outlines of vespene geysers
	BOOST_FOREACH(BWAPI::Unit* gu, BWAPI::Broodwar->getGeysers()){
		TilePosition q=gu->getInitialTilePosition();
		Broodwar->drawBox(CoordinateType::Map,q.x()*32,q.y()*32,q.x()*32+4*32,q.y()*32+2*32,Colors::Orange,false);
	}

	//we will iterate through all the base locations, and draw their outlines.
	BOOST_FOREACH( BWTA::BaseLocation* b, TerrainDetectionManager::get_instance()->get_base_locations()){
		TilePosition p= b->getTilePosition();
		Position c= b->getPosition();

		//draw outline of center location
		Broodwar->drawBox(CoordinateType::Map,p.x()*32,p.y()*32,p.x()*32+4*32,p.y()*32+3*32,Colors::Blue,false);
#if 0
		//draw a circle at each mineral patch
		BOOST_FOREACH(BWAPI::Unit* mu, b->getStaticMinerals()){
			Position q= mu->getInitialPosition();
			Broodwar->drawCircle(CoordinateType::Map,q.x(),q.y(),30,Colors::Cyan,false);
		}

		//draw the outlines of vespene geysers
		BOOST_FOREACH(BWAPI::Unit* gu, b->getGeysers()){
			TilePosition q=gu->getInitialTilePosition();
			Broodwar->drawBox(CoordinateType::Map,q.x()*32,q.y()*32,q.x()*32+4*32,q.y()*32+2*32,Colors::Orange,false);
		}
#endif
		//if this is an island expansion, draw a yellow circle around the base location
		if (b->isIsland())
			Broodwar->drawCircle(CoordinateType::Map,c.x(),c.y(),80,Colors::Yellow,false);
	}

	//we will iterate through all the regions and draw the polygon outline of it in green.
	BOOST_FOREACH( BWTA::Region* r, TerrainDetectionManager::get_instance()->get_regions()){
		BWTA::Polygon p=r->getPolygon();
		for(int j=0;j<(int)p.size();j++)
		{
			Position point1=p[j];
			Position point2=p[(j+1) % p.size()];
			Broodwar->drawLine(CoordinateType::Map,point1.x(),point1.y(),point2.x(),point2.y(),Colors::Green);
		}
	}

	//we will visualize the chokepoints with red lines
	BOOST_FOREACH( BWTA::Chokepoint* cp, TerrainDetectionManager::get_instance()->get_choke_points()){
		Position point1=cp->getSides().first;
		Position point2=cp->getSides().second;
		Broodwar->drawLine(CoordinateType::Map,point1.x(),point1.y(),point2.x(),point2.y(),Colors::Red);
	}


#ifdef USE_BWTA
	//we will iterate through all the base locations, and draw their outlines.
	for(std::set<BWTA::BaseLocation*>::const_iterator i=BWTA::getBaseLocations().begin();i!=BWTA::getBaseLocations().end();i++)
	{
		TilePosition p=(*i)->getTilePosition();
		Position c=(*i)->getPosition();

		//draw outline of center location
		Broodwar->drawBox(CoordinateType::Map,p.x()*32,p.y()*32,p.x()*32+4*32,p.y()*32+3*32,Colors::Blue,false);

		//draw a circle at each mineral patch
		for(std::set<BWAPI::Unit*>::const_iterator j=(*i)->getStaticMinerals().begin();j!=(*i)->getStaticMinerals().end();j++)
		{
			Position q=(*j)->getInitialPosition();
			Broodwar->drawCircle(CoordinateType::Map,q.x(),q.y(),30,Colors::Cyan,false);
		}

		//draw the outlines of vespene geysers
		for(std::set<BWAPI::Unit*>::const_iterator j=(*i)->getGeysers().begin();j!=(*i)->getGeysers().end();j++)
		{
			TilePosition q=(*j)->getInitialTilePosition();
			Broodwar->drawBox(CoordinateType::Map,q.x()*32,q.y()*32,q.x()*32+4*32,q.y()*32+2*32,Colors::Orange,false);
		}

		//if this is an island expansion, draw a yellow circle around the base location
		if ((*i)->isIsland())
			Broodwar->drawCircle(CoordinateType::Map,c.x(),c.y(),80,Colors::Yellow,false);
	}

	//we will iterate through all the regions and draw the polygon outline of it in green.
	for(std::set<BWTA::Region*>::const_iterator r=BWTA::getRegions().begin();r!=BWTA::getRegions().end();r++)
	{
		BWTA::Polygon p=(*r)->getPolygon();
		for(int j=0;j<(int)p.size();j++)
		{
			Position point1=p[j];
			Position point2=p[(j+1) % p.size()];
			Broodwar->drawLine(CoordinateType::Map,point1.x(),point1.y(),point2.x(),point2.y(),Colors::Green);
		}
	}

	//we will visualize the chokepoints with red lines
	for(std::set<BWTA::Region*>::const_iterator r=BWTA::getRegions().begin();r!=BWTA::getRegions().end();r++)
	{
		for(std::set<BWTA::Chokepoint*>::const_iterator c=(*r)->getChokepoints().begin();c!=(*r)->getChokepoints().end();c++)
		{
			Position point1=(*c)->getSides().first;
			Position point2=(*c)->getSides().second;
			Broodwar->drawLine(CoordinateType::Map,point1.x(),point1.y(),point2.x(),point2.y(),Colors::Red);
		}
	}
#endif // USE_BWTA

}

void TerrainDetector::showPlayers()
{
	std::set<Player*> players=Broodwar->getPlayers();
	for(std::set<Player*>::iterator i=players.begin();i!=players.end();i++)
	{
		// Broodwar->printf("Player [%d]: %s is in force: %s",(*i)->getID(),(*i)->getName().c_str(), (*i)->getForce()->getName().c_str());
	}
}

void TerrainDetector::showForces()
{
	std::set<Force*> forces=Broodwar->getForces();
	for(std::set<Force*>::iterator i=forces.begin();i!=forces.end();i++)
	{
		std::set<Player*> players=(*i)->getPlayers();
		//Broodwar->printf("Force %s has the following players:",(*i)->getName().c_str());
		for(std::set<Player*>::iterator j=players.begin();j!=players.end();j++)
		{
			// Broodwar->printf("  - Player [%d]: %s",(*j)->getID(),(*j)->getName().c_str());
		}
	}
}

void TerrainDetector::onUnitComplete(BWAPI::Unit *unit)
{
	if (!Broodwar->isReplay() && Broodwar->getFrameCount()>1){

	}
#ifdef DEBUGGING_OUTPUT
	//Broodwar->sendText("A %s [%x] has been completed at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
#endif // DEBUGGING_OUTPUT


}

void TerrainDetector::worker_production()
{
	if ( Broodwar->getFrameCount()%5 == 0 )
	{
		//send each worker to the mineral field that is closest to it
		for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
		{
			if ((*i)->getType().isResourceDepot())
			{
				//if this is a center, tell it to build the appropiate type of worker
				if ((*i)->getType().getRace()!=Races::Zerg)
				{
					(*i)->train(Broodwar->self()->getRace().getWorker());
				}
				else //if we are Zerg, we need to select a larva and morph it into a drone
				{
					std::set<Unit*> myLarva=(*i)->getLarva();
					if (myLarva.size()>0)
					{
						Unit* larva=*myLarva.begin();
						larva->morph(UnitTypes::Zerg_Drone);
					}
				}
			}
		}
	}	

}

void TerrainDetector::probe_seeding()
{
	int i = 1;
	int x_tile_pos;
	int y_tile_pos;

#ifdef DEBUGGING_OUTPUT
	if ( Broodwar->getFrameCount() % 24 == 0)
	{
		//Broodwar->printf("SeedInterval %d, Map_weith %d, Map_Height %d, Map_weith_account %d, Map_height_account %d"
		//	, seed_interval, map_width_tile, map_height_tile, map_width_count, map_height_count);
	}	

#endif	
	BOOST_FOREACH( BWAPI::Unit* unit, terrain_detecting_workers){
		x_tile_pos = seed_interval / 2 +  ( i % map_width_count - 1) * seed_interval;
		y_tile_pos = seed_interval / 2 +  ( i / map_width_count ) * seed_interval;
		BWAPI::Position p(x_tile_pos,y_tile_pos);
#ifdef DEBUGGING_OUTPUT
		if ( Broodwar->getFrameCount() % 24 == 0 && i % 5 == 0)
		{
			Broodwar->printf("Unit %d was assigned to move to ( %d, %d ).", unit->getID(), x_tile_pos, y_tile_pos);
		}	

#endif		
		unit->move(p);
		i++;
	}
}

void TerrainDetector::supply_production()
{
	// find a worker to produce pylon

#ifdef DEBUGGING_OUTPUT
	//Broodwar->sendText("Supply_production called");
	if ( Broodwar->getFrameCount() % 10 == 0)
	{
		Broodwar->printf(" Current SupplyTotal %d, SupplyUsed %d.", Broodwar->self()->supplyTotal(), Broodwar->self()->supplyUsed());
	}	

#endif // DEBUGGING_OUTPUT

	if ( ( Broodwar->self()->supplyTotal() <  ( Broodwar->self()->supplyUsed() + 6)) 
		&& ( Broodwar->self()->supplyTotal() < 390 ) 
		&& !construting_flag)
	{

		UnitType supplyProvider = Broodwar->self()->getRace().getSupplyProvider();

#ifdef DEBUGGING_OUTPUT
		if ( Broodwar->getFrameCount() % 30 == 0)
		{
			Broodwar->printf(" Preparing constructing a pylon.");
			Broodwar->printf(" Minerals %d, gas %d", Broodwar->self()->minerals(), Broodwar->self()->gas());
			Broodwar->printf(" MineralPrice %d, gasPrice %d", supplyProvider.mineralPrice(), supplyProvider.gasPrice());

			if ( supplyProvider != UnitTypes::Protoss_Pylon)
			{
				Broodwar->printf( "Building Type is not correct!");
			}
		}

#endif // DEBUGGING_OUTPUT

		if ( Broodwar->self()->minerals() >= supplyProvider.mineralPrice()
			&& Broodwar->self()->gas() >= supplyProvider.gasPrice())
		{
			// find a place to build
			if ( mineral_gathering_workers.size() > 3)
			{
				std::vector<BWAPI::Unit*>::iterator first_unit = mineral_gathering_workers.begin();

				if (building_placer( *first_unit))
				{
					mineral_gathering_workers.erase(first_unit);
					building_construction_workers.push_back(*first_unit);
					construting_flag = true;
				}
			}else{

#ifdef DEBUGGING_OUTPUT
				if ( Broodwar->getFrameCount() % 30 == 0)
				{
					Broodwar->printf("Mineral gathering workers account %d", mineral_gathering_workers.size());
				}

#endif // DEBUGGING_OUTPUT

			}
		}
	}

}

void TerrainDetector::on_creat_worker( BWAPI::Unit* unit)
{
	if ( unit == NULL)
	{
		assert(unit);
	}

	// if a worker has been created, arrange it to detect terrain
	if ( unit->getType().isWorker() && unit->getPlayer() == BWAPI::Broodwar->self() && unit->getHitPoints() >= 0){
		//probe_seeding(unit);
		terrain_detecting_workers.push_back(unit);
	}

}

bool TerrainDetector::building_placer(BWAPI::Unit* unit)
{
	if ( !unit->getType().isWorker())
	{
		return false;
	}

	BWAPI::TilePosition center = Broodwar->self()->getStartLocation();

	int random_x = rand() % 100 - 50;
	int random_y = rand() % 100 - 50;

	int try_count = 0;

	BWAPI::TilePosition pylon_placer( center.x() + random_x, center.y() + random_y );



	while ( !Broodwar->canBuildHere(unit, pylon_placer, BWAPI::UnitTypes::Protoss_Pylon))
	{
		random_x = rand() % 100 - 50;
		random_y = rand() % 100 - 50;
		BWAPI::TilePosition temp( center.x() + random_x, center.y() + random_y );
		pylon_placer = temp;

		try_count ++;
		if ( try_count > 100)
		{
			return false;
		}
	}
#ifdef DEBUGGING_OUTPUT
	Broodwar->sendText(" Worker %d was assigned to build a Pylon in (%d, %d).", unit->getID(), pylon_placer.x(), pylon_placer.y());
#endif // DEBUGGING_OUTPUT

	return unit->build(pylon_placer, UnitTypes::Protoss_Pylon);

}

void TerrainDetector::handle_builders()
{

	BWAPI::Unit* unit = NULL;
	size_t i = 0; 

	for (; i < building_construction_workers.size() ; i++)
	{
		unit = building_construction_workers.at(i);

		if (!unit->isConstructing() && ! unit->isMoving())
		{
#ifdef DEBUGGING_OUTPUT
			Broodwar->printf( "Worker went back!");
#endif // DEBUGGING_OUTPUT

			Unit* closestMineral=NULL;
			for(std::set<Unit*>::iterator m=Broodwar->getMinerals().begin();m!=Broodwar->getMinerals().end();m++)
			{
				if (closestMineral==NULL || unit->getDistance(*m)< unit->getDistance(closestMineral))
					closestMineral=*m;
			}
			if (closestMineral!=NULL){
				unit->rightClick(closestMineral);
				mineral_gathering_workers.push_back(unit);
			}
			//building_construction_workers.erase(unit);
			construting_flag = false;
		}
		break;
	}

	if ( !construting_flag && i < building_construction_workers.size())
	{
		building_construction_workers.erase( building_construction_workers.begin() + i);
	}


}

void TerrainDetector::search_worker_scout()
{
    //find a worker which is near to depot for scouting
	BWAPI::Unit * closestUnit = NULL;
	double closestDist = 100000;
	BWAPI::Position target = TerrainDetectionManager::get_instance()->get_start_location()->getPosition();

	BOOST_FOREACH (BWAPI::Unit * unit, Broodwar->self()->getUnits())
	{
		if (unit->getType().isWorker() && UnitManager::get_instance()->is_free(unit))
		{
			double dist = unit->getDistance( target );
			if (!closestUnit || dist < closestDist)
			{
				closestUnit = unit;
				closestDist = dist;
			}
		}
	}
}

void TerrainDetector::drawDebuggingData()
{
	TerrainTile*** tile_map = TerrainRuler::get_instance()->get_tiles_map();
	int height = TerrainRuler::get_instance()->get_map_height();
	int width = TerrainRuler::get_instance()->get_map_width();
	std::pair<BWAPI::Unit*, ScoutProperty*> scout_pair;
	

	for ( int i = 0; i < height; ++i)
	{
		for ( int j = 0; j < width; ++j)
		{

			if ( tile_map[i][j]->pass_flag == PASS_UNWALKABLE )
			{
				BWAPI::Broodwar->drawBoxMap( tile_map[i][j]->pos.x() + RATIO / 4, tile_map[i][j]->pos.y() + RATIO / 4, tile_map[i][j]->pos.x() - RATIO / 4, tile_map[i][j]->pos.y() - RATIO / 4, 
					BWAPI::Color(0, 0, 255), true);
			}

#if 0
			if ( tile_map[i][j]->pass_flag == PASS_UNKONW )
			{

			}else if ( tile_map[i][j]->pass_flag == PASS_UNWALKABLE)
			{
				BWAPI::Broodwar->drawBoxMap( tile_map[i][j]->pos.x() + RATIO / 4, tile_map[i][j]->pos.y() + RATIO / 4, tile_map[i][j]->pos.x() - RATIO / 4, tile_map[i][j]->pos.y() - RATIO / 4, 
					BWAPI::Color(0, 0, 255), true);
			}else{
				BWAPI::Broodwar->drawBoxMap( tile_map[i][j]->pos.x() + RATIO / 4, tile_map[i][j]->pos.y() + RATIO / 4, tile_map[i][j]->pos.x() - RATIO / 4, tile_map[i][j]->pos.y() - RATIO / 4, 
					BWAPI::Color(0, tile_map[i][j]->a_potential_value * 50 * 23, 0), true);
				//BWAPI::Broodwar->drawBoxMap( tile_map[i][j]->pos.x() + RATIO / 4, tile_map[i][j]->pos.y() + RATIO / 4, tile_map[i][j]->pos.x() - RATIO / 4, tile_map[i][j]->pos.y() - RATIO / 4, 
				//	BWAPI::Color(0, 255, 0), true);
			}
#endif

		}
	}

	//Draw the track of scouts
	BOOST_FOREACH( scout_pair, TerrainDetectionManager::get_instance()->get_scouts_map()){
		
		tile_coordiante previous_point;
		tile_coordiante current_point;
		int visit_iterator = 0; 
		int list_size = scout_pair.second->pre_visited_tiles.size();

		if ( list_size > 1)
		{
			previous_point = scout_pair.second->pre_visited_tiles[visit_iterator];
			visit_iterator++;
			for ( ; visit_iterator < list_size; visit_iterator++)
			{
				current_point = scout_pair.second->pre_visited_tiles[visit_iterator];

				Broodwar->drawLine(CoordinateType::Map, tile_map[previous_point.x][previous_point.y]->pos.x(), tile_map[previous_point.x][previous_point.y]->pos.y(),
					tile_map[current_point.x][current_point.y]->pos.x(), tile_map[current_point.x][current_point.y]->pos.y(), Colors::Green);

				previous_point = current_point;
			}

		}

	}

}


