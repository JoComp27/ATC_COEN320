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

static int ufoId = 0;

class Plane {
private:

	int id;
	int releaseTime;

	int heightLimit = 1;
	int sideLimit = 3;

	bool isHolding = false;
	double magnetudeOfVelocity;
	double circleRadius;

	Location spawnLocation;
	Location wantedLocation;

	Location currentLocation;
	Velocity currentVelocity;

	bool ufo = false;



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

	void setCurrentPosition(int x, int y, int z) {
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

	int getReleaseTime() const {
		return releaseTime;
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
		Location plane1FL = getFutureLocation(currentLocation, time);
		Location plane2FL = a.getFutureLocation(a.getCurrentLocation(), time);

		return isInsideTheBlock(plane2FL, plane1FL.getX() + sideLimit, plane1FL.getY() + sideLimit, plane1FL.getZ() + heightLimit, plane1FL.getX() - sideLimit, plane1FL.getY() - sideLimit, plane1FL.getZ() - heightLimit);
	}

	void toggleHoldingPattern() {
		if (!isHolding) {
			isHolding = true;
			currentVelocity = goBackToNormal();
			
		}
		else {
			isHolding = false;
			currentVelocity = getCircleVelocity();
		}
	}

	void updateLocation() {
		if (!isHolding) {
			currentLocation = getFutureLocation(currentLocation, 1);
		}
		else {
			currentLocation = getFutureLocation(currentLocation, 1);
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
			magnetudeOfVelocity = sqrt(pow(currentVelocity.getVx(), 2) + pow(currentVelocity.getVy(), 2) + pow(currentVelocity.getVz(), 2)); //Magnetude of the current Velocity
			circleRadius = sqrt(pow(50000 - currentLocation.getX(), 2) + pow(50000 - currentLocation.getY(), 2)); // The radius of the circle around the center

			double xVel = magnetudeOfVelocity * ((50000 - currentLocation.getY()) / circleRadius);
			double yVel = -magnetudeOfVelocity*((50000 - currentLocation.getX())/circleRadius);

			return Velocity(xVel,yVel,0);
	}

	void print(){
		cout << "Plane ";
		if(ufo){
			cout << "UFO";
		}
		cout << id << ": ";
		currentLocation.print();
		currentVelocity.print();
	}

};

#endif /* PLANE_PLANE_H_ */
