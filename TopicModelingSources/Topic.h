#ifndef TOPIC_H
#define	TOPIC_H

#include <cstdlib>
#include <vector>

class Topic
{
public:
    Topic();
    ~Topic();

private:

    std::vector<double> featureDistribution;

};

#endif	/* TOPIC_H */