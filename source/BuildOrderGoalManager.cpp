#include "Common.h"
#include "BuildOrderGoalManager.h"

BuildOrderGoalManager::BuildOrderGoalManager()
{
	
}

bool BuildOrderGoalManager::is_completed(const BuildOrderGoal & bog) const
{
	// for each item in the goal
	BOOST_FOREACH (const BuildOrderGoalItem & bogi, bog.get_goal())
	{
		if (bogi.metaType().type == MetaType::Unit)
		{
			// if we do not have that many of the unit type, return false
			if (BWAPI::Broodwar->self()->allUnitCount(bogi.metaType().unit_type) < bogi.num())
			{
				return false;
			}
		}
		// if we have not researched that tech, return false
		else if (bogi.metaType().type == MetaType::Tech)
		{
			if (!BWAPI::Broodwar->self()->hasResearched(bogi.metaType().tech_type))
			{
				return false;
			}
		}
		// if we have not upgraded to that level, return false
		else if (bogi.metaType().type == MetaType::Upgrade)
		{
			if (BWAPI::Broodwar->self()->getUpgradeLevel(bogi.metaType().upgrade_type) < bogi.num())
			{
				return false;
			}
		}
	}

	// return true only if everything meets the criteria
	return true;
}

void BuildOrderGoalManager::add_goal(const MetaType t, int num, int p)
{
	// create the build order item
	BuildOrderGoalItem bogi(t, num);

	// see if an item with this priority already exists
	int existingIndex = -1;
	for (int i(0); i<(int)goals.size(); ++i)
	{
		if (goals[i].get_priority() == p)
		{
			existingIndex = i;
			break;
		}
	}

	// if it already exists, add it to that goal
	if (existingIndex != -1)
	{
		goals[existingIndex].add_item(bogi);
	}
	// otherwise create a new goal
	else
	{
		BuildOrderGoal temp(p);
		temp.add_item(bogi);
		goals.push_back(temp);
	}

}

void BuildOrderGoalManager::set_build_order_goals()
{
	add_goal(MetaType(BWAPI::UnitTypes::Protoss_Nexus), 1, 1000);	// 1	Nexus
	add_goal(MetaType(BWAPI::UnitTypes::Protoss_Probe), 1, 1000);	// 1	Probe

	add_goal(MetaType(BWAPI::UnitTypes::Protoss_Zealot),		12,		98);	// 12	Zealot
	add_goal(MetaType(BWAPI::UnitTypes::Protoss_Nexus),		2,		96);	// 2	Nexus
	add_goal(MetaType(BWAPI::UnitTypes::Protoss_Gateway),	3,		94);	// 3	Gateway
	add_goal(MetaType(BWAPI::UnitTypes::Protoss_Zealot),		18,		92);	// 18	Zealot
	add_goal(MetaType(BWAPI::UnitTypes::Protoss_Gateway),	5,		90);

	// sort the goals to make things quicker for us
	std::sort(goals.begin(), goals.end());
}
/*
BuildOrderGoal & BuildOrderGoalManager::getNextBuildOrderGoal() const
{
	
}*/