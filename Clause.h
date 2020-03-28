#ifndef _ITPAIR_H
#define _ITPAIR_H

#include "Set.h"

extern int dmin_sup;
extern int dmax_sup;
extern int cmin_sup;
extern int cmax_sup;

class Clause
{
public:
    Iset    _iS;
    Tset    _tS;
    bool    _is_tset_update;    // is tset update or not

public:
    //constructor
    Clause()
	{
		_is_tset_update = false;
	}

    Clause(int item, const Tset* data, const Tset* complement)
    {
        _iS._S.push_back(item);
		get_item_tset(item, data, complement, _tS);
        _is_tset_update = true;
    }

    //copy constructor
    Clause(const Clause& cl)
    {
        _iS = cl._iS;
        _tS = cl._tS;
        _is_tset_update = cl._is_tset_update;
    }

    void clear()
    {
        _iS.clear();
        _tS.clear();
        _is_tset_update = false;
    }

    int getSum()
    {
        int clause_sum = 0;

        CSIt csit;
        for(csit=_iS._S.begin(); csit!=_iS._S.end(); csit++)
        {
            clause_sum += *csit;
        }

        return clause_sum;
    }

    bool operator==(const Clause &c) const
    {
        CSIt csit1, csit2;
        if(_iS._S.size() != c._iS._S.size())
            return false;

        for(csit1=_iS._S.begin(), csit2=c._iS._S.begin(); (csit1!=_iS._S.end() && csit2!=c._iS._S.end());
                csit1++, csit2++)
        {
            if(*csit1 != *csit2)  return false;
        }
        return true;
    }

    bool operator!=(const Clause &c) const
    {
        CSIt csit1, csit2;
        if(_iS._S.size() != c._iS._S.size())
            return true;

        for(csit1=_iS._S.begin(), csit2=c._iS._S.begin(); (csit1!=_iS._S.end() && csit2!=c._iS._S.end() );
                csit1++, csit2++)
        {
            if(*csit1 != *csit2)  return true;
        }
        return false;
    }

    bool operator<(const Clause &c) const
    {
        CSIt csit1, csit2;
        csit1 = _iS._S.begin();
        csit2 = c._iS._S.begin();

        while ( (csit1 != _iS._S.end()) && (csit2 != c._iS._S.end()) )
        {
            if ( *csit1 == *csit2 )
            {
                csit1++;
                csit2++;
            }
            else if ( *csit1 < *csit2 )
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        //In C++, your "compare" predicate must be a strict weak ordering.
        //In particular, "compare(X,X)" must return "false" for any X.
        if ( (csit1 == _iS._S.end()) && (csit2 == c._iS._S.end()) )
            return false;

        if ( csit1 == _iS._S.end() )
            return true;
        else
            return false;
    }

	// sort items in a clause
	void sort_clause()
	{
		_iS.sort_set();
	}

	// update tidset by recomputation (used when delete an item from the clause)
    void update_tset_delete(const Tset* data, const Tset* complement)
    {
		if (_iS._S.empty()) // if after deletion, the _iS becomes empty
		{
			_tS._S.clear();
			_is_tset_update = true;
		}
		else			    // if not empty
		{
			_iS.getTset(data, complement, _tS);
			_is_tset_update = true;
		}
    }

	// update tidset by intersecion (used when adding an item into the clause)
	void update_tset_add(int item, const Tset* data, const Tset* complement)
	{
		if (_tS._S.empty())  //when adding an item to an empty clause
		{
			get_item_tset(item,data, complement, _tS);
		}
		else
		{
			Tset	local_ts;
			Tset*   plocal_ts = get_item_tset(item,data,complement);
			// data[0] is void!
			Intersect( plocal_ts->_S, _tS._S, local_ts._S );
			_tS._S = local_ts._S;
		}

		_is_tset_update = true;
	}

    bool insert_an_item(int item, const Tset* data, const Tset* complement)
    {
        SIt  sit;
        for(sit = _iS._S.begin(); sit != _iS._S.end(); sit++ )
        {
            //avoid adding both positive and negative of the item
			if( (*sit == item) || (( *sit + item)==0) )
                return false;
        }

		// insert the item and update the tidset
        _iS.insert(item);
		// update the clause tset
		update_tset_add(item, data, complement);

        // check the support
		if ( (_tS._S.size() < cmin_sup) || (_tS._S.size() > cmax_sup ) )
		{
            return false;
		}
		else
		{
			return true;
		}
    }

	bool delete_an_item(int item_pos, const Tset* data, const Tset* complement)
	{
		// item_pos starts from 0
	    SIt sit;
		for(sit = _iS._S.begin(); sit != _iS._S.end(); sit++)
		{
			if( (sit-_iS._S.begin()) != item_pos )
				continue;

			_iS._S.erase(sit);
			break;
		}
		//update the clause tset
		update_tset_delete(data, complement);

        //check the support
		if ( (_tS._S.size() < cmin_sup) || (_tS._S.size() > cmax_sup ) )
            return false;
        else
            return true;
	}

    bool is_MG(const Tset* data, const Tset* complement)
    {
		if(_iS.size() == 1)
		{
			// single item is always the minimal generator
			return true;
		}

        SIt sit1, sit2;

		// delete items one by one
        for(sit1 = _iS._S.begin(); sit1 != _iS._S.end(); sit1++)
        {
            Tset TS[2];
            int idx=0;

			sit2 = _iS._S.begin();
			if (sit1 == sit2)
				sit2++;

			get_item_tset(*sit2, data,complement, TS[idx]);
			Tset* plocal_ts;
            for(; sit2 != _iS._S.end(); sit2++)
            {
                // ignore the current item
                if(sit1 == sit2)
                    continue;

				plocal_ts = get_item_tset(*sit2, data, complement);
				Intersect(plocal_ts->_S, TS[idx]._S, TS[1-idx]._S);

                idx = 1-idx;
            }

            if (_tS == TS[idx])
            {
                return false;
            }
        }
        return true;
    }

    void show() const
    {
        const_cast<Iset&>(_iS).show();
    }

	void show()
	{
		_iS.show();
	}
};

#endif
