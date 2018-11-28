/*
 * Plane.h
 *
 *  Created on: Nov. 14, 2018
 *      Author: Administrateur
 */

#include "Location.cpp"
#include "Velocity.cpp"

#include <iostream>

using namespace std;

#ifndef PLANE_PLANE_H_
#define PLANE_PLANE_H_

class Plane {
private:

	static ufoId = 0;
	int id;
	int releaseTime;

	Location spawnLocation;
	Location wantedLocation;

	Location currentLocation;
	Velocity currentVelocity;

	bool ufo ;



public:

	Plane(int id, int vx, int vy, int vz, int x, int y, int z, int releaseTime){
		
		setId(id);
		this->currentVelocity = new Velocity(vx, vy, vz);
		this->currentLocation = new Location(x, y, z);
		this->spawnLocation = new Location(x, y, z);
		this->wantedLocation = getWantedLocation();
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

	void setCurrentLocation(int vx, int vy, int vz) {
		this->currentLocation = new Location(x, y, z);
	}

	void setCurrentVelocity(int x, int y, int z) {
		this->currentVelocity = new Velocity(x, y, z);
	}

	const Location& getCurrentLocation() const{
		return currentLocation;
	}

	const Velocity& getCurrentVelocity() const{
		return currentVelocity;
	}


	const Location& getSpawnLocation() const{
		return spawnLocation;
	}

	const Location& getWantedLocation() const{
		return wantedLocation;
	}

	Location getFutureLocation(int time){
		int x = currentLocation.getX() + time*currentVelocity.getVx();
		int y = currentLocation.getY() + time*currentVelocity.getVy();
		int z = currentLocation.getZ() + time*currentVelocity.getVz();
		return Location(x,y,z);
	}

	Location getWantedLocation(){
		Location tempLocation = currentLocation;
		while(isInsideTheBlock(tempLocation, 100000,100000, 25000, 0, 0, 0)){
			tempLocation = getNextLocation(tempLocation);
		}

	}

	bool isInsideTheBlock(Location a, int maxX, int maxY, int maxZ, int minX, int minY, int minZ){

		bool withinX = a.getX() <= maxX && a.getX() >= minX;
		bool withinY = a.getY() <= maxY && a.getY() >= minY;
		bool withinZ = a.getZ() <= maxZ && a.getZ() >= minZ;

		return withinX && withinY && withinZ;
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
