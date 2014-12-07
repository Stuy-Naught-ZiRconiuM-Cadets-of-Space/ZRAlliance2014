//Find precise upload location
//edit closestPOILoc else cases
//make faster
//improve memory
#define ST_GET_POI 0
#define ST_OUTER 10
#define ST_INNER 20
#define ST_SHADOW 30
#define ST_UPLOAD 40
#define ST_FLARE 50

ZRState myState;
int state, picNum, POIID, solarFlareBegin, time, takePic, checkNum;
int goodPOI[3]; //This array says which POIS are able to be gone to
float POI[3], POIproj[3], uploadPos[3], facing[3], target[3];
float origin[3], earth[3];
bool wp;

float originalVecBetween[3], waypoint[3],vecBetween[3],tempTarget[3];

void init() {
    state = ST_GET_POI;
    for(int i = 0; i < 3; i++){
        origin[i] = 0.00f;
        goodPOI[i] = 1;
    }
    earth[0] = 0.64f; 
    earth[1] = 0.00f;
    earth[2] = 0.00f;
    checkNum = 0;
    takePic = -1;
    wp = false;
}

void loop() {
    api.getMyZRState(myState);
    time = api.getTime();
    picNum = game.getMemoryFilled();   
    DEBUG(("%d picture(s) have been taken\n", picNum));
    DEBUG(("STATE = %d\n",state));
    DEBUG(("ARRAY = %d,%d,%d\n",goodPOI[0],goodPOI[1],goodPOI[2]));
    DEBUG(("TARGET = %f,%f,%f\n",target[0],target[1],target[2]));
    if(takePic > -1){ //takePic is used to make sure you take an accurate picture.
        takePic--;
    }
    if(time%60 == 0){
        for(int i = 0; i < 3; i++){
            goodPOI[i] = 1;
        }
        state = ST_GET_POI;
    }
    if((time > solarFlareBegin - 25)&&(time < solarFlareBegin)){
        if((state < ST_SHADOW)||(state > ST_SHADOW + 9)){
            state = ST_SHADOW; //if not in shadow state go there.
        }
    }
    else if(time == solarFlareBegin + 3){
        state = ST_GET_POI + 1;
    }
    else if (game.getNextFlare() != -1) {
	    solarFlareBegin = api.getTime() + game.getNextFlare();
	    DEBUG(("Next solar flare will occur at %ds.\n", solarFlareBegin));
	}
    switch(state){
        case ST_GET_POI: //POI selection
            closestPOI(goodPOI,POI,23);
            getPOILoc(POIproj, POIID, 23);
            takePic = 23; //25 probably needs to be changed. only good for the first cycle.
            DEBUG(("POI Coors = %f,%f,%f\n",POI[0],POI[1],POI[2]));
            state = ST_OUTER;
            break;
            
        case ST_GET_POI + 1: //if coming from shadow zone
            closestPOI(goodPOI,POI,30);
            getPOILoc(POIproj, POIID, 30);
            takePic = 30; //25 probably needs to be changed. only good for the first cycle.
            DEBUG(("POI Coors = %f,%f,%f\n",POI[0],POI[1],POI[2]));
            state = ST_OUTER;
            break;
            
        case ST_OUTER: //outer picture
            wp = false;
            //find closest place to take picture
            memcpy(target, POIproj, 3*sizeof(float)); //copy the projected POI location to target
            for(int i = 0; i < 3; i++){
                target[i] *= 0.465/mathVecMagnitude(POI,3); //dilate target to outer zone
            }
            mathVecSubtract(facing,origin,target,3); 
            mathVecNormalize(facing,3);
            api.setAttitudeTarget(facing);
            if(pathToTarget(myState,target,waypoint)){
                setWaypoint(waypoint,originalVecBetween);
                state = ST_OUTER + 1;
                for (int i = 0; i < 3; i++) tempTarget[i] = target[i];
            }
            else{
                state = ST_OUTER + 2;
            }
            break;
        
        case ST_OUTER + 1: //something is in the way so go to waypoint
            wp = true;
            mathVecSubtract(facing,POIproj,target,3); 
            mathVecNormalize(facing,3);
            api.setAttitudeTarget(facing);
            if(goToWaypoint(target,waypoint,tempTarget,originalVecBetween)){
                goToWaypoint(target,waypoint,tempTarget,originalVecBetween);
            }
            else{
                state = ST_OUTER + 2;
            }
            break;
            
        case ST_OUTER + 2://go to target
            mathVecSubtract(facing,origin,myState,3); 
            mathVecNormalize(facing,3);
            api.setAttitudeTarget(facing);
            toTarget();
            if(takePic == 0){
                game.takePic(POIID);
            }
            if(game.getMemoryFilled() > checkNum){
                checkNum++;
                getPOILoc(POIproj, POIID, 5);
                state = ST_INNER;
            }
            break;

      
        case ST_INNER: //inner picture
            wp = false;
            memcpy(target, POIproj, 3*sizeof(float));
            for(int i = 0; i < 3; i++){
                target[i] *= 0.34/mathVecMagnitude(POI,3);
            }
            mathVecSubtract(facing,POIproj,target,3); 
            mathVecNormalize(facing,3);
            api.setAttitudeTarget(facing);
            haulAssTowardTarget(target,8);
            state = ST_INNER + 1;
            break;
        
        case ST_INNER + 1: //after outer picture is taken, rush to inner zone. 
            mathVecSubtract(facing,POIproj,myState,3); 
            mathVecNormalize(facing,3);
            api.setAttitudeTarget(facing);
            if(distance(myState,target)<0.02){
                haulAssTowardTarget(target,10);
            }
            else{
                haulAssTowardTarget(target,2);
            }
            if(game.alignLine(POIID)){
                game.takePic(POIID);
            }
            if(game.getMemoryFilled() > checkNum){
                checkNum++;
                state = ST_UPLOAD;
            }
            break;
            
            
        case ST_SHADOW: //shadow zone
            wp = false;
            target[0] = 0.39; //arbitrary point in the shadow zone 
            target[1] = 0.00;
            target[2] = 0.00;
            if(pathToTarget(myState,target,waypoint)){
                setWaypoint(waypoint,originalVecBetween);
                goToWaypoint(target,waypoint,tempTarget,originalVecBetween);
                state = 31;
            }
            else{
                state = 32;
            }
            break;
            
        case ST_SHADOW + 1:
            wp = true;
            if(goToWaypoint(target,waypoint,tempTarget,originalVecBetween)){
                goToWaypoint(target,waypoint,tempTarget,originalVecBetween);
            }
            else{
                toTarget();
                state = 32;
            }
            break;
            
        case ST_SHADOW + 2:
            mathVecSubtract(facing,earth,myState,3); 
            mathVecNormalize(facing,3);
            api.setAttitudeTarget(facing);
            toTarget();
            game.uploadPic();
            break;
           
        case ST_UPLOAD: //upload, needs to be fixed
            for(int i = 0; i < 3; i++){
                uploadPos[i] = myState[i] / mathVecMagnitude(myState, 3) * 0.65;
            }
            mathVecSubtract(facing,earth,uploadPos,3); 
            mathVecNormalize(facing,3);
            api.setAttitudeTarget(facing);
            haulAssTowardTarget(uploadPos,1.8);
            state = ST_UPLOAD + 1;
            break;
            
        case ST_UPLOAD + 1: 
            //get to the closest place, keep trying to upload
            api.setAttitudeTarget(facing);
            if(distance(myState,origin)>0.54){
                api.setPositionTarget(myState);
                game.uploadPic();
            }
            else{
                if(distance(myState,origin)>0.51){
                    api.setPositionTarget(uploadPos);
                }
                else{
                    haulAssTowardTarget(uploadPos,1.8);
                }
            }
            if(picNum == 0){
                DEBUG(("LOOKING FOR POI"));
                state = ST_GET_POI;
            }
            break;
    }
}

bool pathToTarget(float pos[], float target[],float waypoint[]){
//returns true if there is something in the way
    float V1[3], V2[3], V3[3], V1p[3];
    mathVecSubtract(V1, target, pos, 3);
    for(int i = 0; i < 3; i++){
        V2[i] = pos[i];
    }
    for(int i = 0; i < 3; i++){
        V1p[i] = V1[i] * mathVecInner(V2,V1,3) / mathVecInner(V1,V1,3);
    }
    mathVecSubtract(V3, V2, V1p, 3);
    if(mathVecMagnitude(V3,3) < 0.31){
        for(int i = 0; i < 3; i++){
            if(V3[i] >= 0){
                waypoint[i] = V3[i] * (0.31 / mathVecMagnitude(V3,3)) + 0.05;
            }
            else{
                waypoint[i] = V3[i] * (0.31 / mathVecMagnitude(V3,3)) - 0.05;
            }
        }
        return true;
    }
    else{
        return false;
    }
}
void setWaypoint(float waypoint[],float originalVecBetween[]){
    float temp[3];
    while(pathToTarget(myState,waypoint, temp)){
        for(int i = 0; i < 3; i++){
            if(waypoint[i] >= 0){
                waypoint[i] += 0.01;
            }
            else{
                waypoint[i] -= 0.01;
            }
        }
    }
    for(int i = 0; i < 3; i++){
        if(waypoint[i] >= 0){
            waypoint[i] += 0.05;
        }
        else{
            waypoint[i] -= 0.05;
        }
    }
    mathVecSubtract(originalVecBetween, waypoint, myState, 3);
}
bool goToWaypoint(float target[],float waypoint[],float tempTarget[], float originalVecBetween[]){
    mathVecSubtract(vecBetween, waypoint, myState, 3);
    float temp[3];
    if(angleBetween(myState,target) > 150 *PI / 180){
        dilateValue(waypoint,tempTarget, -1.12,tempTarget);
    }
    else{
        dilateValue(waypoint,tempTarget, -1.02,tempTarget);
    }
    if(pathToTarget(myState,tempTarget,temp)){
        api.setVelocityTarget(originalVecBetween);
//magnitude stays the same as beginning magnitude
        return true; //going to waypoint still
    }
    else{
        return false; //not going to waypoint anymore
    }
}
void toTarget(){
    if(wp){
        if(distance(myState,target)>0.06){
            haulAssTowardTarget(target,1.2);
        }
        else{
            api.setPositionTarget(target);
        }
    }
    else{
        if(distance(myState,target)>0.08){
            haulAssTowardTarget(target,4);
        }
        else{
            api.setPositionTarget(target);
        }
    }
}

float angleBetween(float pt1[3], float pt2[3]){
    float dot;
    float vectorBetweenS1[3], vectorBetweenS2[3];
    mathVecSubtract(vectorBetweenS1,pt1, origin ,3);
    mathVecNormalize(vectorBetweenS1,3);
    mathVecSubtract(vectorBetweenS2,pt2,origin,3);
    mathVecNormalize(vectorBetweenS2,3);
    dot = mathVecInner(vectorBetweenS1, vectorBetweenS2, 3);
    return acosf(dot);
}



void closestPOI(int goodPOIS[], float nextPOI[], int t){
//nextPOI should be 3 float array
    float distances[3] = {0.00, 0.00, 0.00};
    for(int i = 0; i<3; i++){
        if(goodPOIS[i] == 0){
            distances[i] = 100.00f; //absurd distance which will not appear so the already gone to POI is avoided
        }
        else{
            float POIN[3] = {0.00, 0.00, 0.00};
            getPOILoc(POIN,i,t);
            distances[i] = distance(myState,POIN);
        }
    }
    float shortestdistance = 50.00; //placeholder number
    for(int i = 0; i<3; i++){
        if (distances[i]<= shortestdistance){
            shortestdistance = distances[i];
            POIID = i;
        }
    }
    getPOILoc(nextPOI,POIID,t);
}

bool getPOILoc(float pos[3], int id, float t) { //returns true if it "flips" in that time
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


void haulAssTowardTarget(float target[], float scalar) {
    // makes you go in the direction of target, but scalar times faster
    float scaledTarget[3];
    memcpy(scaledTarget, myState, 3*sizeof(float));
    for (int i = 0; i < 3; i++) scaledTarget[i]+= scalar * (target[i] - myState[i]);
    api.setPositionTarget(scaledTarget);
}

float distance(float p1[], float p2[]){
    float d=0;
    for(int i = 0; i<3; i++){
        d += (p2[i]-p1[i])*(p2[i]-p1[i]);
    }
    return sqrtf(d);
}
void dilateValue(float pt1[3], float pt2[3], float dilation , float dst[3]){
    for(int i=0; i < 3; i++){
        dst[i] = dilation * (pt1[i] - pt2[i]) + pt1[i];
    }
}
