#include <gmp.h>
#include <iostream>
#include <fstream>

mpz_t p;
mpz_t q;

mpz_t one;
mpz_t two;

mpz_t random_int;
gmp_randstate_t state;

void gen_prime() {
    gmp_randinit_mt(state);
    mpz_urandomb(q, state, 2000);

    mpz_init_set_ui(one, 1);
    mpz_init_set_ui(two, 2);
    
    do {
        mpz_nextprime(q, q);
        
        mpz_mul(p, q, two);
        mpz_add(p, p, one);
    } while (mpz_probab_prime_p(p, 50) == 0);    

    FILE *file;
    file = fopen("prime.txt", "wt");
    gmp_fprintf(file, "%Zd\n%Zd\n", p, q);

    fclose(file);
}

int main(void) {
    gen_prime();
}
