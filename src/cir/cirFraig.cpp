/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void
CirMgr::strash()
{
	HashMap<HashKey, CirGate *> hash(_maxVarId);
	CirGate *matchGate;
	for(size_t s = 0, n = _dfsList.size(); s < n; ++s){
		if(_dfsList[s]->getType() != AIG_GATE || _dfsList[s]->_undefined)
			continue;
		HashKey key(_dfsList[s]->_faninList[0](), _dfsList[s]->_faninList[1]());
		matchGate = hash.search(key);
		if(matchGate)
			mergeGates(_dfsList[s], matchGate); // order matters!!!
		else
			hash.push(key, _dfsList[s]);
	}
	updateDFS();
}

void
CirMgr::fraig()
{
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
void CirMgr::mergeGates(CirGate *const &first, CirGate *const &second){
	cout << "Strashing: " << second->_id << " merging " << first->_id << "...\n";
	// disconnect fanins of first
	CirGate *temp;
	for(int i = 0; i < 2; ++i){
		temp = first->_faninList[i].gate();
		for(size_t s = 0; s < temp->_fanoutList.size(); ++s){
			if(temp->_fanoutList[s] == first){
				temp->_fanoutList.erase(temp->_fanoutList.begin() + s);
				break;
			}
		}
	}
	// connect fanouts of first to second
	for(size_t s = 0; s < first->_fanoutList.size(); ++s){
		temp = first->_fanoutList[s];
		for(size_t t = 0; t < temp->_faninList.size(); ++t){
			if(temp->_faninList[t].gate() == first){
				temp->_faninList[t] = CirGateV(second, temp->_faninList[t].isInv());
				second->_fanoutList.push_back(temp);
				break;
			}
		}
	}
	// delete first
	unsigned index = first->_id;
	delete _gates[index];
	_gates[index] = 0;
	--_nAIG;
}