//============================================================================
// Name        : ATC_Project.cpp
// Author      : Jonathan Cournoyer - Aida Bumbu
// Version     :
// Description : Build a Real-Time
//============================================================================


#include "./Plane/Plane.cpp"
#include <vector>
#include <iostream>
//#include <pthread.h>
#include <time.h>
#include <thread>
#include <fstream>
#include <string>
#include <algorithm>
#include <atomic>
#include <mutex>
using namespace std;



int time = 0;

const int height = 25000;
const int width = 100000;
const int depth = 100000;

vector<Plane> ordered;	//contains planes ordered by released time
vector<Plane> released;	//contains planes that are released but not yet in active zone
vector<Plane> active;	//contains planes that are in active zone
vector<Plane> done;		//contains planes that left the active zone or planes that will never get into the active zone

clock_t tStart;
clock_t tEnd;

clock_t emptyBlock;
vector<clock_t> orderedToReleased;
vector<clock_t> releasedToActive;
vector<clock_t> activeToDone;
vector<clock_t> checkCollisions;
vector<clock_t> updateLocations;
vector<clock_t> userConsole;

void updateLocation();
bool isNeverEntering(Plane a);
void checkForCollision();
void printStatus();
void menu();


string getExitDirection(Plane a);
void checkForCollision();
bool isNeverEntering(Plane a);

void broadcast(Plane a) {

	ofstream out;
	out.open(fileAddress);

	out << " -------- SPORADIC ATC BROADCAST AT TIME " << time << " ---------" << endl << "Plane ";
	cout << " -------- SPORADIC ATC BROADCAST AT TIME " << time << " ---------" << endl << "Plane ";
	
	if (a.getUfo()) {
		out << "with unknown ID ";
		cout << "with unknown ID ";
	}
	else {
		out << a.getId() << " ";
		cout << a.getId() << " ";
	}

	string outDirection = getExitDirection(a);

	out << "has exited our airspace towards the " << outDirection << " Sector, over." << endl << endl;
	cout << "has exited our airspace towards the " << outDirection << " Sector, over." << endl << endl;

	out.close(fileAddress);
}

void receiveBroadcast(Plane a) {

	ofstream out;
	out.open(fileAddress);

	out << " -------- SPORADIC ATC BROADCAST RECEPTION AT TIME " << time << " ---------" << endl << "A Plane ";
	cout << " -------- SPORADIC ATC BROADCAST RECEPTION AT TIME " << time << " ---------" << endl << "A Plane ";

	if (a.getUfo()) {
		out << "with unknown ID ";
		cout << "with unknown ID ";
	}
	else {
		out << a.getId() << " ";
		cout << a.getId() << " ";
	}

	string outDirection = getExitDirection(a);

	out << "has exited our airspace towards your airspace at the position" 
		<< " x: " << a.getCurrentLocation().getX() 
		<< " y: " << a.getCurrentLocation().getY() 
		<< " z: " << a.getCurrentLocation().getZ() 
		<< ", over." << endl << endl;
	cout << "has exited the airspace towards your airspace at the position" 
		<< " x: " << a.getCurrentLocation().getX()
		<< " y: " << a.getCurrentLocation().getY() 
		<< " z: " << a.getCurrentLocation().getZ() 
		<< ", over." << endl << endl;

	out.close();

}

void request(Plane a, int messageType, int n = 1) {

	ofstream out;
	out.open(fileAddress);

	out << " -------- SPORADIC ATC TO PLANE REQUEST AT TIME " << time << " ---------" << endl;
	cout << " -------- SPORADIC ATC TO PLANE REQUEST AT TIME " << time << " ---------" << endl;

	if (a.getUfo()) {
		out << "UFO " << a.getId();
		cout << "UFO " << a.getId();
	}
	else {
		out << "Plane " << a.getId();
		cout << "Plane " << a.getId();
	}

	switch (messageType) {
	case 1: //Location Request

		out << ",ATC requests your current position, over." << endl << endl;
		cout << ",ATC requests your current position, over." << endl << endl;

		break;
	case 2: //Velocity Request

		out << ",ATC requests your current velocity, over." << endl << endl;
		cout << ",ATC requests your current velocity, over." << endl << endl;

		break;

	case 3: //Future Position Request

		out << ",ATC requests your future position at time " << (time + n) << ", over" << endl << endl;
		cout << ",ATC requests your future position at time " << (time + n) << ", over" << endl << endl;

		break;
	case 4: //Elevation Change request
		out << ",ATC requests an elevation change of " << n << "000 feet, over" << endl << endl;
		cout << ",ATC requests an elevation change of " << n << "000 feet, over" << endl << endl;
	}
	


	out.close(fileAddress);
}

void response(Plane a, int messageType, int n = 1) {
	ofstream out;
	out.open(fileAddress);

	out << " -------- SPORADIC PLANE RESPONSE AT TIME " << time << " ---------" << endl;
	cout << " -------- SPORADIC PLANE RESPONSE AT TIME " << time << " ---------" << endl;

	Location futureLoc = a.getFutureLocation(n);

		if (a.getUfo()) {
			out << "This is UFO " << a.getId();
			cout << "This is UFO " << a.getId();
		}
		else {
			out << "This is Plane " << a.getId();
			cout << "This is Plane " << a.getId();
		}

	switch (messageType) {
	case 1: //Location Request

		out << ", our current position is"
			<< " x: " << a.getCurrentLocation().getX()
			<< " y: " << a.getCurrentLocation().getY()
			<< " z: " << a.getCurrentLocation().getZ()
			<< ", over." << endl << endl;

		cout << ", our current position is"
			<< " x: " << a.getCurrentLocation().getX()
			<< " y: " << a.getCurrentLocation().getY()
			<< " z: " << a.getCurrentLocation().getZ()
			<< ", over." << endl << endl;

		break;
	case 2: //Velocity Request

		out << ", our current velocity is"
			<< " vx: " << a.getCircleVelocity().getVx()
			<< " vy: " << a.getCurrentVelocity().getVy()
			<< " vz: " << a.getCurrentVelocity().getVz()
			<< ", over." << endl << endl;

		cout << ", our current velocity is"
			<< " vx: " << a.getCircleVelocity().getVx()
			<< " vy: " << a.getCurrentVelocity().getVy()
			<< " vz: " << a.getCurrentVelocity().getVz()
			<< ", over." << endl << endl;

		break;
	case 3: //Future Position Request

		out << ", our future position at time " << (time + n)
			<< " is x: " << futureLoc.getX()
			<< " y: " << futureLoc.getY()
			<< " z: " << futureLoc.getZ()
			<< ", over." << endl << endl;

		cout << ", our future position at time " << (time + n)
			<< " is x: " << futureLoc.getX()
			<< " y: " << futureLoc.getY()
			<< " z: " << futureLoc.getZ()
			<< ", over." << endl << endl;
		break;

	case 4: //Change altitude Response
		out << " we have received your message and have changed our altitude by " << n << "000 feet, over" << endl << endl;
		cout << " we have received your message and have changed our altitude by " << n << "000 feet, over" << endl << endl;
	}

	out.close(fileAddress);
}

void collisionWarning(Plane a, Plane b) {

	ofstream out;
	out.open(fileAddress);

	out << " ********** SPORADIC COLLISION WARNING AT TIME " << time << " ********** " << endl;
	cout << " ********** SPORADIC COLLISION WARNING AT TIME " << time << " ********** " << endl;

	if (a.getUfo() && b.getUfo()) {
		out << "A possible collision has been detected between UFO " << a.getId() << " and UFO " << b.getId() << endl;
		cout << "A possible collision has been detected between UFO " << a.getId() << " and UFO " << b.getId() << endl;
	}
	else if (a.getUfo()) {
		out << "A possible collision has been detected between UFO " << a.getId() << " and Plane " << b.getId() << endl;
		cout << "A possible collision has been detected between UFO " << a.getId() << " and Plane " << b.getId() << endl;
	}
	else if (b.getUfo()) {
		out << "A possible collision has been detected between Plane " << a.getId() << " and UFO " << b.getId() << endl;
		cout << "A possible collision has been detected between Plane " << a.getId() << " and UFO " << b.getId() << endl;
	}
	else {
		out << "A possible collision has been detected between Plane " << a.getId() << " and Plane " << b.getId() << endl;
		cout << "A possible collision has been detected between Plane " << a.getId() << " and Plane " << b.getId() << endl;
			
	}

	out << "Making Appropriate Flight Path Changes to Avoid Collision" << endl << endl;
	cout << "Making Appropriate Flight Path Changes to Avoid Collision" << endl << endl;


	out.close(fileAddress);

}

void printHitList() {
	ofstream out;
	out.open(fileAddress);

	out << " -------- PERIODIC HIT LIST AT TIME " << time <<" -------- " << endl;
	cout << " -------- PERIODIC HIT LIST AT TIME " << time << " -------- " << endl;
	
	for (int i = 0; i < active.size(); i++) {
		Plane temp = active[i];

		out << "ID: ";
		cout << "ID: ";

		if (temp.getUfo()) {
			out << "Unknown , ";
			cout << "Unknown , ";
		}
		else {
			out << temp.getId() << " : ";
			cout << temp.getId() << " : ";
		}

		out << " x : " << temp.getCurrentLocation().getX() << " , "
			<< " y : " << temp.getCurrentLocation().getY() << " , "
			<< " z : " << temp.getCurrentLocation().getZ() << " , "
			<< endl;

		cout << " x : " << temp.getCurrentLocation().getX() << " , "
			<< " y : " << temp.getCurrentLocation().getY() << " , "
			<< " z : " << temp.getCurrentLocation().getZ() << " , "
			<< endl;

	}

	out << endl;
	cout << endl;

	out.close(fileAddress);
}

void printResponseTimes() {
	
	sort(orderedToReleased.begin(), orderedToReleased.end());
	sort(releasedToActive.begin(), releasedToActive.end());
	sort(activeToDone.begin(), activeToDone.end());
	sort(checkCollisions.begin(), checkCollisions.end());
	sort(updateLocations.begin(), updateLocations.end());
	sort(userConsole.begin(), userConsole.begin());

	clock_t max1 = orderedToReleased.back();
	clock_t min1 = orderedToReleased.front();

	clock_t max2 = releasedToActive.back();
	clock_t min2 = releasedToActive.front();

	clock_t max3 = activeToDone.back();
	clock_t min3 = activeToDone.front();

	clock_t max4 = checkCollisions.back();
	clock_t min4 = checkCollisions.front();

	clock_t max5 = updateLocations.back();
	clock_t min5 = updateLocations.front();

	clock_t max6 = userConsole.back();
	clock_t min6 = userConsole.front();

	ofstream out;
	out.open(fileAddress);

	out << " ---------- RESPONSE TIME FOR PROCESSES GATHERED ---------- " << endl
		<< "1. Ordered To Released	=> Max: " << max1 << " , Min: " << min1 << endl
		<< "2. Released to Active	=> Max: " << max2 << " , Min: " << min2 << endl
		<< "3. Active to Done		=> Max: " << max3 << " , Min: " << min3 << endl
		<< "4. Check Collisions		=> Max: " << max4 << " , Min: " << min4 << endl
		<< "5. Update Locations		=> Max: " << max5 << " , Min: " << min5 << endl
		<< "6. User Console			=> Max: " << max6 << " , Min: " << min6 << endl;

	cout << " ---------- RESPONSE TIME FOR PROCESSES GATHERED ---------- " << endl
		<< "1. Ordered To Released	=> Max: " << max1 << " , Min: " << min1 << endl
		<< "2. Released to Active	=> Max: " << max2 << " , Min: " << min2 << endl
		<< "3. Active to Done		=> Max: " << max3 << " , Min: " << min3 << endl
		<< "4. Check Collisions		=> Max: " << max4 << " , Min: " << min4 << endl
		<< "5. Update Locations		=> Max: " << max5 << " , Min: " << min5 << endl
		<< "6. User Console			=> Max: " << max6 << " , Min: " << min6 << endl;

	out.close(fileAddress);

}



void endClock(int processID) {
	tEnd = clock();
	switch (processID) {
	case 0:
		emptyBlock = tEnd - tStart;
		break;
	case 1:
		orderedToReleased.push_back(tEnd - tStart - emptyBlock);
		break;
	case 2:
		releasedToActive.push_back(tEnd - tStart - emptyBlock);
		break;
	case 3:
		activeToDone.push_back(tEnd - tStart - emptyBlock);
		break;
	case 4:
		checkCollisions.push_back(tEnd - tStart - emptyBlock);
		break;
	case 5:
		updateLocations.push_back(tEnd - tStart - emptyBlock);
		break;
	case 6:
		userConsole.push_back(tEnd - tStart - emptyBlock);
		break;
	}
}

void emptyBlockTest() {
	tStart = clock();
	//Null Program
	endClock(0);
}

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

emptyBlockTest();


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




	tStart = clock();

	//Check when first plane is released and store into Release array
	if (ordered.size() != 0) {
		if (ordered[0].getReleaseTime() >= time) {
			released.push_back(ordered[0]);	//put the plane into release array
			ordered.erase(ordered.begin());	//erase the plane from the ordered array
		}
	}

	endClock(1);

	tStart = clock();

	//Check when plane is active and store into Active zone
	for (int i = 0; i < released.size(); i++) {
		//checks if plane is in the active block
		if (isNeverEntering(released[i])) {
			done.push_back(released[i]);
			released.erase(released.begin() + i);	//erase plane from Released array
		}
		else if (released[i].isInsideTheBlock(released[i].getCurrentLocation(), width, depth, height, 0, 0, 0)) {	
			active.push_back(released[i]);			//put plane into Active array
			receiveBroadcast(released[i]);			//Receive Message from other ATC about new plane
			released.erase(ordered.begin() + i);	//erase plane from Released array
			
		}
	}

	endClock(2);

	tStart = clock();

	//Check when plane is gets out of active zone
	for (int i = 0; i < active.size(); i++) {
		if (!active[i].isInsideTheBlock(active[i].getCurrentLocation(), width, depth, height, 0, 0, 0)) {
			done.push_back(active[i]);			//plane is put into Done array
			broadcast(active[i]);				//send message to next ATC
			active.erase(active.begin() + i);	//erase plane from Active zone
		}
	}

	endClock(3);

	tStart = clock();

	//Check for collisions in active list
	checkForCollision();

	endClock(4);

	tStart = clock();

	//update the location of all planes
	updateLocation();

	endClock(5);
	


printHitList();

printResponseTimes(); // Returns the max and min response time of main processes 

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
	bool x = a.getCurrentLocation().getX() > width && a.getCurrentVelocity().getVx() > 0 || a.getCurrentLocation().getX() < 0 && a.getCurrentVelocity().getVx() < 0;
	bool y = a.getCurrentLocation().getY() > depth && a.getCurrentVelocity().getVy() > 0 || a.getCurrentLocation().getY() < 0 && a.getCurrentVelocity().getVy() < 0;
	bool z = a.getCurrentLocation().getZ() > height && a.getCurrentVelocity().getVz() > 0 || a.getCurrentLocation().getZ() < 0 && a.getCurrentVelocity().getVz() < 0;

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
					collisionWarning(active[i], active[j]);
					if (active[i].getCurrentVelocity().getVz() < active[j].getCurrentVelocity().getVz()) {
						active[j].redirect(active[i]);	//redirects the planes according to their respective velocity
					}
					else {
						active[i].redirect(active[j]);
					}
				}
				while (active[i].collisionCheck(active[j], 2)) { //check if two planes will collide at Time + 2
					collisionWarning(active[i], active[j]);
					if (active[i].getCurrentVelocity().getVz() < active[j].getCurrentVelocity().getVz()) {
						active[j].redirect(active[i]);	//redirects the planes according to their respective velocity
					}
					else {
						active[i].redirect(active[j]);
					}
				}
				while (active[i].collisionCheck(active[j], 3)) { //check if two planes will collide at Time + 3
					collisionWarning(active[i], active[j]);
					if (active[i].getCurrentVelocity().getVz() < active[j].getCurrentVelocity().getVz()) {
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

string getExitDirection(Plane a) {
	string answer = "";
	int eastDiff = abs(a.getCurrentLocation().getX() - width);
	int westDiff = abs(a.getCurrentLocation().getX());
	int northDiff = abs(a.getCurrentLocation().getY() - depth);
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


void menu() {
	cout << "************  Menu  ************" << endl;
	cout << "Do you want to : \na) Change altitude by n*1000 ft\n";
	cout << "b) Increase or decrease speed by a factor\n";
	cout << "c) Change direction in horizontal plane\n";
	cout << "d) Enter or leave a holding pattern\n";
	cout << "e) Report current position and velocity\n";
	cout << "f) Add or delete an aircraft\n";
	cout << "g) Change position of aircraft\n";
	cout << "h) Display data record for a given aircraft\n";
	cout << "i) Project aircraft positions in future\n";
}

void choice(Plane a, char choice) {
	switch (choice) {
	case 'a':
		cout << "By how much do you want to change the altitude?\n";
		int altitude;
		cin >> altitude;
		while (a.collisionCheck(a, 1)) { //Check for collisions
			cout << "Please enter another value that won't create any collision\n";
			cin >> altitude;
		}
		a.setCurrentPosition(a.getCurrentLocation().getX(), a.getCurrentLocation().getY(), a.getCurrentLocation().getZ() + altitude);
		break;
	case 'b':
		double speed;
		cout << "By what factor do you wish to change the speed?";
		cin >> speed;
		while (a.collisionCheck(a, 1)) { //Check for collisions
			cout << "Please enter another value that won't create any collision\n";
			cin >> speed;
		}
		a.setCurrentVelocity(a.getCurrentVelocity().getVx()*speed, a.getCurrentVelocity().getVy()*speed, a.getCurrentVelocity().getVz()*speed);
		break;

	case 'c':
		int x, y;
		double tempMagnitude;
		cout << "Please input the x direction: ";
		cin >> x;
		cout << "Please input the y direction: ";
		cin >> y;
	
		tempMagnitude = sqrt(pow(x, 2) + pow(y, 2));
		a.setCurrentVelocity(x / tempMagnitude * a.getMagnitudeVelocity(), y / tempMagnitude * a.getMagnitudeVelocity(), a.getCurrentVelocity().getVz());
		break;
	case 'd':
		a.toggleHoldingPattern();
		break;
	case 'e':
		a.print();
		break;
	case 'f':
		bool add;
		cout << "Enter 1. Add Plane or 0. Delete Plane";
		cin >> add;
		if (add) {
			//make a plane and insert it into ordered array according to release time
		}
		else {
			int id;
			cout << "Please enter the id of the plane you wish to delete.\n";
			cin >> id;
			for (int p = 0; p < active.size(); p++) {
				if (active[p].getId() == id) {
					cout << "Plane " << id << "was deleted sucessfully.\n";
					active.erase(ordered.begin() + p);	//erase the plane from the active array
					break;
				}
			}
			
			cout << "Plane " << id << " is not in the active block.";
			
		}
		break;
	case 'g':
		int tempX, tempY, tempZ;
		cout << "Please input the x location: ";
		cin >> tempX;
		cout << "Please input the y location: ";
		cin >> tempY;
		cout << "Please input the z location: ";
		cin >> tempZ;
		a.setCurrentPosition(tempX, tempY, tempZ);
		break;
	case 'h':
		a.print(); //NEED TO PRINT RECORD OF PAST LOCATIONS *******
		break;
	case 'i':
		int time;
		Location future;
		cout << "At what time do you want to project the future location of the aircraft?\n";
		cin >> time;
		future = a.getFutureLocation(time);
		cout << "Plane " << a.getId() << " will be at location ( " << future.getX() << " , " << future.getY() << " , " << future.getZ() << " )\n";
		break;
	default:
		cout << "Please enter a valid option.\n";
		break;
	}
}

