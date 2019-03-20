/****************************************************************************
	FileName     [ cirGate.cpp ]
	PackageName  [ cir ]
	Synopsis     [ Define class CirAigGate member functions ]
	Author       [ Chung-Yang (Ric) Huang ]
	Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;
unsigned CirGate::_globalTraversed = 0;

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() const
{
	for(int i = 0; i < 80; ++i)
		cout << '=';
	cout << "\n= " << getTypeStr() << '(' << _id << ')';
	if(_name.size())
		cout << '\"' << _name << '\"';
	cout << ", line " << _line;
	// fec groups
	cout << "\n= FECs:";
	for(unsigned i = 0, n = cirMgr->_fecGrps.size(); i < n; ++i){
		for(unsigned j = 0, m = cirMgr->_fecGrps[i].size(); j < m; ++j){
			if(_id == cirMgr->_fecGrps[i][j]->_id){
				for(unsigned k = 0, l = cirMgr->_fecGrps[i].size(); k < l; ++k){
					if(k == j)
						continue;
					cout << ' ';
					if(cirMgr->_fecGrps[i][k]->_inv != cirMgr->_fecGrps[i][j]->_inv)
						cout << '!';
					cout << cirMgr->_fecGrps[i][k]->_id;
				}
				break;
			}
		}
	}

	// simulation value
	cout << "\n= Value: ";
	for(unsigned i = 0, n = sizeof(size_t) * 8; i < n; ++i){
		cout << ((_value >> (n - i - 1)) & 1);
		if(i % 8 == 7 && i != n - 1)
			cout << '_';
	}

	cout << endl;
	for(int i = 0; i < 80; ++i)
		cout << '=';
	cout << endl;
}

void
CirGate::reportFanin(int level) const
{
	assert (level >= 0);
	cout << getTypeStr() << ' ' << _id << endl;
	if(level == 0)
		return ;

	int curLevel = 1;
	++CirGate::_globalTraversed;
	for(size_t s = 0; s < _faninList.size(); ++s)
		printFanin(_faninList[s].gate(), _faninList[s].isInv(), curLevel, level);
}

void
CirGate::reportFanout(int level) const
{
	assert (level >= 0);
	cout << getTypeStr() << ' ' << _id << endl;
	if(level == 0)
		return ;

	int curLevel = 1;
	++CirGate::_globalTraversed;
	for(size_t s = 0; s < _fanoutList.size(); ++s)
		printFanout(this, _fanoutList[s], curLevel, level);
}

void CirGate::printFanin(CirGate *const &g, bool inv, int &curLevel, int &level) const{
	 // print
	 for(int i = 0; i < curLevel; ++i)
		 cout << "  ";
	 if(inv)
			cout << '!';
	 cout << g->getTypeStr() << ' ' << g->_id;
	 if(g->_traversed == _globalTraversed && curLevel < level && !g->_undefined)
			cout << " (*)";
	 cout << endl;

	 // traverse
	 if(curLevel == level || g->_traversed == _globalTraversed)
			return ;
	 g->_traversed = _globalTraversed;
	 ++curLevel;
	 for(size_t s = 0; s < g->_faninList.size(); ++s)
			printFanin(g->_faninList[s].gate(), g->_faninList[s].isInv(), curLevel, level);
	 --curLevel;
}

void CirGate::printFanout(const CirGate *pre, CirGate *const &cur, int &curLevel, int &level) const{
	 // print
	 for(int i = 0; i < curLevel; ++i)
		 cout << "  ";
	 for(size_t s = 0; s < cur->_faninList.size(); ++s){
			if(cur->_faninList[s].gate() == pre){
				 if(cur->_faninList[s].isInv())
						cout << '!';
				 cout << cur->getTypeStr() << ' ' << cur->_id;
				 if(cur->_traversed == _globalTraversed && curLevel < level && !cur->_unused)
						cout << " (*)";
				 cout << endl;
				 break;
			}
	 }

	 // traverse
	 if(curLevel == level || cur -> _traversed == _globalTraversed)
			return ;
	 cur -> _traversed = _globalTraversed;
	 ++curLevel;
	 for(size_t s = 0; s < cur -> _fanoutList.size(); ++s)
			printFanout(cur, cur -> _fanoutList[s], curLevel, level);
	 --curLevel;
}