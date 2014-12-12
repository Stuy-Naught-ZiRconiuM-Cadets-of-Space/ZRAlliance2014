// Problems
// Sometimes fails to take pic 2!!
// Doesn't take picture when it comes out of the shadow zone

#define Chose_POI 0
#define TakePic_One 1
#define TakePic_Shadow 2
//#define intermediaryRun 3
#define GO_TO_SHADOW 4

float myState[12];
float POI[3][3];
float origin[3];
float initialPosition[3];
int time;
int state;
int bestPOI;
int memoryFilled;
int nextFlare;
int lastState;
int flareNum;
float brakingPt[3];
float POILoc[3];
float facing[3];
float earth[3];
float uploadPos[3];
float temp[3];
float percentFuelRemaining;

void init() {
	api.getMyZRState(myState);
	time = -1;
	state = 0;
	flareNum = 0;
	origin[0] = origin[1] = origin[2] = 0.f;
	earth[0] = 0.64f;
	earth[1] = earth[2] = 0.f;
	uploadPos[0] = 0.47f;
	uploadPos[1] = uploadPos[2] = 0.f;
	lastState = TakePic_Shadow;
	memcpy(initialPosition,myState,3*sizeof(float));
}

void loop() {
	int i; // COUNTER!!!!

	api.getMyZRState(myState);
	DEBUG(("\n%d\n",state));
	DEBUG(("\n%f\n",distance(myState,brakingPt)));
    DEBUG(("TARGET = %f,%f,%f\n",brakingPt[0],brakingPt[1],brakingPt[2]));

	for (i = 0; i < 3; i++) {game.getPOILoc(POI[i], i);}
	time++;
	nextFlare = game.getNextFlare();
	memoryFilled = game.getMemoryFilled();
    percentFuelRemaining = game.getFuelRemaining() * 100 / 90.0;
    
    if (nextFlare == -1) {
		DEBUG(("\nI don't know when the next flare is, so stop asking.\n"));
	}
	else if (nextFlare <= 30) {
	    if(nextFlare == 0){
	        flareNum++;
	    }
		state = GO_TO_SHADOW;
		DEBUG(("\nnextFlare: %d\nOH NO IT'S A FLARE!!!!!\n",nextFlare));
	}

	if (flareNum == 3) {
		lastState = Chose_POI;
	}
	
	if (time >= 30) {
		state = GO_TO_SHADOW;
	}

	if (time >= 230 || percentFuelRemaining <= 7) {
		state = GO_TO_SHADOW;
	}
	
	switch (state) {
		case Chose_POI:
			{
			float currentMinDist = distance(POI[0],initialPosition);
			bestPOI = 0;
			for (i = 1 ; i < 3 ; i++)  {
			    if (distance(POI[i],initialPosition) < currentMinDist) {
					currentMinDist = distance(POI[1],initialPosition);
					bestPOI = i;
				}

			}

			memcpy(brakingPt, POI[bestPOI], 3*sizeof(float));

            if(fabsf(brakingPt[0])>0.15){
                if(getPOILoc(brakingPt,bestPOI,18)==getPOILoc(brakingPt,bestPOI,15)){
                    
                }
                else{
                    getPOILoc(brakingPt,bestPOI,18);
                }
            }
            else{
                getPOILoc(brakingPt, bestPOI, 29);
            }

            temp[0] = mathVecMagnitude(brakingPt,3);

            for (i = 0 ; i < 3 ; i++) {
			    brakingPt[i] = 0.43 * brakingPt[i] / temp[0];
		    }
            
			setPositionTarget(brakingPt,1);
			mathVecSubtract(facing,origin,brakingPt,3);
			api.setAttitudeTarget(facing);

			state = TakePic_One;

			break;
			}

		case TakePic_One:
			{
			if(memoryFilled == 1) {
				state = GO_TO_SHADOW;
			}
			
			setPositionTarget(brakingPt,2);
			api.setAttitudeTarget(facing);
			if (game.alignLine(bestPOI)) {
				game.takePic(bestPOI);
				DEBUG(("\nI AM THE CHAMPION MY FRIENDS!\n"));
			}

			break;
			}
		case TakePic_Shadow:
		    {
		    if(memoryFilled == 2) {
		        state = GO_TO_SHADOW;
		    }
		    setPositionTarget(brakingPt,3);
		    api.setAttitudeTarget(facing);
		    if (game.alignLine(bestPOI)) {
				game.takePic(bestPOI);
				DEBUG(("\nI AM THE CHAMPION MY FRIENDS!\n"));
			}
		    break;   
		    }
		    
		case GO_TO_SHADOW:
			//DEBUG(("IMPLEMENT LATER"));
			{
			if(!destroySouls()) {
				setPositionTarget(uploadPos,1);
			}
			game.takePic(bestPOI); // spam is life
			mathVecSubtract(facing,earth,myState,3);
			api.setAttitudeTarget(facing);
			game.uploadPic();
			/*if (memoryFilled == 0) {
				DEBUG(("WE'LL JUST STAY HERE"));
				state = lastState;
			}*/
			state = lastState; // Always GO_TO_SHADOW unless flareNum > 3
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

void mathVecRotateToBottom(float a[]) {
	// Rotates the picture vector so that it's close to da shadow zone
	a[2] = sqrtf(a[0]*a[0] + a[2]*a[2]); // Rotate so that it's vertical and pointing down
	a[0] = 0;
	mathVecRotationXZ(a,-0.3); // So it's not exactly at the top
}

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
	mathVecRotationXZ(a,0.3); // So it's not exactly at the top
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
	
	if (minDistanceFromOrigin(target) > 0.32) {
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
	
	else if (meMag >= 0.22 && meMag <= 0.32) {
		for (int i = 0; i < 3; i++) {
			myPos[i] = myPos[i] * 2;
		}
		
		api.setPositionTarget(myPos);
		DEBUG(("TOO CLOSE\n"));
	}
	
	else {
		float opposite[3], perpendicular[3], mePrep[3], path[3];
		
		mathVecProject(opposite,target,myPos,3);
		mathVecSubtract(perpendicular,target,opposite,3);
		
		for (int i = 0; i < 3; i++) {
		    mePrep[i] = perpendicular[i] / mathVecMagnitude(perpendicular,3);
		}
		
		for (int i = 0; i < 3; i++) {
			mePrep[i] = (mePrep[i] * 0.32 * meMag) / (sqrtf(meMag*meMag - 0.32*0.32));
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

bool destroySouls(){
	//gameplan: project the y and z cors, then project the xcor.
	ZRState other;
	api.getOtherZRState(other);
	float target[3];
	float temp = 0.1f;
	if((other[0] > 0)&&(inShadow(other,target,2))){ //loose check that should be improved later.
		while(!inShadow(other,target,temp)&&(temp<2)){
			temp+=0.1f;
		}
		setPositionTarget(target,1.2);
		return true;
	}
	return false;
}

bool getPOILoc(float pos[3], int id, float t) { 
    float POILoc[3];
    game.getPOILoc(POILoc, id);
    
    for (int i = 0; i < 3; i++) {
        pos[0] = POILoc[2] * sinf(0.1 * t) - POILoc[0] * cosf(0.1 * t);
        pos[1] = POILoc[1];
        pos[2] = -(POILoc[2] * cosf(0.1 * t) + POILoc[0] * sinf(0.1 * t));
    }
    
    if (pos[0] >= 0) {
        pos[0] *= -1;
        pos[2] *= -1;
        return true;
    }
    return false;
    
}

bool inShadow(ZRState other, float target[3], int t){
    if((other[1]+other[4]*t)*(other[1]+other[4]*t) + (other[2]+other[5]*t)*(other[2]+other[5]*t) <0.04){ //projected coor is in the shadow
        DEBUG(("\nIfIsTrue\n"));
        target[0] = 0;
        target[1] = other[1] + other[4]*t;
        target[2] = other[2] + other[5]*t;
        for(int i = 0; i < 3; i++) {
    		target[i] *= 0.195/mathVecMagnitude(target,3);
    	}
    	target[0] = other[0] + other[3];
    	if(target[0] < 0.29){
    	    target[0] = 0.29;
    	}
    	else if(target[0] > 0.48){
    	    target[0] = 0.48;
    	}
        return true;
    }
    return false;
}
