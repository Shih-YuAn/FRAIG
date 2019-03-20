/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{
   ifstream inFile(fileName.c_str(), ios::in);
   if(!inFile){
      cerr << "Cannot open design \"" << fileName << "\"!!\n";
      return false;
   }

   string inStr;
   unsigned inNum;

   // header
   inFile >> inStr; // aag
   inFile >> _maxVarId;
   inFile >> _nPI;
   inFile >> inNum; // L
   inFile >> _nPO;
   inFile >> _nAIG;

   _gates.resize(_maxVarId + 1 +_nPO);
   _gates[0] = new CirConstGate();
   for(size_t s = 1; s < _gates.size(); ++s)
      _gates[s] = 0;
   
	// inputs
	// problem: not sure about the order of fanins!!!!!
	unsigned line = 1;
   for(unsigned i = 0; i < _nPI; ++i){
      inFile >> inNum;
      _PIList.push_back(inNum / 2);
      _gates[inNum / 2] = new CirPIGate(inNum / 2, ++line);
   }

	// outputs
	for(unsigned i = 0; i < _nPO; ++i){
      inFile >> inNum;
      _gates[_maxVarId + i + 1] = new CirPOGate(_maxVarId + i + 1, ++line);
      if(_gates[inNum / 2] == 0)
         _gates[inNum / 2] = new CirAigGate(inNum / 2);
      
      // connect PO and its fanin
      _gates[_maxVarId + i + 1]->_faninList.push_back(CirGateV(_gates[inNum / 2], inNum % 2));
      _gates[inNum / 2]->_fanoutList.push_back(_gates[_maxVarId + i + 1]);
   }

	// AND gates
	CirGate *cur;
	unsigned fi[2];
   for(unsigned i = 0; i < _nAIG; ++i){
      inFile >> inNum;
      if(_gates[inNum / 2] == 0)
         _gates[inNum / 2] = new CirAigGate(inNum / 2, ++line);
      else
         _gates[inNum / 2] -> _line = ++line;
      
      cur = _gates[inNum / 2];
      for(unsigned j = 0; j < 2; ++j){
         inFile >> inNum;
         if(_gates[inNum / 2] == 0)
            _gates[inNum / 2] = new CirAigGate(inNum / 2);
         
         // connect two fanins to the gate
         cur->_faninList.push_back(CirGateV(_gates[inNum / 2], inNum % 2));
         _gates[inNum / 2]->_fanoutList.push_back(cur);
      }
   }

	// symbols
   while(inFile >> inStr && inStr != "c"){
      if(inStr[0] == 'i'){
         inNum = stoi(inStr.substr(1, inStr.size() - 1));
         inFile.get();
         getline(inFile, inStr);
         _gates[_PIList[inNum]]->_name = inStr;
      }
      else{ // inStr[0] == 'o'
         inNum = _maxVarId + 1 + stoi(inStr.substr(1, inStr.size() - 1));
         inFile.get();
         getline(inFile, inStr);
         _gates[inNum]->_name = inStr;
      }
   }

	updateOpen();
	updateDFS();

   return true;
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
	cout << "\nCircuit Statistics"
        << "\n=================="
        << "\n  PI" << setw(12) << _nPI
        << "\n  PO" << setw(12) << _nPO
        << "\n  AIG" << setw(11) << _nAIG
        << "\n------------------"
        << "\n  Total" << setw(9) << _nPI + _nPO + _nAIG << endl;
}

void
CirMgr::printNetlist() const
{
   cout << endl;
   for (unsigned i = 0, n = _dfsList.size(); i < n; ++i) {
      cout << "[" << i << "] ";
      _dfsList[i]->printGate();
   }
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
	for(unsigned i = 0; i < _nPI; ++i)
      cout << ' ' << _PIList[i];
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
	for(unsigned i = 0; i < _nPO; ++i)
      cout << ' ' << _maxVarId + i + 1;
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
	if(_undefList.size()){
		cout << "Gates with floating fanin(s):";
		for(unsigned i = 0, n = _undefList.size(); i < n; ++i){
			for(size_t s = 0; s < _undefList[i]->_fanoutList.size(); ++s)
         	cout << ' ' << _undefList[i]->_fanoutList[s]->_id; // what if printed already?
		}
		cout << endl;
	}
		
	if(_unusedList.size()){
		cout << "Gates defined but not used  :";
		for(unsigned i = 0, n = _unusedList.size(); i < n; ++i)
			cout << ' ' << _unusedList[i]->_id;
		cout << endl;
	}
}

void
CirMgr::printFECPairs() const
{	
	for(unsigned i = 0, n = _fecGrps.size(); i < n; ++i){
		cout << '[' << i << ']';
		for(unsigned j = 0, m = _fecGrps[i].size(); j < m; ++j){
			cout << ' ';
			if(_fecGrps[i][j]->_inv != _fecGrps[i][0]->_inv)
				cout << '!';
			cout << _fecGrps[i][j]->_id;
		}
		cout << endl;
	}
}

void
CirMgr::writeAag(ostream& outfile) const
{
	// header line
   outfile << "aag " << _maxVarId << ' ' << _nPI << " 0 " << _nPO << ' ';
   // buid a GateList with only aig gates in DFS order
	GateList aigList;
	for (unsigned i = 0, n = _dfsList.size(); i < n; ++i){
		if(_dfsList[i]->getType() == AIG_GATE)
			aigList.push_back(_dfsList[i]);
	}
	outfile << aigList.size() << endl;

	// inputs
	for(unsigned i = 0; i < _nPI; ++i)
      outfile << _PIList[i] * 2 << endl;
	
	// outputs
	for(unsigned i = 0; i < _nPO; ++i){
      if(_gates[_maxVarId + 1 + i]->_faninList[0].isInv())
         outfile << _gates[_maxVarId + 1 + i]->_faninList[0].gate()->_id * 2 + 1;
      else
         outfile << _gates[_maxVarId + 1 + i]->_faninList[0].gate()->_id * 2;
      outfile << endl;
   }

	// AND gates
	for(unsigned i = 0, n = aigList.size(); i < n; ++i){
		outfile << aigList[i]->_id * 2;
		for(int j = 0; j < 2; ++j){
			if(aigList[i]->_faninList[j].isInv())
            outfile << ' ' << aigList[i]->_faninList[j].gate()->_id * 2 + 1;
         else
            outfile << ' ' << aigList[i]->_faninList[j].gate()->_id * 2;
		}
		outfile << endl;
	}

	// symbols
	for(unsigned i = 0; i < _nPI; ++i){
      if(_gates[_PIList[i]]->_name.size())
         outfile << 'i' << i << ' ' << _gates[_PIList[i]]->_name << endl;
   }
	for(unsigned i = 0; i < _nPO; ++i){
      if(_gates[_maxVarId + 1 + i]->_name.size())
         outfile << 'o' << i << ' ' << _gates[_maxVarId + 1 + i]->_name << endl;
   }
   
   // comments
   outfile << "c\nAAG output by Yu-An Shih\n";
}

void
CirMgr::writeGate(ostream& outfile, CirGate *g) const
{
	GateList gDfsList;
	//buildGdfs(g, gDfsList);
}

// supporting functions
void CirMgr::updateOpen(){
   _undefList.clear();
	_unusedList.clear();
	
	for(unsigned i = 1; i <= _maxVarId; ++i){
      if(_gates[i] == 0)
         continue;
      if(_gates[i]->getType() == AIG_GATE && _gates[i]->_faninList.empty()){
			_gates[i]->_undefined = true;
			_undefList.push_back(_gates[i]);
		}
      if(_gates[i]->_fanoutList.empty()){ // including PIs?
			_gates[i]->_unused = true;
			_unusedList.push_back(_gates[i]);
		}
   }
}

void CirMgr::updateDFS(){
	_dfsList.clear();

	++CirGate::_globalTraversed;
	for(unsigned i = 0; i < _nPO; ++i)
      dfsTraverse(_gates[_maxVarId + 1 + i]);
}

void CirMgr::dfsTraverse(CirGate *const &g){
	if(g->_undefined || g->_traversed == CirGate::_globalTraversed)
		return ;
	for(size_t s = 0; s < g->_faninList.size(); ++s)
      dfsTraverse(g->_faninList[s].gate());
	
	_dfsList.push_back(g);
	g->_traversed = CirGate::_globalTraversed;
}