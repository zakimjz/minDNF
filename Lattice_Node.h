#ifndef _LATTICE_NODE_H
#define _LATTICE_NODE_H

#include <numeric>
#include <assert.h>
#include <set>
#include "DNF.h"

class Lattice_Node;

typedef vector<Lattice_Node> Family;
typedef vector<Lattice_Node>::iterator	FAMIt;

typedef vector<double> DoubleArr;
typedef vector<double>::iterator DIt;

typedef vector<int> IntArr;
typedef vector<int>::iterator IIt;

class Lattice_Node
{
public:
    DNF _ln_DNF;
    Family _ln_parents;
    Family _ln_children;

    int _ln_degree;
    int _ln_nparen;         // number of parents
    int _ln_nchild;         // number of children
    DoubleArr _edge_weights;
    DoubleArr _trans_accuprob;  // store accumulated transition probabilites
    DoubleArr _trans_prob;      // store transition probabilites

    int     _num_neigh_min;
    int     _num_neigh_nonmin;

public:

    Lattice_Node()
    {
        _ln_degree = 0;
        _ln_nparen = 0;
        _ln_nchild = 0;

        _num_neigh_min = 0;
        _num_neigh_nonmin = 0;
    }

    Lattice_Node(int item, const Tset* data,const Tset* complement)
    {
        DNF dnf(item, data, complement);
        _ln_DNF = dnf;
        // node degree is set to 0, which means an isolated node initially
        _ln_degree = 0;
        _ln_nparen = 0;
        _ln_nchild = 0;

        _num_neigh_min = 0;
        _num_neigh_nonmin = 0;
    }

    //copy constructor
    Lattice_Node(const Lattice_Node& ln)
    {
        _ln_DNF = ln._ln_DNF;
        _ln_parents = ln._ln_parents;
        _ln_children = ln._ln_children;

        _ln_degree = ln._ln_degree;
        _ln_nparen = ln._ln_nparen;
        _ln_nchild = ln._ln_nchild;

        _edge_weights = ln._edge_weights;
        _trans_accuprob = ln._trans_accuprob;
        _trans_prob = ln._trans_prob;

        _num_neigh_min = ln._num_neigh_min;
        _num_neigh_nonmin = ln._num_neigh_nonmin;
    }

    Lattice_Node(const DNF& DNF_node)
    {
        _ln_DNF = DNF_node;
        // an isolated node at first
        _ln_degree = 0;
        _ln_nparen = 0;
        _ln_nchild = 0;

        _num_neigh_min = 0;
        _num_neigh_nonmin = 0;
    }

    void clear()
    {
        _ln_DNF.clear();
        _ln_parents.clear();
        _ln_children.clear();
    }

    void clear_fam()
    {
        _ln_parents.clear();
        _ln_children.clear();
    }

    int get_num_parent()
    {
        return _ln_parents.size();
    }

    int get_num_children()
    {
        return _ln_children.size();
    }

    Lattice_Node& sel_next_ln(unsigned int idx)
    {
        int nparen = get_num_parent();
        int nchild = get_num_children();

        FAMIt paren_lnit = _ln_parents.begin();
        FAMIt child_lnit = _ln_children.begin();

        if(idx > (nparen+nchild))
        {
            cout<<"index error!"<<endl;
            exit(0);
        }

        idx--;  // since array starts from 0
        if(idx < nparen)
            return *(paren_lnit+idx);
        else if(idx< (nparen+nchild) )
            return *(child_lnit+(idx-nparen));
    }

    int get_num_clauses()
    {
        return _ln_DNF._cls.size();
    }

    void generate_candidate_items_to_add(const int item, const CachedTable& freq_two_items, Iset* freq_item_cand, int& idx)
    {
        Iset tempISet;
        CachedFind p = freq_two_items.equal_range(item);
        for (CTIt i = p.first; i != p.second; ++i)
        {
            tempISet.insert((*i).second);
        }

        if (tempISet._S.empty())
            return;

        // before the intersection, we need to sort iset
        tempISet.sort_set();

        // take the intersection, since the qualified item candidates of the clause should be the intersection of each
        // item's qualified items
        if( freq_item_cand[idx]._S.empty() )
        {
            freq_item_cand[idx] = tempISet;
            freq_item_cand[1-idx] = tempISet;
            idx = 1-idx;
        }
        else
        {
            Intersect(freq_item_cand[idx]._S, tempISet._S, freq_item_cand[1-idx]._S);
            idx = 1-idx;
        }
    }

    // get all possible parents(add an item)
    // getNum: true--get the number only
    void get_ln_parent_candidates(const int itemNum, const Tset* data, const Tset* complement,
                                  const CachedTable& freq_two_items_and, const CachedTable& freq_two_items_or,
                                  bool fNegative, bool fOutput, bool* algorithm, bool bGetNum)
    {
        _ln_nparen = 0;
        _ln_parents.clear();

        CLSIt clsit;

        if (algorithm[1])                         // minimal DNF generator
        {
            //----------------------------------- (1) add a single item as a clause
            if(fOutput)
                cout<< "Start to add a single item as a clause" <<endl;

            bool found_one_item_as_clause = false;
            Iset freq_item_cand_or[2];
            int idx_or = 0;

            for(clsit = _ln_DNF._cls.begin(); clsit != _ln_DNF._cls.end(); clsit++)
            {
                if( clsit->_iS._S.size() == 1 )
                {
                    found_one_item_as_clause = true; // found!
                    generate_candidate_items_to_add(*clsit->_iS._S.begin(), freq_two_items_or, freq_item_cand_or, idx_or);

                    if (freq_item_cand_or[idx_or]._S.empty())
                        break;
                }
            }

            if (!found_one_item_as_clause)        // cannot use OR cache, since no single item which is a clause exists
            {
                for(int i=1; i < (itemNum+1); i++)
                {
                    //cout<<"processing item idx (add a single item as a clause): "<<i<<endl;
                    int litem = i*(-1);
                    int niter = fNegative? 2:1;
                    //support both positive and negative
                    for (int iter=0; iter<niter; iter++)
                    {
                        litem = litem*(-1);

                        // check the support of the single item which will be a clause
                        Tset* lts = get_item_tset(litem, data, complement);
                        if ( lts->_S.size() < cmin_sup || lts->_S.size() > cmax_sup )
                            continue;

                        Lattice_Node local_ln(_ln_DNF);
                        if( local_ln._ln_DNF.add_item_as_clause(litem, data, complement) )
                        {
                            local_ln._ln_DNF.sort_DNF();
                            if(local_ln._ln_DNF.is_qualified(data, complement))
                            {
                                if (local_ln._ln_DNF.is_min_gen_DNF(data, complement))
                                    _num_neigh_min++;
                                else
                                    _num_neigh_nonmin++;

                                // get the number only
                                if( bGetNum )
                                    _ln_nparen++;
                                else
                                    _ln_parents.push_back(local_ln);
                            }
                        }
                    }
                }
            }
            else if (freq_item_cand_or[idx_or]._S.empty())   // we cannot add a single item which is a clause
            {
                if(fOutput)
                    cout<<"No single item could be added as a clause!!"<<endl;
            }
            else
            {
                // add an item as a clause from candidates
                SIt freq_item_it;
                for( freq_item_it = freq_item_cand_or[idx_or]._S.begin();
                freq_item_it != freq_item_cand_or[idx_or]._S.end(); freq_item_it++ )
                {
                    Lattice_Node local_ln(_ln_DNF);
                    if( local_ln._ln_DNF.add_item_as_clause(*freq_item_it, data, complement) )
                    {
                        local_ln._ln_DNF.sort_DNF();
                        if(local_ln._ln_DNF.is_qualified(data, complement))
                        {
                            if (local_ln._ln_DNF.is_min_gen_DNF(data, complement))
                                _num_neigh_min++;
                            else
                                _num_neigh_nonmin++;

                            // get the number only
                            if( bGetNum )
                                _ln_nparen++;
                            else
                                _ln_parents.push_back(local_ln);
                        }
                    }
                }
            }
            if(fOutput)
            {
                cout<< "Finish adding a single item as a clause" <<endl;
                /*vector<Lattice_Node>::iterator laiter;
                for( laiter = _ln_parents.begin(); laiter != _ln_parents.end(); laiter++)
                {
                    laiter->_ln_DNF.show();
                }*/
            }
        }

        //--------------------------------------------------------------------
        //---------------------------------- (2) add an item to an existing clause
        if(fOutput)
            cout<< "Start to add an item to an existing clause" <<endl;
        SIt sit;
        // for each clause, try to add qualified items to the clause
        for (clsit = _ln_DNF._cls.begin(); clsit != _ln_DNF._cls.end(); clsit++)
        {
            Iset freq_item_cand_and[2];
            int idx_and = 0;

            for( sit = clsit->_iS._S.begin(); sit != clsit->_iS._S.end();  sit++ )
            {
                generate_candidate_items_to_add(*sit, freq_two_items_and, freq_item_cand_and, idx_and);
                if (freq_item_cand_and[idx_and]._S.empty())
                    break;
            }

            // find candidates, if it is empty, we need to break the current loop.
            if (freq_item_cand_and[idx_and]._S.empty())
            {
                continue;
            }

            // add an item to an existing clause
            SIt freq_item_it;
            for( freq_item_it = freq_item_cand_and[idx_and]._S.begin();
            freq_item_it != freq_item_cand_and[idx_and]._S.end(); freq_item_it++ )
            {
                Lattice_Node local_ln(_ln_DNF);
                if( local_ln._ln_DNF.add_item_to_clause(*freq_item_it, (clsit-_ln_DNF._cls.begin()), data, complement) )
                {
                    local_ln._ln_DNF.sort_DNF();

                    if (algorithm[0])                        // minimal AND clause
                    {
                        if(local_ln._ln_DNF.is_all_MGs(data, complement))
                        {
                            // get the number only
                            if( bGetNum )
                                _ln_nparen++;
                            else
                                _ln_parents.push_back(local_ln);
                        }
                    }

                    if (algorithm[1])                         // minimal DNF generator
                    {
                        if(local_ln._ln_DNF.is_qualified(data, complement))
                        {
                            if (local_ln._ln_DNF.is_min_gen_DNF(data, complement))
                                _num_neigh_min++;
                            else
                                _num_neigh_nonmin++;

                            // get the number only
                            if( bGetNum )
                                _ln_nparen++;
                            else
                            {
                                if(!_ln_DNF._is_min_gen)  //optimization, for the current node
                                {
                                    if(!_ln_DNF._is_prop_a)       //if property (a) not satistified
                                    {
                                        if(_ln_DNF.prop_a_clause_id == (clsit-_ln_DNF._cls.begin()))
                                        {
                                            local_ln._ln_DNF._is_min_gen = false;
                                        }
                                    }

                                    if(!_ln_DNF._is_prop_b)       //if property (b) not satistified
                                    {
                                        if(_ln_DNF.prop_b_clause_id == (clsit-_ln_DNF._cls.begin()))
                                        {
                                            local_ln._ln_DNF._is_min_gen = false;
                                        }
                                    }
                                }
                                _ln_parents.push_back(local_ln);
                            }
                        }
                    }
                }
            }
        }
        if(fOutput)
        {
            cout<< "Finish adding an item to an existing clause" <<endl;
            cout<< "The parents are:"<<endl;
            vector<Lattice_Node>::iterator laiter;
            for( laiter = _ln_parents.begin(); laiter != _ln_parents.end(); laiter++)
            {
                laiter->_ln_DNF.show();
            }
        }
    }

    // get all possible children(delete an item)
    // getNum: true--get the number only
    void get_ln_children_candidates(const int itemNum, const Tset* data, const Tset* complement,
                                    bool fOutput, bool* algorithm, bool bGetNum)
    {
        _ln_nchild = 0;
        _ln_children.clear();

        CLSIt clsit;

        if(fOutput)
            cout<< "Start deleting an item" <<endl;

        for(clsit = _ln_DNF._cls.begin(); clsit != _ln_DNF._cls.end(); clsit++)
        {
            Lattice_Node local_ln(_ln_DNF);
            //delete a single item which is a clause
            if( local_ln._ln_DNF.delete_item_as_clause(clsit - _ln_DNF._cls.begin()) )
            {
                local_ln._ln_DNF.sort_DNF();
                if (algorithm[0])                        // minimal AND clause
                {
                    if(local_ln._ln_DNF.is_all_MGs(data, complement))
                    {
                        if (bGetNum)
                            _ln_nchild++;
                        else
                            _ln_children.push_back(local_ln);
                    }
                }

                if (algorithm[1])                         // minimal DNF generator
                {
                    if(local_ln._ln_DNF.is_qualified(data, complement))
                    {
                        if (local_ln._ln_DNF.is_min_gen_DNF(data, complement))
                            _num_neigh_min++;
                        else
                            _num_neigh_nonmin++;

                        if (bGetNum)
                            _ln_nchild++;
                        else
                            _ln_children.push_back(local_ln);
                    }
                }
            }
            else // the clause contains more than one item, the deletion is not successful
            {
                for(int i=0; i<clsit->_iS.size(); i++)
                {
                    Lattice_Node locallocal_ln(_ln_DNF);
                    if( locallocal_ln._ln_DNF.delete_item_in_clause((clsit-_ln_DNF._cls.begin()), i, data, complement) )
                    {
                        locallocal_ln._ln_DNF.sort_DNF();

                        if (algorithm[0])                         // minimal AND clause
                        {                                         // since all subset of a minimal AND-clause are minimal
                            if (bGetNum)
                                _ln_nchild++;
                            else
                                _ln_children.push_back(locallocal_ln);
                        }

                        if (algorithm[1])                         // minimal DNF generator
                        {
                            if(locallocal_ln._ln_DNF.is_qualified(data, complement))
                            {
                                if (locallocal_ln._ln_DNF.is_min_gen_DNF(data, complement))
                                    _num_neigh_min++;
                                else
                                    _num_neigh_nonmin++;

                                if (bGetNum)
                                    _ln_nchild++;
                                else
                                    _ln_children.push_back(locallocal_ln);
                            }
                        }
                    }
                }
            }
        }

        if(fOutput)
        {
            cout<< "Finish deleting an item" <<endl;
            cout<< "The children are:"<<endl;
            vector<Lattice_Node>::iterator laiter;
            for( laiter = _ln_children.begin(); laiter != _ln_children.end(); laiter++)
            {
                laiter->_ln_DNF.show();
            }
        }
    }

    //get node degree
    void get_ln_degree_by_func()
    {
        _ln_nparen = get_num_parent();
        _ln_nchild = get_num_children();
        //cout<<"_ln_nparen"<<_ln_nparen<<endl;
        //cout<<"_ln_nchild"<<_ln_nchild<<endl;
        _ln_degree =  _ln_nparen + _ln_nchild;
    }

    void get_ln_degree_by_val()
    {
        _ln_degree =  _ln_nparen + _ln_nchild;
    }

    void is_ln_DNF(const Tset* data,const Tset* complement)
    {
        _ln_DNF.is_min_gen_DNF(data, complement);
    }

    void determine_neighbor_degree_AND(const int itemNum, const Tset* data, const Tset* complement,
                                       const CachedTable& freq_two_items_and, const CachedTable& freq_two_items_or,
                                       bool fNegative, bool fOutput, bool* algorithm)
    {
        FAMIt paren_lnit;
        FAMIt child_lnit;

        for(paren_lnit = _ln_parents.begin(); paren_lnit != _ln_parents.end(); paren_lnit++)
        {
            paren_lnit->get_ln_parent_candidates(itemNum, data, complement,
                                                   freq_two_items_and, freq_two_items_or,
                                                   fNegative, fOutput, algorithm, true);
            paren_lnit->get_ln_children_candidates(itemNum, data, complement, fOutput, algorithm, true);
            paren_lnit->get_ln_degree_by_val();
        }

        for(child_lnit = _ln_children.begin(); child_lnit != _ln_children.end(); child_lnit++)
        {
            child_lnit->get_ln_parent_candidates(itemNum, data, complement,
                                                   freq_two_items_and, freq_two_items_or,
                                                   fNegative, fOutput, algorithm, true);
            child_lnit->get_ln_children_candidates(itemNum, data, complement, fOutput, algorithm, true);
            child_lnit->get_ln_degree_by_val();
        }
    }

    //test whether neighboring nodes' min-DNF-ability
    void determine_neighborDNF_minimality(const Tset* data,const Tset* complement)
    {
        _num_neigh_min = 0;
        _num_neigh_nonmin = 0;

        FAMIt paren_lnit;
        FAMIt child_lnit;

        for(paren_lnit = _ln_parents.begin(); paren_lnit != _ln_parents.end(); paren_lnit++)
        {
            paren_lnit->_ln_DNF.is_min_gen_DNF(data,complement);

            if (paren_lnit->_ln_DNF._is_min_gen)
                _num_neigh_min++;
            else
                _num_neigh_nonmin++;
        }

        for(child_lnit = _ln_children.begin(); child_lnit != _ln_children.end(); child_lnit++)
        {
            child_lnit->_ln_DNF.is_min_gen_DNF(data,complement);

            if (child_lnit->_ln_DNF._is_min_gen)
                _num_neigh_min++;
            else
                _num_neigh_nonmin++;
        }
    }

    void determine_neighbor_degree_DNF_2(const int itemNum, const Tset* data, const Tset* complement,
                                   const CachedTable& freq_two_items_and, const CachedTable& freq_two_items_or,
                                   bool fNegative, bool fOutput, bool* algorithm)
    {
        FAMIt paren_lnit;
        FAMIt child_lnit;

        int paren_cnt = 0;
        int child_cnt = 0;

        if(fOutput)
            cout<<"the number of parents are: "<<_ln_parents.size()<<endl;
        for(paren_lnit = _ln_parents.begin(); paren_lnit != _ln_parents.end(); paren_lnit++)
        {
            if(fOutput)
                cout<<"calc parent idx: "<<++paren_cnt<<endl;

            //only if the current node is NOT minimal generator DNF and its parent is minimal generator DNF,
            //we need to know the degree of the parent
            //if( (! _ln_DNF._is_min_gen) && (paren_lnit->_ln_DNF._is_min_gen) )

            if( paren_lnit->_ln_DNF._is_min_gen )
            {
                paren_lnit->_ln_degree = 0;
                paren_lnit->_ln_nparen = 0;
                paren_lnit->_ln_nchild = 0;
                paren_lnit->_num_neigh_min = 0;
                paren_lnit->_num_neigh_nonmin = 0;

                paren_lnit->get_ln_parent_candidates(itemNum, data, complement,
                        freq_two_items_and, freq_two_items_or, fNegative, fOutput, algorithm, true);
                paren_lnit->get_ln_children_candidates(itemNum, data, complement, fOutput, algorithm, true);

                paren_lnit->get_ln_degree_by_val();
            }
            else
            {
                paren_lnit->_ln_degree = -1;
                paren_lnit->_ln_nparen = -1;
                paren_lnit->_ln_nchild = -1;
                paren_lnit->_num_neigh_min = -1;
                paren_lnit->_num_neigh_nonmin = -1;
            }
        }

        if(fOutput)
            cout<<"the number of children are: "<<_ln_children.size()<<endl;
        for(child_lnit = _ln_children.begin(); child_lnit != _ln_children.end(); child_lnit++)
        {
            if(fOutput)
                cout<<"calc child idx: "<<++child_cnt<<endl;

            //only if the current node is NOT minimal generator DNF and its child is minimal generator DNF,
            //we need to know the degree of the child
            //if ( (! _ln_DNF._is_min_gen) && (child_lnit->_ln_DNF._is_min_gen) )

            if ( child_lnit->_ln_DNF._is_min_gen )
            {
                child_lnit->_ln_degree = 0;
                child_lnit->_ln_nparen = 0;
                child_lnit->_ln_nchild = 0;
                child_lnit->_num_neigh_min = 0;
                child_lnit->_num_neigh_nonmin = 0;

                child_lnit->get_ln_parent_candidates(itemNum, data, complement,
                        freq_two_items_and, freq_two_items_or, fNegative, fOutput, algorithm, true);
                child_lnit->get_ln_children_candidates(itemNum, data, complement, fOutput, algorithm, true);

                child_lnit->get_ln_degree_by_val();
            }
            else
            {
                child_lnit->_ln_degree = -1;
                child_lnit->_ln_nparen = -1;
                child_lnit->_ln_nchild = -1;
                child_lnit->_num_neigh_min = -1;
                child_lnit->_num_neigh_nonmin = -1;
            }
        }
    }

    void determine_neighbor_degree_DNF_3(const int itemNum, const Tset* data, const Tset* complement,
                                   const CachedTable& freq_two_items_and, const CachedTable& freq_two_items_or,
                                   bool fNegative, bool fOutput, bool* algorithm)
    {
        FAMIt paren_lnit;
        FAMIt child_lnit;

        int paren_cnt = 0;
        int child_cnt = 0;

        if(fOutput)
            cout<<"the number of parents are: "<<_ln_parents.size()<<endl;
        for(paren_lnit = _ln_parents.begin(); paren_lnit != _ln_parents.end(); paren_lnit++)
        {
            if(fOutput)
                cout<<"calc parent idx: "<<++paren_cnt<<endl;

            //only if the current node is NOT minimal generator DNF and its parent is minimal generator DNF,
            //we need to know the degree of the parent
            if( (! _ln_DNF._is_min_gen) && (paren_lnit->_ln_DNF._is_min_gen) )
            {
                paren_lnit->get_ln_parent_candidates(itemNum, data, complement,
                        freq_two_items_and, freq_two_items_or, fNegative, fOutput, algorithm, true);
                paren_lnit->get_ln_children_candidates(itemNum, data, complement, fOutput, algorithm, true);
                paren_lnit->get_ln_degree_by_val();
            }
            else
            {
                paren_lnit->_ln_degree = -1;
                paren_lnit->_ln_nparen = -1;
                paren_lnit->_ln_nchild = -1;
            }
        }

        if(fOutput)
            cout<<"the number of children are: "<<_ln_children.size()<<endl;
        for(child_lnit = _ln_children.begin(); child_lnit != _ln_children.end(); child_lnit++)
        {
            if(fOutput)
                cout<<"calc child idx: "<<++child_cnt<<endl;

            //only if the current node is NOT minimal generator DNF and its child is minimal generator DNF,
            //we need to know the degree of the child
            if ( (! _ln_DNF._is_min_gen) && (child_lnit->_ln_DNF._is_min_gen) )
            {
                child_lnit->get_ln_parent_candidates(itemNum, data, complement,
                        freq_two_items_and, freq_two_items_or, fNegative, fOutput, algorithm, true);
                child_lnit->get_ln_children_candidates(itemNum, data, complement, fOutput, algorithm, true);
                child_lnit->get_ln_degree_by_val();
            }
            else
            {
                child_lnit->_ln_degree = -1;
                child_lnit->_ln_nparen = -1;
                child_lnit->_ln_nchild = -1;
            }
        }
    }

    //calc edge weight
    void get_edge_weight_2(/*double aver_tlen, double max_tlen,*/ double a_val, double c_val)
    {
        _edge_weights.clear();

        FAMIt paren_lnit;
        FAMIt child_lnit;

        // c_value
        const double cc = c_val;

        for(paren_lnit = _ln_parents.begin(); paren_lnit != _ln_parents.end(); paren_lnit++)
        {
            // if the current node is minimal generator DNF
            if( _ln_DNF._is_min_gen)
            {
                if( paren_lnit->_ln_DNF._is_min_gen )
                {
                    assert( _num_neigh_min > 0);
                    assert( paren_lnit->_num_neigh_min > 0 );

                    double prob_value = (1.0-a_val)*cc / max(_num_neigh_min, paren_lnit->_num_neigh_min);
                    _edge_weights.push_back(prob_value);
                }
                else // need to know the current node's degree
                {
                    assert( _num_neigh_nonmin > 0 );

                    double prob_value = a_val*cc / _num_neigh_nonmin;
                    _edge_weights.push_back(prob_value);
                }
            }
            else // if the current node is not
            {
                if( paren_lnit->_ln_DNF._is_min_gen ) // need to know current node's parent's degree
                {
                    assert( paren_lnit->_num_neigh_nonmin > 0 );

                    double prob_value = a_val*cc / paren_lnit->_num_neigh_nonmin;
                    _edge_weights.push_back(prob_value);
                }
                else
                {
                    _edge_weights.push_back(1.0);
                }
            }
        }

        for(child_lnit = _ln_children.begin(); child_lnit != _ln_children.end(); child_lnit++)
        {
            // if the current node is minimal generator DNF
            if(_ln_DNF._is_min_gen)
            {
                if( child_lnit->_ln_DNF._is_min_gen )
                {
                    assert( _num_neigh_min > 0 );
                    assert( child_lnit->_num_neigh_min > 0 );

                    double prob_value = (1.0-a_val)*cc / max(_num_neigh_min, child_lnit->_num_neigh_min);
                    _edge_weights.push_back(prob_value);
                }
                else // need to know the current node's degree
                {
                    assert( _num_neigh_nonmin > 0 );

                    double prob_value = a_val*cc / _num_neigh_nonmin;
                    _edge_weights.push_back(prob_value);
                }
            }
            else
            {
                if( child_lnit->_ln_DNF._is_min_gen )
                {
                    assert( child_lnit->_num_neigh_nonmin > 0 );

                    double prob_value = a_val*cc / child_lnit->_num_neigh_nonmin;
                    _edge_weights.push_back(prob_value);
                }
                else
                {
                    _edge_weights.push_back(1.0);
                }
            }
        }
    }

    //calc edge weight
    void get_edge_weight_3(double c_val)
    {
        _edge_weights.clear();

        FAMIt paren_lnit;
        FAMIt child_lnit;

        const double cc = c_val;
        for(paren_lnit = _ln_parents.begin(); paren_lnit != _ln_parents.end(); paren_lnit++)
        {
            // if the current node is minimal generator DNF
            if( _ln_DNF._is_min_gen)
            {
                if( paren_lnit->_ln_DNF._is_min_gen )
                {
                    _edge_weights.push_back(1);
                }
                else // need to know the current node's degree
                {
                    assert( _ln_degree > 0 );
                    _edge_weights.push_back(cc/_ln_degree);
                }
            }
            else // if the current node is not
            {
                if( paren_lnit->_ln_DNF._is_min_gen ) // need to know current node's parent's degree
                {
                    assert( paren_lnit->_ln_degree > 0 );
                    _edge_weights.push_back(cc/paren_lnit->_ln_degree);
                }
                else
                {
                    _edge_weights.push_back(0.5);
                }
            }
        }

        for(child_lnit = _ln_children.begin(); child_lnit != _ln_children.end(); child_lnit++)
        {
            // if the current node is minimal generator DNF
            if(_ln_DNF._is_min_gen)
            {
                if( child_lnit->_ln_DNF._is_min_gen )
                {
                    _edge_weights.push_back(1);
                }
                else // need to know the current node's degree
                {
                    assert( _ln_degree > 0 );
                    _edge_weights.push_back(cc/_ln_degree);
                }
            }
            else
            {
                if( child_lnit->_ln_DNF._is_min_gen )
                {
                    assert( child_lnit->_ln_degree > 0 );
                    _edge_weights.push_back(cc/child_lnit->_ln_degree);
                }
                else
                {
                    _edge_weights.push_back(0.5);
                }
            }
        }
    }

    void get_trans_prob_AND()
    {
        _trans_prob.clear();

        FAMIt paren_lnit;
        FAMIt child_lnit;

        for( paren_lnit = _ln_parents.begin(); paren_lnit != _ln_parents.end(); paren_lnit++ )
        {
            if( _ln_degree < paren_lnit->_ln_degree )
                _trans_prob.push_back( 1.0 / paren_lnit->_ln_degree );
            else
                _trans_prob.push_back( 1.0 / _ln_degree );
        }

        for(child_lnit = _ln_children.begin(); child_lnit != _ln_children.end(); child_lnit++)
        {
            if( _ln_degree < child_lnit->_ln_degree )
                _trans_prob.push_back( 1.0 / child_lnit->_ln_degree );
            else
                _trans_prob.push_back( 1.0 / _ln_degree );
        }

        double prob_sum = accumulate( _trans_prob.begin(), _trans_prob.end(), 0.0 );

        _trans_prob.push_back(1.0 - prob_sum);

        //copy(_trans_prob.begin(), _trans_prob.end(), ostream_iterator<double>(cout, " "));
        //cout<<endl<<endl;
    }

    // Random Jump
    void get_trans_accuprob_rj()
    {
        _trans_accuprob.clear();
        _trans_prob.clear();

        double weight_sum = accumulate( _edge_weights.begin(), _edge_weights.end(), 0.0 );
        DIt wtit;

        double accu_prob = 0.0;
        _trans_accuprob.push_back(accu_prob);
        for (wtit = _edge_weights.begin(); wtit != _edge_weights.end(); wtit++)
        {
            double tran_prob = *wtit / weight_sum;
            accu_prob += tran_prob;

            _trans_prob.push_back(tran_prob);
            _trans_accuprob.push_back(accu_prob);
        }

        //assert( 1 == accumulate( _trans_accuprob.begin(), _trans_accuprob.end(), 0 ) );
    }

    // With Restart
    void get_trans_accuprob_wr(double restart_prob) // with the probability it jumps to the origin
    {
        _trans_accuprob.clear();
        _trans_prob.clear();

        double weight_sum = accumulate( _edge_weights.begin(), _edge_weights.end(), 0.0 );
        DIt wtit;

        double accu_prob = 0.0;
        _trans_accuprob.push_back(accu_prob);
        for (wtit = _edge_weights.begin(); wtit != _edge_weights.end(); wtit++)
        {
            double tran_prob = (*wtit / weight_sum)*(1.0-restart_prob);
            accu_prob += tran_prob;

            _trans_prob.push_back(tran_prob);
            _trans_accuprob.push_back(accu_prob);
        }
        _trans_accuprob.push_back(1.0);
        _trans_prob.push_back(restart_prob);

        //assert( 1 == accumulate( _trans_accuprob.begin(), _trans_accuprob.end(), 0 ) );
    }
};
#endif
