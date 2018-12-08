//============================================================================
// Name        : ATC_Project.cpp
// Author      : Jonathan Cournoyer - Aida Bumbu
// Version     :
// Description : Build a Real-Time
//============================================================================


#include <vector>
#include <iostream>
#include <time.h>
#include <thread>
#include <chrono>
#include <functional>
#include <fstream>
#include <string>
#include <unistd.h>
#include <string.h>
#include <algorithm>
#include <cmath>
#include <mutex>

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

chrono::steady_clock::time_point tStart;

double emptyBlock;
vector<double> orderedToReleased;
vector<double> releasedToActive;
vector<double> activeToDone;
vector<double> checkCollisions;
vector<double> updateLocations;

vector<string> fileOutput;

static int ufoId = 0;

void updateLocation();
void checkForCollision();
chrono::steady_clock::time_point beginTime;

mutex infoMtxLock;

ofstream out;

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

	int getX() const {
		return x;
	}

	int getY() const {
		return y;
	}

	int getZ() const {
		return z;
	}

	void setPosition(int x, int y, int z) {
		this->x = x;
		this->y = y;
		this->y = z;
	}

};


class Plane {
private:

	int id;
	int releaseTime;

	//limit set to check for collisions
	int heightLimit = 1000;
	int sideLimit = 3000;

	double magnitudeOfVelocity;
	double circleRadius;

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
		this->releaseTime = releaseTime;
		magnitudeOfVelocity = sqrt(vx*vx + vy * vy + vz * vz);
	}

	Plane() {

	}


	~Plane() {

	}
	double getMagnitudeVelocity() const {
		return magnitudeOfVelocity;
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

	void setCurrentVelocity(int vx, int vy, int vz) {
		this->currentVelocity = Velocity(vx, vy, vz);
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
		setCurrentVelocity(currentVelocity.getVx(), currentVelocity.getVy(), currentVelocity.getVz() + 50);
		planeGoDown.setCurrentVelocity(planeGoDown.getCurrentVelocity().getVx(), planeGoDown.getCurrentVelocity().getVy(), planeGoDown.getCurrentVelocity().getVz() - 50);
		setWantedLocation();				//set the exit location
		planeGoDown.setWantedLocation();	//set the exit location
	}

	void toggleHoldingPattern() {
		if (!isHolding) {
			isHolding = true;
			magnitudeOfVelocity = sqrt(pow(currentVelocity.getVx(), 2) + pow(currentVelocity.getVy(), 2) + pow(currentVelocity.getVz(), 2)); //Magnitude of the current Velocity
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
		double xVel = magnitudeOfVelocity * (wantedLocation.getX() - currentLocation.getX()) / vectorMagn;
		double yVel = magnitudeOfVelocity * (wantedLocation.getY() - currentLocation.getY()) / vectorMagn;
		double zVel = magnitudeOfVelocity * (wantedLocation.getZ() - currentLocation.getZ()) / vectorMagn;

		return Velocity(xVel, yVel, zVel); //Reset the velocity to go to destination

	}

	Velocity getCircleVelocity() { // Calculates the velocity vector according to the tangent of the circle wanted

		double xVel = magnitudeOfVelocity * ((currentLocation.getY() - 50000) / circleRadius);
		double yVel = -magnitudeOfVelocity * ((currentLocation.getX() - 50000) / circleRadius);

		return Velocity(xVel, yVel, 0);
	}


};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

vector<Plane> ordered;	//contains planes ordered by released time
vector<Plane> released;	//contains planes that are released but not yet in active zone
vector<Plane> active;	//contains planes that are in active zone
vector<Plane> done;		//contains planes that left the active zone or planes that will never get into the active zone

bool isNeverEntering(Plane a);
string getExitDirection(Plane a);

int getTime() {
	auto currentTime = chrono::steady_clock::now();
	chrono::duration<double> diff = currentTime - beginTime;
	return diff.count();
}



void broadcast(Plane a) {

	int diffTime = getTime();


	fileOutput.push_back(" -------- SPORADIC ATC BROADCAST AT TIME " + to_string(diffTime) + " ---------\n" "Plane ");
	cout << " -------- SPORADIC ATC BROADCAST AT TIME " << diffTime << " ---------" << endl << "Plane ";

	if (a.getUfo()) {
		fileOutput.push_back("with unknown ID ");
		cout << "with unknown ID ";
	}
	else {
		fileOutput.push_back(a.getId() + " ");
		cout << a.getId() << " ";
	}

	string outDirection = getExitDirection(a);

	fileOutput.push_back("has exited our airspace towards the " + outDirection + " Sector, over.\n\n");
	cout << "has exited our airspace towards the " << outDirection << " Sector, over." << endl << endl;
}

void receiveBroadcast(Plane a) {
	double diffTime = getTime();


	fileOutput.push_back(" -------- SPORADIC ATC BROADCAST RECEPTION AT TIME " + to_string(diffTime) + " ---------\n A Plane ");
	cout << " -------- SPORADIC ATC BROADCAST RECEPTION AT TIME " << diffTime << " ---------" << endl << "A Plane ";

	if (a.getUfo()) {
		fileOutput.push_back("with unknown ID ");
		cout << "with unknown ID ";
	}
	else {
		fileOutput.push_back(to_string(a.getId()) + " ");
		cout << a.getId() << " ";
	}

	string outDirection = getExitDirection(a);

	fileOutput.push_back("has entered your airspace at the position x: " + to_string(a.getCurrentLocation().getX())
		+ " y: " + to_string(a.getCurrentLocation().getY())
		+ " z: " + to_string(a.getCurrentLocation().getZ())
		+ ", over.\n\n");
	cout << "has entered the airspace towards your airspace at the position"
		<< " x: " << a.getCurrentLocation().getX()
		<< " y: " << a.getCurrentLocation().getY()
		<< " z: " << a.getCurrentLocation().getZ()
		<< ", over." << endl << endl;


}


void request(Plane &a, int messageType, int n) {
	double diffTime = getTime();
	while (!infoMtxLock.try_lock()) {
	}

	fileOutput.push_back(" -------- SPORADIC ATC TO PLANE REQUEST AT TIME " + to_string(diffTime) + " ---------\n");
	cout << " -------- SPORADIC ATC TO PLANE REQUEST AT TIME " << diffTime << " ---------" << endl;

	if (a.getUfo()) {
		fileOutput.push_back("UFO " + to_string(a.getId()));
		cout << "UFO " << a.getId();
	}
	else {
		fileOutput.push_back("Plane " + to_string(a.getId()));
		cout << "Plane " << a.getId();
	}

	switch (messageType) {
	case 1: //Location Request

		fileOutput.push_back(",ATC requests your current position, over.");
		cout << ",ATC requests your current position, over." << endl << endl;

		break;
	case 2: //Velocity Request

		fileOutput.push_back(",ATC requests your current velocity, over.");
		cout << ",ATC requests your current velocity, over." << endl << endl;

		break;

	case 3: //Future Position Request

		fileOutput.push_back(",ATC requests your future position at time " + to_string(diffTime + n) + ", over");
		cout << ",ATC requests your future position at time " << (diffTime + n) << ", over" << endl << endl;

		break;
	case 4: //Elevation Change request
		fileOutput.push_back(",ATC requests an elevation change of " + to_string(n) + "000, over");
		cout << ",ATC requests an elevation change of " << n << "000, over" << endl << endl;
	}

	infoMtxLock.unlock();
}

void response(Plane &a, int messageType, int n) {
	double diffTime = getTime();
	while (!infoMtxLock.try_lock()) {
	}

	fileOutput.push_back("-------- SPORADIC PLANE RESPONSE AT TIME " + to_string(diffTime) + " ---------");
	cout << " -------- SPORADIC PLANE RESPONSE AT TIME " << diffTime << " ---------" << endl;

	Location futureLoc = a.getFutureLocation(n);

	if (a.getUfo()) {
		fileOutput.push_back("This is UFO " + a.getId());
		cout << "This is UFO " << a.getId();
	}
	else {
		fileOutput.push_back("This is Plane " + a.getId());
		cout << "This is Plane " << a.getId();
	}

	switch (messageType) {
	case 1: //Location Request

		fileOutput.push_back(", our current position is x: " + to_string(a.getCurrentLocation().getX())
			+ " y: " + to_string(a.getCurrentLocation().getY())
			+ " z: " + to_string(a.getCurrentLocation().getZ())
			+ ", over.");

		cout << ", our current position is"
			<< " x: " << a.getCurrentLocation().getX()
			<< " y: " << a.getCurrentLocation().getY()
			<< " z: " << a.getCurrentLocation().getZ()
			<< ", over." << endl << endl;

		break;
	case 2: //Velocity Request

		fileOutput.push_back(", our current velocity is vx: " + to_string(a.getCircleVelocity().getVx())
			+ " vy: " + to_string(a.getCurrentVelocity().getVy())
			+ " vz: " + to_string(a.getCurrentVelocity().getVz())
			+ ", over.");

		cout << ", our current velocity is"
			<< " vx: " << a.getCurrentVelocity().getVx()
			<< " vy: " << a.getCurrentVelocity().getVy()
			<< " vz: " << a.getCurrentVelocity().getVz()
			<< ", over." << endl << endl;

		break;
	case 3: //Future Position Request

		fileOutput.push_back(", our future position at time " + to_string(diffTime + n)
			+ " is x: " + to_string(futureLoc.getX())
			+ " y: " + to_string(futureLoc.getY())
			+ " z: " + to_string(futureLoc.getZ())
			+ ", over.");

		cout << ", our future position at time " << (diffTime + n)
			<< " is x: " << futureLoc.getX()
			<< " y: " << futureLoc.getY()
			<< " z: " << futureLoc.getZ()
			<< ", over." << endl << endl;
		break;
	case 4: //Change altitude Response
		a.setCurrentPosition(a.getCurrentLocation().getX(), a.getCurrentLocation().getY(), a.getCurrentLocation().getZ() + n * 1000);
		fileOutput.push_back(" We have received your message and have changed our altitude by " + to_string(n) + "000, over");
		cout << " We have received your message and have changed our altitude by " << n << "000, over" << endl << endl;
	}

	infoMtxLock.unlock();
}

void collisionWarning(Plane a, Plane b) {
	double diffTime = getTime();

	fileOutput.push_back(" ********** SPORADIC COLLISION WARNING AT TIME " + to_string(diffTime) + " ********** ");
	cout << " ********** SPORADIC COLLISION WARNING AT TIME " << diffTime << " ********** " << endl;

	if (a.getUfo() && b.getUfo()) {
		fileOutput.push_back("A possible collision has been detected between UFO " + to_string(a.getId()) + " and UFO " + to_string(b.getId()));
		cout << "A possible collision has been detected between UFO " << a.getId() << " and UFO " << b.getId() << endl;
	}
	else if (a.getUfo()) {
		fileOutput.push_back("A possible collision has been detected between UFO " + to_string(a.getId()) + " and UFO " + to_string(b.getId()));
		cout << "A possible collision has been detected between UFO " << a.getId() << " and Plane " << b.getId() << endl;
	}
	else if (b.getUfo()) {
		fileOutput.push_back("A possible collision has been detected between Plane " + to_string(a.getId()) + " and UFO " + to_string(b.getId()));
		cout << "A possible collision has been detected between Plane " << a.getId() << " and UFO " << b.getId() << endl;
	}
	else {
		fileOutput.push_back("A possible collision has been detected between Plane " + to_string(a.getId()) + " and UFO " + to_string(b.getId()));
		cout << "A possible collision has been detected between Plane " << a.getId() << " and Plane " << b.getId() << endl;

	}

	fileOutput.push_back("Making Appropriate Flight Path Changes to Avoid Collision");
	cout << "Making Appropriate Flight Path Changes to Avoid Collision" << endl << endl;


}

void printHitList() {
	double diffTime = getTime();
	while (!infoMtxLock.try_lock()) {
	}


	fileOutput.push_back(" -------- PERIODIC HIT LIST AT TIME " + to_string(diffTime) + " -------- \n");
	cout << " -------- PERIODIC HIT LIST AT TIME " << diffTime << " -------- " << endl;

	for (unsigned int i = 0; i < active.size(); i++) {
		Plane temp = active[i];

		fileOutput.push_back("ID: ");
		cout << "ID: ";

		if (temp.getUfo()) {
			fileOutput.push_back("Unknown , ");
			cout << "Unknown , ";
		}
		else {
			fileOutput.push_back(temp.getId() + " : ");
			cout << temp.getId() << " : ";
		}

		fileOutput.push_back(" x : " + to_string(temp.getCurrentLocation().getX()) + " , "
			+ " y : " + to_string(temp.getCurrentLocation().getY()) + " , z : " + to_string(temp.getCurrentLocation().getZ()) + " , ");

		cout << " x : " << temp.getCurrentLocation().getX() << " , "
			<< " y : " << temp.getCurrentLocation().getY() << " , "
			<< " z : " << temp.getCurrentLocation().getZ()
			<< endl;

	}

	fileOutput.push_back("\n");
	cout << endl;

	infoMtxLock.unlock();
}

void printResponseTimes() {
	if ((orderedToReleased.size() > 1) & (releasedToActive.size() > 1) & (activeToDone.size() > 1) & (checkCollisions.size() > 1)  & (updateLocations.size() > 1)) {

		sort(orderedToReleased.begin(), orderedToReleased.end());
		sort(releasedToActive.begin(), releasedToActive.end());
		sort(activeToDone.begin(), activeToDone.end());
		sort(checkCollisions.begin(), checkCollisions.end());
		sort(updateLocations.begin(), updateLocations.end());

		double max1 = orderedToReleased.back();
		double min1 = orderedToReleased.front();

		double max2 = releasedToActive.back();
		double min2 = releasedToActive.front();

		double max3 = activeToDone.back();
		double min3 = activeToDone.front();

		double max4 = checkCollisions.back();
		double min4 = checkCollisions.front();

		double max5 = updateLocations.back();
		double min5 = updateLocations.front();


		fileOutput.push_back(" ---------- RESPONSE TIME FOR PROCESSES GATHERED ---------- \n1. Ordered To Released	=> Max: " + to_string(max1) + " , Min: " + to_string(min1)
			+ "\n2. Released to Active	=> Max: " + to_string(max2) + " , Min: " + to_string(min2)
			+ "\n3. Active to Done		=> Max: " + to_string(max3) + " , Min: " + to_string(min3)
			+ "\n4. Check Collisions		=> Max: " + to_string(max4) + " , Min: " + to_string(min4)
			+ "\n5. Update Locations		=> Max: " + to_string(max5) + " , Min: " + to_string(min5));

		cout << " ---------- RESPONSE TIME FOR PROCESSES GATHERED ---------- " << endl
			<< "1. Ordered To Released	=> Max: " << max1 << " , Min: " << min1 << endl
			<< "2. Released to Active	=> Max: " << max2 << " , Min: " << min2 << endl
			<< "3. Active to Done		=> Max: " << max3 << " , Min: " << min3 << endl
			<< "4. Check Collisions		=> Max: " << max4 << " , Min: " << min4 << endl
			<< "5. Update Locations		=> Max: " << max5 << " , Min: " << min5 << endl;

	}

}

void endClock(int processID) {
	auto tEnd = chrono::steady_clock::now();
	chrono::duration<double> diff = tEnd - tStart;
	switch (processID) {
	case 0:
		emptyBlock = diff.count();
		break;
	case 1:
		orderedToReleased.push_back(diff.count());
		break;
	case 2:
		releasedToActive.push_back(diff.count());
		break;
	case 3:
		activeToDone.push_back(diff.count());
		break;
	case 4:
		checkCollisions.push_back(diff.count());
		break;
	case 5:
		updateLocations.push_back(diff.count());
		break;
	};
}

void emptyBlockTest() {
	tStart = chrono::steady_clock::now();
	//Null Program
	endClock(0);
}

void timer_start(function<void(void)> func, unsigned int interval) {
	thread([func, interval]()
	{
		while (true) {
			auto x = chrono::steady_clock::now() + chrono::milliseconds(interval);
			func();
			this_thread::sleep_until(x);
		}
	}).detach();
}

//checks if plane never enters the active zone
bool isNeverEntering(Plane a) {
	bool x = (a.getCurrentLocation().getX() > width && a.getCurrentVelocity().getVx() > 0) || (a.getCurrentLocation().getX() < 0 && a.getCurrentVelocity().getVx() < 0);
	bool y = (a.getCurrentLocation().getY() > depth && a.getCurrentVelocity().getVy() > 0) || (a.getCurrentLocation().getY() < 0 && a.getCurrentVelocity().getVy() < 0);
	bool z = (a.getCurrentLocation().getZ() > height && a.getCurrentVelocity().getVz() > 0) || (a.getCurrentLocation().getZ() < 0 && a.getCurrentVelocity().getVz() < 0);

	return x || y || z;
}

void checkForCollision() {
	if (active.size() < 2) { //No need to check for collisions if less than two planes inside active area
		return;
	}
	else {		//go through active array and check for collisions
		for (unsigned int i = 0; i < active.size() - 1; i++) {
			for (unsigned int j = i + 1; j < active.size(); j++) {
				bool messageSent = false;
				while (active[i].collisionCheck(active[j], 1)) { //check if two planes will collide at Time + 1
					if (!messageSent) {
						collisionWarning(active[i], active[j]);
						messageSent = true;
					}
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
	while (!infoMtxLock.try_lock()) {
	}

	for (auto& plane : released) {
		plane.updateLocation();
	}
	for (auto& plane : active) {
		plane.updateLocation();
	}
	infoMtxLock.unlock();

}

bool planeIsActive(int id) {
	for (int i = 0; i < active.size(); i++) {
		if (active[i].getId() == id) {
			return true;
		}
	}
	return false;
}


Plane& getPlaneById(int id) {
	for (int i = 0; i < active.size(); i++) {
		if (active[i].getId() == id) {
			return active[i];
		}
	}
	cout << "Plane " << id << " is not in the active block.\n";
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
	cout << "h) Project aircraft positions in future\n";
}

void choice(Plane a, char Choice) {

	Location future;
	switch (Choice) {
	case 'm':
	{
		menu();
		break;
	}
	case 'x':
	{
		break;
	}

	case 'a':
	{
		cout << "By how much do you want to change the altitude?\n";
		int altitude;
		cin >> altitude;
		while (a.collisionCheck(a, 1)) { //Check for collisions
			cout << "Please enter another value that won't create any collision\n";
			cin >> altitude;
		}
		request(a, 4, altitude);
		response(a, 4, altitude);
		break;
	}
	case 'b':
	{
		double speed;
		cout << "By what factor do you wish to change the speed?";
		cin >> speed;
		while (a.collisionCheck(a, 1)) { //Check for collisions
			cout << "Please enter another value that won't create any collision\n";
			cin >> speed;
		}
		a.setCurrentVelocity(a.getCurrentVelocity().getVx()*speed, a.getCurrentVelocity().getVy()*speed, a.getCurrentVelocity().getVz()*speed);
		response(a, 2, 1);
		break;
	}
	case 'c':
	{
		int x, y;
		double tempMagnitude;
		cout << "Please input the x direction: ";
		cin >> x;
		cout << "Please input the y direction: ";
		cin >> y;

		tempMagnitude = sqrt(pow(x, 2) + pow(y, 2));
		a.setCurrentVelocity(x / tempMagnitude * a.getMagnitudeVelocity(), y / tempMagnitude * a.getMagnitudeVelocity(), a.getCurrentVelocity().getVz());
		response(a, 2, 1);
		break;
	}
	case 'd':
		a.toggleHoldingPattern();
		break;
	case 'e':
		response(a, 2, 1);
		response(a, 1, 1);
		break;
	case 'f':
	{
		bool add;
		cout << "Enter 1. Add Plane or 0. Delete Plane";
		cin >> add;
		if (add) {
			int positionX, positionY, positionZ;
			int velocityX, velocityY, velocityZ;
			int releaseTime;

			cout << "Please insert the following positions. \n";
			cout << "Position x: ";
			cin >> positionX;
			cout << "Position y: ";
			cin >> positionY;
			cout << "Position z: ";
			cin >> positionZ;
			cout << endl;
			cout << "Please enter velocities: \n";
			cout << "Velocity x: ";
			cin >> velocityX;
			cout << "Velocity y: \n";
			cin >> velocityY;
			cout << "Velocity z: \n";
			cin >> velocityZ;
			cout << endl;
			cout << "input release time: \n";
			cin >> releaseTime;
			Plane plane = Plane(-1, velocityX, velocityY, velocityZ, positionX, positionY, positionZ, releaseTime);
			if (isNeverEntering(plane)) {
				done.push_back(plane);
			}
			else if (releaseTime < getTime()) {
				for (unsigned int p = 0; p < ordered.size(); p++) {
					if (releaseTime < ordered[p].getReleaseTime()) {
						ordered.insert(p + ordered.begin(), plane);
						break;
					}
					else if (p == ordered.size() - 1) {		//if release time is larger than the last plane release time
						ordered.push_back(plane);
						cout << "Plane added successfuly.\n";
					}
				}
			}
			else if (plane.isInsideTheBlock(plane.getCurrentLocation(), 100000, 100000, 25000, 0, 0, 0)) {
				active.push_back(plane);
				cout << "Plane added successfuly.\n";
			}
			else {
				released.push_back(plane);
				cout << "Plane added successfuly.\n";
			}
		}
		else {
			int id;
			cout << "Please enter the id of the plane you wish to delete.\n";
			cin >> id;
			for (int p = 0; p < active.size(); p++) {
				if (active[p].getId() == id) {
					cout << "Plane " << id << "was deleted successfully.\n";
					active.erase(ordered.begin() + p);	//erase the plane from the active array
					break;
				}
			}
			cout << "Plane " << id << " is not in the active block.";
		}
		break;
	}
	case 'g':
	{
		int tempX, tempY, tempZ;
		cout << "Please input the x location: ";
		cin >> tempX;
		cout << "Please input the y location: ";
		cin >> tempY;
		cout << "Please input the z location: ";
		cin >> tempZ;
		a.setCurrentPosition(tempX, tempY, tempZ);
		break;
	}
	case 'h':
	{
		int time;
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
}

void pushToFile() {
	out.open(fileAddress);
	for (int i = 0; i < fileOutput.size(); i++) {
		out << fileOutput[i];
	}
	out.close();
}

void userInput() {
	string input;
	int mainMenu = 0, subMenu = 0, subsubMenu = 0;
	int savePlaneID = -1;
	while (1) {
		getline(cin, input);
		char storeInput[input.size() + 1];
		input.copy(storeInput, input.size() + 1);
		storeInput[input.size()] = '\0';

		// menu();
		if (mainMenu == 0) {
			if (!strcmp(storeInput, "1")) {
				cout << "Enter plane ID for which you wish to change altitude.\n";
				subMenu = 1;
				mainMenu = 1;
				continue;
			}
			else if (!strcmp(storeInput, "2")) {
				cout << "Enter plane ID for which you wish to get status. \n";
				subMenu = 2;
				mainMenu = 1;
				continue;
			}
			else if (!strcmp(storeInput, "3")) {
				subMenu = 3;
				mainMenu = 1;
			}
			else {
				cout << "Enter 1 to change altitude or 2 to get status\n";
				continue;
			}
		}
		if (subMenu == 1) {
			cout << "Enter desired altitude: \n";
			savePlaneID = atoi(storeInput);
			subsubMenu = 1;
			subMenu = 0;
			continue;
		}
		else if (subMenu == 2) {
			if (planeIsActive(atoi(storeInput))) {
				response(getPlaneById(atoi(storeInput)), 1, 1);
				response(getPlaneById(atoi(storeInput)), 2, 1);
			}
			else {
				cout << "Plane is not in the active block.\n";
			}
			mainMenu = 0; // go back to initial menu
			subMenu = 0; //reset subMenu
			continue;
		}
		else if (subMenu == 3) {
			printResponseTimes();
			mainMenu = 0;
		}
		if (subsubMenu == 1) {
			//sets altitude of Plain
			if (planeIsActive(savePlaneID)) {
				request(getPlaneById(savePlaneID), 4, atoi(storeInput));
				response(getPlaneById(savePlaneID), 4, atoi(storeInput));
			}
			else {
				cout << "Plane is not in the active block.\n";
			}
			mainMenu = 0; //return to main menu
			subsubMenu = 0; //reset subsub;
			continue;
		}




	}
}


int main() {

	thread menuthread(userInput);
	menuthread.detach();

	int airplane_schedule[160] = {
		0, -641, 283, 500, 95000, 101589, 10000, 13,
		1, -5000, 0, 0, 71000, 100000, 10100, 16,
		-1, 5000, 0, 0, 41000, 100000, 10000, 18,
		3, 474, -239, 428, 38000, 0, 14000, 44,
		4, -25, 20, 30, 9500, 1000, 1000, 3,
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
	for (unsigned int i = 0; i < sizeof(airplane_schedule) / sizeof(*airplane_schedule); i += 8) {


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

	beginTime = chrono::steady_clock::now();

	timer_start(printHitList, 5000);
	timer_start(pushToFile, 5000);
	////thread printResponse(printResponseTimes);
	//printResponse.detach();
	char option = 'm';

	while (option != 'x') {	//while time is running and planes are not done

		tStart = chrono::steady_clock::now();

		//Check when first plane is released and store into Release array
		if (ordered.size() != 0) {
			if (ordered[0].getReleaseTime() <= (getTime())) {
				//cout <<  "\n" << getTime() << "This is current time \n";
				released.push_back(ordered[0]);	//put the plane into release array
				ordered.erase(ordered.begin());	//erase the plane from the ordered array
			}
		}

		endClock(1);

		tStart = chrono::steady_clock::now();

		//Check when plane is active and store into Active zone
		for (unsigned int i = 0; i < released.size(); i++) {
			//checks if plane is in the active block
			if (isNeverEntering(released[i])) {
				done.push_back(released[i]);
				released.erase(released.begin() + i);	//erase plane from Released array
			}
			else if (released[i].isInsideTheBlock(released[i].getCurrentLocation(), width, depth, height, 0, 0, 0)) {
				active.push_back(released[i]);			//put plane into Active array
				receiveBroadcast(released[i]);			//Receive Message from other ATC about new plane

				released.erase(released.begin() + i);	//erase plane from Released array


			}
		}

		endClock(2);

		tStart = chrono::steady_clock::now();

		//Check when plane is gets out of active zone
		for (unsigned int i = 0; i < active.size(); i++) {
			if (!active[i].isInsideTheBlock(active[i].getCurrentLocation(), width, depth, height, 0, 0, 0)) {
				done.push_back(active[i]);			//plane is put into Done array
				broadcast(active[i]);				//send message to next ATC
				active.erase(active.begin() + i);	//erase plane from Active zone
			}
		}

		endClock(3);

		tStart = chrono::steady_clock::now();

		//Check for collisions in active list
		checkForCollision();

		endClock(4);

		tStart = chrono::steady_clock::now();

		//update the location of all planes
		updateLocation();

		endClock(5);

		auto x = chrono::steady_clock::now() + chrono::milliseconds(1000);
		this_thread::sleep_until(x);

	}

	printResponseTimes(); // Returns the max and min response time of main processes

	system("pause");
	return 0;
}
