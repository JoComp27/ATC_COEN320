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

	Location getFutureLocation(int time){
		int x = currentLocation.getX() + time*currentVelocity.getVx();
		int y = currentLocation.getY() + time*currentVelocity.getVy();
		int z = currentLocation.getZ() + time*currentVelocity.getVz();
		return Location(x,y,z);
	}

	void setWantedLocation() {
		int i = 1;
		while (isInsideTheBlock(getFutureLocation(i), 100000, 100000, 25000, 0, 0, 0)) {
			i++;
		}
		this->wantedLocation = getFutureLocation(i);
	}

	bool isInsideTheBlock(Location location, int maxX, int maxY, int maxZ, int minX, int minY, int minZ){

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
