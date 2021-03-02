
#include "Hashgraphs.hpp"
#include "Person.hpp"

// global variables
int  runTime;
bool makeForks;
bool writeLog;
std::array<Person*, N> people;


int main(){

	// init global vars
    runTime   = 2;
    makeForks = 0;
    writeLog  = 1;

    // init randomness
    srand(time(NULL));

    // init hasher
    md5Init();

    // create new persons
	for (int i = 0; i < N; i++)
		people[i] = new Person(i);

    while (1) {

        // select a random node
        int i = std::rand() % N;

        // select a second distinct node not gossip with
        int j;
        while ((j = std::rand() % N) == i);

        // send gossip
        people[i]->gossip(*(people[j]));

        runTime++;
    }
}
