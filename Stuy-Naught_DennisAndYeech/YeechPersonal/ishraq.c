void myShadow(){
	float other[12];
	api.getOtherZRState(other);
	if(other[0]>0.0f &&
	(fabsf(other[1]) - 0.2f <= 0.1f || fabsf(other[2])-0.2f <=0.1f)){
		if(fabsf(me[1]) - 0.2f > 0.15f && fabsf(me[2]) - 0.2f > 0.15f){
		api.setVelocityTarget({other[3], -.5f*other[4], -.5f*other[5]});
		}else{
			api.setVelocityTarget({other[3], 0, 0});
		}
	}else{
		setPositionTarget({0.32f, 0.0f, 0.0f}, 1);
	}
}
