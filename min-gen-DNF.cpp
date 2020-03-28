//#include <ext/hash_map>
#include <vector>

#include "Set.h"
#include "DB.h"
#include "Random_Walk.h"

void parseInput( int argc, char* argv[],
                 char	* inputf,
                 int	& dmin_sup,
                 int	& dmax_sup,
                 int    & cmin_sup,
                 int    & cmax_sup,
                 int    & minoverlap,
                 int    & nmin_DNFs,
                 int	& njump,
                 double & restart_prob,
                 bool	& fOutput,
                 bool	* algorithm,
                 bool   * strategy,
                 bool	& fNegative,
                 bool   & bVar_dist,
                 int    & nStates,
                 int    & eqv,
                 double & a_val,
                 double & c_val
               )
{
    if(argc < 2)
    //if(argc < 0)
    {
        cout << endl << "Usage: " << argv[0] << " -fFile -sMinSup(DNF) -xMaxSup(DNF) -pMinSup(Clause) -qMaxSup(Clause) -gNumDNFs -aAD -n -c" << endl << endl;
        cout << "==================== OPTIONS =====================" << endl << endl;
        cout << "Input File:                                    -fString"                      << endl;
        cout << "Transaction min_sup(DNF):                      -sInteger (default:1)"         << endl;
        cout << "Transaction max_sup(DNF):                      -xInteger (default:10000000)"  << endl;
        cout << "Transaction min_sup(Clause):                   -pInteger (default:1)"         << endl;
        cout << "Transaction max_sup(Clause):                   -qInteger (default:10000000)"  << endl;
        cout << "Minimum overlap among clauses:                 -oInteger (default:0)"         << endl;
        cout << "number of min-gen-DNFs to generate:            -gInteger (default:1000)"	   << endl;
        cout << "random jump after # of consecutive non-mins:   -jInteger (default:3)"         << endl;
        cout << "restart probability:                           -rDouble  (default:0.02)"      << endl;
        cout << "algorithm: (must speficied)                    -aString /*A, D*/"             << endl;
        cout << "strategy:  (must speficied if sampling DNF)    -tString /*J, R*/"             << endl;
        cout << "calc viriation distance?                       -vInteger (# of states)"       << endl;
        cout << "verbose?:                                      -n"                            << endl;
        cout << "Complementary Set:                             -c"                            << endl;
        cout << "Tran. Prob. Matrix version:                    -m(2(default) or 3)"           << endl;
        cout << "Value a_v?                                     -eDouble(default:0.9)"        << endl;
        cout << "Value c_v?                                     -l(-1:aver_tlen (default); -2:max_tlen; or other positive values)"<< endl;
        cout << endl;
        cout << "==================== CONSTRAINTS ===================" << endl << endl;
        cout << "minoverlap <= cmin_sup <= dmin_sup ; cmax_sup <= dmax_sup"                                  << endl;
        exit(1);
    }

    for(int i=1; i<argc; i++)
    {
        if( argv[i][0] == '-' )
            switch( argv[i][1] )
            {
            case 'f':
                strcpy( inputf, argv[i]+2 );
                break;
            case 's':
                dmin_sup = atoi( argv[i]+2 );
                break;
            case 'x':
                dmax_sup = atoi( argv[i]+2 );
                break;
            case 'p':
                cmin_sup = atoi( argv[i]+2 );
                break;
            case 'q':
                cmax_sup = atoi( argv[i]+2 );
                break;
            case 'o':
                minoverlap = atoi( argv[i]+2 );
                break;
            case 'g':
                nmin_DNFs = atoi( argv[i]+2 );
                break;
            case 'j':
                njump = atoi( argv[i]+2 );
                break;
            case 'r':
                restart_prob = atof( argv[i]+2 );
                break;
            case 'n':
                fOutput = false;
                break;
            case 'c':
                fNegative = true;
                break;
            case 'v':
                bVar_dist = true;
                nStates = atoi( argv[i]+2 );
                break;
            case 'm':
                eqv = atoi( argv[i]+2 );
                break;
            case 'e':
                a_val = atof( argv[i]+2 );
                break;
            case 'l':
                c_val = atof( argv[i]+2 );
                break;
            case 'a':
                for(int j=2; argv[i][j]!='\0'; j++)
                {
                    switch(argv[i][j])
                    {
                    case 'A':
                        algorithm[0] = true;
                        break;
                    case 'D':
                        algorithm[1] = true;
                        break;
                    default:
                        cout << "Error Input Options: " << argv[i] << endl;
                        exit(0);
                        break;
                    }
                }
                break;
            case 't':
                for(int j=2; argv[i][j]!='\0'; j++)
                {
                    switch(argv[i][j])
                    {
                    case 'J':
                        strategy[0] = true;
                        break;
                    case 'R':
                        strategy[1] = true;
                        break;
                    default:
                        cout << "Error Input Options: " << argv[i] << endl;
                        exit(0);
                        break;
                    }
                }
                break;
            default:
                cout << "Error Input Options: " << argv[i] << endl;
                exit(0);
                break;
            }
        else
        {
            cout << "Error Input Options: " << argv[i] << endl;
            exit(0);
        }
    }

    cout << endl;
    for(int i=0; i<argc; i++)
        cout << argv[i] << " ";
    cout << endl << endl;

    cout << "$$ Specified Support Info ...... " << endl;
    cout << "DNF min_sup:  " << dmin_sup << endl;
    cout << "DNF max_sup:  " << dmax_sup << endl;
    cout << "Clause min_sup:  " << cmin_sup << endl;
    cout << "Clause max_sup:  " << cmax_sup << endl;
    cout << "Minimum overlap among the clauses... " << minoverlap << endl;
    cout << endl;
}

int dmin_sup = 1;
int dmax_sup = 10000000;
int cmin_sup = 1;
int cmax_sup = 10000000;
int minoverlap = 0;

int main(int argc, char *argv[])
{

    // cmin_sup < dmin_sup ; cmax_sup < dmax_sup
    char	inputf[100];                          // filename
    //char	inputf[100] = "Datasets/example.dat";
    int     nmin_DNFs = 1000;                     // number of min-gen-DNFs....
    int     njump = 3;
    double  restart_prob = 0.02;
    bool	fOutput = true;
    bool	algorithm[2];                         //  AND; DNF
    bool	fNegative = false;
    bool    strategy[2];                          //  random jump to the history; random walk with restart
    int     eqv = 2;
    double  a_val = 0.9;
    double  c_val = -1.0;

    bool    bVar_dist = false;
    int     nStates = 0;

    DB 		db;
    for( int i=0; i<2; i++ )
        algorithm[i] = false;
    //algorithm[1] = true;

    for( int i=0; i<2; i++)
        strategy[i] = false;
    //strategy[0] = true;

    parseInput( argc, argv, inputf, dmin_sup, dmax_sup, cmin_sup, cmax_sup, minoverlap, nmin_DNFs, njump, restart_prob,
                fOutput, algorithm, strategy, fNegative, bVar_dist, nStates, eqv, a_val, c_val );

    if ( (!algorithm[0]) && (!algorithm[1]) )
    {
        cout<<"Please specify sampling: AND or DNF?"<<endl;
        exit(0);
    }

    if (algorithm[0])
    {
        if ( (cmin_sup  != dmin_sup) || (cmax_sup != dmax_sup) )
        {
            cout<<"Specify c_min = d_min, c_max = d_max for sampling AND."<<endl;
            exit(0);
        }
    }

    //cmin_sup < dmin_sup ; cmax_sup < dmax_sup
//    if ( (cmin_sup > dmin_sup) || (cmax_sup > dmax_sup) )
//    {
//        cout<<"Please satisfy the constraint: cmin_sup <= dmin_sup ; cmax_sup <= dmax_sup"<<endl;
//        exit(0);
//    }
//
//    if ( minoverlap > cmin_sup )
//    {
//        cout<<"Please satisfy the constraint: minoverlap <= cmin_sup"<<endl;
//        exit(0);
//    }
//
    if (algorithm[1])
    {
        if ( (!strategy[0]) && (!strategy[1]) )
        {
            cout<<"Please specify strategies: Random Jump or Restart?"<<endl;
            exit(0);
        }
    }

    if( (c_val<=0)&&(c_val != -1.0)&&(c_val != -2.0) )
    {
        cout<<"please specify a positive value for c, or -1, -2"<<endl;
        exit(0);
    }

    // check the tran. prob. eq. version
    if ( (eqv!=2) && (eqv!=3) )
    {
        cout<<"please specify a correct tran. prob. matrix version"<<endl;
        exit(0);
    }

    cout<<"Reading Dataset..."<<endl;
    srand( time(NULL) );
    db.readsize( inputf, fNegative );
    db.readfile( inputf, fNegative );

    // determine the value of c
    if( -1.0 == c_val )
    {
        c_val = db.aver_tlen;
    }
    else if( -2.0 == c_val )
    {
        c_val = db.max_tlen;
    }

    // L1 cache
    cout<< "Caching dataset, store frequent 2-and-item set... "<<endl;
    db.getCached_and( fNegative, fOutput );
    cout<<endl;

    if (algorithm[1])  // minimal DNF generator
    {
        // L2 cache
        cout<< "Caching dataset, store frequent 2-or-item set... "<<endl;
        db.getCached_or( fNegative, fOutput );
        cout<<endl;
    }

    // Random Walk initialization
    cout<<"Random Walk initializing..."<<endl;
    Random_Walk walker;
    walker.rw_initialize(db.itemNum, db.data, db.complement, fNegative, fOutput);
    cout<<"Initialization done, start walking"<<endl<<endl;

    // walking
    int counter = 1;
    if (algorithm[0])   // minimal AND-clause
    {
        // Here we do not need to restart since the partial order graph is connected
        while(walker.get_min_gen_DNFs_size() <= nmin_DNFs)
        {
            if( (walker.get_min_gen_DNFs_size() / 1000) == counter )
            {
                cout<<"Min DNF gen size: "<<walker.get_min_gen_DNFs_size()<<endl;
                if ( bVar_dist && walker.get_min_gen_DNFs_size()> 10000 )
                {
                    walker.calc_var_dist(nStates);
                }
                counter++;
            }
            walker.walk_to_next_AND(db.itemNum, db.data, db.complement,
                                    db.freq_two_items_and, db.freq_two_items_or, fNegative,
                                    fOutput, algorithm);
        }
    }

    if (algorithm[1] && strategy[0])   // minimal DNF generator && random jump to the history
    {
        while(walker.get_min_gen_DNFs_size() <= nmin_DNFs)
        {
            if ( walker._non_mdnf_cnt < njump )
            {
                if( (walker.get_min_gen_DNFs_size() / 1000) == counter )
                {
                    cout<<"Min DNF gen size: "<<walker.get_min_gen_DNFs_size()<<endl;
                    if (bVar_dist && walker.get_min_gen_DNFs_size()> 10000 )
                    {
                        walker.calc_var_dist(nStates);
                    }
                    counter++;
                }
                walker.walk_to_next_DNF(db.itemNum, /*db.aver_tlen, db.max_tlen,*/ db.data, db.complement,
                                    db.freq_two_items_and, db.freq_two_items_or, fNegative, fOutput,
                                    algorithm, strategy, restart_prob, a_val, c_val, eqv);
            }
            else
            {
                if(fOutput)
                    cout<<"jump from an earlier node."<<endl;
                walker.random_jump(db.itemNum, db.data, db.complement, fNegative, fOutput);
            }
        }
    }

    if (algorithm[1] && strategy[1])   // minimal DNF generator && random walk with restart
    {
        while(walker.get_min_gen_DNFs_size() <= nmin_DNFs)
        {
            if( (walker.get_min_gen_DNFs_size() / 1000) == counter )
            {
                cout<<"Min DNF gen size: "<<walker.get_min_gen_DNFs_size()<<endl;
                if (bVar_dist && walker.get_min_gen_DNFs_size()> 10000 )
                {
                    walker.calc_var_dist(nStates);
                }
                counter++;
            }
            walker.walk_to_next_DNF(db.itemNum, /*db.aver_tlen, db.max_tlen,*/ db.data, db.complement,
                                    db.freq_two_items_and, db.freq_two_items_or, fNegative, fOutput,
                                    algorithm, strategy, restart_prob, a_val, c_val, eqv);
        }
    }

    // output
    cout<<"-----------------------------------------------"<<endl<<endl;
    cout<<"Sort the result and print...:"<<endl<<endl;
    walker.show_stat(inputf);
    cout<<"alpha value is: "<<a_val<<endl;
    cout<<"c value is: "<<c_val<<endl;
    cout<<"using tran. prob. matrix version "<<eqv<<endl;
    cout<<"Sampling task finished@!!!"<<endl;
}
