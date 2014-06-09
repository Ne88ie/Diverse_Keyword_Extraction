#include "modeled_topics.h"

using mas::modeled_topics;

int main(int argc, char* argv[])
{
    modeled_topics(2);
    modeled_topics(5);
    modeled_topics(2, true, true);
    modeled_topics(5, true, true);
    return 0;
}
