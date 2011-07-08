
#include "math/Angle.h"

float AngleDifference(float d, float e) {
	
	float da = e - d;
	
	if(da > 180.f) {
		da -= 360.f;
	} else if(da < -180.f) {
		da += 360.f;
	}
	
	return da;
}

float InterpolateAngle(float a1, float a2, const float pour) {
	
	float a = MAKEANGLE(a1);
	
	float diff = AngleDifference(a, MAKEANGLE(a2));
	
	return MAKEANGLE(a + (diff * pour));
}
