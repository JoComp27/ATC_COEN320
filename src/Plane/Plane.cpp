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

	int id;
	int releaseTime;

	Location spawnLocation;
	Location wantedLocation;

	Location currentLocation;
	Velocity currentVelocity;

	bool UFO;



public:

	Plane(int id, int vx, int vy, int vz, int x, int y, int z, int releaseTime){
		if(id == -1){
		this->id = id;
		}

		this->currentVelocity = Velocity(vx, vy, vz);
		this->currentLocation = Location(x, y, z);
		this->spawnLocation = Location(x, y, z);
		this->wantedLocation = getWantedLocation();
		this->releaseTime = releaseTime;
	}

	~Plane(){

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
		cout << "Plane( " << id << ")" << endl;
		currentLocation.print();
		currentVelocity.print();
	}

};

#endif /* PLANE_PLANE_H_ */
