//============================================================================
// Name        : ATC_Project.cpp
// Author      : Jonathan Cournoyer - Aida Bumbu
// Version     :
// Description : Build a Real-Time
//============================================================================


#include <vector>
#include <iostream>
#include <pthread.h>
#include <time.h>
#include <thread>
#include <fstream>
#include <string>
#include <algorithm>
#include <cmath>

/*
 * Plane.h
 *
 *  Created on: Nov. 14, 2018
 *      Author: Administrateur
 */

using namespace std;

string fileAddress = "TrackFile.txt";
int t = 0;

const int height = 25000;
const int width = 100000;
const int depth = 100000;

clock_t tStart;
clock_t tEnd;

clock_t emptyBlock;
vector<clock_t> orderedToReleased;
vector<clock_t> releasedToActive;
vector<clock_t> activeToDone;
vector<clock_t> checkCollisions;
vector<clock_t> updateLocations;
vector<clock_t> userConsole;

static int ufoId = 0;

void updateLocation();

void checkForCollision();
void printStatus();
void checkForCollision();


class Velocity {
public:

	int vx;
	int vy;
	int vz;


	Velocity() {}


	Velocity(int vx, int vy, int vz) {
		this->vx = vx;
		this->vy = vy;
		this->vz = vz;
	}

	virtual ~Velocity() {}


	int getVx() const {
		return vx;
	}

	int getVy() const {
		return vy;
	}

	int getVz() const {
		return vz;
	}

	void print() {

		cout << "vx : " << vx << " , vy : " << vy << ", vz : " << vz;

	}

};


class Location {

	int x;
	int y;
	int z;

public:

	Location() {}

	Location(int x, int y, int z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}

	virtual ~Location() {}

	int distanceBetween(Location a) {
		double result = sqrt(pow(getX() - a.getX(), 2) + pow(getY() - a.getY(), 2) + pow(getZ() - a.getZ(), 2));
		return round(result);
	}

	int getX() const {
		return x;
	}

	int getY() const {
		return y;
	}

	int getZ() const {
		return z;
	}

	void updatePosititon(Velocity a) {

		setPosition(x + a.getVx(), y + a.getVy(), z + a.getVz());

	}

	void setPosition(int x, int y, int z) {
		this->x = x;
		this->y = y;
		this->y = z;
	}

	void print() {

		cout << "x : " << x << " , y : " << y << " , z : " << z << " , ";

	}

};


class Plane {
private:

	int id;
	int releaseTime;

	//limit set to check for collisions
	int heightLimit = 1000;
	int sideLimit = 3000;

	double magnetudeOfVelocity;
	double circleRadius;

	Location spawnLocation;
	Location wantedLocation;
	Location currentLocation;

	Velocity currentVelocity;

	bool ufo = false;
	bool isHolding = false;

public:

	Plane(int id, int vx, int vy, int vz, int x, int y, int z, int releaseTime) {

		setId(id);
		this->currentVelocity = Velocity(vx, vy, vz);
		this->currentLocation = Location(x, y, z);
		this->spawnLocation = Location(x, y, z);
		this->releaseTime = releaseTime;
	}

	Plane() {

	}


	~Plane() {

	}

	void setId(int id) {
		if (id == -1) {
			this->ufo = true;
			this->id = ufoId++;
		}
		else {
			this->ufo = false;
			this->id = id;
		}
	}

	void setReleaseTime(int releaseTime) {
		this->releaseTime = releaseTime;
	}

	void setCurrentPosition(int x, int y, int z) {
		this->currentLocation = Location(x, y, z);
	}

	void setCurrentVelocity(int x, int y, int z) {
		this->currentVelocity = Velocity(x, y, z);
	}

	Location getCurrentLocation() const {
		return currentLocation;
	}

	Velocity getCurrentVelocity() const {
		return currentVelocity;
	}

	int getReleaseTime() const {
		return releaseTime;
	}

	Location getSpawnLocation() const {
		return spawnLocation;
	}

	Location getWantedLocation() const {
		return wantedLocation;
	}

	int getId() const {
		return id;
	}

	Location getFutureLocation(int time) {
		int x = currentLocation.getX() + time * currentVelocity.getVx();
		int y = currentLocation.getY() + time * currentVelocity.getVy();
		int z = currentLocation.getZ() + time * currentVelocity.getVz();
		return Location(x, y, z);
	}

	void setWantedLocation() {
		int i = 1;
		while (isInsideTheBlock(getFutureLocation(i), 100000, 100000, 25000, 0, 0, 0)) {
			i++;
		}
		this->wantedLocation = getFutureLocation(i);
	}

	bool getUfo() {
		return ufo;
	}

	bool isInsideTheBlock(Location location, int maxX, int maxY, int maxZ, int minX, int minY, int minZ) {

		bool withinX = location.getX() <= maxX && location.getX() >= minX;
		bool withinY = location.getY() <= maxY && location.getY() >= minY;
		bool withinZ = location.getZ() <= maxZ && location.getZ() >= minZ;

		return withinX && withinY && withinZ;
	}


	//checks for future collisions
	bool collisionCheck(Plane a, int time) {
		Location plane1FL = getFutureLocation(time);
		Location plane2FL = a.getFutureLocation(time);

		return isInsideTheBlock(plane2FL, plane1FL.getX() + sideLimit, plane1FL.getY() + sideLimit, plane1FL.getZ() + heightLimit, plane1FL.getX() - sideLimit, plane1FL.getY() - sideLimit, plane1FL.getZ() - heightLimit);
	}

	//redirect a plane in case of future collision
	void redirect(Plane planeGoDown) {
		setCurrentVelocity(currentVelocity.getVx(), currentVelocity.getVy(), currentVelocity.getVz() + 10);
		planeGoDown.setCurrentVelocity(planeGoDown.getCurrentVelocity().getVx(), planeGoDown.getCurrentVelocity().getVy(), planeGoDown.getCurrentVelocity().getVz() - 10);
		setWantedLocation();				//set the exit location
		planeGoDown.setWantedLocation();	//set the exit location
	}

	void toggleHoldingPattern() {
		if (!isHolding) {
			isHolding = true;
			magnetudeOfVelocity = sqrt(pow(currentVelocity.getVx(), 2) + pow(currentVelocity.getVy(), 2) + pow(currentVelocity.getVz(), 2)); //Magnetude of the current Velocity
			circleRadius = sqrt(pow(50000 - currentLocation.getX(), 2) + pow(50000 - currentLocation.getY(), 2)); // The radius of the circle around the center
			currentVelocity = getCircleVelocity();
		}
		else {
			isHolding = false;
			currentVelocity = goBackToNormal();
		}
	}

	//updates the current location of the plane
	void updateLocation() {
		if (!isHolding) {
			currentLocation = getFutureLocation(1);
		}
		else {
			currentLocation = getFutureLocation(1);
			currentVelocity = getCircleVelocity();
		}
	}

	Velocity goBackToNormal() {

		double vectorMagn = sqrt(pow(wantedLocation.getX() - currentLocation.getX(), 2) + pow(wantedLocation.getY() - currentLocation.getY(), 2) + pow(wantedLocation.getZ() - currentLocation.getZ(), 2));
		double xVel = magnetudeOfVelocity * (wantedLocation.getX() - currentLocation.getX()) / vectorMagn;
		double yVel = magnetudeOfVelocity * (wantedLocation.getY() - currentLocation.getY()) / vectorMagn;
		double zVel = magnetudeOfVelocity * (wantedLocation.getZ() - currentLocation.getZ()) / vectorMagn;

		return Velocity(xVel, yVel, zVel); //Reset the velocity to go to destination

	}

	Velocity getCircleVelocity() { // Calculates the velcity vector according to the tangent of the circle wanted

		double xVel = magnetudeOfVelocity * ((currentLocation.getY() - 50000) / circleRadius);
		double yVel = -magnetudeOfVelocity * ((currentLocation.getX() - 50000) / circleRadius);

		return Velocity(xVel, yVel, 0);
	}

	void print() {
		cout << "ID : ";
		if (ufo) {
			cout << "Unknown , ";
		}
		else {
			cout << id << " , ";
		}
		currentLocation.print();
		currentVelocity.print();
		cout << endl;
	}

};

vector<Plane> ordered;	//contains planes ordered by released time
vector<Plane> released;	//contains planes that are released but not yet in active zone
vector<Plane> active;	//contains planes that are in active zone
vector<Plane> done;		//contains planes that left the active zone or planes that will never get into the active zone

bool isNeverEntering(Plane a);
string getExitDirection(Plane a);


void broadcast(Plane a) {

	ofstream out;
	out.open(fileAddress);

	out << " -------- SPORADIC ATC BROADCAST AT TIME " << t << " ---------" << endl << "Plane ";
	cout << " -------- SPORADIC ATC BROADCAST AT TIME " << t << " ---------" << endl << "Plane ";

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

	out.close();
}

void receiveBroadcast(Plane a) {

	ofstream out;
	out.open(fileAddress);

	out << " -------- SPORADIC ATC BROADCAST RECEPTION AT TIME " << t << " ---------" << endl << "A Plane ";
	cout << " -------- SPORADIC ATC BROADCAST RECEPTION AT TIME " << t << " ---------" << endl << "A Plane ";

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
		out << ",ATC requests an elevation change of " << n << "000, over" << endl << endl;
		cout << ",ATC requests an elevation change of " << n << "000, over" << endl << endl;
	}



	out.close();
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
		out << " we have received your message and have changed our altitude by " << n << "000, over" << endl << endl;
		cout << " we have received your message and have changed our altitude by " << n << "000, over" << endl << endl;
	}

	out.close();
}

void collisionWarning(Plane a, Plane b) {

	ofstream out;
	out.open(fileAddress);

	out << " ********** SPORADIC COLLISION WARNING AT TIME " << t << " ********** " << endl;
	cout << " ********** SPORADIC COLLISION WARNING AT TIME " << t << " ********** " << endl;

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


	out.close();

}

void printHitList() {
	ofstream out;
	out.open(fileAddress);

	out << " -------- PERIODIC HIT LIST AT TIME " << t << " -------- " << endl;
	cout << " -------- PERIODIC HIT LIST AT TIME " << t << " -------- " << endl;

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

	out.close();
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

	out.close();

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
	};
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
	for (int i = 0; i < sizeof(airplane_schedule) / sizeof(*airplane_schedule); i += 8) {


		//create planes and set their values
		Plane plane = Plane();
		plane.setId(airplane_schedule[i]);
		plane.setCurrentVelocity(airplane_schedule[i + 1], airplane_schedule[i + 2], airplane_schedule[i + 3]);
		plane.setCurrentPosition(airplane_schedule[i + 4], airplane_schedule[i + 5], airplane_schedule[i + 6]);
		plane.setReleaseTime(airplane_schedule[i + 7]);

		//Put plane into ordered vector

		if (ordered.size() == 0) {			//if ordered vector is empty put the plane inside
			ordered.push_back(plane);
		}
		else {								//else go through ordered vector and put it at its right position
			int size = ordered.size();
			for (int p = 0; p < size; p++) {
				if (plane.getReleaseTime() < ordered[p].getReleaseTime()) {
					ordered.insert(p + ordered.begin(), plane);
					break;
				}
				else if (p == size - 1) {		//if release time is larger than the last plane release time
					ordered.push_back(plane);
				}
			}
		}
	}	//finished sorting the planes in ordered list


	//while (done.size() < sizeof(airplane_schedule) / sizeof(*airplane_schedule) / 8) {	//while time is running and planes are not done

	tStart = clock();

	//Check when first plane is released and store into Release array
	if (ordered.size() != 0) {
		if (ordered[0].getReleaseTime() >= t) {
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

	//}

	printHitList();

	printResponseTimes(); // Returns the max and min response time of main processes

	system("pause");
	return 0;
}


//prints the status of the planes that are Active
//void printStatus() {
//
//	for (int p = 0; p < active.size(); p++) {
//		active[p].print();
//		cout << endl;
//	}
//}

//checks if plane never enters the active zone
bool isNeverEntering(Plane a) {
	bool x = (a.getCurrentLocation().getX() > width && a.getCurrentVelocity().getVx() > 0) || a.getCurrentLocation().getX() < 0 && a.getCurrentVelocity().getVx() < 0;
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

//PARAMETERS OF PROJECT
// (ID, Vx, Vy, Vz, X, Y, Z, Release time)
// Height of block : 25000
// Width and length of block : 100000


