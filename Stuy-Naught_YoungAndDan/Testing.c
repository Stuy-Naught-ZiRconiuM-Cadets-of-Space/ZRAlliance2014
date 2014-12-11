float distance(float p1[], float p2[]){
	// Thanks Cornel
	float diff[3];
	mathVecSubtract(diff,p1,p2,3);
	return mathVecMagnitude(diff,3);
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

float minDistanceFromOrigin(float target[]) {
	float temp[3] = {0,0,0}; //temp is the origin
	
	if (angle(temp,myState,target) > PI/2) { //going away from origin
		return mathVecMagnitude(myState, 3);
	}
	
	else if (angle(temp,target,myState) > PI/2) { //going in direction of origin
		return mathVecMagnitude(target,3);
	}
	for(int i = 0; i < 3; i ++) temp[i] = 0.0f;
	else { // accurate to the nearest 0.05;
	    float a[3];
	    if(distance(temp,myState)>distance(temp,target)){
	    	mathVecSubtract(a,myState,target,3);
	    	for(int i = 0; i < 3; i ++) a[i] = 0.05/mathVecMagnitude(a,3);
	    	while(angle(temp,target,myState) < PI/2){
	    		for(int i = 0; i < 3; i ++) target[i] += a[i];
	    	}
	    	return mathVecMagnitude(target,3);
	    }
	    else{
	    	mathVecSubtract(a,target,myState,3);
	    	for(int i = 0; i < 3; i ++) a[i] = 0.05/mathVecMagnitude(a,3);
	    	while(angle(temp,myState,target) < PI/2){
	    		for(int i = 0; i < 3; i ++) myState[i] += a[i];
	    	}
	    	return mathVecMagnitude(myState,3);
	    }
	}
}

float minDistanceFromOrigin(float target[3]) {
	float cos;
	float temp[3];
	float targetMag = mathVecMagnitude(target,3);
	float meMag = mathVecMagnitude(myState,3);
	mathVecSubtract(temp,target,myState,3);
	float tempMag = mathVecMagnitude(temp,3);
	cos = (targetMag*targetMag - meMag*meMag - tempMag*tempMag) / (-2 * meMag * tempMag);
	if (cos < 0) {
		DEBUG(("\nENDPOINT RETURNED\n"));
		return meMag; // Shortest at endpoint
	}
	cos = (meMag*meMag - targetMag*targetMag - tempMag*tempMag) / (-2 * targetMag * tempMag);
	if (cos < 0) {
		DEBUG(("\nENDPOINT RETURNED\n"));
		return targetMag; // Shortest at endpoint
	}
	else {
		DEBUG(("\nANGLE IS: %f\n", acosf(cos)));
		mathVecProject(temp,myState,temp,3);
		mathVecSubtract(temp,myState,temp,3);
		
		return mathVecMagnitude(temp,3);
	}

}
