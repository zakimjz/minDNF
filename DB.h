#include <iostream>
#include <fstream>
#include <assert.h>
#include <ext/hash_map>
using namespace __gnu_cxx;

extern int cmin_sup;
extern int cmax_sup;
extern int dmin_sup;
extern int dmax_sup;

class Input
{
#define MAX_LINE        100000
#define MAX_WORD        1000
    char      buf[MAX_LINE], sub[MAX_WORD];
    int       len, k;
    ifstream  infile;
public:
    void open(char *file)
    {
        infile.open(file);
    }

    void close()
    {
        infile.close();
    }

    bool getLine()
    {
        if(infile.eof())
            return false;
        infile.getline(buf, MAX_LINE, '\n');
        len = strlen( buf );
        assert(len <= MAX_LINE);
        k=0;
        return true;
    }

    char* getWord( char sep )  // '\t'
    {
        int p=0;

        while( buf[k]==sep && k<len )
            k++;
        if( buf[k]=='\n' || buf[k]==0xd || k>=len )
            return NULL;

        do
        {
            sub[p++] = buf[k++];
        }
        while( buf[k]!=sep && buf[k]!=0xd && k<len );  //oxd for ^M
        k++;
        sub[p] = '\0';
        assert( p <= MAX_WORD );
        return (char *)sub;
    }
};


typedef hash_multimap<int, int>	CachedTable;
typedef pair<CachedTable::const_iterator, CachedTable::const_iterator> CachedFind;
typedef CachedTable::value_type CachedPair;
typedef CachedTable::const_iterator CTIt;

class DB
{
public:
    Tset	*data;
    Tset	*complement;
    Set	whole;
    int	itemNum, tranNum;
    bool	fConverted;
    char	convertedName[50];
    bool bHasZero;
    double aver_tlen;
    double max_tlen;

    CachedTable freq_two_items_and;
    CachedTable freq_two_items_or;

    ~DB()
    {
        if( data != NULL )
        {
            delete[] data;
            data = NULL;
        }
        if( complement != NULL )
        {
            delete[] complement;
            complement = NULL;
        }
    }

    Set &operator[]( int i )
    {
        return data[i];
    }

    DB()
    {
        itemNum = 0;
        tranNum = 0;
        aver_tlen = 0.0;
        max_tlen = 0.0;

        data = NULL;
        complement = NULL;
        fConverted = false;
        bHasZero = false;
    }

    bool converted( char *filename, bool fNegative )
    {
        FILE	*fp;
        int 	*Trans;
        int	len, i, j;

        len = strlen( filename );
        assert( len < 50 );
        strcpy( convertedName, filename );
        for(i=0; i<len; i++)
            if( convertedName[i] == '.' )  break;
        strcpy( convertedName+i, ".mung" );

        fp = fopen( convertedName, "rb" );
        if( fp == NULL )
        {
            fConverted = false;
            return false;
        }
        fConverted = true;

        size_t result;
        result = fread( &tranNum, 1, sizeof(int), fp );
        if(result != sizeof(int))
        {
            cout<< "reading error!"<<endl;
            exit(0);
        }
        Trans = new int[tranNum];
        assert( Trans != NULL );

        result = fread( Trans, tranNum, sizeof(int), fp );
        if(result != sizeof(int))
        {
            cout<< "reading error!"<<endl;
            exit(0);
        }
        whole.clear();
        for(i=0; i<tranNum; i++)
            whole._S.push_back( Trans[i] );

        result = fread( &itemNum, 1, sizeof(int), fp );
        if(result != sizeof(int))
        {
            cout<< "reading error!"<<endl;
            exit(0);
        }
        data = new Tset[itemNum+1];
        assert( data != NULL );

        //if we want the formula to support negative
        if(fNegative)
        {
            complement = new Tset[itemNum+1];
            assert( complement != NULL );
        }
        else
        {
            complement = NULL;
        }

        cout << "$$ Input Dataset Info..." << endl;
        cout << "itemNum: " << itemNum << endl;
        cout << "tranNum: " << tranNum << endl;

        for(j=1; j<=itemNum; j++)
        {
            result = fread( &len, 1, sizeof(int), fp );
            if(result != sizeof(int))
            {
                cout<< "reading error!"<<endl;
                exit(0);
            }
            result = fread( Trans, len, sizeof(int), fp );
            if(result != sizeof(int))
            {
                cout<< "reading error!"<<endl;
                exit(0);
            }

            for(i=0; i<len; i++)
                data[j]._S.push_back( Trans[i] );
            if(fNegative)
                whole.sub( data[j], complement[j] );
        }
        delete[] Trans;
        fclose( fp );
        return true;
    }

    void saveConvertedFile()
    {
        FILE	*fp;
        int 	*Trans;
        int	len, i, j;
        SIt	it;

        fp = fopen( convertedName, "wb" );
        assert( fp != NULL );

        Trans = new int[tranNum];
        assert( Trans != NULL );

        fwrite( &tranNum, 1, sizeof(int), fp );
        for(it=whole._S.begin(), i=0; it!=whole._S.end(); it++, i++)
            Trans[i] = *it;
        fwrite( Trans, tranNum, sizeof(int), fp );

        fwrite( &itemNum, 1, sizeof(int), fp );
        for(j=1; j<=itemNum; j++)
        {
            len = data[j].size();
            fwrite( &len, 1, sizeof(int), fp );
            for(it=data[j]._S.begin(), i=0; it!=data[j]._S.end(); it++, i++)
                Trans[i] = *it;
            fwrite( Trans, len, sizeof(int), fp );
        }
        delete[] Trans;
        fclose( fp );
        return;
    }

    void readsize( char *filename, bool fNegative )
    {
        Input	in;
        char*	p;
        int	x, t;
        FILE	*fp;

        /*if( converted(filename, fNegative) )
            return;*/

        fp = fopen( filename, "rb" );
        if( fp == NULL )
        {
            cout << "Failed to open file: " << filename << endl << endl;
            exit(0);
        }
        else	fclose(fp);
        // get item number
        itemNum = 0;
        tranNum = 0;
        in.open( filename );

        double total_len = 0.0;
        while( in.getLine() )
        {
            p = in.getWord(' '); // transaction #
            if( p == NULL )
                break;
            p = in.getWord(' '); // transaction #
            p = in.getWord(' '); // length

            if ( max_tlen < atoi(p))
                max_tlen = atoi(p);
            total_len += atoi(p);

            p = in.getWord(' ');
            while( p != NULL )
            {
                x = atoi(p);
                if( x == 0 )
                    bHasZero = true;
                if( x > itemNum )
                    itemNum = x;
                p = in.getWord(' ');
            }
            tranNum++;
        }
        in.close();

        aver_tlen = total_len / tranNum;
        if (bHasZero)
        {
            itemNum++;  // has '0' in dataset
            cout << "$$ Input Dataset Info..." << endl;
            cout << "itemNum: " << itemNum << endl;
            cout << "tranNum: " << tranNum << endl;
            cout << "averaged trans lengh: " << aver_tlen << endl;
            cout << "maximum trans length: " << max_tlen << endl;
        }
        else
        {
            cout << "$$ Input Dataset Info..." << endl;
            cout << "itemNum: " << itemNum << endl;
            cout << "tranNum: " << tranNum << endl;
            cout << "averaged trans lengh: " << aver_tlen << endl;
            cout << "maximum trans length: " << max_tlen << endl;
        }
    }

    void readfile( char *filename, bool fNegative )
    {
        Input	in;
        char*	p;
        int	x, t;

        /*if( fConverted )
        {
            cout << "binary vertical file: " << convertedName << " exists, done with reading." << endl << endl;
            return;
        }*/

        //isets = NULL;
        data = new Tset[itemNum+1];
        assert( data != NULL );

        // if negative is true, means we want to mine all formulas that have positive and negative
        if(fNegative)
        {
            complement = new Tset[itemNum+1];
            assert( complement != NULL );
        }
        else
        {
            complement = NULL;
        }

        in.open( filename );
        while( in.getLine() )
        {
            p = in.getWord(' ');
            if( p == NULL )
                break;
            p = in.getWord(' ');
            t = atoi(p);
            p = in.getWord(' ');

            p = in.getWord(' ');
            while( p != NULL )
            {
                x = atoi(p);
                if (x != 0)
                {
                    data[x].insert(t);  //overloaded funtion, actually insert into data[x]._S
                                        //now, for each item, e.g., A, is a class. A._S is contains the tid that has A.
                }
                else                    // actually we insert '0'
                {
                    data[itemNum].insert(t);
                }
                p = in.getWord(' ');
            }
        }

        in.close();
        getWholeSet_real();

        if(fNegative)
        {
            for(int i=1; i<=itemNum; i++ )
                whole.sub( data[i], complement[i] );
        }

        //saveConvertedFile();
        cout << "done with reading and re-formating file: " << filename << "." << endl;

        if (bHasZero)
        {
            cout<< "dataset has '0' item... change '0' into maximum item number." <<endl;
        }

        cout<<endl;
    }

    int getWholeSet_real()
    // need to pay attention to the zeros, which may change 50543/70446 for chess300.ascii with minsup=60
    {
        //
        for(int i=1; i<=tranNum; i++)
            whole.insert(i);
        cout << "whole set size: " << whole.size();
        cout << endl << endl;
        return whole.size();
    }

    // first level caching
    void getCached_and(bool fNegative, bool fOutput)
    {
        int nstart = fNegative? itemNum*(-1):1;
        for(int i=nstart; i<=itemNum; i++)
        {
            for(int j=i+1; j<=itemNum; j++)
            {
                if (i == -j)
                    continue;

                if ((i==0) || (j==0))
                    continue;

                Tset	TS;
                if(i<0)
                {
                    if(j<0)
                    {
                        if ( complement[-i]._S.size() < cmin_sup || complement[-j]._S.size() < cmin_sup )
                            continue;
                        Intersect( complement[-i]._S, complement[-j]._S, TS._S );
                    }
                    else // j> 0
                    {
                        if ( complement[-i]._S.size() < cmin_sup || data[j]._S.size() < cmin_sup )
                            continue;
                        Intersect( complement[-i]._S, data[j]._S, TS._S );
                    }
                }
                else  // i > 0
                {
                    if ( data[i]._S.size() < cmin_sup || data[j]._S.size() < cmin_sup )
                        continue;
                    Intersect( data[i]._S, data[j]._S, TS._S );
                }

                // we do not need to test cmax_sup!
                if( TS._S.size() >= cmin_sup ) //&& TS._S.size() <= cmax_sup )
                {
                    freq_two_items_and.insert(CachedPair(i,j));
                    freq_two_items_and.insert(CachedPair(j,i));
                }
            }
        }

        cout<< "Done and_caching..."<<endl;
        cout<< "There are "<<freq_two_items_and.size()<<" frequent 2-and-itemset in the cache. "<<endl;
        if (fOutput)
        {
            cout<< "They are: "<<endl;
            for( CTIt ftit = freq_two_items_and.begin(); ftit != freq_two_items_and.end(); ftit++ )
            {
                cout<< "\"" << ftit->first << "&" << ftit->second << "\" ";
            }
            cout<<endl;
        }
    }

    // second level caching
    void getCached_or(bool fNegative, bool fOutput)
    {
        int nstart = fNegative? itemNum*(-1):1;
        for(int i=nstart; i<=itemNum; i++)
        {
            for(int j=i+1; j<=itemNum; j++)
            {
                if (i == -j)
                    continue;

                if ((i==0) || (j==0))
                    continue;

                Tset	TS;
                if(i<0)
                {
                    if(j<0)
                    {
                        if ( complement[-i]._S.size() < cmin_sup || complement[-i]._S.size() > cmax_sup
                             || complement[-j]._S.size() < cmin_sup || complement[-j]._S.size() > cmax_sup )
                            continue;
                        Union( complement[-i]._S, complement[-j]._S, TS._S );
                    }
                    else // j> 0
                    {
                        if ( complement[-i]._S.size() < cmin_sup || complement[-i]._S.size() > cmax_sup
                            || data[j]._S.size() < cmin_sup || data[j]._S.size() > cmax_sup )
                            continue;
                        Union( complement[-i]._S, data[j]._S, TS._S );
                    }
                }
                else  // i > 0
                {
                    if ( data[i]._S.size() < cmin_sup || data[i]._S.size() > cmax_sup
                         || data[j]._S.size() < cmin_sup || data[j]._S.size() > cmax_sup )
                        continue;
                    Union( data[i]._S, data[j]._S, TS._S );
                }

                // we do not need to test dmin_sup!
                if( TS._S.size() <= dmax_sup )
                {
                    freq_two_items_or.insert(CachedPair(i,j));
                    freq_two_items_or.insert(CachedPair(j,i));
                }
            }
        }

        cout<< "Done or_caching..."<<endl;
        cout<< "There are "<<freq_two_items_or.size()<<" frequent 2-or-itemset in the cache. "<<endl;
        if (fOutput)
        {
            cout<< "They are: "<<endl;
            for( CTIt ftit = freq_two_items_or.begin(); ftit != freq_two_items_or.end(); ftit++ )
            {
                cout<< "\"" << ftit->first << "|" << ftit->second << "\" ";
            }
            cout<<endl;
        }
    }

};

