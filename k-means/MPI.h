#pragma once
#include "defintion.h"
void createMpiPoint(MPI_Datatype *pointType);
void createMpiCluster(MPI_Datatype *clusterType);
void sendKLimitTdTtoSlave(int numprocs, int LIMIT, double T, double dT, int numCluster);
void sendPointsToSlaves(int numOfCluster, int numPoints, int numProcs, Point* allPoints, Cluster* allCluster, MPI_Datatype pointType, MPI_Datatype clusterType,int sizePointSet);
void sendResultToMaster(Point* slavePoints, Cluster* slaveClusters, int numPoints, int numOfClusters, int isClasterChange, MPI_Datatype pointType, MPI_Datatype clusterType);
void recvLimitTdtFromMaster(int* limit, double* T, double* dt,int* numCluster, MPI_Status* status);
