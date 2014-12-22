int mathVecRotateToBottom(float a[]) {
	// Rotates the picture vector so that it's close to da shadow zone
	a[2] = sqrtf(a[0]*a[0] + a[2]*a[2]); // Rotate so that it's vertical and pointing down
	a[0] = 0;
	mathVecRotationXZ(a,-0.3); // So it's not exactly at the top
}

int mathVecRotateToHorizontal(float a[]) {
	// Rotates the vector to negative x
	a[0] = -sqrtf(a[0]*a[0] + a[2]*a[2]);
	a[2] = 0;
	//mathVecRotationXZ(a,-0.5);
}

int mathVecRotateToTop(float a[]){
	// Rotates the vector a to the top
	a[2] = -sqrtf(a[0]*a[0] + a[2]*a[2]); // Rotate so that it's vertical and pointing down
	a[0] = 0;
	mathVecRotationXZ(a,0.3); // So it's not exactly at the top
}

