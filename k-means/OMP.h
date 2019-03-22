#pragma once
#include "defintion.h"

void matchPointToCluster(int numOfCluster, int numPoints, Point* allPoints, Cluster* allCluster, int* isClasterChange);
void calculateCenterCluster(Point* allPoints, Cluster* allClusters, int numPoints, int numClusters);