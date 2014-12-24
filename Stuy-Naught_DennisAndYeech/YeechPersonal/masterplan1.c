// Lesson learned --> Trust No One
// Yeech's Masterplan

// Change of Plans --> Since this is obviously not working out, we'll take real photos

// Chose_POI --> TakePic_One --> GO_TO_SHADOW --> TakePic_Two --> GO_TO_SHADOW --> TakePic_Two ...

#define Chose_POI 0
#define TakePic_One 1
#define TakePic_Two 2
#define GO_TO_SHADOW 3

float myState[12];
float POI[3][3];
float origin[3];
float initialPosition[3];
int time;
int state;
int bestPOI;
int middlePOI;
int memoryFilled;
int nextFlare;
float brakingPt[3];
float POILoc[3];
float facing[3];
float earth[3];
float uploadPos[3];
float temp[3];

void init() {
	api.getMyZRState(myState);
	time = -1;
	state = 0;
	origin[0] = origin[1] = origin[2] = 0.f;
	earth[0] = 0.64f;
	earth[1] = earth[2] = 0.f;
	uploadPos[0] = 0.4f;
	uploadPos[1] = 0.0f;
	uploadPos[2] = -0.17f;
	state = Chose_POI;
	memcpy(initialPosition,myState,3*sizeof(float));
}

void loop() {
	
	DEBUG(("\nSTATE:%d\n", state));

	int i;

	api.getMyZRState(myState);
	//DEBUG(("\n%d\n",state));

	time++;
	nextFlare = game.getNextFlare();
	memoryFilled = game.getMemoryFilled();

	if (nextFlare == -1) {
		DEBUG(("\nI don't know when the next flare is, so stop asking.\n"));
	}
	else if (nextFlare <= 20 || time > 155) {
		state = GO_TO_SHADOW;
		DEBUG(("\nnextFlare: %d\nOH NO IT'S A FLARE!!!!!\n",nextFlare));
	}

	switch (state) {
		case Chose_POI:
			{
			for (i = 0; i < 3; i++) {game.getPOILoc(POI[i], i);}
			float currentMinDist = distance(POI[0], initialPosition);
			bestPOI = 0;
			for (i = 1 ; i < 3 ; i++) {
				if (distance(POI[i], initialPosition) < currentMinDist) {
					currentMinDist = distance(POI[i], initialPosition);
					bestPOI = i;
				}

				if (POI[i][1] == 0) {
					middlePOI = i;
				}
			}

			memcpy(brakingPt, POI[bestPOI], 3*sizeof(float));

			getPOILoc(brakingPt,bestPOI,18);

			temp[0] = mathVecMagnitude(brakingPt,3);

	        	for (i = 0 ; i < 3 ; i++) {
				brakingPt[i] = 0.45 * brakingPt[i] / temp[0];
			}
            
			setPositionTarget(brakingPt,1);
			mathVecSubtract(facing,origin,brakingPt,3);
			api.setAttitudeTarget(facing);

			state = TakePic_One;

			break;
			}

		case TakePic_One:
			{
			if (memoryFilled == 1) {
				state = GO_TO_SHADOW;
			}

			setPositionTarget(brakingPt,0.8);
			api.setAttitudeTarget(facing);
			if (game.alignLine(bestPOI)) {
				game.takePic(bestPOI);
				DEBUG(("\nI am the champion my friends\n"));
			}

			break;
			}

		case TakePic_Two:
			{
			if (memoryFilled == 1) {
				DEBUG(("I AM THE CHAMPION MY FRIENDS"));
				setPositionTarget(uploadPos, 1);
				state = GO_TO_SHADOW;
			}

			setPositionTarget(brakingPt,0.7);
			api.setAttitudeTarget(facing);

			if (game.alignLine(middlePOI)) {
				game.takePic(middlePOI);
			}

			break;
			}

		case GO_TO_SHADOW:
			{
			setPositionTarget(uploadPos, 0.8);
			mathVecSubtract(facing, earth, uploadPos, 3);
			api.setAttitudeTarget(facing);

			game.uploadPic();

			if (memoryFilled == 0) {
				memcpy(POILoc, POI[middlePOI], 3*sizeof(float));

				getPOILoc(POILoc, middlePOI, 15);
				mathVecRotateToTop(POILoc);
				memcpy(brakingPt, POILoc, 3*sizeof(float));
				
				for (int i = 0 ; i < 3; i++) {
					brakingPt[i] = brakingPt[i] * 2.25;
				}

				mathVecSubtract(facing, POILoc, brakingPt, 3);

				state = TakePic_Two;
			}
			break;
			}
	}
}

float distance(float p1[], float p2[]){
	// Thanks Cornel
	mathVecSubtract(temp,p1,p2,3);
	return mathVecMagnitude(temp,3);
}

void mathVecProject(float c[], float a[], float b[], int n) {
    // finds the projection of a onto b, puts the result in c
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

void mathVecRotateToBottom(float a[]) {
	// Rotates the picture vector so that it's close to da shadow zone
	a[2] = sqrtf(a[0]*a[0] + a[2]*a[2]); // Rotate so that it's vertical and pointing down
	a[0] = 0;
	mathVecRotationXZ(a,-0.2); // So it's not exactly at the bottom
}

void mathVecRotateToTop(float a[]){
	// Rotates the vector a to the top
	a[2] = -sqrtf(a[0]*a[0] + a[2]*a[2]); // Rotate so that it's vertical and pointing down
	a[0] = 0;
	mathVecRotationXZ(a,0.2); // So it's not exactly at the top
}

float minDistanceFromOrigin(float target[3]) {
	float cos;

	float targetMag = mathVecMagnitude(target,3);
	float meMag = mathVecMagnitude(myState,3);

	mathVecSubtract(temp,target,myState,3);

	float tempMag = mathVecMagnitude(temp,3);
	cos = (targetMag*targetMag - meMag*meMag - tempMag*tempMag) / (-2 * meMag * tempMag);

	if (cos < 0) {
		return meMag; // Shortest at endpoint
	}

	cos = (meMag*meMag - targetMag*targetMag - tempMag*tempMag) / (-2 * targetMag * tempMag);
	if (cos < 0) {
		return targetMag; // Shortest at endpoint
	}

	else {
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
	
	if (minDistanceFromOrigin(target) > 0.33) {
		api.setPositionTarget(target);

		DEBUG(("GOING STRAIGHT\n"));
	}

	else if (meMag >= 0.22 && meMag <= 0.33) {
		for (int i = 0 ; i < 3 ; i++) {
			myPos[i] = myPos[i] * 2;
		}

		api.setPositionTarget(myPos);
		DEBUG(("TOO SUB\n"));
	}
	
	else {
		float opposite[3], perpendicular[3], mePrep[3], path[3];
		
		mathVecProject(opposite,target,myPos,3);
		mathVecSubtract(perpendicular,target,opposite,3);
		
		for (int i = 0; i < 3; i++) {
		    mePrep[i] = perpendicular[i] / mathVecMagnitude(perpendicular,3);
		}
		
		for (int i = 0; i < 3; i++) {
			mePrep[i] = (mePrep[i] * 0.33 * meMag) / (sqrtf(meMag*meMag - 0.33*0.33));
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

bool getPOILoc(float pos[3], int id, float t) { 
    game.getPOILoc(temp, id);
    
    for (int i = 0; i < 3; i++) {
        pos[0] = temp[2] * sinf(0.1 * t) - temp[0] * cosf(0.1 * t);
        pos[1] = temp[1];
        pos[2] = -(temp[2] * cosf(0.1 * t) + temp[0] * sinf(0.1 * t));
    }
    
    if (pos[0] >= 0) {
        pos[0] *= -1;
        pos[2] *= -1;
        return true;
    }
    return false;
}
