#include <stdio.h>
#include <iostream>
#include <string>
#include "SimCPP/random.h"
#include "SimCPP/sim_environment.h"


void Customer(Sim::Environment* env, const char* name, Sim::Resource* counters, double time_in_bank)
{
    const int MIN_PATIENCE = 1; // Min. customer patience
    const int MAX_PATIENCE = 3; // Max. customer patience

    // Customer arrives, is served and leaves.
    double arrive_time = env->now();
    printf("%7.4f %s: Here I am\n", arrive_time, name ); fflush(stdout);

    auto counter_available = counters->request();
    double patience = Sim::Random::uniform(MIN_PATIENCE, MAX_PATIENCE);
    auto timeout = env->timeout(patience);

    if( !counter_available->ready() )
    {
        printf("%7.4f %s: --- must wait :( -----\n", arrive_time, name ); fflush(stdout);
    }

    // Wait until any of these two events takes place
    env->wait( counter_available | timeout );

    double wait_time = env->now() - arrive_time;

    if( counter_available->ready() && !timeout->ready() )
    {
        // We got to the counter
        printf("%7.4f %s: Waited %6.3f\n", env->now(), name, wait_time); fflush(stdout);

        double time_at_counter =  time_in_bank;
        env->wait( env->timeout(time_at_counter) );

        printf("%7.4f %s: Finished. Total time in bank %7.4f\n",
               env->now(), name,
               env->now() - arrive_time); fflush(stdout);

        // This will release explicitly the resource.
        // Then the object counter_available goes out of scope, this is called automatically
//        counter_available.reset();
    }
    else if( timeout->ready() )
    {
        printf("%7.4f %s: RENEGED after %6.3f\n", env->now(), name, wait_time); fflush(stdout);
    }
}


void CustomerGenerator(Sim::Environment* env, Sim::Resource* counters )
{
    const int INTERVAL_CUSTOMERS = 5.0; // Generate new customers roughly every x seconds
    const int NEW_CUSTOMERS = 10; // Total number of customers

    for (int i=0; i< NEW_CUSTOMERS; i++)
    {
        std::string name = std::string("Customer-") + std::to_string(i);

        env->addProcess([=]()
        {
            Customer(env, name.c_str(), counters, 2.2);
        } );

        double t = 1.0;
        env->wait( env->timeout(t) );
    }
}


int main()
{
    std::cout << "Bank renege!" << std::endl;

    const int RANDOM_SEED = 42;
    Sim::Random::seed(RANDOM_SEED);
    Sim::Environment env;

    Sim::Resource counters( &env, 2);

    env.addProcess( [&]()
    {
        CustomerGenerator(&env, &counters);
    });

    env.run();

    return 0;
}




