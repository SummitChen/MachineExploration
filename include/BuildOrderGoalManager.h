#pragma once

#include "Common.h"
#include "MetaType.h"

class BuildOrderGoalItem
{
	// the MetaType to be constructed
	MetaType	_meta_type;

	// the number we wish to construct
	int			_num;

public:

	// constructor
	BuildOrderGoalItem(const MetaType & t, const int n) : _meta_type(t), _num(n) {}

	// getters
	const MetaType & metaType()	const	{ return _meta_type; }
	const int num() const				{ return _num; }


};

class BuildOrderGoal
{
	// the goals
	std::vector<BuildOrderGoalItem>		goal;

	// priority
	int priority;

public:
	
	BuildOrderGoal() : priority(0) {}
	BuildOrderGoal(const int p) : priority(p) {}
	~BuildOrderGoal() {}

	void add_item(const BuildOrderGoalItem & bogi)
	{
		goal.push_back(bogi);
	}

	void set_priority(const int p)
	{
		priority = p;
	}

	const int get_priority() const
	{
		return priority;
	}

	const std::vector<BuildOrderGoalItem> & get_goal() const
	{
		return goal;
	}

	bool operator < (const BuildOrderGoal & bog)
	{
		return get_priority() < bog.get_priority();
	}
};

class BuildOrderGoalManager {

	std::vector<BuildOrderGoal>		goals;

	// add a build order goal item with a priority
	void				add_goal(const MetaType t, int num, int p);

	// checks to see if a goal is completed by using BWAPI data
	bool				is_completed(const BuildOrderGoal & bog) const;
	
	// set the build order goals based on expert knowledge
	void				set_build_order_goals();

public:

	BuildOrderGoalManager();
	~BuildOrderGoalManager();

	// gets the highest priority goal which isn't completed
	BuildOrderGoal &	get_next_build_order_goal() const;
};
