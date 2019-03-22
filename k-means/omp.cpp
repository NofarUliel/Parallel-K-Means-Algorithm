#include "OMP.h"
#include "kMeans.h"

// find the min distance between point and cluster 
void matchPointToCluster(int numOfCluster, int numPoints, Point* allPoints, Cluster* allCluster, int* isClasterChange)
{
	double currentDistance =0, minDistance;
	int minClusterId=0;
	resetCluster(allCluster, numOfCluster);
#pragma omp parallel for private(currentDistance,minClusterId,minDistance)
	for (int i = 0; i < numPoints; i++) {
		minClusterId = 0;
		minDistance = calculateDistance(allPoints[i].x, allPoints[i].y, allPoints[i].z, allCluster[0].x, allCluster[0].y, allCluster[0].z);
		for (int j = 1; j < numOfCluster; j++) {
			currentDistance = calculateDistance(allPoints[i].x, allPoints[i].y, allPoints[i].z, allCluster[j].x, allCluster[j].y, allCluster[j].z);
			if (currentDistance <minDistance) {
				minClusterId = j;
				minDistance = currentDistance;
			}
		}
		if (allPoints[i].clusterId != minClusterId) 
			*isClasterChange = 1;// the cluster is change
		allPoints[i].clusterId = minClusterId;
		

	}
}
//calculate new center of cluster by avg points
void calculateCenterCluster(Point* allPoints, Cluster* allClusters, int numPoints, int numClusters) {
double sumX, sumY, sumZ;
#pragma omp parallel for private(sumX, sumY, sumZ)
	for (int c = 0; c < numClusters; c++) {
		sumX = 0;
		sumY = 0;
		sumZ = 0;
		for (int p = 0; p < numPoints; p++) {
			if (allPoints[p].clusterId == allClusters[c].clusterId) {
				allClusters[c].countPoints = allClusters[c].countPoints + 1;
				sumX += allPoints[p].x;
				sumY += allPoints[p].y;
				sumZ += allPoints[p].z;
			}
		}

		//update new sum center cluster
		allClusters[c].x = sumX;
		allClusters[c].y = sumY;
		allClusters[c].z = sumZ;


	}
}