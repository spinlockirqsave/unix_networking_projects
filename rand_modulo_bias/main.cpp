/* 
 * File:   main.cpp
 * Author: piter
 *
 * Created on June 6, 2014, 7:52 PM
 * 
 * Examine the bias introduced by operation modulo
 * applied to generator of uniform distribution
 * in order to trim the domain
 * 
 * http://stackoverflow.com/a/24069874/1141471
 */

#include <iostream>
#include <boost/random.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <vector>

/*
 * 
 */
int main()
{
    /* 
     * random generator
     */
    boost::mt19937 rng; // I don't seed it on purpouse (it's not relevant)
    boost::uniform_int<> ud( 0, 19);
    boost::variate_generator<boost::mt19937&,
            boost::uniform_int<> > dist( rng, ud);

    std::vector<int> v(15);
    const int runs = 1000 * 1000;
    
    /*
     * Example 1
     * 
     * N = 20, ( 0, 19)
     * rand() % ( n + 1), n + 1 = 15, k = 1, p = 19 % 15 = 4
     * f_0 = ( k + 1) / N = 2 / 20 - probability of repeated outcomes 
     *                               ( from [0,p] = [0,4] range)
     * f_1 = k / N = 1 / 20        - probability of elements from 
     *                               [p+1,n] = [5,19] range
     * bias = f_0 / f_1 = 2
     */
    for ( int i = 0; i < runs; ++i)
    {
        ++v[ dist() % v.size()];
    }

    for ( int i = 0; i < v.size(); ++i)
    {
        std::cout << i << ": " << v[i] << "\n";
    }
    
    /*
     * Example 2
     * 
     * N = 20, ( 0, 19)
     * rand() % ( n + 1), n + 1 = 6, k = 3, p = 19 % 6 = 1
     * f_0 = ( k + 1) / N = 4 / 20 - probability of repeated outcomes 
     *                               ( from [0,p] = [0,1] range)
     * f_1 = k / N = 3 / 20        - probability of elements from 
     *                               [p+1,n] = [2,19] range
     * bias = f_0 / f_1 = 4 / 3
     */
    v.clear();
    v.resize( 6);
    for ( int i = 0; i < runs; ++i)
    {
        ++v[ dist() % v.size()];
    }

    for ( int i = 0; i < v.size(); ++i)
    {
        std::cout << i << ": " << v[i] << "\n";
    }
}

