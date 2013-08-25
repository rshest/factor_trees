#ifndef _FACTORIZATION_
#define _FACTORIZATION_

#include <vector>
#include <algorithm>

template <typename T>
void find_primes(T up_bound, std::vector<T>& out_primes)
{
    if (out_primes.size() > 0 && out_primes.back() >= up_bound) 
    {
        //  already have the needed prime range
        return;
    }

    const T BAKED_PRIMES[] = {
        2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 
        101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 
        211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 
        307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 
        401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 
        503, 509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 
        601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 
        709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 
        809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 
        907, 911, 919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997 };
    const int NUM_BAKED_PRIMES = sizeof(BAKED_PRIMES)/sizeof(BAKED_PRIMES[0]);

    //  search for the upper bound inside the baked range
    const T* ptop = std::lower_bound(BAKED_PRIMES, BAKED_PRIMES + NUM_BAKED_PRIMES, up_bound + 1);
        
    if (ptop < BAKED_PRIMES + NUM_BAKED_PRIMES)
    {
        //  got all the needed primes from the baked range
        out_primes = std::vector<T>(BAKED_PRIMES, ptop);
        return;
    }

    if (out_primes.size() < NUM_BAKED_PRIMES)
    {
        //  take all the backed primes range before started sieving
        out_primes = std::vector<T>(BAKED_PRIMES, BAKED_PRIMES + NUM_BAKED_PRIMES);
    }

    //  do the Sieve of Eratosthenes on the rest of the range, segment by segment
    const T SEGMENT_SIZE = 512;
    const T SIEVED_ARR_SIZE = (SEGMENT_SIZE + 7)/8;
    char sieved[SIEVED_ARR_SIZE]; // bit array of the "sieved" flags

    T last_sieved = out_primes.back();
    while (last_sieved < up_bound - 1)
    {
        const T num_sieved   = std::min(SEGMENT_SIZE, (up_bound - last_sieved)/2);
        const T first_sieved = last_sieved + 2;
        last_sieved = (num_sieved - 1)*2 + first_sieved;
    
        //  reset the "sieved" flags
        const T sieved_bytes = (num_sieved + 7)/8;
        std::fill(sieved, sieved + sieved_bytes, 0);

        //  sieve out the composites from the given range using the previously found primes 
        //  (only the odd numbers are considered)
        const T max_to_check = (T)sqrt((double)last_sieved) + 1;
        for (int i = 1, e = out_primes.size(); i < e; i++)
        {
            const T cp = out_primes[i];
            if (cp > max_to_check) break;
            const T fs   = std::max(first_sieved, cp*cp); // number to start sieving from
            const T fd   = ((fs + cp - 1)/cp)*cp;         // first dividend in the range
            const T fdo  = fd + cp*(1 - (fd&1));          // first odd dividend in the range
            const T di   = (fdo - first_sieved)/2;        // index of the first odd dividend
            for (T j = di; j < num_sieved; j += cp) 
            {
                //  set the corresponding "sieved" bit
                sieved[j/8] |= (1 << (j%8));
            }
        }

        //  collect all the primes (the numbers with the "sieved" bit not set)
        for (T i = 0; i < sieved_bytes; i++) 
        {
            if (sieved[i] == 0xFF) continue; // no primes in the whole byte
            for (int j = 0; j < 8; j++)
            {
                const T n = (i*8 + j);
                if (!(sieved[i]&(1 << j)) && n < num_sieved)
                {
                    out_primes.push_back(n*2 + first_sieved);
                }
            }
        }
    }
}

template <typename T>
void prime_factors(T val, std::vector<T>& factors, std::vector<T>* primes_cache = NULL)
{
    std::vector<T> _primes;
    std::vector<T>& primes = primes_cache ? *primes_cache : _primes;

    factors.clear();

    T max_to_check = (T)sqrt((double)val) + 1;
    find_primes(max_to_check, primes);
    const int np = primes.size();

    //  do trial division on the primes one by one
    T cval = val;
    int pidx = 0;
    while (pidx < np)
    {
        const T cp = primes[pidx];
        if (cp > max_to_check) break;
        while (cval % cp == 0) 
        {
            factors.push_back(cp);
            cval /= cp;
        }
        pidx++;
    }

    if (cval > 1) factors.push_back(cval);
}

#endif // _FACTORIZATION_
