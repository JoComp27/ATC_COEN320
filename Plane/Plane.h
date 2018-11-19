/*
 * Plane.h
 *
 *  Created on: Nov. 14, 2018
 *      Author: Administrateur
 */

#include "Location.h"
#include "Velocity.h"

#include <iostream>

using namespace std;

#ifndef PLANE_PLANE_H_
#define PLANE_PLANE_H_

class Plane {
private:

	int id;

	Location spawnLocation;
	Location wantedLocation;

	Location currentLocation;
	Velocity currentVelocity;



public:

	Plane();

	virtual ~Plane();

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

	Location getNextLocation(){
		return Location(currentLocation.getX() + );
	}

	void print(){
		cout << "Plane( " << id << ")" << endl;
		currentLocation.print();
		currentVelocity.print();
	}

};

#endif /* PLANE_PLANE_H_ */
