#pragma once

#include "Common.h"

struct MetaType {

	enum type_enum {Unit, Tech, Upgrade, Command, Default};
	type_enum type;

	BWAPI::UnitCommandType command_type;
	BWAPI::UnitType unit_type;
	BWAPI::TechType tech_type;
	BWAPI::UpgradeType upgrade_type;

	MetaType () : type(MetaType::Default) {}
	MetaType (BWAPI::UnitType t) :        unit_type(t),    type(MetaType::Unit) {}
	MetaType (BWAPI::TechType t) :        tech_type(t),    type(MetaType::Tech) {}
	MetaType (BWAPI::UpgradeType t) :     upgrade_type(t), type(MetaType::Upgrade) {}
	MetaType (BWAPI::UnitCommandType t) : command_type(t), type(MetaType::Command) {}

	bool is_unit()		{ return type == Unit; }
	bool is_tech()		{ return type == Tech; }
	bool is_upgrade()	{ return type == Upgrade; }
	bool is_command()	{ return type == Command; }
	bool is_building()	{ return type == Unit && unit_type.isBuilding(); }
	bool is_refinery()	{ return is_building() && unit_type.isRefinery(); }

	int supply_required()
	{
		if (is_unit())
		{
			return unit_type.supplyRequired();
		}
		else
		{
			return 0;
		}
	}

	int mineral_price()
	{
		return is_unit() ? unit_type.mineralPrice() : (is_tech() ? tech_type.mineralPrice() : upgrade_type.mineralPrice());
	}

	int gas_price()
	{
		return is_unit() ? unit_type.gasPrice() : (is_tech() ? tech_type.gasPrice() : upgrade_type.gasPrice());
	}

	BWAPI::UnitType what_builds()
	{
		return is_unit() ? unit_type.whatBuilds().first : (is_tech() ? tech_type.whatResearches() : upgrade_type.whatUpgrades());
	}

	std::string get_name()
	{
		if (is_unit())
		{
			return unit_type.getName();
		}
		else if (is_tech())
		{
			return tech_type.getName();
		}
		else if (is_upgrade())
		{
			return upgrade_type.getName();
		}
		else if (is_command())
		{
			return command_type.getName();
		}
		else
		{
			assert(false);
			return "LOL";	
		}
	}
};