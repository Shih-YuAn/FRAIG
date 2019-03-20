/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"

extern CirMgr *cirMgr;

class CirMgr
{
public:
   CirMgr() {}
   ~CirMgr() {} 

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const {
      // will print out UNDEF for _undefined gates
		if(gid < _gates.size())
			return _gates[gid];
      return 0;
   }

   // Member functions about circuit construction
   bool readCircuit(const string&);

   // Member functions about circuit optimization
   void sweep();
   void optimize();

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*) const;

	vector<FecGroup> _fecGrps;

private:
   ofstream           *_simLog;
   
   // AIGER
   unsigned _maxVarId;
   unsigned _nPI;
   unsigned _nPO;
   unsigned _nAIG;

   GateList _gates; // for finding gates through Id
   IdList _PIList;
	GateList _dfsList;
	GateList _unusedList;
	GateList _undefList;

   void updateOpen(); // update _unusedList, undefList
	void updateDFS(); // update _dfsList
	void dfsTraverse(CirGate *const &);

	// for optimization
	void dfsSweep(CirGate *const &);
	void replaceG0(CirGate* const &);
	void replaceGG(CirGate* const &, int);

	// for simulation
	void patternGen();
	void simulate();
	bool divideGrps();
	bool checkPat(const vector< vector<char> >&, unsigned&);
	void sortFecGrps();
	
	// for fraig
	void mergeGates(CirGate *const &, CirGate *const &);
};

#endif // CIR_MGR_H
