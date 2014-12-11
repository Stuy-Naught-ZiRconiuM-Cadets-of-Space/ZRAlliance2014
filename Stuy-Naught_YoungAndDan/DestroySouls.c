//CODE SHOULD BE IMPLEMENTED IN THIS WAY: UPLOAD POS[0] = 0.5f; AND THE X AND Y CORS ARE 0.0F;
/*if(!destroySouls()){
			    setPositionTarget(uploadPos,1);
			}*/


bool destroySouls(){
    //gameplan: project the y and z cors, then project the xcor. 
    ZRState other;
    api.getOtherZRState(other);
    float target[3];
    if((other[0] > 0)&&(inShadow(other,target,2))){ //loose check that should be improved later. 
        setPositionTarget(target,1.2);
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
