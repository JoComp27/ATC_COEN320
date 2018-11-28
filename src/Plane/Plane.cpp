/*
 * Plane.h
 *
 *  Created on: Nov. 14, 2018
 *      Author: Administrateur
 */

#include "Location.cpp"
#include "Velocity.cpp"

#include <iostream>
#include <cmath>

using namespace std;

#ifndef PLANE_PLANE_H_
#define PLANE_PLANE_H_

class Plane {
private:

	static ufoId = 0;
	int id;
	int releaseTime;

	bool isHolding = false;
	double magnetudeOfVelocity;
	double circleRadius;

	Location spawnLocation;
	Location wantedLocation;

	Location currentLocation;
	Velocity currentVelocity;

	bool ufo ;



public:

	Plane(int id, int vx, int vy, int vz, int x, int y, int z, int releaseTime){
		
		setId(id);
		this->currentVelocity = Velocity(vx, vy, vz);
		this->currentLocation = Location(x, y, z);
		this->spawnLocation = Location(x, y, z);
		this->releaseTime = releaseTime;
	}

	Plane() {
		
	}
	

	~Plane(){

	}

	void hasEnteredBlock() {
		this->wantedLocation = getWantedLocation();
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

	void setCurrentLocation(int vx, int vy, int vz) {
		this->currentLocation = Location(x, y, z);
	}

	void setCurrentVelocity(int x, int y, int z) {
		this->currentVelocity = Velocity(x, y, z);
	}

	Location getCurrentLocation() const{
		return currentLocation;
	}

	Velocity getCurrentVelocity() const{
		return currentVelocity;
	}


	Location getSpawnLocation() const{
		return spawnLocation;
	}

	Location getWantedLocation() const{
		return wantedLocation;
	}

	Location getFutureLocation(Location a, int time){
		int x = a.getX() + time*currentVelocity.getVx();
		int y = a.getY() + time*currentVelocity.getVy();
		int z = a.getZ() + time*currentVelocity.getVz();
		return Location(x,y,z);
	}

	Location getWantedLocation(){
		Location tempLocation = currentLocation;
		while(isInsideTheBlock(tempLocation, 100000,100000, 25000, 0, 0, 0)){
			tempLocation = getFutureLocation(tempLocation, 1);
		}
		return tempLocation;
	}

	bool isInsideTheBlock(Location a, int maxX, int maxY, int maxZ, int minX, int minY, int minZ){

		bool withinX = a.getX() <= maxX && a.getX() >= minX;
		bool withinY = a.getY() <= maxY && a.getY() >= minY;
		bool withinZ = a.getZ() <= maxZ && a.getZ() >= minZ;

		return withinX && withinY && withinZ;
	}

	bool collisionCheck(Plane a, int time) {
		Location plane1FL = getFutureLocation(time);
		Location plane2FL = a.getFutureLocation(time);

		return isInsideTheBlock(plane2FL, plane1FL.getX() + 3, plane1FL.getY() + 3, plane1FL.getZ() + 1, plane1FL.getX() - 3, plane1FL.getY() - 3, plane1FL.getZ() - 1);
	}

	void toggleHoldingPattern() {
		if (!holdingPattern) {
			isHolding = true;
			//Reset the velocity to 
		}
		else {
			isHolding = false;
			magnetudeOfVelocity = sqrt(pow(currentVelocity.getVx(), 2) + pow(currentVelocity.getVy(), 2) + pow(currentVelocity.getVz(), 2)); //Magnetude of the current Velocity
			circleRadius = sqrt(pow(x - 50000, 2) + pow(y - 50000, 2)); // The radius of the circle around the 
			currentVelocity = Velocity();
		}
	}

	updateLocation() {
		if (!isHolding) {
			currentLocation = getFutureLocation(currentLocation, 1);
		}
		else {
			currentVelocity = getCircleVelocity;
			currentLocation = getFutureLocation(currentLocation, 1);
		}
	}

	Velocity getCircleVelocity() { // Calculates the velcity vector according to the tangent of the circle wanted 

	}

	void print(){
		cout << "Plane ";
		if(UFO){
			cout << "UFO";
		}
		cout << id << ": ";
		currentLocation.print();
		currentVelocity.print() << endl;
	}

};

#endif /* PLANE_PLANE_H_ */
