#pragma once
#include "defintion.h"
cudaError_t updatePointByTime_CUDA(Point* allPoints, Point* result, int numOfPoints, double time);