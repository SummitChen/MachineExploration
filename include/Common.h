#pragma once

#define _USE_MATH_DEFINES

#include "BWAPI.h"
#include <BWTA.h>
#include <assert.h>
#include "boost/foreach.hpp"

#include <cmath>
#include <math.h>
#include <cstdlib>
#include <cassert>

#include <stddef.h>

#include <stdexcept>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <deque>
#include <list>
#include <set>
#include <map>

typedef std::vector<BWAPI::Unit*> UnitVector;
typedef std::set<BWAPI::Position> PosSet;

typedef std::set<BWTA::Chokepoint*> ChokeSet;
typedef std::set<BWTA::Region*> RegionSet;


#define USED_BWTA
#define DEBUGGING_OUTPUT
#define  WORKER_PER_REFINERY  3
#define  PI 3.1415926
#define DEBUGGING


struct double2
{
	double x,y;

	double2() {}
	double2(double x, double y) : x(x), y(y) {}
	double2(const BWAPI::Position & p) : x(p.x()), y(p.y()) {}

	operator BWAPI::Position()				const { return BWAPI::Position(static_cast<int>(x),static_cast<int>(y)); }

	double2 operator + (const double2 & v)	const { return double2(x+v.x,y+v.y); }
	double2 operator - (const double2 & v)	const { return double2(x-v.x,y-v.y); }
	double2 operator * (double s)			const { return double2(x*s,y*s); }
	double2 operator / (double s)			const { return double2(x/s,y/s); }

	double dot(const double2 & v)			const { return x*v.x + y*v.y; }
	double lenSq()							const { return x*x + y*y; }
	double len()							const { return sqrt(lenSq()); }
	double2 normal()						const { return *this / len(); }

	void normalise() { double s(len()); x/=s; y/=s; } 
	void rotate(double angle) 
	{ 	
		angle = angle*M_PI/180.0;
		*this = double2(x * cos(angle) - y * sin(angle), y * cos(angle) + x * sin(angle));
	}
};



