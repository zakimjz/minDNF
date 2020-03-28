#ifndef _RANDOM_WALK_H
#define _RANDOM_WALK_H

#include "random.h"
#include "Lattice_Node.h"
#include <math.h>

// store DNFs
typedef vector<DNF> DNFS;
typedef vector<DNF>::iterator	DNFSIt;
// store lattice node
typedef vector<Lattice_Node> Nodes;
//typedef vector<unsigned int> IntArr;

typedef hash_multimap<int, DNF>	Min_Gen_DNF_Htable;
typedef pair<Min_Gen_DNF_Htable::const_iterator, Min_Gen_DNF_Htable::const_iterator> MGDFind;
typedef Min_Gen_DNF_Htable::value_type MGDPair;
typedef Min_Gen_DNF_Htable::iterator MGDIt;
typedef Min_Gen_DNF_Htable::const_iterator MGDCIt;

bool sort_DNFs_criterion(const DNF &dnf1, const DNF &dnf2)
{
    return dnf1 < dnf2;
}

extern int dmin_sup;
extern int dmax_sup;
extern int cmin_sup;
extern int cmax_sup;

class Random_Walk
{
public:
    int _nwalks;
    int _nparen;
    int _nchild;
    DNFS _min_gen_DNFs;
    Min_Gen_DNF_Htable _min_gen_DNF_candidates;

    Lattice_Node _current_node;

    int _min_dnf_idx;

    // consecutive non minimal DNF generator counts
    int _non_mdnf_cnt ;

    // variation distance
    DoubleArr _var_dist;

public:
    //random walk initialization
    void rw_initialize(const int itemNum, const Tset* data, const Tset* complement, bool fNegative, bool fOutput)
    {
        int dice = get_a_random_number(1, itemNum+1);

        // generate a single item as a clause, which is a DNF
        // cmin_sup < dmin_sup ; cmax_sup < dmax_sup
        while( (data[dice]._S.size() < dmin_sup) || (data[dice]._S.size() > dmax_sup) )
        {
            dice = get_a_random_number(1, itemNum+1);
        }

        // starting node
        Lattice_Node initial_node(dice, data, complement);
        // current location
        _current_node = initial_node;
        _current_node._ln_DNF.getClausesTset();
        if (fOutput)
            _current_node._ln_DNF.show();

        // number of walks
        _nwalks = 0;
        // start from 0
        _min_dnf_idx = 0;
        _non_mdnf_cnt = 0;
    }

    int get_num_parents()
    {
        return _current_node.get_num_parent();
    }

    int get_num_children()
    {
        return _current_node.get_num_children();
    }

    void walk_to_next_AND(const int itemNum, const Tset* data, const Tset* complement,
                          const CachedTable& freq_two_items_and, const CachedTable& freq_two_items_or,
                          bool fNegative, bool fOutput, bool* algorithm)
    {
        // we do not need to test whether it is minimality since the graph is connected

        if( _current_node._ln_degree <= 0 )  // to avoid recompute
        {
            // get the neighborhood and determine whether they are DNFs
            if(fOutput)
                cout<<"Getting all parent candidates..."<<endl;
            _current_node.get_ln_parent_candidates(itemNum, data, complement,
                                                   freq_two_items_and, freq_two_items_or,
                                                   fNegative, fOutput, algorithm, false);
            if(fOutput)
                cout<<"Getting all children candidates..."<<endl;
            _current_node.get_ln_children_candidates(itemNum, data, complement, fOutput, algorithm, false);

            if(fOutput)
                cout<<"Get current node's degree"<<endl;
            _current_node.get_ln_degree_by_func();
        }

        if (_current_node._ln_degree == 0)
        {
            //cout<<"Error!! isolated point!!!..."<<endl;
            //exit(0);

            cout<<"Isolated point!!!... Reinitialize..."<<endl;
            _min_gen_DNFs.push_back(_current_node._ln_DNF);
            rw_initialize(itemNum, data, complement, fNegative, fOutput);
            return;
        }

        while(true)
        {
            unsigned int walk_ind = get_a_random_number(1, _current_node._ln_degree+1);
            // temporarily select the next node
            Lattice_Node next_ln = _current_node.sel_next_ln(walk_ind);
            next_ln.get_ln_parent_candidates(itemNum, data, complement,
                                             freq_two_items_and, freq_two_items_or,
                                             fNegative, fOutput, algorithm, false);
            next_ln.get_ln_children_candidates(itemNum, data, complement, fOutput, algorithm, false);

            // get temporary node's degree
            next_ln.get_ln_degree_by_func();

            // calc acceptance rate
            double toss = random_uni01();
            double ratio = double(_current_node._ln_degree) / next_ln._ln_degree;
            bool bAccept = ( toss <= ( ratio < 1.0 ? ratio: 1.0 ) );

            if(bAccept)
            {
                // collect the minimal generators DNF
                _min_gen_DNFs.push_back(next_ln._ln_DNF);
                // print the minimal generators

                if(fOutput)
                {
                    cout<<"minimal AND clause index:"<<++_min_dnf_idx<<endl;
                    next_ln._ln_DNF.show();
                    cout<<endl<<endl;
                }
                _current_node = next_ln;

                break;
            }
        }
    }

    void walk_to_next_DNF(const int itemNum, /*const double aver_tlen, const double max_tlen,*/ const Tset* data, const Tset* complement,
                          const CachedTable& freq_two_items_and, const CachedTable& freq_two_items_or,
                          bool fNegative, bool fOutput, bool* algorithm, bool* strategy, double restart_prob, double a_val, double c_val, int eqv)
    {
        // is current node DNF
        _current_node.is_ln_DNF(data, complement);

        // get the neighborhood and determine whether they are DNFs
        if(fOutput)
            cout<<"Getting all parent candidates..."<<endl;
        _current_node.get_ln_parent_candidates(itemNum, data, complement,
                                               freq_two_items_and, freq_two_items_or,
                                               fNegative, fOutput, algorithm, false);

        if(fOutput)
            cout<<"Getting all children candidates..."<<endl;
        _current_node.get_ln_children_candidates(itemNum, data, complement, fOutput, algorithm, false);

        if(fOutput)
            cout<<"Getting current node's degree"<<endl;
        _current_node.get_ln_degree_by_func();

        if (_current_node._ln_degree == 0)
        {
            //cout<<"Error!! isolated point!!!..."<<endl;
            //exit(0);

            cout<<"Isolated point!!!... Reinitialize..."<<endl;
            _min_gen_DNFs.push_back(_current_node._ln_DNF);
            rw_initialize(itemNum, data, complement, fNegative, fOutput);
            return;
        }

        if(fOutput)
        {
            cout<<"..........................................................."<<endl;
            cout<<"Determing whether neighborhood nodes are min DNF generators"<<endl;
        }
        _current_node.determine_neighborDNF_minimality(data, complement);

        if(fOutput)
        {
            cout<<".............................."<<endl;
            cout<<"Getting neighbor nodes' degree"<<endl;
        }
        if(2==eqv)
            _current_node.determine_neighbor_degree_DNF_2(itemNum, data, complement,
                                                freq_two_items_and, freq_two_items_or,
                                                fNegative, fOutput, algorithm);  // the most expensive part
        else if(3==eqv)
            _current_node.determine_neighbor_degree_DNF_3(itemNum, data, complement,
                                                freq_two_items_and, freq_two_items_or,
                                                fNegative, fOutput, algorithm);  // optimized

        // get the edge weights and trans prob.
        if(fOutput)
            cout<<"Get edge weights and trans probability"<<endl;
        if(2==eqv)
            _current_node.get_edge_weight_2(/*aver_tlen, max_tlen,*/ a_val, c_val);
        else if(3==eqv)
            _current_node.get_edge_weight_3(c_val);

        if(strategy[0])  //random jump to the history
            _current_node.get_trans_accuprob_rj();
        if(strategy[1])  //random walk with restart
            _current_node.get_trans_accuprob_wr(restart_prob);

        // choose a neighbor
        unsigned int walk_ind = randomWithDiscreteProbability(_current_node._trans_accuprob); // range: 1 -- size()
        if (fOutput)
            cout<<"number of walks up to now:"<<++_nwalks<<endl;

        Lattice_Node next_ln;
        if(walk_ind == (_current_node._ln_degree+1)) // Random walk with restart, we choose to restart from the root
        {
            if (fOutput)
                cout<<"Restart from the root node!!!"<<endl;

            int dice = get_a_random_number(1, itemNum+1);

            // generate a single item as a clause, which is a DNF
            // cmin_sup < dmin_sup ; cmax_sup < dmax_sup
            while( (data[dice]._S.size() < dmin_sup) || (data[dice]._S.size() > dmax_sup) )
            {
                dice = get_a_random_number(1, itemNum+1);
            }

            // starting node
            Lattice_Node initial_node(dice, data, complement);
            next_ln = initial_node;
            next_ln._ln_DNF.getClausesTset();
            next_ln._ln_DNF._is_min_gen = true;
        }
        else // choose a neighbor
        {
            // select the next node
            next_ln = _current_node.sel_next_ln(walk_ind);
        }

        // after choosing the neighbor
        if (next_ln._ln_DNF._is_min_gen)
        {
            // collect the minimal generators DNF
            _min_gen_DNFs.push_back(next_ln._ln_DNF);

            bool bfound_dnf = false;
            MGDFind p = _min_gen_DNF_candidates.equal_range(next_ln._ln_DNF.getSum());
            for (MGDCIt i = p.first; i != p.second; ++i)
            {
                if( (*i).second == next_ln._ln_DNF)
                    bfound_dnf = true;
            }
            if(!bfound_dnf)
                _min_gen_DNF_candidates.insert(MGDPair(next_ln._ln_DNF.getSum(), next_ln._ln_DNF));

            if (fOutput)
            {
                // print the minimal generators
                cout<<"minimal DNF generator index:"<<++_min_dnf_idx<<endl;
                next_ln._ln_DNF.show();
                cout<<endl<<endl;
            }

            _non_mdnf_cnt = 0;
        }
        else
        {
            if (fOutput)
            {
                // produce a non min DNF generator
                cout<<"!It is NOT min_gen:"<<endl;
                next_ln._ln_DNF.show();
                cout<<endl<<endl;
            }

            _non_mdnf_cnt++;
        }

        _current_node = next_ln;
    }

    void random_jump(const int itemNum, const Tset* data, const Tset* complement, bool fNegative, bool fOutput)
    {
        unsigned int n_min_dnf_candidates = get_min_gen_DNF_candidates_size();

        unsigned int jump_idx;
        if (n_min_dnf_candidates > 0)
            jump_idx = get_a_random_number(0, n_min_dnf_candidates);
        else// up to now there is no minimal DNF generator found
        {
            //restart from the very beginning
            rw_initialize(itemNum, data, complement, fNegative, fOutput);
            return;
        }
        if(fOutput)
            cout<<"---- returns to an earlier idx at: "<<jump_idx+1<<", go to its neighbor:"<<endl;

        // jump to the history
        unsigned int counter = 0;
        for(MGDIt mgdIt = _min_gen_DNF_candidates.begin(); mgdIt != _min_gen_DNF_candidates.end(); mgdIt++)
        {
            if( counter == jump_idx )
            {
                _current_node = mgdIt->second;
                _min_gen_DNF_candidates.erase(mgdIt);
                break;
            }
            else
            {
                counter++;
            }
        }

        //recount the consecutive non-min-DNF
        _non_mdnf_cnt = 0;
    }

    unsigned int get_min_gen_DNFs_size()
    {
        return _min_gen_DNFs.size();
    }

    unsigned int get_min_gen_DNF_candidates_size()
    {
        return _min_gen_DNF_candidates.size();
    }

    void sort_DNFs()
    {
        sort(_min_gen_DNFs.begin(), _min_gen_DNFs.end(), sort_DNFs_criterion);
    }

    double calc_var_dist(int nStates)
    {
        sort_DNFs();

        IntArr arr_sample_times;
        int cnt = 0;

        DNF tmpDNF;
        DNFSIt dnfsit = _min_gen_DNFs.begin();
        //dnfsit->show();
        tmpDNF = *dnfsit;
        cnt++; dnfsit++;
        for(; dnfsit != _min_gen_DNFs.end(); dnfsit++)
        {
            if ( *dnfsit == tmpDNF )  // meet the same DNF
            {
                cnt++;
            }
            else
            {
                //cout<<"Sampled Times: "<<cnt<<endl<<endl;
                arr_sample_times.push_back(cnt);
                tmpDNF = *dnfsit;
                cnt = 1;
            }
        }
        //cout<<"Sampled Times: "<<cnt<<endl<<endl;
        arr_sample_times.push_back(cnt);

        double dist = 0.0;
        double total_cnt = accumulate( arr_sample_times.begin(), arr_sample_times.end(), 0.0 );
        for( IIt iit = arr_sample_times.begin(); iit != arr_sample_times.end(); iit++ )
        {
            dist += abs( (*iit / total_cnt) - (1.0 / nStates) );
        }
        dist += ( nStates - arr_sample_times.size() ) * (1.0 / nStates);

        _var_dist.push_back(dist);
    }

    void show()
    {
        if (_min_gen_DNFs.size()==0)
        {
            cout<<"There are NO qualified expressions"<<endl<<endl;
            return;
        }

        DNFSIt dnfsit;
        int cnt = 0;
        for(dnfsit = _min_gen_DNFs.begin(); dnfsit != _min_gen_DNFs.end(); dnfsit++)
        {
            cout<<"Sample index "<<++cnt<<": "<<endl;
            dnfsit->show();
            cout<<endl;
        }
    }

    // output statistics for research purpose
    void show_stat(char* inputf)
    {
        if (_min_gen_DNFs.size()==0)
        {
            cout<<"There are NO qualified expressions"<<endl<<endl;
            return;
        }

        cout<< "Total number of min-DNF-gens is "<<_min_gen_DNFs.size()<<endl<<endl<<endl;

        // on sorted DNFs, count the distinct DNFs: like this way: 1 1 1 2 2 2 2 2 2 3 3 4 4 4 4 4......
        sort_DNFs();
        IntArr arr_sample_times;
        int cnt = 0;

        DNF tmpDNF;
        DNFS unique_min_gen_DNFs;
        DNFSIt dnfsit = _min_gen_DNFs.begin();
        dnfsit->show();
        tmpDNF = *dnfsit;
        cnt++; dnfsit++;
        for(; dnfsit != _min_gen_DNFs.end(); dnfsit++)
        {
            if ( *dnfsit == tmpDNF )  // meet the same DNF
            {
                cnt++;
            }
            else
            {
                cout<<"Sampled Times: "<<cnt<<endl<<endl;
                arr_sample_times.push_back(cnt);
                unique_min_gen_DNFs.push_back(*dnfsit);

                dnfsit->show();
                tmpDNF = *dnfsit;
                cnt = 1;
            }
        }
        cout<<"Sampled Times: "<<cnt<<endl<<endl;
        arr_sample_times.push_back(cnt);

        // output sampled times for plot
        char outputfile[80]="Results/";
        int org_len = strlen( outputfile );
        int len = strlen( inputf );
        int i=0, j=0;
        for(; i<len; i++)
            if( inputf[i] == '/' )  break;
        for(; j<len; j++)
            if( inputf[j] == '.' )  break;
        strcat(outputfile, inputf+i+1);
        outputfile[org_len+j-i-1]='\0';
        strcat(outputfile+j-i-1, "_arr_sample_times.txt");
        cout<<"Output filename: "<<outputfile<<endl;
        ofstream outfile(outputfile, ios::out);
        ostream_iterator<double> oi(outfile, " ");
        copy(arr_sample_times.begin(), arr_sample_times.end(), oi);

        // calculate statistics
        int ndist_samples = arr_sample_times.size();
        // mean
        double mean = accumulate( arr_sample_times.begin(), arr_sample_times.end(), 0.0 ) / double(ndist_samples);
        // variance
        double variance = 0;
        for(int i = 0; i<ndist_samples; ++i)
        {
            variance += (arr_sample_times[i]-mean)*(arr_sample_times[i]-mean)/ndist_samples;
        }

        cout<<"Distinct samples: "  <<ndist_samples<<endl;
        cout<<"Sampling mean: "     <<mean<<endl;
        cout<<"Sampling std: "      <<sqrt(variance)<<endl;

        cout<<"Maximal count is: "  <<*( max_element(arr_sample_times.begin(), arr_sample_times.end()) )<<endl;
        cout<<"Minimal count is: "  <<*( min_element(arr_sample_times.begin(), arr_sample_times.end()) )<<endl;

        nth_element(arr_sample_times.begin(), arr_sample_times.begin() + ndist_samples/2, arr_sample_times.end());
        cout<<"Median count is: "   <<arr_sample_times[ndist_samples/2]<<endl;

        cout<<endl<<endl;

        // variation distance
        if ( _var_dist.size()>0 )
        {
            char vdoutputfile[80]="Results/";
            int vdorg_len = strlen( vdoutputfile );
            int vdlen = strlen( inputf );
            int i=0, j=0;
            for(; i<vdlen; i++)
                if( inputf[i] == '/' )  break;
            for(; j<vdlen; j++)
                if( inputf[j] == '.' )  break;
            strcat(vdoutputfile, inputf+i+1);
            vdoutputfile[vdorg_len+j-i-1]='\0';
            strcat(vdoutputfile+j-i-1, "_variation_distance.txt");

            cout<<"Output filename: "<<vdoutputfile<<endl;
            ofstream vdoutfile(vdoutputfile, ios::out);
            ostream_iterator<double> vdoi(vdoutfile, " ");
            copy(_var_dist.begin(), _var_dist.end(), vdoi);
        }

        // now we have
        // _min_gen_DNFs, unique_min_gen_DNFs
        // 1. length distribution
        IntArr min_gen_len;
        IntArr unique_min_gen_len;
        for (dnfsit = _min_gen_DNFs.begin(); dnfsit != _min_gen_DNFs.end(); dnfsit++)
        {
            min_gen_len.push_back( dnfsit->getLength() );
        }
        ofstream mgoutfile("Results/min_gen_len", ios::out);
        ostream_iterator<double> mgoi(mgoutfile, " ");
        copy(min_gen_len.begin(), min_gen_len.end(), mgoi);

        for (dnfsit = unique_min_gen_DNFs.begin(); dnfsit != unique_min_gen_DNFs.end(); dnfsit++)
        {
            unique_min_gen_len.push_back( dnfsit->getLength() );
        }
        ofstream umgoutfile("Results/unique_min_gen_len", ios::out);
        ostream_iterator<double> umgoi(umgoutfile, " ");
        copy(unique_min_gen_len.begin(), unique_min_gen_len.end(), umgoi);

        // 2. support distribution
        IntArr min_gen_support;
        IntArr unique_min_gen_support;
        for (dnfsit = _min_gen_DNFs.begin(); dnfsit != _min_gen_DNFs.end(); dnfsit++)
        {
            min_gen_support.push_back( dnfsit->getSupport() );
        }
        ofstream mgsoutfile("Results/min_gen_support", ios::out);
        ostream_iterator<double> mgsoi(mgsoutfile, " ");
        copy(min_gen_support.begin(), min_gen_support.end(), mgsoi);

        for (dnfsit = unique_min_gen_DNFs.begin(); dnfsit != unique_min_gen_DNFs.end(); dnfsit++)
        {
            unique_min_gen_support.push_back( dnfsit->getSupport() );
        }
        ofstream umgsoutfile("Results/unique_min_gen_support", ios::out);
        ostream_iterator<double> umgsoi(umgsoutfile, " ");
        copy(unique_min_gen_support.begin(), unique_min_gen_support.end(), umgsoi);
    }
};

#endif
