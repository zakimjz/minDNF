#ifndef _SET_H
#define _SET_H

#include <vector>
#include <iostream>
#include <algorithm>
#include "helper_funs.h"

using namespace std;

// Iset: is a class
#define Iset	Set
#define Tset	Set
#define CONTAIN		1
#define BE_COND		2

// SET: is an int vector array
typedef vector<int>	SET;
typedef SET::iterator	SIt;
typedef SET::const_iterator CSIt;

class Set;
void get_item_tset(const int item, const Tset* data,const Tset* complement, Tset& T);
Tset* get_item_tset(const int item, const Tset* data, const Tset* complement);

class Set
{
public:
    SET _S;

public:
    Set(){}

    Set(int item)
    {
        _S.push_back(item);
    }

    Set(const Set& s)
    {
        _S = s._S;
    }

    void clear()
    {
        _S.clear();
    }

    void insert( int x )
    {
        _S.push_back(x);
    }

    // overloaded function, actually applies on _S.
    int size() const
    {
        return _S.size();
    }
    // store the items in _S that not in s into r
    void sub(Set &s, Set &r )
    {
        SIt it1, it2;

        r.clear();
        it1=_S.begin();
        it2=s._S.begin();

        while(it1!=_S.end() && it2!=s._S.end())
        {
            if( *it1 == *it2 )
            {
                it1++;
                it2++;
            }
            else if( *it1 < *it2 )
            {
                r.insert( *it1 );
                it1++;
            }
            else	it2++;	// *it2 < *it1
        }
        for( ; it1!=_S.end(); it1++ )
            r.insert( *it1 );
    }

    // get the tidset of OR/AND clause
    void getTset(const Tset* data, const Tset* complement, Tset &T )
    {
        Tset	TS[2];
        int	idx=0;

		if (_S.empty())
		{
			T.clear();
			return;
		}

        get_item_tset(*_S.begin(), data, complement, TS[idx]);

		Tset* plocal_ts;
        for(SIt it=_S.begin(); it!=_S.end(); it++)
        {
			plocal_ts = get_item_tset(*it,data,complement);
			Intersect( plocal_ts->_S, TS[idx]._S, TS[1-idx]._S );
			idx = 1-idx;
        }
        T = TS[idx];
    }

    bool operator==( Set &s )
    {
        SIt it1, it2;
        if(_S.size() != s.size())
            return false;
        for(it1=_S.begin(), it2=s._S.begin(); it1!=_S.end(); it1++, it2++)
            if(*it1 != *it2)  return false;
        return true;
    }

    bool operator!=( Set &s )
    {
        SIt it1, it2;
        if(_S.size() != s.size())
            return true;
        for(it1=_S.begin(), it2=s._S.begin(); it1!=_S.end(); it1++, it2++)
            if(*it1 != *it2)  return true;
        return false;
    }

    // just sort
    void sort_set()
    {
        sort(_S.begin(), _S.end());
    }

    // sort and remove duplicate
    void sort_del_dup()
    {
        int	x;
        SIt	it;

        sort(_S.begin(), _S.end());
        it = _S.begin();

        if( it != _S.end() )
        {
            x = *it;
            it++;
        }

        while( it != _S.end() )
        {
            while( it!=_S.end() && *it == x )
                it = _S.erase(it);
            if( it!=_S.end() )
            {
                x = *it;
                it++;
            }
        }
    }

    void operator+=( Set &s )
    {
        CopyEnd( s._S, _S );
        this->sort_del_dup();
    }

    bool operator< ( Set &s )
    {
        SIt it1, it2;

        if(_S.size() < s.size())	return true;
        else if(_S.size() > s.size())	return false;
        else
        {
            for(it1=_S.begin(), it2=s._S.begin(); it1!=_S.end(); it1++, it2++)
                if( *it1 < *it2 )	return true;
                else if( *it1 > *it2 )	return false;
            return false; //==
        }
    }
    // whether _S contains s
    bool contain( Set &s )
    {
        SIt it1, it2;

        if( _S.size() < s.size() )
            return false;
        it1=_S.begin();
        it2=s._S.begin();
        while( it2 != s._S.end() )
        {
            while( it1!=_S.end() && *it1<*it2 )
                it1++;
            if( it1==_S.end() || *it1 != *it2 )
                return false;
            it2++;
        }
        return true;
    }

    int hashVal()
    {
        SIt it;
        int ret = 0;
        for(it=_S.begin(); it!=_S.end(); it++)
            ret = (ret + *it) % 32768;
        return ret;
    }

    int sum()
    {
        SIt it;
        int ret = 0;
        for(it=_S.begin(); it!=_S.end(); it++)
            ret += *it;
        return ret;
    }

	void show()
	{
		SIt sit;
		cout<<'{';
		for(sit = _S.begin(); sit != _S.end(); sit++)
		{
			if(*sit>0)
				cout<<*sit;
			else if(*sit<0)
				cout<<'-'<<(*sit*(-1));
			else
				cout<<"item error!!(item = 0)"<<endl;
			if((sit+1)!= _S.end())
			{
				cout<<'&';
			}
		}
		cout<<'}';
	}

    void show( char *seperation, bool letter )
    {
        SIt it;
        for(it=_S.begin(); it!=_S.end(); it++)
        {
            if(it != _S.begin())
                cout << seperation;
            if( letter )
                cout << char(*it+'A');
            else
                cout << *it;
        }
    }

    void show(char *seperation, bool letter ) const
    {
        CSIt cit;
        for(cit=_S.begin(); cit!=_S.end(); cit++)
        {
            if(cit != _S.begin())
                cout << seperation;
            if( letter )
                cout << char(*cit+'A');
            else
                cout << *cit;
        }
    }
};

// get the tset for a single item
void get_item_tset(const int item, const Tset* data,const Tset* complement, Tset& T)
{
	if(item > 0)
	{
		T._S = data[item]._S;
	}
	else if(item<0)
	{
		T._S= complement[-item]._S;
	}
	else
		cout<<"get_item_tset(), item error!!(item = 0)"<<endl;
}

// get the tset for a single item
Tset* get_item_tset(const int item, const Tset* data, const Tset* complement)
{
	Tset* T;

	if(item > 0)
	{
		T = const_cast<Tset*>(data+item);
	}
	else if(item<0)
	{
		T = const_cast<Tset*>(complement+(-item));
	}
	else
	{
		cout<<"get_item_tset(), item error!!(item = 0)"<<endl;
		exit(0);
	}

	return T;
}

#endif



