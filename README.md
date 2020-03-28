# minDNF sampling algorithm

The minDNF method samples minimal boolean expressions in DNF
(Disjunctive Normal Form)


**Relevant Publications**

*[2012-sigkdd] Geng Li and Mohammed J. Zaki. Sampling minimal frequent boolean (dnf) patterns. In 18th ACM SIGKDD International Conference on Knowledge Discovery and Data Mining. August 2012.

*[2016-dmkd] Geng Li and Mohammed J. Zaki. Sampling frequent and minimal boolean patterns: theory and application in classification. Data Mining and Knowledge Discovery, 30(1):181–225, January 2016. doi:10.1007/s10618-015-0409-y.


# HOW TO

1) The program supports ibm format dataset (see https://github.com/zakimjz/IBMGenerator).  If the dataset contains '0' item, then the program will reassign the item to the maximal number. For example, if the program contains 28 items, from 0-27. Then the program will assign '0' to '28'. Now the items are 1-28.

2) Type 'min-gen-DNF' to see the options.

==================== OPTIONS =====================

    Input File:                                    -fString                        // data file
    Transaction min_sup(DNF):                      -sInteger (default:1)           // d_min
    Transaction max_sup(DNF):                      -xInteger (default:10000000)    // d_max
    Transaction min_sup(Clause):                   -pInteger (default:1)           // c_min
    Transaction max_sup(Clause):                   -qInteger (default:10000000)    // c_max
    number of min-gen-DNFs to generate:            -gInteger (default:1000)        
    random jump after # of consecutive non-mins:   -jInteger (default:3)           
    restart probability:                           -rDouble  (default:0.02)         
    algorithm: (must speficied)                    -aString /*A, D*/               // A: AND clause;  D: DNF
    strategy:  (must speficied if sampling DNF)    -tString /*J, R*/               // J: random jump to history; R: restart
    calc viration distance?                        -vInteger (# of states)         
    verbose?:                                       -n
    Complementary Set:                              -c                             // support negative items

   
Typical commands:

a) minimal AND:

    $ time ./min-gen-DNF -fDatasets/conv_chess.dat -s2600 -x3196 -p2600 -q3196 -aA -g393500 -n

Here we specify c_min = d_min=2600, c_max = d_max = 3196(# of transtions in Chess).  We use Minimal AND sampling. We do 393500 iterations. Note that for AND clause we need to specify d_min, d_max, since each AND is also a DNF. Make sure that c_min = d_min, c_max = d_max for sampling AND.      
	
b) minimal DNF:

    $ time ./min-gen-DNF -fDatasets/conv_chess.dat -s2500 -x3196 -p2500 -q3196 -aD -g1000 -tJ -j3 -n

We use minimal DNF sampling. The strategy is randomly jump back to history without replacement. If we meet consecutive 3 non-minimal DNF, we make jump. The desired number of minimal DNF is 1000.
   
    $ time ./min-gen-DNF -fDatasets/conv_chess.dat -s2500 -x3196 -p2500 -q3196 -aD -g1000 -tR -r0.02 -n 

The strategy is for walk there is a probability (0.02) to return to the origin. 
