//============================================================================
// Name        : ATC_Project.cpp
// Author      : Jonathan Cournoyer - Aida Bumbu
// Version     :
// Description : Build a Real-Time
//============================================================================


#include "./Plane/Plane.cpp"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
using namespace std;

string fileAddress = "TrackFile.txt";

int time = 0;

const int height = 25000;
const int width = 100000;
const int depth = 100000;

vector<Plane> ordered;

vector<Plane> released;

vector<Plane> active;

vector<Plane> done;

string getExitDirection(Plane a);
void checkForCollision();
bool isNeverEntering(Plane a);

void broadcast(Plane a) {

	ofstream out;
	out.open(fileAddress);

	out << " -------- SPORADIC ATC BROADCAST AT TIME " << time << "---------" << endl << "Plane ";
	cout << " -------- SPORADIC ATC BROADCAST AT TIME " << time << "---------" << endl << "Plane ";
	
	if (a.getUfo()) {
		out << "with unknown ID ";
		cout << "with unknown ID ";
	}
	else {
		out << a.getId() << " ";
		cout << a.getId() << " ";
	}

	string outDirection = getExitDirection(a);

	out << "has exited the airspace towards the " << outDirection << " Sector." << endl << endl;
	cout << "has exited the airspace towards the " << outDirection << " Sector." << endl << endl;

	out.close();
}

void request(Plane a, int messageType) {

	ofstream out;
	out.open(fileAddress);

	out << " -------- SPORADIC ATC TO PLANE REQUEST AT TIME " << time << "---------" << endl;
	cout << " -------- SPORADIC ATC TO PLANE REQUEST AT TIME " << time << "---------" << endl;

	switch (messageType) {
	case 1: //Location Request
		if (a.getUfo()) {
			out << "UFO " << a.getId << ",ATC requests your current position, over." << endl << endl;
			cout << "UFO " << a.getId << ",ATC requests your current position, over." << endl << endl;
		}
		else {
		out << "Plane " << a.getId << ",ATC requests your current position, over." << endl << endl;
		cout << "Plane " << a.getId << ",ATC requests your current position, over." << endl << endl;
		}
		break;
	case 2: //Velocity Request
		if (a.getUfo()) {
			out << "UFO " << a.getId << ",ATC requests your current velocity, over." << endl << endl;
			cout << "UFO " << a.getId << ",ATC requests your current velocity, over." << endl << endl;
		}
		else {
			out << "Plane " << a.getId << ",ATC requests your current velocity, over." << endl << endl;
			cout << "Plane " << a.getId << ",ATC requests your current velocity, over." << endl << endl;
		}
		break;
	case 3: //Future Position Request
		if (a.getUfo()) {
			out << "UFO " << a.getId << ",ATC requests your future position, over" << endl << endl;
			cout << "UFO " << a.getId << ",ATC requests your future position, over" << endl << endl;
		}
		else {
			out << "Plane " << a.getId << ",ATC requests your future position, over" << endl << endl;
			cout << "Plane " << a.getId << ",ATC requests your future position, over" << endl << endl;
		}
		break;
	default:
		
	}

	out.close();
}

void printHitList() {
	ofstream out;
	out.open(fileAddress);

	out << " -------- PERIODIC HIT LIST AT TIME " << time <<" -------- " << endl;
	cout << " -------- PERIODIC HIT LIST AT TIME " << time << " -------- " << endl;
	
	for (int i = 0; i < active.size(); i++) {
		Plane temp = active[i];

		out << "ID: ";

		if (temp.getUfo()) {
			out << "Unknown , ";
		}
		else {
			out << temp.getId() << " : ";
		}

		out << " x : " << temp.getCurrentLocation.getX() << " , ";
		out << " y : " << temp.getCurrentLocation.getY() << " , ";
		out << " z : " << temp.getCurrentLocation.getZ() << " , ";
		out << endl;

		temp.print();
		cout << endl;
	}

	out << endl;
	cout << endl;

	out.close();
}

//void printStatus() {
//
//	for (int p = 0; p < active.size(); p++) {
//		active[p].print();
//		cout << endl << endl;
//	}
//}

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
			else if( p == size -1){						//if release time is larger than the last plane release time 
				ordered.push_back(plane);
			}
		}
	}
}


while (done.size() < ordered.size()) {	//while time is running and planes are not done

	//Check when released and store into Release array
	for (int i = 0; i < ordered.size(); i++) {
		if (ordered[i].getReleaseTime() >= time) {
			released.push_back(ordered[i]);
			ordered.erase(ordered.begin() + i);
		}

	}

	//Check when active and store into Active array
	for (int i = 0; i < released.size(); i++) {
		if (released[i].isInsideTheBlock(released[i].getCurrentLocation(), 100000, 100000, 25000, 0, 0, 0)) {
			active.push_back(released[i]);
			//Send Console Message saying which ATC and which plane was sent
		}
		else if (isNeverEntering(released[i])) {
			done.push_back(released[i]);
			released.erase(released.begin() + i);
		}
	}

	//Check for active list
	checkForCollision();

}

printStatus();

system("pause");
	return 0;
}

bool isNeverEntering(Plane a) {
	bool x = a.getCurrentLocation().getX() > width && a.getCurrentVelocity().getVx() > 0 || a.getCurrentLocation().getX() < 0 && a.getCurrentVelocity().getVx < 0;
	bool y = a.getCurrentLocation().getY() > depth && a.getCurrentVelocity().getVy() > 0 || a.getCurrentLocation().getY() < 0 && a.getCurrentVelocity().getVy < 0;
	bool z = a.getCurrentLocation().getZ() > height && a.getCurrentVelocity().getVz() > 0 || a.getCurrentLocation().getZ() < 0 && a.getCurrentVelocity().getVz < 0;

	return x || y || z;
}

void checkForCollision() {
	if (active.size() < 2) {
		return;
	}
	else {
		for (int i = 0; i < active.size() - 1; i++) {
			for (int j = i+1; j < active.size(); j++) {
				while (active[i].collisionCheck(active[j], 1)) {
					//TODO: Print Message saying collision could happen between active[i] and active[j]
					//TODO : Attempt to fix the issue
				}
			}
		}
	}
}

string getExitDirection(Plane a) {
	string answer = "";
	int eastDiff = abs(a.getCurrentLocation().getX() - 100000);
	int westDiff = abs(a.getCurrentLocation().getX());
	int northDiff = abs(a.getCurrentLocation().getY() - 100000);
	int southDiff = abs(a.getCurrentLocation().getX());

	int minimum = min(eastDiff, min(westDiff, min(northDiff, southDiff)));

	if (eastDiff == minimum) {
		return "East";
	}
	else if (westDiff == minimum) {
		return "West";
	}
	else if (northDiff == minimum) {
		return "North";
	}
	else {
		return "South";
	}

}


//PARAMETERS OF PROJECT
// (ID, Vx, Vy, Vz, X, Y, Z, Release time)
// Height of block : 25000
// Width and length of block : 100000


