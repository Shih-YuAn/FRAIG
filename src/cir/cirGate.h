/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"
#include "sat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class CirGate;
class CirGateV;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------

class SimKey
{
public:
	SimKey(size_t id, size_t val, bool inv)
	:_id(id), _value(val), _inv(inv) {}
	~SimKey(){}

   size_t operator() () const {
		return _value;
	}

   bool operator == (const SimKey& k) const {
		return _value == k._value;
	}

	size_t _id;
	size_t _value;
	bool _inv;
};

class CirGate
{
public:
   CirGate()
	:_undefined(false), _unused(false), _traversed(0),
	 _value(0) {}
   virtual ~CirGate() {}

   // Basic access methods
   virtual string getTypeStr() const = 0;
   unsigned getLineNo() const { return _line; }
   virtual bool isAig() const = 0;

   virtual GateType getType() const = 0;

   // Printing functions
   virtual void printGate() const = 0;
   void reportGate() const;
   void reportFanin(int level) const;
   void reportFanout(int level) const;

	void printFanin(CirGate *const &, bool, int &, int &) const;
   void printFanout(const CirGate *, CirGate *const &, int &, int&) const;
	
	// for simulation
	virtual void simulate() = 0;
	
	// variables
	unsigned _id;
   string _name;
   unsigned _line;
   vector<CirGateV> _faninList;
   GateList _fanoutList;

   bool _undefined;
   bool _unused;
   unsigned _traversed;
   static unsigned _globalTraversed;
	// for simulation
	size_t _value;
	vector<SimKey> _wires;
	//vector<size_t> _values(8);
	//static unsigned _curVal;

private:
protected:
};

class CirGateV
{
	#define NEG 0x1
public:
   CirGateV(CirGate* g, size_t phase)
   :_gateV(size_t(g) + phase){}
   
   CirGate *gate() const{
      return (CirGate*)(_gateV & ~size_t(NEG));
   }
   bool isInv() const{
      return (_gateV & NEG);
   }

   size_t operator() () const {
		return _gateV;
   }
private:
	size_t _gateV;
};

class CirPIGate : public CirGate
{
public:
   CirPIGate(unsigned id, unsigned l = 0){
      _id = id;
      _line = l;
   }
	virtual ~CirPIGate() {}

   void printGate() const{
		cout << "PI  " << _id;
		if(_name.size())
      	cout << " (" << _name << ')';
		cout << endl;
	}
   
   virtual GateType getType() const{
      return PI_GATE;
   }
   virtual string getTypeStr() const{
      return "PI";
   }
	virtual bool isAig() const {
		return false;
	}

	virtual void simulate() {}
};

class CirPOGate : public CirGate
{
public:
   CirPOGate(unsigned id, unsigned l = 0){
      _id = id;
      _line = l;
   }
	virtual ~CirPOGate() {}
   
   void printGate() const{
		cout << "PO  " << _id << ' ';
      if(_faninList[0].gate()->_undefined)
         cout << '*';
      if(_faninList[0].isInv())
         cout << '!';
      cout << _faninList[0].gate()->_id;
		if(_name.size())
      	cout << " (" << _name << ')';
		cout << endl;
	}
   
   virtual GateType getType() const{
      return PO_GATE;
   }
   virtual string getTypeStr() const{
      return "PO";
   }
	virtual bool isAig() const {
		return false;
	}
	virtual void simulate() {
		_value = (_faninList[0].isInv() ? ~_faninList[0].gate()->_value : _faninList[0].gate()->_value);
	}
};

class CirConstGate : public CirGate
{
public:
   CirConstGate(){
      _id = 0;
      _line = 0;
		_wires.push_back(SimKey(0, 0, false));
		_wires.push_back(SimKey(0, ~0, true));
   }
	virtual ~CirConstGate() {}

   void printGate() const{
		cout << "CONST0\n";
	}

   virtual GateType getType() const{
      return CONST_GATE;
   }
   virtual string getTypeStr() const{
      return "CONST";
   }
	virtual bool isAig() const {
		return false;
	}
	virtual void simulate() {}
};

class CirAigGate : public CirGate
{
public:
   CirAigGate(unsigned id, unsigned l = 0){
      _id = id;
      _line = l;
		_wires.push_back(SimKey(_id, _value, false));
		_wires.push_back(SimKey(_id, ~_value, true));
   }
	virtual ~CirAigGate() {}

   void printGate() const{
		cout << "AIG " << _id;
		for(int i = 0; i < 2; ++i){
     		cout << ' ';
      	if(_faninList[i].gate()->_undefined)
         	cout << '*';
      	if(_faninList[i].isInv())
         	cout << '!';
      	cout << _faninList[i].gate()->_id;
   	}
   	if(_name.size())
      	cout << " (" << _name << ')';
		cout << endl;
	}
   
   virtual GateType getType() const{
      return AIG_GATE;
   }
   virtual string getTypeStr() const{
      if(_undefined)
         return "UNDEF";
      return "AIG";
   }
	virtual bool isAig() const {
		return true;
	}

	virtual void simulate(){
		_value = (_faninList[0].isInv() ? ~_faninList[0].gate()->_value : _faninList[0].gate()->_value)
					& (_faninList[1].isInv() ? ~_faninList[1].gate()->_value : _faninList[1].gate()->_value);
		_wires[0]._value = _value;
		_wires[1]._value = ~_value;
	}
};

#endif // CIR_GATE_H
