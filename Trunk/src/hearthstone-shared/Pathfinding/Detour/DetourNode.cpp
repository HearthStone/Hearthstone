/***
 * Demonstrike Core
 */

#include "DetourNode.h"
#include "../../Common.h"
#include "DetourCommon.h"
#include <string.h>

inline unsigned int dtHashRef(dtPolyRef a)
{
    a = (~a) + (a << 18);
    a = a ^ (a >> 31);
    a = a * 21;
    a = a ^ (a >> 11);
    a = a + (a << 6);
    a = a ^ (a >> 22);
    return (unsigned int)a;
}

//////////////////////////////////////////////////////////////////////////////////////////
dtNodePool::dtNodePool(int maxNodes, int hashSize) :
	m_nodes(0),
	m_first(0),
	m_next(0),
	m_maxNodes(maxNodes),
	m_hashSize(hashSize),
	m_nodeCount(0)
{
	ASSERT(dtNextPow2(m_hashSize) == (unsigned int)m_hashSize);
	ASSERT(m_maxNodes > 0);

	m_nodes = (dtNode*)malloc(sizeof(dtNode)*m_maxNodes);
	m_next = (unsigned short*)malloc(sizeof(unsigned short)*m_maxNodes);
	m_first = (unsigned short*)malloc(sizeof(unsigned short)*hashSize);

	ASSERT(m_nodes);
	ASSERT(m_next);
	ASSERT(m_first);

	memset(m_first, 0xff, sizeof(unsigned short)*m_hashSize);
	memset(m_next, 0xff, sizeof(unsigned short)*m_maxNodes);
}

dtNodePool::~dtNodePool()
{
	free(m_nodes);
	free(m_next);
	free(m_first);
}

void dtNodePool::clear()
{
	memset(m_first, 0xff, sizeof(unsigned short)*m_hashSize);
	m_nodeCount = 0;
}

dtNode* dtNodePool::findNode(dtPolyRef id)
{
	unsigned int bucket = dtHashRef(id) & (m_hashSize-1);
	unsigned short i = m_first[bucket];
	while (i != DT_NULL_IDX)
	{
		if (m_nodes[i].id == id)
			return &m_nodes[i];
		i = m_next[i];
	}
	return 0;
}

dtNode* dtNodePool::getNode(dtPolyRef id)
{
	unsigned int bucket = dtHashRef(id) & (m_hashSize-1);
	unsigned short i = m_first[bucket];
	dtNode* node = 0;
	while (i != DT_NULL_IDX)
	{
		if (m_nodes[i].id == id)
			return &m_nodes[i];
		i = m_next[i];
	}
	
	if (m_nodeCount >= m_maxNodes)
		return 0;
	
	i = (unsigned short)m_nodeCount;
	m_nodeCount++;
	
	// Init node
	node = &m_nodes[i];
	node->pidx = 0;
	node->cost = 0;
	node->total = 0;
	node->id = id;
	node->flags = 0;
	
	m_next[i] = m_first[bucket];
	m_first[bucket] = i;
	
	return node;
}


//////////////////////////////////////////////////////////////////////////////////////////
dtNodeQueue::dtNodeQueue(int n) :
	m_heap(0),
	m_capacity(n),
	m_size(0)
{
	ASSERT(m_capacity > 0);
	
	m_heap = (dtNode**)malloc(sizeof(dtNode*)*(m_capacity+1));
	ASSERT(m_heap);
}

dtNodeQueue::~dtNodeQueue()
{
	free(m_heap);
}

void dtNodeQueue::bubbleUp(int i, dtNode* node)
{
	int parent = (i-1)/2;
	// note: (index > 0) means there is a parent
	while ((i > 0) && (m_heap[parent]->total > node->total))
	{
		m_heap[i] = m_heap[parent];
		i = parent;
		parent = (i-1)/2;
	}
	m_heap[i] = node;
}

void dtNodeQueue::trickleDown(int i, dtNode* node)
{
	int child = (i*2)+1;
	while (child < m_size)
	{
		if (((child+1) < m_size) && 
			(m_heap[child]->total > m_heap[child+1]->total))
		{
			child++;
		}
		m_heap[i] = m_heap[child];
		i = child;
		child = (i*2)+1;
	}
	bubbleUp(i, node);
}
