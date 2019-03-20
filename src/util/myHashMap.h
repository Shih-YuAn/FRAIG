/****************************************************************************
  FileName     [ myHashMap.h ]
  PackageName  [ util ]
  Synopsis     [ Define HashMap and Cache ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2009-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_MAP_H
#define MY_HASH_MAP_H

#include <vector>

using namespace std;

// TODO: (Optionally) Implement your own HashMap and Cache classes.

//-----------------------
// Define HashMap classes
//-----------------------
// To use HashMap ADT, you should define your own HashKey class.
// It should at least overload the "()" and "==" operators.
//
class HashKey
{
public:
   HashKey(size_t s1, size_t s2)
	:_in0(s1), _in1(s2) {}

   size_t operator() () const {
		return _in0 + _in1; // maybe not good!!!
	}

   bool operator == (const HashKey& k) const {
		return ((_in0 == k._in0 && _in1 == k._in1) || (_in0 == k._in1 && _in1 == k._in0));
	}

private:
	size_t _in0, _in1;
};
/*
// for simulation
class SimKey
{
public:
	SimKey(){}

	size_t operator() () const {
		return _value;
	}

   bool operator == (const HashKey& k) const {
		if(_value == k._value)
			return true;
		if(_value == ~k._value){
			_inverted = !_inverted;
			return true;
		}
	}

private:
	size_t _value;
	bool _inverted;
};
*/
template <class HashKey, class HashData>
class HashMap
{
typedef pair<HashKey, HashData> HashNode;

public:
   HashMap(size_t b=0) : _numBuckets(0), _buckets(0) { if (b != 0) init(b); }
   ~HashMap() { reset(); }

   // [Optional] TODO: implement the HashMap<HashKey, HashData>::iterator
   // o An iterator should be able to go through all the valid HashNodes
   //   in the HashMap
   // o Functions to be implemented:
   //   - constructor(s), destructor
   //   - operator '*': return the HashNode
   //   - ++/--iterator, iterator++/--
   //   - operators '=', '==', !="
   //
   class iterator
   {
      friend class HashMap<HashKey, HashData>;
   public:
		iterator(vector<HashNode> *b, size_t n, size_t x, size_t y)
      :_buckets(b), _numBuckets(n), _x(x), _y(y) {}
      iterator(const iterator &i)
      :_buckets(i._buckets), _numBuckets(i._numBuckets), _x(i._x), _y(i._y) {}
      ~iterator(){}

		const HashNode& operator * () const {
         return _buckets[_x][_y];
      }

		iterator& operator ++ () {
         // unpredictable when at end()
         if(_y < _buckets[_x].size() - 1) ++_y;
         else{
            _y = 0;
            while(++_x < _numBuckets)
               if(_buckets[_x].size()) break;
         }
         return *this;
      }
      iterator operator ++ (int){
         iterator temp(*this);
         if(_y < _buckets[_x].size() - 1) ++_y;
         else{
            _y = 0;
            while(++_x < _numBuckets)
               if(_buckets[_x].size()) break;
         }
         return temp;
      }
      iterator& operator -- (){
         // unpredictable when at begin()
         if(_y > 0) --_y;
         else{
            while(_x > 0){
              --_x;
              if(_buckets[_x].size()){
                 _y = _buckets[_x].size() - 1;
                 break;
              }
            }
         }
         return *this;
      }
      iterator operator -- (int){
         iterator temp(*this);
         if(_y > 0) --_y;
         else{
            while(_x > 0){
              --_x;
              if(_buckets[_x].size()){
                 _y = _buckets[_x].size() - 1;
                 break;
              }
            }
         }
         return temp;
      }

		iterator& operator = (const iterator& i){
			_x = i._x;
         _y = i._y;
         return *this;
      }
      bool operator == (const iterator& i) const{
         return (_x == i._x && _y == i._y);
      }
      bool operator != (const iterator& i) const {
         return (_x != i._x || _y != i._y);
      }

   private:
		vector<HashNode> *_buckets;
      size_t _numBuckets;
      size_t _x, _y;
   };

   void init(size_t b) {
      reset(); _numBuckets = b; _buckets = new vector<HashNode>[b]; }
   void reset() {
      _numBuckets = 0;
      if (_buckets) { delete [] _buckets; _buckets = 0; }
   }
   void clear() {
      for (size_t i = 0; i < _numBuckets; ++i) _buckets[i].clear();
   }
   size_t numBuckets() const { return _numBuckets; }

   vector<HashNode>& operator [] (size_t i) { return _buckets[i]; }
   const vector<HashNode>& operator [](size_t i) const { return _buckets[i]; }

   // TODO: implement these functions
   //
   // Point to the first valid data
   iterator begin() const {
		for(size_t s = 0; s < _numBuckets; ++s){
      	if(_buckets[s].size())
      		return iterator(_buckets, _numBuckets, s, 0);
   	}
   	// if empty
		return end();
	}
   // Pass the end
   iterator end() const {
		return iterator(_buckets, _numBuckets, _numBuckets, 0);
	}
   // return true if no valid data
   bool empty() const {
		for(size_t s = 0; s < _numBuckets; ++s){
			if(_buckets[s].size)
      		return false;
		}
      return true;
	}
   // number of valid data
   size_t size() const {
		size_t s = 0;
      for(size_t t = 0; t < _numBuckets; ++t)
         s += _buckets[t].size();
      return s;
	}

   // check if k is in the hash...
   // if yes, return true;
   // else return false;
   bool check(const HashKey& k) const {
		size_t num = bucketNum(k);
		for(size_t s = 0, n = _buckets[num].size(); s < n; ++s){
			if(k == _buckets[num][s].first)
				return true;
		}
		return false;
	}

   // query if k is in the hash...
   // if yes, replace d with the data in the hash and return true;
   // else return false;
   bool query(const HashKey& k, HashData& d) const {
		size_t num = bucketNum(k);
		for(size_t s = 0, n = _buckets[num].size(); s < n; ++s){
			if(k == _buckets[num][s].first){
				d = _buckets[num][s].second;
				return true;
			}
		}
		return false;
	}

   // update the entry in hash that is equal to k (i.e. == return true)
   // if found, update that entry with d and return true;
   // else insert d into hash as a new entry and return false;
   bool update(const HashKey& k, HashData& d) {
		size_t num = bucketNum(k);
		for(size_t s = 0, n = _buckets[num].size(); s < n; ++s){
			if(k == _buckets[num][s].first){
				_buckets[num][s].second = d;
				return true;
			}
		}
		_buckets[num].push_back(HashNode(k, d));
		return false;
	}

   // return true if inserted d successfully (i.e. k is not in the hash)
   // return false is k is already in the hash ==> will not insert
   bool insert(const HashKey& k, const HashData& d) {
		size_t num = bucketNum(k);
		for(size_t s = 0, n = _buckets[num].size(); s < n; ++s){
			if(k == _buckets[num][s].first)
				return false;
		}
		_buckets[num].push_back(HashNode(k, d));
		return true;
	}

   // return true if removed successfully (i.e. k is in the hash)
   // return fasle otherwise (i.e. nothing is removed)
   bool remove(const HashKey& k) {
		size_t num = bucketNum(k);
		for(size_t s = 0, n = _buckets[num].size(); s < n; ++s){
			if(k == _buckets[num][s].first){
				_buckets[num].erase(_buckets[num].begin() + s);
				return true;
			}
		}
		return false;
	}

	// for strash, simulation
	HashData search(const HashKey &k){
		size_t num = bucketNum(k);
		for(size_t s = 0, n = _buckets[num].size(); s < n; ++s){
			if(k == _buckets[num][s].first)
				return _buckets[num][s].second;
		}
		return 0;
	}
	HashData& findGrp(const HashKey& k, HashData &d, bool& found) const {
		size_t num = bucketNum(k);
		for(size_t s = 0, n = _buckets[num].size(); s < n; ++s){
			if(k == _buckets[num][s].first){
				found = true;
				return _buckets[num][s].second;
			}
		}
		found = false;
		_buckets[num].push_back(HashNode(k, d));
		return d;
	}
	void push(const HashKey &k, const HashData &d){
		_buckets[bucketNum(k)].push_back(HashNode(k, d));
	}

private:
   // Do not add any extra data member
   size_t                   _numBuckets;
   vector<HashNode>*        _buckets;

   size_t bucketNum(const HashKey& k) const {
      return (k() % _numBuckets); }

};


//---------------------
// Define Cache classes
//---------------------
// To use Cache ADT, you should define your own HashKey class.
// It should at least overload the "()" and "==" operators.
//
// class CacheKey
// {
// public:
//    CacheKey() {}
//    
//    size_t operator() () const { return 0; }
//   
//    bool operator == (const CacheKey&) const { return true; }
//       
// private:
// }; 
// 
template <class CacheKey, class CacheData>
class Cache
{
typedef pair<CacheKey, CacheData> CacheNode;

public:
   Cache() : _size(0), _cache(0) {}
   Cache(size_t s) : _size(0), _cache(0) { init(s); }
   ~Cache() { reset(); }

   // NO NEED to implement Cache::iterator class

   // TODO: implement these functions
   //
   // Initialize _cache with size s
   void init(size_t s) { reset(); _size = s; _cache = new CacheNode[s]; }
   void reset() {  _size = 0; if (_cache) { delete [] _cache; _cache = 0; } }

   size_t size() const { return _size; }

   CacheNode& operator [] (size_t i) { return _cache[i]; }
   const CacheNode& operator [](size_t i) const { return _cache[i]; }

   // return false if cache miss
   bool read(const CacheKey& k, CacheData& d) const {
      size_t i = k() % _size;
      if (k == _cache[i].first) {
         d = _cache[i].second;
         return true;
      }
      return false;
   }
   // If k is already in the Cache, overwrite the CacheData
   void write(const CacheKey& k, const CacheData& d) {
      size_t i = k() % _size;
      _cache[i].first = k;
      _cache[i].second = d;
   }

private:
   // Do not add any extra data member
   size_t         _size;
   CacheNode*     _cache;
};


#endif // MY_HASH_H
