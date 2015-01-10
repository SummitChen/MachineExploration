#pragma once

#include "Common.h"
#include <BWTA.h>
#include "MetaType.h"

#define PRIORITY_TYPE int

template <class T>
class BuildOrderItem {

public:

	MetaType			meta_type;		// the thing we want to 'build'
	T					priority;	// the priority at which to place it in the queue
	bool				blocking;	// whether or not we block further items

	BuildOrderItem(MetaType m, T p, bool b) : meta_type(m), priority(p), blocking(b) {}

	bool operator<(const BuildOrderItem<T> &x) const
	{
		return priority < x.priority;
	}
};

class BuildOrderQueue {

	std::deque< BuildOrderItem<PRIORITY_TYPE> >			queue;

	PRIORITY_TYPE lowest_priority;		
	PRIORITY_TYPE highest_priority;
	PRIORITY_TYPE default_priority_spacing;

	int num_skipped_items;

public:

	BuildOrderQueue();

	void clear_all();											// clears the entire build order queue
	void skip_item();											// increments skippedItems
	void queue_as_highest_priority(MetaType m, bool blocking);		// queues something at the highest priority
	void queue_as_lowest_priority(MetaType m, bool blocking);		// queues something at the lowest priority
	void queue_item(BuildOrderItem<PRIORITY_TYPE> b);			// queues something with a given priority
	void remove_highest_priority_item();								// removes the highest priority item
	void remove_current_highest_priority_item();

	int get_highest_priority_value();								// returns the highest priority value
	int	get_lowest_priority_value();								// returns the lowest priority value
	size_t size();													// returns the size of the queue

	bool is_empty();

	void remove_all(MetaType m);									// removes all matching meta types from queue

	BuildOrderItem<PRIORITY_TYPE> & get_highest_priority_item();	// returns the highest priority item
	BuildOrderItem<PRIORITY_TYPE> & get_next_highest_priority_item();	// returns the highest priority item

	bool can_skip_item();
	bool has_next_highest_priority_item();								// returns the highest priority item

	void draw_queue_information(int x, int y);

	// overload the bracket operator for ease of use
	BuildOrderItem<PRIORITY_TYPE> operator [] (int i); 
};