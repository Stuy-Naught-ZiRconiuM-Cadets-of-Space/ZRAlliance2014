//Begin page functions
void M(float &theta, float &phi, float Dth, float Dph)
{
    float V[3]; 
    float A=angles(myState[0],myState[1],myState[2],P[0],P[1],P[2]);
    float D0=DistFromObj(P,myState);
    mathVecSubtract(V,P,myState,3);
    for(int i=0;i<=2;i++) V[i]=0.2*V[i];
    
    targetPos[0]=targetRadius*cosf(theta)*sinf(phi);
    targetPos[1]=targetRadius*sinf(theta)*sinf(phi); 
    targetPos[2]=targetRadius*cosf(phi);
    
    if(stage==0) api.setPositionTarget(targetPos);

    if(57.32*A>90) 
    {   
        theta=theta+color*Dth*PI/180/10;
        phi=phi-Dph*PI/180/10;
    }
    else if(D0>0.2&&time>60) api.setVelocityTarget(V);
            else if(stage==0) api.setPositionTarget(P);
}


int GoSearch()
{  
    if(time%60==0) check[0]=check[1]=check[2]=0;
   
    d0=DistFromObj(myState,poi0);
    d1=DistFromObj(myState,poi1);
    d2=DistFromObj(myState,poi2);
   
    if(check[0]==0&&check[1]==0&&check[2]==0)
    {
        if(d0<d1&&d0<d2) {for(int i=0;i<=2;i++) POI[i]=poi0[i]; return 0;}
        else if(d1<d0&&d1<d2) {for(int i=0;i<=2;i++) POI[i]=poi1[i]; return 1;}
        else if(d2<d0&&d2<d1) {for(int i=0;i<=2;i++) POI[i]=poi2[i]; return 2;}
    }
    if(check[0]!=0)
    {
        if(d1<d2) {for(int i=0;i<=2;i++) POI[i]=poi1[i]; return 1;}
        else {for(int i=0;i<=2;i++) POI[i]=poi2[i]; return 2;}
    }
    if(check[1]!=0)
    {
        if(d0<d2) {for(int i=0;i<=2;i++) POI[i]=poi0[i]; return 0;}
        else {for(int i=0;i<=2;i++) POI[i]=poi2[i]; return 2;}
    }
    if(check[2]!=0)
    {
        if(otherState[1]*poi1[1]<0) {for(int i=0;i<=2;i++) POI[i]=poi1[i]; return 1;}
            else if(otherState[1]*poi0[1]<0) {for(int i=0;i<=2;i++) POI[i]=poi0[i]; return 0;}
    }
}


void QuatAttitude(float poi[3])
{
    q[0]=(-poi[0])*sin(a/2.0);
    q[1]=(-poi[1])*sin(a/2.0);
    q[2]=(-poi[2])*sin(a/2.0);
    q[3]=cos(a/2.0);
    
    api.setAttitudeTarget(q);
}


float DistFromObj(float *target,float *who)
{   
    float dif[3];
    mathVecSubtract (dif,target,who,3);
    return mathVecMagnitude(dif,3);
}


float angles(float x1, float y1, float z1, float x2, float y2, float z2)
{ 
    float uv=x1*x2+y1*y2+z1*z2;
    float u_v= sqrtf(powf(x1,2)+powf(y1,2)+powf(z1,2))*sqrtf(powf(x2,2)+powf(y2,2)+powf(z2,2));
    float anglez=(uv/u_v);
    return acosf(anglez);
}


void AvoidSolarFlares()
{
    clock=game.getNextFlare();
    
    if(clock==1) 
    {
        game.turnOff();
        timeuntilturnon=0;
    }
    
    timeuntilturnon++;
    if(timeuntilturnon==3) game.turnOn();
    
    DEBUG(("SF: %d ",clock));
}

//End page functions
//Begin page init
ZRState myState, otherState;
float q[4];
float a;
float x;
float y; 
float d;
float d0;
float d1;
float d2;
float dest[3];
float stop[3];
float P[3];
float POI[3];
float poi0[3];
float poi1[3];
float poi2[3];
float center[3];
float targetRadius;
float targetPos[3];
float phi;
float theta;
int time;
int clock;
int timeuntilturnon;
int stage;
int color;
int id;
int check[3];

void init()
{ 
    stage=0; 
    a=1;
    time=0;
    timeuntilturnon=4;
    center[0]=center[1]=center[2]=0.0f;
    targetRadius=0.4f;
}
//End page init
//Begin page main
void loop()
{
    api.getMyZRState(myState);
    
    api.getOtherZRState(otherState);
 
    game.getPOILoc(poi0,0);
    game.getPOILoc(poi1,1);
    game.getPOILoc(poi2,2);
    
    float th2=angles( 0.5, 0, 0, POI[0], POI[1], 0);
    float th1=angles( 0.5, 0, 0, myState[0], myState[1], 0);
    float ph2=angles(0, 0, 0.5, POI[0], POI[1], POI[2]);
    float ph1=angles(0, 0, 0.5, myState[0], myState[1], myState[2]);
    float Dth=57.32*(th2-th1); 
    float Dph=-57.32*(ph2-ph1);
    
    if (myState[1]<0) {color=-1; x=2*PI-th1; y=ph1;}//red
	    else {color=1; y=ph1; x=th1;}//blue
    
    if(!(myState[0]>0.0f&&myState[0]<0.64f&&myState[1]>-0.2f&&myState[1]<0.2f&&myState[2]>-0.2f&&myState[2]<0.2f)) AvoidSolarFlares();
        else api.setControlMode(CTRL_PD,CTRL_PID);
  
    if (game.getFuelRemaining()==0) game.takePic(id);
  
    mathVecSubtract(stop,center,center,3);

    id=GoSearch(); 
    
    for(int i=0;i<=2;i++) {P[i]=2.0f*POI[i]; dest[i]=4.2f*POI[i];}
    
    d=DistFromObj(myState,center); 
    
    DEBUG(("d: %f ", d)); 
    DEBUG(("Dth: %f Dph: %f", Dth, Dph));
    DEBUG(("Stg: %d ",stage));
    DEBUG(("id: %d ",id));

    if(stage!=-1) M(x, y, Dth, Dph);
    
    QuatAttitude(POI);
    
    if(d<=0.33) stage=-1;
    
    if(stage==-1){
                    if(d>0.33) stage=0; 
                    
                    float defend[3];
                    mathVecSubtract(defend,myState,center,3);
                    
                    api.setVelocityTarget(defend);
                    
                    api.setControlMode(CTRL_PD,CTRL_PID);
                 }
    
    if(stage==0){
                    if(game.alignLine(id)&&game.getMemoryFilled()==0) 
                    {
                        if(d<0.42) {if(fabsf(Dth)<4&&fabsf(Dph)<4) game.takePic(id);}
                            else api.setPositionTarget(POI);
                    }
                    
                    api.setControlMode(CTRL_PD,CTRL_PID);
                    
                    if(game.getMemoryFilled()==1) stage++;
                    
                    if(clock<=10&&clock!=-1) stage=3; 
                }

    if(stage==1){
                    api.setPositionTarget(dest);
                    
                    api.setControlMode(CTRL_PD,CTRL_PID);
                    
                    if(game.alignLine(id)&&d<0.53&&d>0.42) game.takePic(id);
                    
                    if(game.getMemoryFilled()==2) stage++;
                    
                    if(time%60==0) {if(game.getMemoryFilled()==1) stage=2;
                                        else stage=0;}
                }
 
    if(stage==2){ 
                    if(game.getMemoryFilled()==0) {if(time%60>5) check[id]=1; stage=0;}
                    
                    if(game.getMemoryFilled()==1||game.getMemoryFilled()==2||d<0.45) 
                        api.setPositionTarget(dest);

                    if(d>=0.53) game.uploadPic();
                    
                    if(d>=0.485) api.setPositionTarget(center);
                }


    if(stage==3){
                    api.setVelocityTarget(stop);
                    
                    api.setControlMode(CTRL_PD,CTRL_PID);
                    
                    if(clock==-1) stage=0; 
                }
 
    time++;
}
//End page main
