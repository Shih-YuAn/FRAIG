/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/
#define MAX_USELESS 10
#define WORD_SIZE   (sizeof(size_t) * 8)

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
	// put all gates into a single FEC group
	_fecGrps.clear();
	_fecGrps.resize(1);
	_fecGrps[0].push_back(&_gates[0]->_wires[0]);
	_fecGrps[0].push_back(&_gates[0]->_wires[1]);
	for(unsigned i = 0, n = _dfsList.size(); i < n; ++i){
		if(_dfsList[i]->getType() == AIG_GATE){
			_fecGrps[0].push_back(&_dfsList[i]->_wires[0]);
			_fecGrps[0].push_back(&_dfsList[i]->_wires[1]);
		}
	}
	// perform simulation to divide FEC groups
	unsigned patNum = 0;
	unsigned useless = 0;
	while(useless < MAX_USELESS){
		patternGen();
		simulate();
		useless = (divideGrps() ? 0 : useless + 1);
		patNum += WORD_SIZE;
	}
	sortFecGrps();
	cout << patNum << " patterns simulated.\n";
}

void
CirMgr::fileSim(ifstream& patternFile)
{
	// store patterns from file into vectors
	vector< vector<char> > patterns;
	vector<char> newPat;
	patterns.push_back(newPat);
	unsigned cursor = 0;
	char buf = patternFile.get();
	while(!patternFile.eof()){
		if(buf == ' ' || buf == '\n'){
			if(patterns[cursor].size()){
				patterns.push_back(newPat);
				++cursor;
			}
		}
		else{
			patterns[cursor].push_back(buf);
		}
		patternFile.get(buf);
	}
	if(patterns[patterns.size() - 1].empty())
		patterns.pop_back();

/*	for(size_t s = 0; s < patterns.size(); ++s){
		for(size_t t = 0; t < patterns[s].size(); ++t)
			cout << patterns[s][t] << ' ';
		cout << endl;
	}*/
	
//	cerr << "1\n";

	_fecGrps.clear();
	// check the correctness of patterns
	unsigned patNum = 0;
	cursor = 0;
	if(!checkPat(patterns, patNum)){
		cout << patNum << " patterns simulated.\n";
		return ;
	}

//	cerr << "2\n";
	// put all gates into a single FEC group
	_fecGrps.resize(1);
	_fecGrps[0].push_back(&_gates[0]->_wires[0]);
	_fecGrps[0].push_back(&_gates[0]->_wires[1]);
	for(unsigned i = 0, n = _dfsList.size(); i < n; ++i){
		if(_dfsList[i]->getType() == AIG_GATE){
			_fecGrps[0].push_back(&_dfsList[i]->_wires[0]);
			_fecGrps[0].push_back(&_dfsList[i]->_wires[1]);
		}
	}

//	cerr << "3\n";
	// generate patterns to PIs
	for(unsigned i = 0; i < _nPI; ++i)
		_gates[_PIList[i]]->_value = 0;
	for(unsigned i = cursor; i < patNum; ++i){
		for(unsigned j = 0; j < _nPI; ++j){
			if(patterns[i][j] == '1')
				_gates[_PIList[j]]->_value |= ((size_t)1 << (i - cursor));
		}
	}
	
//	cerr << "4\n";

	simulate();

//	cerr << "5\n";
	divideGrps();
//	cerr << "6\n";
	if(patNum == patterns.size()){
		sortFecGrps();
		cout << "\nTotal #FEC Group = " << _fecGrps.size() << endl;
		cout << patNum << " patterns simulated.\n";
		return ;
	}

	cursor = patNum;
	while(checkPat(patterns, patNum)){
		for(unsigned i = 0; i < _nPI; ++i)
			_gates[_PIList[i]]->_value = 0;
		for(unsigned i = cursor; i < patNum; ++i){
			for(unsigned j = 0; j < _nPI; ++j){
				if(patterns[i][j] == '1')
					_gates[_PIList[j]]->_value |= ((size_t)1 << (i - cursor));
			}
		}
		
		simulate();
		divideGrps();
		if(patNum == patterns.size()){
			sortFecGrps();
			cout << "\nTotal #FEC Group = " << _fecGrps.size() << endl;
			cout << patNum << " patterns simulated.\n";
			return ;
		}
		cursor = patNum;
	}
	sortFecGrps();
	cout << patNum << " patterns simulated.\n";
	return ;
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
void CirMgr::patternGen(){
	for(unsigned i = 0, n = _PIList.size(); i < n; ++i)
		_gates[_PIList[i]]->_value = rnGen(INT_MAX);
}

void CirMgr::simulate(){
	for(unsigned i = 0, n = _dfsList.size(); i < n; ++i){
		_dfsList[i]->simulate();
	}
}

bool CirMgr::divideGrps(){
	bool divided = false;
	for(int i = _fecGrps.size() - 1; i >= 0; --i){
		HashMap<SimKey, FecGroup> grpsHash(_nAIG);
		FecGroup grpRef;
		bool found;
		for(unsigned j = 0, m = _fecGrps[i].size(); j < m; ++j){
			FecGroup buf(1, _fecGrps[i][j]);
			grpsHash.findGrp(*_fecGrps[i][j], buf, found).push_back(_fecGrps[i][j]);
		}

		HashMap<SimKey, FecGroup>::iterator it = grpsHash.begin();
		if((*it).second.size() < _fecGrps[i].size()){
			divided = true;
			_fecGrps.erase(_fecGrps.begin() + i);
			for(; it != grpsHash.end(); ++it){
				if((*it).second.size() > 1 && !(*it).second[0]->_inv)
					_fecGrps.push_back((*it).second);
			}
		}
	}
	return divided;
}

bool CirMgr::checkPat(const vector< vector<char> >& patterns, unsigned &patNum){
	for(unsigned i = 0; i < WORD_SIZE; ++i){
		if(i + patNum == patterns.size()){
			patNum += i;
			return true;
		}
		if(patterns[i + patNum].size() != _nPI){
			cout << "\nError: Pattern(";
			for(size_t s = 0; s < patterns[i + patNum].size(); ++s)
				cout << patterns[i + patNum][s];
			cout << ") length(" << patterns[i + patNum].size()
				  << ") does not match the number of inputs(" << _nPI << ") in a circuit!!\n\n";
			return false;
		}
		for(unsigned j = 0; j < _nPI; ++j){
			if(patterns[i + patNum][j] != '0' && patterns[i + patNum][j] != '1'){
				cout << "\nError: Pattern(";
				for(size_t s = 0; s < patterns[i + patNum].size(); ++s)
					cout << patterns[i + patNum][s];
				cout  << ") contains a non-0/1 character (\'" << patterns[i + patNum][j] << "\').\n\n";
				return false;
			}
		}
	}
	patNum += WORD_SIZE;
	return true;
}

void CirMgr::sortFecGrps(){
	// sort _fecGrps
	FecGroup tmp;
	for(unsigned i = 1, n = _fecGrps.size(); i < n; ++i){
		for(unsigned j = i; j > 0; --j){
			if(_fecGrps[j][0]->_id < _fecGrps[j - 1][0]->_id){
				tmp = _fecGrps[j];
				_fecGrps[j] = _fecGrps[j - 1];
				_fecGrps[j - 1] = tmp;
			}
			else
				break;
		}
	}
}