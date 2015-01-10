#include "Common.h"
#include "BuildingData.h"

ConstructionData::ConstructionData()
{
	// set up the buildings vector
	buildings = std::vector< std::vector<Building> > (NumBuildingStates, std::vector<Building>());

	// set up the index vector
	building_index = std::vector< size_t >(NumBuildingStates, 0);
}

bool ConstructionData::has_next_building(BuildingState bs)
{
	// is the current index valid
	return building_index[bs] < buildings[bs].size();
}

int ConstructionData::get_num_buildings(BuildingState bs)
{
	return buildings[bs].size();
}

Building & ConstructionData::get_next_building(BuildingState bs)
{
	// assert this index is valid
	assert(has_next_building(bs));

	// get the building
	Building & returnBuilding = buildings[bs][building_index[bs]];

	// increment the index
	building_index[bs]++;

	// return the building
	return returnBuilding;
}

void ConstructionData::begin(BuildingState bs)
{
	// set the index to 0
	building_index[bs] = 0;
}

void ConstructionData::remove_current_building(BuildingState bs)
{
	// erase the element of the vector 1 before where we are now
	buildings[bs].erase(buildings[bs].begin() + building_index[bs] - 1);

	// set the index back one
	building_index[bs]--;
}

void ConstructionData::add_building(BuildingState bs, const Building & b)
{
	// add the building to the vector
	buildings[bs].push_back(b);
}

bool ConstructionData::is_being_built(BWAPI::UnitType type)
{
	// for each building vector
	BOOST_FOREACH (std::vector<Building> & buildingVector, buildings)
	{
		BOOST_FOREACH (Building & b, buildingVector)
		{
			if (b.type == type)
			{
				return true;
			}
		}
	}

	return false;
}