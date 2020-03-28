#ifndef _DNF_H
#define _DNF_H

#include "Set.h"
#include "Clause.h"
#include <algorithm>

extern int minoverlap;

// sort clauses
bool sort_clauses_criterion_backup(const Clause &cl1, const Clause &cl2)
{
    CSIt csit1, csit2;
	csit1 = cl1._iS._S.begin();
	csit2 = cl2._iS._S.begin();

	while ( (csit1 != cl1._iS._S.end()) && (csit2 != cl2._iS._S.end()) )
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
    if ( (csit1 == cl1._iS._S.end())&&(csit2 == cl2._iS._S.end()) )
        return false;

	if ( csit1 == cl1._iS._S.end() )
		return true;
	else //if ( sit2 == cl2._iS._S.end() )
		return false;
}

// sort clauses
bool sort_clauses_criterion(const Clause &cl1, const Clause &cl2)
{
    return cl1 < cl2;
}

typedef vector<Clause>  	    	Clauses;
typedef vector<Clause>::iterator	CLSIt;
typedef vector<Clause>::const_iterator	CCLSIt;

class DNF
{
public:
    Clauses	_cls;
    Tset    _cls_ts;              // the union of all tidset of the clauses

    bool    _is_ctset_update;
    bool    _is_min_gen;

    bool    _is_prop_a;           // true if satisfied
    int     prop_a_clause_id;
    bool    _is_prop_b;           // true if satisfied
    int     prop_b_clause_id;



public:
    DNF(){}

    DNF(int item, const Tset* data, const Tset* complement)
    {
        Clause cl(item, data, complement);
        _cls.push_back(cl);

        _is_ctset_update = false;
		_is_min_gen = false;

		_is_prop_a = false;
		prop_a_clause_id = -1;
		_is_prop_b = false;
		prop_b_clause_id = -1;

    }

    DNF(const DNF& d)
    {
        _cls = d._cls;
        _cls_ts = d._cls_ts;
        _is_ctset_update = d._is_ctset_update;
		_is_min_gen = d._is_min_gen;

		_is_prop_a = d._is_prop_a;
		prop_a_clause_id = d.prop_a_clause_id;
		_is_prop_b = d._is_prop_b;
		prop_b_clause_id = d.prop_b_clause_id;

    }

    void clear()
    {
        _cls.clear();
        _cls_ts._S.clear();
        _is_ctset_update = false;
		_is_min_gen = false;

        _is_prop_a = false;
		prop_a_clause_id = -1;
		_is_prop_b = false;
		prop_b_clause_id = -1;

    }

    bool operator==(const DNF &d ) const
    {
		CCLSIt cclsit1, cclsit2;
		if(_cls.size() != d._cls.size())
			return false;
		for(cclsit1=_cls.begin(), cclsit2=d._cls.begin(); (cclsit1!=_cls.end() && cclsit2!=d._cls.end() );
            cclsit1++, cclsit2++)
        {
            if(*cclsit1 != *cclsit2)  return false;
        }
		return true;
	}

	bool operator!=(const DNF &d ) const
	{
        CCLSIt cclsit1, cclsit2;
		if(_cls.size() != d._cls.size())
			return true;
		for(cclsit1=_cls.begin(), cclsit2=d._cls.begin(); (cclsit1!=_cls.end() && cclsit2!=d._cls.end() );
            cclsit1++, cclsit2++)
        {
            if(*cclsit1 != *cclsit2)  return true;
        }
		return false;
	}

	bool operator<(const DNF &d ) const
	{
	    CCLSIt cclsit1, cclsit2;
	    cclsit1 = _cls.begin();
	    cclsit2 = d._cls.begin();

	    while( (cclsit1 != _cls.end()) && (cclsit2 != d._cls.end()) )
        {
            if( *cclsit1 == *cclsit2 )
            {
                cclsit1++;
                cclsit2++;
            }
            else if ( *cclsit1 < *cclsit2 )
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
        if ( (cclsit1 == _cls.end()) && (cclsit2 == d._cls.end()) )
            return false;

        if ( cclsit1 == _cls.end() )
            return true;
        else
            return false;
	}

    int size()
    {
        return _cls.size();
    }

    void insert( Clause &it )
    {
        _cls.push_back( it );
		_is_ctset_update = false;
		_is_min_gen = false;
    }

    // add an item as an individual clause
    bool add_item_as_clause(int item, const Tset* data, const Tset* complement)
    {
        SIt  sit;
        CLSIt clsit;

        // avoid insert the item(which is a clause) which is already contained in other clauses
        clsit = _cls.begin();
        while(clsit != _cls.end())
        {
            // to avoid like "~B|B"
            if ( clsit->_iS.size() == 1 && (*clsit->_iS._S.begin()+item)==0 )
                return false;

            // since the itemset is sorted...
            if (*clsit->_iS._S.begin() > item)
                break;

			// to avoid the added clause is a subset of some other clause
            for(sit = clsit->_iS._S.begin(); sit != clsit->_iS._S.end(); sit++)
            {
                // since the itemset is sorted...
                if (*sit > item)
                    break;

                if(*sit == item)
                    return false;
            }
            clsit++;
        }

        Clause cl;
		if(cl.insert_an_item(item, data, complement))
		{
            // added by Geng: Aug 4th
            if(minoverlap > 0)
            {
                Tset* plocal_ts = get_item_tset(item,data,complement);
                for(clsit = _cls.begin(); clsit != _cls.end(); clsit++)
                {
                    Tset local_ts;
                    Intersect(plocal_ts->_S, clsit->_tS._S, local_ts._S);
                    if(local_ts._S.size() < minoverlap )
                        return false;
                }
            }
            // end added

			_cls.push_back(cl);
			// update clauses' tset
			if (updateClausesTset(item, data, complement))
                return true;
            else
                return false;
		}
		else
			return false;
    }

	// add an item to an existing clause
    bool add_item_to_clause(int item, int clause_pos, const Tset* data, const Tset* complement)
    {
		// the position of the clause
        int cnt = -1;
        CLSIt clsit;

        clsit = _cls.begin();
        while(clsit != _cls.end())
        {
            cnt++;
            if(cnt != clause_pos)
            {
                clsit++;
                continue;
            }
            else
            {
				if( clsit->insert_an_item(item, data, complement) )
				{
                    // added by Geng: Aug 4th
                    if(minoverlap>0)
                    {
                        CLSIt clsit_tmp;
                        for(clsit_tmp = _cls.begin(); clsit_tmp != _cls.end(); clsit_tmp++)
                        {
                            if(clsit_tmp == clsit)
                                continue;
                            Tset local_ts;
                            Intersect(clsit_tmp->_tS._S, clsit->_tS._S, local_ts._S);
                            if(local_ts._S.size() < minoverlap )
                                return false;
                        }
                    }
                    // end added

					// update clauses' tset
					if(getClausesTset())
					{
					    // sort items in clause
					    clsit->sort_clause();
					    return true;
					}
                    else
                        return false;
				}
				else
				{
					// if insertion unsuccessfully
					return false;
				}
            }
        }
		// wrong clause pos!!
		cout<< "add_item_to_clause(), wrong clause pos"<<endl;
        return false;
    }

	// delete a clause which is a single item
    bool delete_item_as_clause(int clause_pos)
    {
		// the position of the clause
        int cnt = -1;

        CLSIt clsit;

        clsit = _cls.begin();
        while(clsit != _cls.end())
        {
            cnt++;
            if(cnt != clause_pos)
            {
                clsit++;
                continue;
            }
            else
            {
                if(clsit->_iS.size() != 1)
                    return false;

                _cls.erase(clsit);
				// update clauses' tset
				if (getClausesTset())
                    return true;
                else
                    return false;
            }
        }
        return false;
    }

	// delete an item in an existing clause
    bool delete_item_in_clause(int clause_pos, int item_pos, const Tset* data, const Tset* complement)
    {
        int cnt = -1;

        CLSIt clsit;

        clsit = _cls.begin();
        while(clsit != _cls.end())
        {
            cnt++;
            if(cnt != clause_pos)
			{
				clsit++;
                continue;
			}
			if(clsit->delete_an_item(item_pos, data, complement))
			{
				// update clauses' tset
				if(getClausesTset())
					return true;
				else
                    return false;
			}
			return false;
        }
    }

    bool is_min_gen_DNF(const Tset* data,const Tset* complement)
    {
		// needs to satisfy property (a)
		if ( !is_subset_tset() )
		{
		    _is_prop_a = true;
		}
		else
		{
		    _is_prop_a = false;
		}

		// needs to satisfy property (b)
		if ( is_simplest_DNF(data, complement) )
		{
		    _is_prop_b = true;
		}
		else
		{
		    _is_prop_b = false;
		}

		// determine the minimality
		if ( _is_prop_a && _is_prop_b )
		{
		    _is_min_gen = true;
		    return true;
		}
		else
		{
		    _is_min_gen = false;
            return false;
		}
    }

    // is qualified to be kept in the lattice network:
    // that's: violate property (a) or (b)
    // needs to satisfy Lemma (2) and (3)
    bool is_qualified(const Tset* data,const Tset* complement)
    {
        if ( is_all_MGs(data, complement) && !is_subset_clause() )
        {
			//qualified to be remained in lattice network
            return true;
        }
        else
        {
            //_is_min_gen = false;
            return false;
        }
    }

    // property 1
    bool is_subset_clause_backup()     // not sorted
    {
        CLSIt clsit1, clsit2;
        for(clsit1=_cls.begin(); clsit1!=_cls.end(); clsit1++)
        {
            for(clsit2=_cls.begin(); clsit2!=_cls.end(); clsit2++)
            {
				if (clsit1 == clsit2)
				{
					continue;
				}

                if( clsit2->_iS.contain(clsit1->_iS) )
                    return true;
            }
        }
        return false;
    }

    // Lemma 3
    bool is_subset_clause()
    {
        CLSIt clsit1, clsit2;
        for(clsit1=_cls.begin(); clsit1!=_cls.end(); clsit1++)
        {
            for(clsit2=_cls.begin(); clsit2!=_cls.end(); clsit2++)
            {
                if(clsit1 == clsit2)
                {
                    continue;
                }

                if( clsit1->_iS.contain(clsit2->_iS) )
                    return true;
            }
        }
        return false;
    }

    // Property (a)
    bool is_subset_tset()
    {
        CLSIt clsit1, clsit2;

        for(clsit1 = _cls.begin(); clsit1 != _cls.end(); clsit1++)
        {
            Tset TS[2];
            int idx = 0;
            for(clsit2 = _cls.begin(); clsit2 != _cls.end(); clsit2++)
            {
                if(clsit1 == clsit2)
                    continue;

                Union( (*clsit2)._tS._S, TS[idx]._S, TS[1-idx]._S );
                idx = 1-idx;
            }

            if ( TS[idx].contain((*clsit1)._tS) )
            {
                prop_a_clause_id = clsit1 - _cls.begin();
                return true;
            }
        }
        return false;
    }

    // Lemma 2
    bool is_all_MGs(const Tset* data, const Tset* complement)
    {
        CLSIt clsit;
        SIt sit;
        for(clsit = _cls.begin(); clsit != _cls.end(); clsit++)
        {
            if( ! clsit->is_MG(data, complement) )
                return false;
        }
        return true;
    }

    // Property (b)
    bool is_simplest_DNF(const Tset* data,const Tset* complement)
    {
		// if the DNF is a single clause with a single item
		if(_cls.size() == 1 && _cls[0]._iS.size() == 1)
			return true;

		CLSIt clsit1, clsit2;
		SIt sit1, sit2;

        for(clsit1 = _cls.begin(); clsit1 != _cls.end(); clsit1++)
        {
            Tset TS_cls[2];
            int idx = 0;

			//(1). get the union of tidsets of all clauses except the current clause
            for(clsit2 = _cls.begin(); clsit2 != _cls.end(); clsit2++)
            {
                if(clsit1 == clsit2)
                    continue;

				if((*clsit2)._is_tset_update)
				{
					Union( (*clsit2)._tS._S, TS_cls[idx]._S, TS_cls[1-idx]._S );
					idx = 1 - idx;
				}
				else
				{
					cout<<"clause's tidset is not updated!!"<<endl;
				}
            }

			//(2). get the tidset of the current clause by deleting one item
            for( sit1 = clsit1->_iS._S.begin(); sit1 != clsit1->_iS._S.end(); sit1++ )
            {
                Tset TS_it[2];
                int idy = 0;
				Tset TS_all;

				sit2 = clsit1->_iS._S.begin();
				if(sit1 == sit2)
				{
					sit2++;

					// if the current clause is a single item to be deleted, we only need to compare tidset in (1)
					if(sit2 == clsit1->_iS._S.end())
					{
						if (TS_cls[idx] == _cls_ts)
							return false;
						else
							continue;
					}
				}

				get_item_tset(*sit2, data, complement, TS_it[idy]);
				Tset* plocal_ts;
                for( ; sit2 != clsit1->_iS._S.end(); sit2++ )
                {
                    if(sit1 == sit2)
                        continue;

					plocal_ts = get_item_tset(*sit2,data,complement);
					Intersect(plocal_ts->_S, TS_it[idy]._S, TS_it[1-idy]._S);

                    idy = 1-idy;
                }

				// take the union of step (1) and (2)
                Union(TS_cls[idx]._S, TS_it[idy]._S, TS_all._S);

                if(TS_all == _cls_ts)
                {
                    prop_b_clause_id = clsit1 - _cls.begin();
                    return false;
                }
            }
        }

        return true;
    }

    // update the tset of clauses, when adding a new item as a clause
	bool updateClausesTset(int item, const Tset* data, const Tset* complement)
	{
		Tset local_ts;
		Tset* plocal_ts = get_item_tset(item,data,complement);

        // check the support of the new item (which is a clause): Already done when insert (add_item_as_clause)
		//if( (plocal_ts->_S.size() < cmin_sup) || (plocal_ts->_S.size() > cmax_sup) )
        //    return false;

		Union(plocal_ts->_S, _cls_ts._S, local_ts._S);
        if ( (local_ts._S.size() < dmin_sup) || (local_ts._S.size() > dmax_sup) )
            return false;

		_cls_ts = local_ts;
		return true;
	}

    // get the tset of clauses: take the union of all tidsets from all clauses
    bool getClausesTset()
    {
        CLSIt clsit;

        Tset TS_cls[2];
        int idx = 0;
        for(clsit = _cls.begin(); clsit != _cls.end(); clsit++)
        {
			if (!(*clsit)._is_tset_update)
			{
				cout<<"Cannot getClausesTset()!"<<endl;
				return false;
			}
            Union( (*clsit)._tS._S, TS_cls[idx]._S, TS_cls[1-idx]._S );
            idx = 1-idx;
        }

        if ( (TS_cls[idx]._S.size() < dmin_sup) || (TS_cls[idx]._S.size() > dmax_sup) )
            return false;

        _cls_ts = TS_cls[idx];
		_is_ctset_update = true;
		return true;
    }

    int getSupport()
    {
        return _cls_ts._S.size();
    }

    int getLength()
    {
        int dnf_length = 0;
        CLSIt clsit;
        for(clsit = _cls.begin(); clsit != _cls.end(); clsit++)
        {
            dnf_length += (*clsit)._iS._S.size();
        }
        return dnf_length;
    }

    int getSum()
    {
        int dnf_sum = 0;
        CLSIt clsit;
        for(clsit = _cls.begin(); clsit != _cls.end(); clsit++)
        {
            dnf_sum += (*clsit).getSum();
        }
        return dnf_sum;
    }

	// sort clauses in a DNF in clause-wise manner
	void sort_DNF()
	{
		sort(_cls.begin(),_cls.end(), sort_clauses_criterion);
	}

    void show()
    {
		// in case it is an empty set
		if(_cls.empty())
		{
			cout<<"empty set!"<<endl;
			return;
		}

        // if the tidset is not very long, then print out
        //if( _cls_ts._S.size() <= 1000 )
        //{
            SIt sit;
            for(sit = _cls_ts._S.begin(); sit != _cls_ts._S.end(); sit++)
            {
                cout<<*sit<<' ';
            }
            cout<<": ";
        //}

        CLSIt clsit;
        for(clsit = _cls.begin(); (clsit+1) != _cls.end(); clsit++)
        {
			clsit->show();
			cout<<"|";
        }
		clsit->show();


		cout<<" (support is: " << _cls_ts._S.size() <<")"<<endl;
    }
};

#endif
