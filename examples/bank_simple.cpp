#include <stdio.h>
#include <iostream>
#include <string>
#include "SimCPP/random.h"
#include "SimCPP/sim_environment.h"

const int RANDOM_SEED = 42;
const int NEW_CUSTOMERS = 5; // Total number of customers
const int INTERVAL_CUSTOMERS = 10.0; // Generate new customers roughly every x seconds

void Customer(Sim::Environment* env, const char* name, Sim::Resource* counters, double time_in_bank)
{
    // Customer arrives, is served and leaves.
    double arrive_time = env->now();
    printf("%7.4f %s: Here I am\n", arrive_time, name );
    fflush(stdout);

    auto request = counters->request();

    // Wait for the counter or abort at the end of our tether
    env->wait( request );

    double wait_time = env->now() - arrive_time;

    if( request->ready() )
    {
        // We got to the counter
        printf("%7.4f %s: Waited %6.3f\n", env->now(), name, wait_time);
        fflush(stdout);

        double tib = 20; //Random::expovariate(1.0 / time_in_bank);
        env->wait( env->timeout(tib) );
        printf("%7.4f %s: Finished\n", env->now(), name);
        fflush(stdout);
    }
    else{
        printf("%7.4f %s: RENEGED after %6.3f\n", env->now(), name, wait_time);
        fflush(stdout);
    }

}


void CustomerGenerator(Sim::Environment* env, int number, double interval, Sim::Resource* counters )
{
    for (int i=0; i< number; i++)
    {
        std::string name = std::string("Customer-") + std::to_string(i);
        env->addProcess([=]()
        {
            Customer(env, name.c_str(), counters, 12.0);
        } );

        double t = Sim::Random::expovariate(1.0 / interval);
        env->wait(  env->timeout(t) );
    }
}


int main()
{
    std::cout << "Bank renege!" << std::endl;

    Sim::Random::seed(RANDOM_SEED);
    Sim::Environment env;

    // Start processes and run
    Sim::Resource counters( &env, 2);

    env.addProcess( [&]()
    {
        CustomerGenerator(&env, NEW_CUSTOMERS, INTERVAL_CUSTOMERS, &counters);
    });

    env.run();

    return 0;
}




