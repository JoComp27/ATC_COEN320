//============================================================================
// Name        : ATC_Project.cpp
// Author      : Jonathan Cournoyer - Aida Bumbu
// Version     :
// Description : Build a Real-Time
//============================================================================


#include "./Plane/Plane.cpp"
#include <vector>
#include <iostream>
#include <pthread.h>
using namespace std;

int time = 0;	//need to implement real time

const int height = 25000;
const int width = 100000;
const int depth = 100000;

vector<Plane> ordered;	//contains planes ordered by released time
vector<Plane> released;	//contains planes that are released but not yet in active zone
vector<Plane> active;	//contains planes that are in active zone
vector<Plane> done;		//contains planes that left the active zone or planes that will never get into the active zone



void updateLocation();
bool isNeverEntering(Plane a);
void checkForCollision();
void printStatus();


int main() {

int airplane_schedule[160] = {
	0, -641, 283, 500, 95000, 101589, 10000, 13, 
	1, -223, -630, -526, 71000, 100000, 13000, 16, 
	-1, -180, -446, -186, 41000, 100000, 6000, 31, 
	3, 474, -239, 428, 38000, 0, 14000, 44, 
	-1, -535, 520, 360, 95000, 100000, 1000, 49, 
	-1, -164, -593, -501, 83000, 100000, 12000, 67, 
	6, 512, 614, 520, 86000, -1571, 9000, 87, 
	7, -275, 153, -399, 47000, 100000, 12000, 103, 
	8, -129, 525, -300, 92000, 100000, 1000, 123, 
	9, 437, 574, 339, 32000, 0, 8000, 129, 
	10, 568, 538, 192, 50000, 0, 4000, 133, 
	11, 347, 529, -599, 83000, -1799, 10000, 140, 
	12, -512, -482, 578, 35000, 100000, 4000, 142, 
	13, 619, -280, 194, 74000, 0, 10000, 157, 
	14, -141, 427, -321, 41000, 102251, 11000, 165, 
	15, -199, 242, -205, 56000, 100000, 4000, 172, 
	16, 315, -197, -365, 77000, 0, 1000, 187, 
	17, -138, 455, 602, 23000, 102290, 14000, 199, 
	18, -150, 557, -356, 38000, 100000, 1000, 204, 
	19, 194, 184, 598, 35000, 0, 2000, 221
};



//Read the List and put planes in the Ordered Vector
for(int i = 0; i < sizeof(airplane_schedule)/sizeof(*airplane_schedule); i+=8){

	
	//create planes and set their values
	Plane plane = Plane();
	plane.setId(airplane_schedule[i]);
	plane.setCurrentVelocity(airplane_schedule[i+1], airplane_schedule[i+2], airplane_schedule[i+3]);
	plane.setCurrentPosition(airplane_schedule[i+4], airplane_schedule[i+5], airplane_schedule[i+6]);
	plane.setReleaseTime(airplane_schedule[i+7]);

	//Put plane into ordered vector 
	
	if (ordered.size() == 0) {			//if ordered vector is empty put the plane inside
		ordered.push_back(plane);
	}
	else {								//else go through ordered vector and put it at its right position
		int size = ordered.size();
		for (int p = 0; p < size; p ++) {
			if (plane.getReleaseTime() < ordered[p].getReleaseTime()) {
				ordered.insert(p + ordered.begin(), plane);
				break;
			}
			else if( p == size -1){		//if release time is larger than the last plane release time 
				ordered.push_back(plane);
			}
		}
	}
}	//finished sorting the planes in ordered list


//while (done.size() < sizeof(airplane_schedule) / sizeof(*airplane_schedule) / 8) {	//while time is running and planes are not done

	//Check when first plane is released and store into Release array
	if (ordered.size() != 0) {
		if (ordered[0].getReleaseTime() >= time) {
			released.push_back(ordered[0]);	//put the plane into release array
			ordered.erase(ordered.begin());	//erase the plane from the ordered array
		}
	}

	//Check when plane is active and store into Active zone
	for (int i = 0; i < released.size(); i++) {
		//checks if plane is in the active block
		if (isNeverEntering(released[i])) {
			done.push_back(released[i]);
			released.erase(released.begin() + i);	//erase plane from Released array
		}
		else if (released[i].isInsideTheBlock(released[i].getCurrentLocation(), 100000, 100000, 25000, 0, 0, 0)) {	
			active.push_back(released[i]);			//put plane into Active array
			released.erase(ordered.begin() + i);	//erase plane from Released array
			//TODO Send Console Message saying which ATC and which plane was sent with time
		}
	}

	//Check when plane is gets out of active zone
	for (int i = 0; i < active.size(); i++) {
		if (active[i].isInsideTheBlock(active[i].getCurrentLocation, 100000, 100000, 25000, 0, 0, 0)) {
			return;
		}
		else {
			//TODO send message to user 
			done.push_back(active[i]);			//plane is put into Done array
			active.erase(active.begin() + i);	//erase plane from Active zone
		}
	}

	//Check for collisions in active list
	checkForCollision();
	//update the location of all planes
	updateLocation();
	
//}

printStatus();

system("pause");
	return 0;
}


//prints the status of the planes that are Active
void printStatus() {

	for (int p = 0; p < active.size(); p++) {
		active[p].print();
		cout << endl;
	}
}

//checks if plane never enters the active zone
bool isNeverEntering(Plane a) {
	bool x = a.getCurrentLocation().getX() > width && a.getCurrentVelocity().getVx() > 0 || a.getCurrentLocation().getX() < 0 && a.getCurrentVelocity().getVx < 0;
	bool y = a.getCurrentLocation().getY() > depth && a.getCurrentVelocity().getVy() > 0 || a.getCurrentLocation().getY() < 0 && a.getCurrentVelocity().getVy < 0;
	bool z = a.getCurrentLocation().getZ() > height && a.getCurrentVelocity().getVz() > 0 || a.getCurrentLocation().getZ() < 0 && a.getCurrentVelocity().getVz < 0;

	return x || y || z;
}

void checkForCollision() {
	if (active.size() < 2) { //No need to check for collisions if less than two planes inside active area
		return;
	}
	else {		//go through active array and check for collisions
		for (int i = 0; i < active.size() - 1; i++) {
			for (int j = i + 1; j < active.size(); j++) {
				while (active[i].collisionCheck(active[j], 1)) { //check if two planes will collide at Time + 1
					//TODO: Print Message saying collision could happen between active[i] and active[j]
					if (active[i].getCurrentVelocity().getVz < active[j].getCurrentVelocity().getVz()) {
						active[j].redirect(active[i]);	//redirects the planes according to their respective velocity
					}
					else {
						active[i].redirect(active[j]);
					}
				}
			}
		}
	}
}

void updateLocation() {
	for (auto& plane : released) {
		plane.updateLocation();
	}
	for (auto& plane : active) {
		plane.updateLocation();
	}
	for (auto& plane : done) {
		plane.updateLocation();
	}
}


//PARAMETERS OF PROJECT
// (ID, Vx, Vy, Vz, X, Y, Z, Release time)
// Height of block : 25000
// Width and length of block : 100000


