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
int memoryFilled;
int nextFlare;
int lastState;
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
	uploadPos[0] = 0.32f;
	uploadPos[1] = uploadPos[2] = 1.5f;
	lastState = GO_TO_SHADOW;
	memcpy(initialPosition,myState,3*sizeof(float));
}

void loop() {
	int i;

	api.getMyZRState(myState);
	DEBUG(("\n%d\n",state));

	for (i = 0; i < 3; i++) {game.getPOILoc(POI[i], i);}
	time++;
	nextFlare = game.getNextFlare();
	memoryFilled = game.getMemoryFilled();
    percentFuelRemaining = game.getFuelRemaining() * 100 / 90.0;

	if (nextFlare == -1) {
		DEBUG(("\nI don't know when the next flare is, so stop asking.\n"));
	}
	else if (nextFlare <= 30) {
		state = GO_TO_SHADOW;
		DEBUG(("\nnextFlare: %d\nOH NO IT'S A FLARE!!!!!\n",nextFlare));
	}

	if (time >= 230 || percentFuelRemaining <= 7) {
		state = GO_TO_SHADOW;
	}

	switch (state) {
		case Chose_POI:
			float currentMinDist = distance(POI[0], initialPosition);
			bestPOI = 0;
			for (i = 1 ; i < 3 ; i++) {
				if (distance(POI[i], initialPosition) < currentMinDist) {
					currentMinDist = distance(POI[i], initialPosition);
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
			    brakingPt[i] = 0.45 * brakingPt[i] / temp[0];
		    }
            
			setPositionTarget(brakingPt,1);
			mathVecSubtract(facing,origin,brakingPt,3);
			api.setAttitudeTarget(facing);

			break;

		case TakePic_One:
			
			if (memoryFilled == 1) {
				if (!getPOILoc(brakingPt, bestPOI, time+3)) {

					temp[0] = mathVecMagnitude(brakingPt, 3);
					for (i = 0 ; i < 3; i++) {
						brakingPt[i] = 0.41 * brakingPt[i] / temp[0];
					}

					mathVecSubtract(facing, origin, brakingPt, 3);
					api.setAttitudeTarget(facing);
					state = TakePic_Two;
				}
			}

			setPositionTarget(brakingPt,2);
			api.setAttitudeTarget(facing);
			if (game.alignLine(bestPOI)) {
				game.takePic(bestPOI);
				DEBUG(("\nI am the champion my friends\n"));
			}

			break;

		case TakePic_Two:

			if (memoryFilled == 2) {
				state = GO_TO_SHADOW;
			}

			setPositionTarget(brakingPt,1);
			mathVecSubtract(facing,origin,brakingPt,3);
			api.setAttitudeTarget(facing);

			break;

		case GO_TO_SHADOW:
			

	}
}
