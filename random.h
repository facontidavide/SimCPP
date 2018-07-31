#ifndef RANDOM_H
#define RANDOM_H

#include <random>

class Random
{
private:

    static std::default_random_engine& engine()
    {
        static std::default_random_engine generator;
        return generator;
    }
public:

    static void seed(unsigned n)
    {
        engine().seed(n);
    }

    static double uniform(double min, double max)
    {
        std::uniform_real_distribution<double> distribution(min, max);
        return distribution( engine() );
    }

    static double uniform_int(int min, int max)
    {
        std::uniform_int_distribution<int> distribution(min, max);
        return distribution( engine() );
    }

    static double expovariate(double val)
    {
        std::exponential_distribution<double> distribution(val);
        return distribution( engine() );
    }

};

#endif // RANDOM_H
