#pragma once
#include "defintion.h"

void kMeans(MPI_Status* status, int numprocs, MPI_Datatype pointType, MPI_Datatype clusterType);
Point* readFromFile(int* numOfPoints, int* numOfClusters, double* time, double* dtime, int* limit, double* qm);
void writeClusterToFile(Cluster* allClusters, int numOfClusters, double time, double q);
void initializeCluster(Cluster* allCluster, Point* allPoints, int numOfClusters);
void calcilateQuality(int numOfPoints, int numOfClusters, Point* allPoints, Cluster* allCluster, double* q);
void calculateDiameterOfCluster(int numOfPoints, int numOfClusters, Point* allPoints, Cluster* allCluster);
double calculateDistance(double x1, double y1, double z1, double x2, double y2, double z2);
void resetCluster(Cluster* allCluster, int numOfClusters);
Cluster* mergeSlavesCenterClusters(Cluster* slavesClusters, int numClusters, int numprocs);
void updatePointsInCluster(int numOfClusters, int numOfPoints, Point* allPoints, Cluster* allCluster);