#pragma once
#include <BWAPI.h>
#include <windows.h>



extern bool analyzed;
extern bool analysis_just_finished;

DWORD WINAPI AnalyzeThread();



class TerrainDetector : public BWAPI::AIModule
{
public:
  virtual void onStart();
  virtual void onEnd(bool isWinner);
  virtual void onFrame();
  virtual void onSendText(std::string text);
  virtual void onReceiveText(BWAPI::Player* player, std::string text);
  virtual void onPlayerLeft(BWAPI::Player* player);
  virtual void onNukeDetect(BWAPI::Position target);
  virtual void onUnitDiscover(BWAPI::Unit* unit);
  virtual void onUnitEvade(BWAPI::Unit* unit);
  virtual void onUnitShow(BWAPI::Unit* unit);
  virtual void onUnitHide(BWAPI::Unit* unit);
  virtual void onUnitCreate(BWAPI::Unit* unit);
  virtual void onUnitDestroy(BWAPI::Unit* unit);
  virtual void onUnitMorph(BWAPI::Unit* unit);
  virtual void onUnitRenegade(BWAPI::Unit* unit);
  virtual void onSaveGame(std::string gameName);
  virtual void onUnitComplete(BWAPI::Unit *unit);
/*
 * Terrain Detection Methods 
**/  

  void worker_production();
  void probe_seeding();
  void supply_production();
  void on_creat_worker(BWAPI::Unit*);
  bool building_placer(BWAPI::Unit*);
  void handle_builders();



/*---------------------------------------------------------*/
  void search_worker_scout();
  void execution_scout();
  

/*---------------------------------------------------------*/

  void drawStats(); //not part of BWAPI::AIModule
  void drawBullets();
  void drawVisibilityData();
  void drawTerrainData();
  void drawDebuggingData();
  void showPlayers();
  void showForces();
  bool show_bullets;
  bool show_visibility_data;

private:

  std::vector<BWAPI::Unit*> mineral_gathering_workers;
  std::vector<BWAPI::Unit*> terrain_detecting_workers;
  std::vector<BWAPI::Unit*> building_construction_workers;  
  std::vector<BWAPI::Unit*> building_pylon;
  
  int map_width_tile;
  int map_height_tile;
  int map_width_count;
  int map_height_count;
  int seed_interval;
  bool construting_flag;



  
};
