#include "bank_renege.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include "random.h"
#include "sim_environment.h"

const int RANDOM_SEED = 42;
const int NEW_CUSTOMERS = 15; // Total number of customers
const int INTERVAL_CUSTOMERS = 10.0; // Generate new customers roughly every x seconds

void CustomerGenerator(Sim::Environment* env, int number, double interval )
{
    for (int i=0; i< number; i++)
    {
        printf("%7.4f: Here I am %d\n", env->now(), i );
        fflush(stdout);
        double t = Random::expovariate(1.0 / interval);
        env->wait( env->timeout(t) );
    }
}

int main()
{
    std::cout << "Bank renege!" << std::endl;

    Random::seed(RANDOM_SEED);
    Sim::Environment env;

    env.addProcess( [&]()
    {
        CustomerGenerator(&env, NEW_CUSTOMERS, INTERVAL_CUSTOMERS);
    });

    env.run();

    return 0;
}




