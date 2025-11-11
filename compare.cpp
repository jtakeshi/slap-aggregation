// g++ compare.cpp -o compare -O3 -std=c++14 -lgmp -lgmpxx

#include <iostream>
#include <chrono>
#include <vector>

#include <gmp.h>
#include <gmpxx.h>

using namespace std;
using namespace std::chrono;
using std::vector;

#define SEED 123
#define LARGE_BITCOUNT 2048 //TODO force schemes to 120 bits of security - should be 2000 to 3000 bits for DLOG
#define BITCOUNT 16

using duration_t = std::chrono::high_resolution_clock::time_point;

//Pollard lambda based on https://github.com/markusju/pollard-rho/blob/master/pollard_rho.py



void xab(mpz_class & x, mpz_class & a, mpz_class & b, 
  const mpz_class & G, const mpz_class & H, const mpz_class & P, const mpz_class & Q){
  mpz_class sub = x % 3;
  if(sub == 0){
    x *= G;
    x %= P;
    //a = (a+1)%Q
    a++;
    if(a == Q){
      a = 0;
    }
  }
  if(sub == 1){
    x *= H;
    x %= P;
    b++;
    if(b == Q){
      b = 0;
    }
  }
  if(sub == 2){
    x *= x;
    x %= P;
    a <<= 1;
    a %= Q;
    b <<= 1;
    b %= Q;
  }
  return;
}

bool verify(const mpz_class & g, const mpz_class & h, 
    const mpz_class & p, const mpz_class & x){
  mpz_class tmp;
  mpz_powm(tmp.get_mpz_t(), g.get_mpz_t(), x.get_mpz_t(), p.get_mpz_t());
  return tmp == h;
}

mpz_class pollard(const mpz_class & G, const mpz_class & H, const mpz_class & P){
  mpz_class Q = (P-1)/2;

  mpz_class x = G*H;
  mpz_class a, b;
  a = 1;
  b = 1;

  mpz_class X = x;
  mpz_class A = a;
  mpz_class B = b;

  //vector<mpz_class> parms = {};

  mpz_class i;
  for(i = 0; i < P; i++){
    xab(x, a, b, G, H, P, Q);

    xab(X, A, B, G, H, P, Q);
    xab(X, A, B, G, H, P, Q);
    if(x == X){
      break;
    }
  }

  mpz_class nom = a-A;
  mpz_class denom = B-b;

  mpz_class res;
  mpz_invert(res.get_mpz_t(), denom.get_mpz_t(), Q.get_mpz_t());
  res *= nom;
  res %= Q;

  if(verify(G, H, P, res)){
    return res;
  }

  return res + Q;
}

template<typename T>
T avg(const std::vector<T> & args){
  T ret = 0;
  for(const T & x : args){
    ret += x;
  }
  return ret / args.size();
}

int main(int argc, char ** argv){

  duration_t start, end;
  gmp_randstate_t state;
  gmp_randinit_default(state);
  gmp_randseed_ui(state, SEED);

  int i, j, n, repeat, k, prod;

  mpz_t small, large;
  mpz_inits(small, large, NULL);

  mpz_t temp, temp2;
  mpz_init(temp);
  mpz_init(temp2);

  mpz_class one(1);
  
  int users = 1000000;
  int delta = 100000;

  for(i = 4; i <= users; i+=delta){
    n = i;

    vector<double> enc_times;
    enc_times.reserve(n);
    
    //Shi
    mpz_urandomb(temp, state, BITCOUNT);
    mpz_urandomb(small, state, BITCOUNT);
    mpz_mul(large, small, small);
    mpz_class h(temp);
    mpz_class L(large); //force this to be prime
    mpz_nextprime(L.get_mpz_t(), L.get_mpz_t());
    for(j = 0; j < n; j++){
      while(!mpz_sgn(small)){
        mpz_urandomb(small, state, BITCOUNT);
      }
      mpz_urandomb(temp, state, BITCOUNT);
      
      mpz_mul(large, small, small);
      start = high_resolution_clock::now();
      mpz_powm(temp, small, small, small);
      mpz_mul(temp, temp, small);
      mpz_mod(temp, temp, small);
      end = high_resolution_clock::now();
      enc_times.push_back(duration_cast<chrono::microseconds>(end-start).count());
      
    }
    cout << "shi_enc " << avg(enc_times) << ' ' << n << endl;
    enc_times.clear();

    
    mpz_urandomb(temp, state, BITCOUNT);
    while(!mpz_sgn(small)){
      mpz_urandomb(small, state, BITCOUNT);
    }
    mpz_mul(large, small, small);
    start = high_resolution_clock::now();
    for(j = 0; j < (n-1); j++){

      //mpz_urandomb(temp, state, BITCOUNT);
      //mpz_urandomb(small, state, BITCOUNT);
      
      mpz_mul(temp, temp, small);
      mpz_mod(temp, temp, small);
    }
    mpz_class t(temp);
    mpz_class ret = pollard(t, h, L);
    end = high_resolution_clock::now();
    cout << "shi_agg " << duration_cast<chrono::microseconds>(end-start).count() << ' ' << n << endl;
    
    //Joye
    mpz_class N(large);
    N *= N;
    mpz_class hash_sec(large);
    hash_sec++;
    for(j = 0; j < n; j++){
      mpz_urandomb(temp, state, BITCOUNT);
      mpz_urandomb(small, state, BITCOUNT);
      
      start = high_resolution_clock::now();
      mpz_mul(temp, temp, small);
      mpz_add(temp, temp, one.get_mpz_t());
      mpz_mul(temp, temp, small);
      mpz_mul(temp, temp, hash_sec.get_mpz_t());
      mpz_mod(temp, temp, large);
      end = high_resolution_clock::now();
      enc_times.push_back(duration_cast<chrono::microseconds>(end-start).count());
    }
    cout << "joye_enc " << avg(enc_times) << ' ' << n << endl;
    enc_times.clear();


    mpz_urandomb(temp, state, BITCOUNT);
    mpz_urandomb(small, state, BITCOUNT);
    mpz_mul(large, small, small);
    start = high_resolution_clock::now();
    for(j = 0; j < (n-1); j++){
      mpz_mul(temp, temp, small);
      mpz_mod(temp, temp, large);
    }
    /*
    mpz_powm(temp, large, large, large);
    mpz_mul(temp, temp, small);
    mpz_mod(temp, temp, large);
    */
    mpz_sub(temp, temp, one.get_mpz_t());
    mpz_mul(temp, temp, large);
    mpz_mod(temp, temp, large);
    end = high_resolution_clock::now();
    cout << "joye_agg " << duration_cast<chrono::microseconds>(end-start).count() << ' ' << n << endl;
    
    //Jung 2013
    for(j = 0; j < n; j++){
      mpz_urandomb(temp, state, BITCOUNT);
      mpz_urandomb(small, state, BITCOUNT);
      start = high_resolution_clock::now();
      mpz_mul(temp, small, small);
      mpz_mul(temp, temp, large);
      mpz_mod(temp, temp, large);
      end = high_resolution_clock::now();
      enc_times.push_back(duration_cast<chrono::microseconds>(end-start).count());
    }
    cout << "jung2013_enc " << avg(enc_times) << ' ' << n << endl;
    enc_times.clear();

    mpz_urandomb(temp, state, BITCOUNT);
    while(!mpz_sgn(small)){
      mpz_urandomb(small, state, BITCOUNT);
    }
    mpz_mul(large, small, small);
    start = high_resolution_clock::now();
    for(j = 0; j < (n-1); j++){
      mpz_mul(temp, temp, small);
      mpz_mod(temp, temp, small);
    }
    mpz_powm(temp, large, large, large);
    end = high_resolution_clock::now();
    cout << "jung2013_agg " << duration_cast<chrono::microseconds>(end-start).count() << ' ' << n << endl;
   
    //Jung PDA
    mpz_set_ui(temp, 1);
    start = high_resolution_clock::now();
    for(k = 0; k <  n; k++){  
      for(j = 0; j < 3; j++){
        mpz_mul(temp, temp, small);
        mpz_mod(temp, temp, small);
      }
      mpz_powm(temp, large, temp, large);
    }
    end = high_resolution_clock::now();
    cout << "pda_user1 " << duration_cast<chrono::microseconds>(end-start).count() << ' ' << n << endl;
    // user 1
    
    start = high_resolution_clock::now();
    for(k = 0; k <  n; k++){
      mpz_powm(temp, small, small, large);
      mpz_mul(temp, temp, small);
      mpz_mod(temp, temp, large);
    }
    end = high_resolution_clock::now();
    cout << "pda_user2 " << duration_cast<chrono::microseconds>(end-start).count() << ' ' << n << endl;
    // user 2
    
    mpz_mod(temp, temp, small);

    start = high_resolution_clock::now();
    for(k = 0; k < n * 3; k++){
      mpz_powm(temp, temp, small, small);
      mpz_mul(temp, temp, small);
      mpz_mod(temp, temp, small);
    }
    end = high_resolution_clock::now();
    cout << "pda_enc " << duration_cast<chrono::microseconds>(end-start).count() << ' ' << n << endl;
    // other user
    
    mpz_set_ui(temp, 1);

    start = high_resolution_clock::now();
    for(k = 0; k < 1; k++){ //Should this be n-1?
      for(j = 0; j < n-1; j++){
        mpz_mul(temp, temp, large);
        mpz_mod(temp, temp, large);
      }
      mpz_powm(temp, large, small, large);
    }
    end = high_resolution_clock::now();
    cout << "pda_agg " << duration_cast<chrono::microseconds>(end-start).count() << ' ' << n << endl;

  }

  return 0;
}
