#define Chose_POI 0
#define TakePic_One 1
#define TakePic_Two 2
#define GO_TO_SHADOW 3

float myState[12];
float POI[3][3];
float initialPosition[3];
int time;
int state;
int bestPOI;
int nextFlare;
int memoryFilled;
int nextFlareTime;
int lastState;
float brakingPt[3];
float POILoc[3];
float facing[3];
float earth[3];
float uploadPos[3];
float percentFuelRemaining;

void init() {
	api.getMyZRState(myState);
	time = -1;
	state = 0;
	earth[0] = 0.64f;
	earth[1] = earth[2] = 0.f;
	uploadPos[0] = 0.55f;
	uploadPos[1] = uploadPos[2] = 0.f;
	POILoc[0] = POILoc[1] = POILoc[2] = 0.f;
	bestPOI = 0; // Stupid compiler
	lastState = Chose_POI;
	memcpy(initialPosition,myState,3*sizeof(float));
}

void loop() {
	int i; // COUNTER!!!!

	api.getMyZRState(myState);
	DEBUG(("\n%d\n",state));

	time++;
	nextFlare = game.getNextFlare();
	memoryFilled = game.getMemoryFilled();
	percentFuelRemaining = game.getFuelRemaining() * 100 / 90.0;

	if (time % 60 == 0 && state == TakePic_One) {
		state = Chose_POI;
	}

	// FLARE EVASION
	/* F**K this for now
	nextFlare = game.getNextFlare();

	if (nextFlare == -1) {
		DEBUG(("\nI don't know when the next flare is, so stop asking.\n"));
	}
	else if (nextFlare <= 15) {
		if (lastState != GO_TO_SHADOW) {
			lastState = state;
		}
		else {
			lastState = Chose_POI;
		}

		state = GO_TO_SHADOW;
		DEBUG(("\nnextFlare: %d\nOH NO IT'S A FLARE!!!!!\n",nextFlare));
	}
	*/

	if (time >= 230) {
		state = GO_TO_SHADOW;
	}

	switch (state) {
		case Chose_POI:
			{
			for (i = 0; i < 3; i++) {game.getPOILoc(POI[i], i);}

			float currentMinDist = distance(POI[0],initialPosition);
			bestPOI = 0;
			for (i = 1 ; i < 3 ; i++)  {
				if (distance(POI[i],initialPosition) < currentMinDist) {
					currentMinDist = distance(POI[1],initialPosition);
					bestPOI = i;
				}

			}

			memcpy(POILoc, POI[bestPOI], 3*sizeof(float));

			memcpy(brakingPt, POILoc, 3*sizeof(float));

			for (i = 0 ; i < 3 ; i++) {
				brakingPt[i] = 0.419 * brakingPt[i] / mathVecMagnitude(brakingPt,3);
			}

			mathVecRotateToTop(POILoc);

			mathVecRotateToTop(brakingPt);
			mathVecRotationXZ(brakingPt,-0.4);
			setPositionTarget(brakingPt,1);
			mathVecSubtract(facing,POILoc,myState,3);
			api.setAttitudeTarget(facing);

			state = TakePic_One;

			break;
			}

		case TakePic_One:
			{
			if(memoryFilled != 0) {

				for (i = 0 ; i < 3 ; i++) {
					brakingPt[i] = 0.421 * brakingPt[i] / mathVecMagnitude(brakingPt,3);
				}

				mathVecRotationXZ(POILoc,-0.5);
				setPositionTarget(brakingPt,2);
				mathVecSubtract(facing,POILoc,myState,3);
				api.setAttitudeTarget(facing);

				state = TakePic_Two;
			}

			setPositionTarget(brakingPt,1);
			mathVecSubtract(facing,POILoc,myState,3);
			api.setAttitudeTarget(facing);
			if (game.alignLine(bestPOI)) {
				game.takePic(bestPOI);
				DEBUG(("\nI AM THE CHAMPION MY FRIENDS!\n"));
			}

			break;
			}

		case TakePic_Two:
			{
			if(memoryFilled == 2) {
				state = GO_TO_SHADOW;
			}
			
			setPositionTarget(brakingPt,2);
			mathVecSubtract(facing,POILoc,myState,3);
			api.setAttitudeTarget(facing);

			if (game.alignLine(bestPOI)) {
				game.takePic(bestPOI);
				DEBUG(("I AM THE CHAMPION MY FRIENDS!"));
			}

			break;
			}
			

		case GO_TO_SHADOW:
			//DEBUG(("IMPLEMENT LATER"));
			{
			setPositionTarget(uploadPos,1);
			mathVecSubtract(facing,earth,myState,3);
			api.setAttitudeTarget(facing);
			game.uploadPic();
			if (memoryFilled == 0) {
				DEBUG(("WE'LL JUST STAY HERE"));
				if (percentFuelRemaining <= 7) {
					lastState = GO_TO_SHADOW; // 5% fuel is not going to get you anywhere
				}
				state = lastState;
			}
			break;
			}
	}
}

float distance(float p1[], float p2[]){
	// Thanks Cornel
	float diff[3];
	mathVecSubtract(diff,p1,p2,3);
	return mathVecMagnitude(diff,3);
}

float angle(float a[], float b[], float c[]) {
    //returns the measure of angle abc
    float side1[3], side2[3], cosine;
    
    for (int i = 0; i < 3; i++) side1[i] = a[i] - c[i];
    cosine = - mathVecMagnitude(side1,3) * mathVecMagnitude(side1,3);
    for (int i = 0; i < 3; i++) side1[i] = b[i] - a[i];
    for (int i = 0; i < 3; i++) side2[i] = c[i] - b[i];
    cosine += mathVecMagnitude(side1,3) * mathVecMagnitude(side1,3) + mathVecMagnitude(side2,3) * mathVecMagnitude(side2,3);
    if (mathVecMagnitude(side1,3) * mathVecMagnitude(side2,3) / 10 == 0) {
        DEBUG(("DIVISION BY ZERO WHILE FINDING ANGLE!"));
    }
    cosine /= 2 * mathVecMagnitude(side1,3) * mathVecMagnitude(side2,3);
    
    return acosf(cosine);
}

void mathVecProject(float c[], float a[], float b[], int n) {
    // finds the projection of a onto b, puts the result in c
    if (mathVecMagnitude(b,3) * mathVecMagnitude(b,3) / 10 == 0) {
        DEBUG(("DIVISION BY ZERO WHILE PROJECTING!"));
    }
    for (int i = 0; i < n; i++) {
        c[i] = (mathVecInner(a,b,3) * b[i]) / (mathVecMagnitude(b,3) * mathVecMagnitude(b,3));
    }
}

void mathVecRotationXZ(float a[], float angle) {
	// Rotate vector a angle degree counter-clockwise on the XZ plane
	float xorg = a[0];
	a[0] = cosf(angle)*a[0] + sinf(angle)*a[2];
	a[2] = -sinf(angle)*xorg + cosf(angle)*a[2];
}
/*
void mathVecRotateToBottom(float a[]) {
	// Rotates the picture vector so that it's close to da shadow zone
	a[2] = sqrtf(a[0]*a[0] + a[2]*a[2]); // Rotate so that it's vertical and pointing down
	a[0] = 0;
	mathVecRotationXZ(a,-0.6); // So it's not exactly at the top
}
*/


void mathVecRotateToHorizontal(float a[]) {
	// Rotates the vector to negative x
	a[0] = -sqrtf(a[0]*a[0] + a[2]*a[2]);
	a[2] = 0;
	mathVecRotationXZ(a,-0.5);
}

void mathVecRotateToTop(float a[]){
	// Rotates the vector a to the top
	a[2] = -sqrtf(a[0]*a[0] + a[2]*a[2]); // Rotate so that it's vertical and pointing down
	a[0] = 0;
	mathVecRotationXZ(a,0.7); // So it's not exactly at the top
}


float minDistanceFromOrigin(float target[]) {
	float temp[3] = {0,0,0}; //temp is the origin
	
	if (cosf(angle(temp,myState,target)) < 0) { //going away from origin
		return mathVecMagnitude(myState, 3);
	}
	
	else if (cosf(angle(temp,target,myState)) < 0) { //going in direction of origin
		return mathVecMagnitude(target,3);
	}
	
	else {
	    mathVecSubtract(temp,target,myState,3);
	    mathVecProject(temp,myState,temp,3);
		mathVecSubtract(temp,myState,temp,3);
		
		return mathVecMagnitude(temp,3);
	}
}


void setPositionTarget(float target[3], float multiplier) {
	api.getMyZRState(myState);
	
	float myPos[3],meMag;
	
	memcpy(myPos, myState, 3*sizeof(float));
	
	meMag = mathVecMagnitude(myPos,3);
	
	if (minDistanceFromOrigin(target) > 0.315) {
		//if (distance(myState, target) < 0.6) { // Save braking distance
		api.setPositionTarget(target);
		//}

		//else { // Or haul ass towards target
		//	float temp[3];

		//	mathVecSubtract(temp,target,myState,3);
			
		//	for (int i = 0 ; i < 3 ; i++) {
		//		temp[i] = myState[i] + temp[i] * multiplier;
		//	}

		//	api.setPositionTarget(temp);
		//}

		DEBUG(("GOING STRAIGHT\n"));
	}
	
	else if (meMag >= 0.22 && meMag <= 0.315) {
		for (int i = 0; i < 3; i++) {
			myPos[i] = myPos[i] * 2;
		}
		
		api.setPositionTarget(myPos);
		DEBUG(("TOO CLOSE\n"));
	}
	
	else {
		float opposite[3], perpendicular[3], mePrep[3], path[3], temp[3];
		
		mathVecProject(opposite,target,myPos,3);
		mathVecSubtract(perpendicular,target,opposite,3);
		
		for (int i = 0; i < 3; i++) {
		    mePrep[i] = perpendicular[i] / mathVecMagnitude(perpendicular,3);
		}
		
		for (int i = 0; i < 3; i++) {
			mePrep[i] = (mePrep[i] * 0.325 * meMag) / (sqrtf(meMag*meMag - 0.315*0.315));
		}
		
		mathVecSubtract(path,mePrep,myPos,3);
		
		for (int i = 0; i < 3; i++) {
			path[i] = path[i] * multiplier;
		}
		
		mathVecAdd(temp,myPos,path,3);

		api.setPositionTarget(temp);
		
		DEBUG(("TAKING THE TANGENT\n"));
	}
}
