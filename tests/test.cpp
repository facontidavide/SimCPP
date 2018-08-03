#include <stdio.h>
#include <iostream>
#include <string>
#include "SimCPP/random.h"
#include "SimCPP/sim_environment.h"

const int RANDOM_SEED = 42;
const int NEW_CUSTOMERS = 10; // Total number of customers
const int INTERVAL_CUSTOMERS = 5.0; // Generate new customers roughly every x seconds

void SerialGenerator(Sim::Environment* env, int number, double interval )
{
    for (int i=0; i< number; i++)
    {
        double t = Sim::Random::expovariate(2.0 / interval);
        printf("%7.4f: Here I am %d. Should wake up at %7.4f\n",
               env->now(), i + 1000, env->now() + t );
        fflush(stdout);
        env->wait( env->timeout(t) );
    }
}

void Customer(Sim::Environment* env, const char* name, double interval )
{
    printf("%7.4f: Here I am %s\n", env->now(), name );
    fflush(stdout);
    double t = Sim::Random::expovariate(1.0 / interval);
    env->wait( env->timeout(t) );
    printf("%7.4f: DONE %s\n", env->now(), name );
    fflush(stdout);
}

void ParallelGenerator(Sim::Environment* env, int number, double interval )
{
    for (int i=0; i< number; i++)
    {
        env->addProcess( [&]()
        {
            Customer(env, std::to_string(i).c_str(), interval);
        });
    }
}


int main()
{
    std::cout << "Test" << std::endl;

    Sim::Random::seed(RANDOM_SEED);
    Sim::Environment env;

    env.addProcess( [&]()
    {
        ParallelGenerator(&env, NEW_CUSTOMERS, INTERVAL_CUSTOMERS);
    });

    std::cout << "---------------" << std::endl;

    env.addProcess( [&]()
    {
        SerialGenerator(&env, NEW_CUSTOMERS, INTERVAL_CUSTOMERS);
    });

    std::cout << "------- Run -------" << std::endl;
    env.run();

    return 0;
}




