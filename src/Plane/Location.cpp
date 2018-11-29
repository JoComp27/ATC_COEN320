/*
 * Location.cpp
 *
 *  Created on: Nov. 14, 2018
 *      Author: Administrateur
 */


#include <math.h>
#include <iostream>

#include "Velocity.cpp"

using namespace std;

#ifndef PLANE_LOCATION_H_
#define PLANE_LOCATION_H_

class Location {

	int x;
	int y;
	int z;

public:

	Location(){}

	Location(int x, int y, int z){
		this->x = x;
		this->y = y;
		this->z = z;
	}

	virtual ~Location() {}

	int distanceBetween(Location a){
		double result = sqrt(pow(getX()-a.getX(),2)+pow(getY() - a.getY(),2)+pow(getZ() - a.getZ(),2));
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

void updatePosititon(Velocity a){

	setPosition(x + a.getVx(), y + a.getVy(), z + a.getVz());

}

void setPosition(int x, int y, int z){
	this->x = x;
	this->y = y;
	this->y = z;
}

void print(){

	cout << "x : " << x  << " , y : " << y << " , z : " << z << " , ";

}

};

#endif /* PLANE_LOCATION_H_ */
