//============================================================================
// Name        : ATC_Project.cpp
// Author      : Jonathan Cournoyer - Aida Bumbu
// Version     :
// Description : Build a Real-Time
//============================================================================

#include "Plane.cpp"
#include <vector>
#include <iostream>
using namespace std;

int time = 0;

const int height = 25000;
const int width = 100000;
const int depth = 100000;

Plane data[] = {
		Plane(1,2,3,4,5,6,7,8),
		Plane(1,2,3,4,5,6,7,8)
};

vector<Plane> released;

vector<Plane> active;

vector<Plane> done;

int main() {

	while(done.size() < data.size())

	return 0;
}

void printStatus(){



}

//PARAMETERS OF PROJECT
// (ID, Vx, Vy, Vz, X, Y, Z, Release time)
// Height of block : 25000
// Width and length of block : 100000


