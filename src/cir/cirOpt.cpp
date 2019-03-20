/****************************************************************************
	FileName     [ cirSim.cpp ]
	PackageName  [ cir ]
	Synopsis     [ Define cir optimization functions ]
	Author       [ Chung-Yang (Ric) Huang ]
	Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep()
{
	for(unsigned i = 0, n = _unusedList.size(); i < n; ++i)
		dfsSweep(_unusedList[i]);
	updateOpen();
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{
	// 1. fanin has constant 1 -> replace by the other fanin
	// 2. fanin has constant 0 -> replace with 0
	// 3. identical fanins -> replace with (fanin + phase)
	// 4. inverted fanins -> replace with 0
	for(unsigned i = 0, n = _dfsList.size(); i < n; ++i){
		if(_dfsList[i]->getType() != AIG_GATE) // note that there are no undefined gates in _dfsList
			continue;
		
		CirGate *&g = _dfsList[i];
		if(g->_faninList[0].gate() == _gates[0]){
			if(g->_faninList[0].isInv()) // 1.
				replaceGG(g, 1);
			else // 2.
				replaceG0(g);
		}
		else if(g->_faninList[1].gate() == _gates[0]){
			if(g->_faninList[1].isInv()) // 1.
				replaceGG(g, 0);
			else // 2.
				replaceG0(g);
		}
		else if(g->_faninList[0].gate() == g->_faninList[1].gate()){
			if(g->_faninList[0].isInv() == g->_faninList[1].isInv()) // 3.
				replaceGG(g, 0);
			else // 4.				
				replaceG0(g);
		}
	}
	updateOpen();
	updateDFS();
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
void CirMgr::dfsSweep(CirGate *const &g){
	if(g == 0)
		return ;
	// break connection with unused fanouts
	CirGate *fo;
	for(int i = g->_fanoutList.size() - 1; i >= 0; --i){
		fo = g->_fanoutList[i];
		if(fo->_unused){
			for(size_t s = 0; s < fo->_faninList.size(); ++s){
				if(fo->_faninList[s].gate() == g){
					fo->_faninList.erase(fo->_faninList.begin() + s);
					break;
				}
			}
			g->_fanoutList.erase(g->_fanoutList.begin() + i);
		}
	}
	
	// sweep if g is a unused AND gate
	if(g->getType() == AIG_GATE && g->_fanoutList.empty()){
		g->_unused = true;
		for(size_t s = 0; s < g->_faninList.size(); ++s)
			dfsSweep(g->_faninList[s].gate());
		
		if(!g->_undefined)
			--_nAIG;
		unsigned index = g->_id;
		cout << "Sweeping: " << g->getTypeStr() << '(' << index << ") removed...\n";
		delete _gates[index];
		_gates[index] = 0;
	}
}

void CirMgr::replaceG0(CirGate* const &g){
	unsigned index = g->_id;
	cout << "Simplifying: 0 merging " << index << "...\n";
	CirGate *temp;
	// disconnect fanin gates from g
	for(size_t s = 0; s < g->_faninList.size(); ++s){
		temp = g->_faninList[s].gate();
		for(size_t t = 0; t < temp->_fanoutList.size(); ++t){
			if(temp->_fanoutList[t] == g){
				temp->_fanoutList.erase(temp->_fanoutList.begin() + t);
				break;
			}
		}
	}
	// connect each fanout of g to const0
	for(size_t s = 0; s < g->_fanoutList.size(); ++s){
		temp = g->_fanoutList[s];
		for(size_t t = 0; t < temp->_faninList.size(); ++t){
			if(temp->_faninList[t].gate() == g){
				temp->_faninList[t] = CirGateV(_gates[0], temp->_faninList[t].isInv()); // not sure
				_gates[0]->_fanoutList.push_back(temp);
				break;
			}
		}
	}
	delete _gates[index];
	_gates[index] = 0;
	--_nAIG;
}

void CirMgr::replaceGG(CirGate* const &g, int fi){
	unsigned index = g->_id;
	CirGate *temp = g->_faninList[fi].gate();
	cout << "Simplifying: " << temp->_id << " merging ";
	if(g->_faninList[fi].isInv())
		cout << '!';
	cout << index << "...\n";
	// disconnect the fanin gate
	for(size_t s = 0; s < temp->_fanoutList.size(); ++s){
		if(temp->_fanoutList[s] == g){
			temp->_fanoutList.erase(temp->_fanoutList.begin() + s);
			break;
		}
	}
	// connect fanouts of g with the fanin gate
	for(size_t s = 0; s < g->_fanoutList.size(); ++s){
		CirGate *temp2 = g->_fanoutList[s];
		temp->_fanoutList.push_back(temp2);
		for(size_t t = 0; t < temp2->_faninList.size(); ++t){
			if(temp2->_faninList[t].gate() == g){
				if(temp2->_faninList[t].isInv() == g->_faninList[fi].isInv())
					temp2->_faninList[t] = CirGateV(temp, 0);
				else
					temp2->_faninList[t] = CirGateV(temp, 1);
				break;
			}
		}
	}
	delete _gates[index];
	_gates[index] = 0;
	--_nAIG;
}