#include "Common.h"
#include "BuildOrderQueue.h"

BuildOrderQueue::BuildOrderQueue() 
	: highest_priority(0), 
	  lowest_priority(0),
	  default_priority_spacing(10),
	  num_skipped_items(0)
{
	
}

void BuildOrderQueue::clear_all() 
{
	// clear the queue
	queue.clear();

	// reset the priorities
	highest_priority= 0;
	lowest_priority = 0;
}

BuildOrderItem<PRIORITY_TYPE> & BuildOrderQueue::get_highest_priority_item() 
{
	// reset the number of skipped items to zero
	num_skipped_items = 0;

	// the queue will be sorted with the highest priority at the back
	return queue.back();
}

BuildOrderItem<PRIORITY_TYPE> & BuildOrderQueue::get_next_highest_priority_item() 
{
	bool size_eval = ( queue.size() - 1 - num_skipped_items >= 0);
	assert(size_eval);
	// the queue will be sorted with the highest priority at the back
	return queue[queue.size() - 1 - num_skipped_items];
}

void BuildOrderQueue::skip_item()
{
	// make sure we can skip
	assert(can_skip_item());

	// skip it
	num_skipped_items++;
}

bool BuildOrderQueue::can_skip_item() {

	// does the queue have more elements
	bool bigEnough = queue.size() > (size_t)(1 + num_skipped_items);

	if (!bigEnough) 
	{
		return false;
	}

	// is the current highest priority item not blocking a skip
	bool highestNotBlocking = !queue[queue.size() - 1 - num_skipped_items].blocking;

	// this tells us if we can skip
	return highestNotBlocking;
}

void BuildOrderQueue::queue_item(BuildOrderItem<PRIORITY_TYPE> b) 
{
	// if the queue is empty, set the highest and lowest priorities
	if (queue.empty()) 
	{
		highest_priority = b.priority;
		lowest_priority = b.priority;
	}

	// push the item into the queue
	if (b.priority <= lowest_priority) 
	{
		queue.push_front(b);
	}
	else
	{
		queue.push_back(b);
	}

	// if the item is somewhere in the middle, we have to sort again
	if ((queue.size() > 1) && (b.priority < highest_priority) && (b.priority > lowest_priority)) 
	{
		// sort the list in ascending order, putting highest priority at the top
		std::sort(queue.begin(), queue.end());
	}

	// update the highest or lowest if it is beaten
	highest_priority = (b.priority > highest_priority) ? b.priority : highest_priority;
	lowest_priority  = (b.priority < lowest_priority)  ? b.priority : lowest_priority;
}

void BuildOrderQueue::queue_as_highest_priority(MetaType m, bool blocking) 
{
	// the new priority will be higher
	PRIORITY_TYPE newPriority = highest_priority + default_priority_spacing;

	// queue the item
	queue_item(BuildOrderItem<PRIORITY_TYPE>(m, newPriority, blocking));
}

void BuildOrderQueue::queue_as_lowest_priority(MetaType m, bool blocking) 
{
	// the new priority will be higher
	int newPriority = lowest_priority - default_priority_spacing;

	// queue the item
	queue_item(BuildOrderItem<PRIORITY_TYPE>(m, newPriority, blocking));
}

void BuildOrderQueue::remove_highest_priority_item() 
{
	// remove the back element of the vector
	queue.pop_back();

	// if the list is not empty, set the highest accordingly
	highest_priority = queue.empty() ? 0 : queue.back().priority;
	lowest_priority  = queue.empty() ? 0 : lowest_priority;
}

void BuildOrderQueue::remove_current_highest_priority_item() 
{
	// remove the back element of the vector
	queue.erase(queue.begin() + queue.size() - 1 - num_skipped_items);
    
	//assert((int)(queue.size()) < size);

	// if the list is not empty, set the highest accordingly
	highest_priority = queue.empty() ? 0 : queue.back().priority;
	lowest_priority  = queue.empty() ? 0 : lowest_priority;
}

size_t BuildOrderQueue::size() 
{
	return queue.size();
}

bool BuildOrderQueue::is_empty()
{
	return (queue.size() == 0);
}

BuildOrderItem<PRIORITY_TYPE> BuildOrderQueue::operator [] (int i)
{
	return queue[i];
}

#if 0
void BuildOrderQueue::drawQueueInformation(int x, int y) {

	//x = x + 25;
	
	std::string prefix = "\x04";

	//if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y, "\x04Priority Queue Information:");
	//if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y+20, "\x04UNIT NAME");

	size_t reps = queue.size() < 10 ? queue.size() : 10;

	// for each unit in the queue
	for (size_t i(0); i<reps; i++) {

		prefix = "\x04";

		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y+(i*10), "%s%s", prefix.c_str(), queue[queue.size() - 1 - i].metaType.getName().c_str());
	}
}
#endif
