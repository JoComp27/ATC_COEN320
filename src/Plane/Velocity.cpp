/*
 * Velocity.h
 *
 *  Created on: Nov. 14, 2018
 *      Author: Administrateur
 */

#include <string>
#include <iostream>

using namespace std;

#ifndef PLANE_VELOCITY_H_
#define PLANE_VELOCITY_H_

class Velocity {
public:

	int vx;
	int vy;
	int vz;

	Velocity();
	virtual ~Velocity();

int getVx() const{
	return vx;
}

int getVy() const{
	return vy;
}

int getVz() const{
	return vz;
}

void print(){

	cout << "Current Velocity: (vx = " << vx << ", vy = " << vy << ", vz = " << vz << " )" << endl;

}

};

#endif /* PLANE_VELOCITY_H_ */
