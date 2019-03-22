#include "defintion.h"

void createMpiPoint(MPI_Datatype *pointType)
{
	Point point;
	int blocklen[STRUCT_POINT_SIZE] = { 1, 1, 1, 1, 1, 1, 1, 1 ,1,1 };
	MPI_Datatype type[STRUCT_POINT_SIZE] = { MPI_INT, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE,MPI_DOUBLE,MPI_DOUBLE,MPI_DOUBLE };
	MPI_Aint disp[STRUCT_POINT_SIZE];

	disp[0] = (char*)&point.clusterId - (char*)&point;
	disp[1] = (char*)&point.x - (char*)&point;
	disp[2] = (char*)&point.y - (char*)&point;
	disp[3] = (char*)&point.z - (char*)&point;
	disp[4] = (char*)&point.orgx - (char*)&point;
	disp[5] = (char*)&point.orgy - (char*)&point;
	disp[6] = (char*)&point.orgz - (char*)&point;
	disp[7] = (char*)&point.vx - (char*)&point;
	disp[8] = (char*)&point.vy - (char*)&point;
	disp[9] = (char*)&point.vz - (char*)&point;
	MPI_Type_create_struct(STRUCT_POINT_SIZE, blocklen, disp, type, pointType);
	MPI_Type_commit(pointType);
}

void createMpiCluster(MPI_Datatype *clusterType)
{
	Cluster cluster;
	int blocklen[STRUCT_CLUSTER_SIZE] = { 1, 1, 1, 1, 1 ,1,1 };
	MPI_Datatype type[STRUCT_CLUSTER_SIZE] = { MPI_INT, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE,MPI_DOUBLE, MPI_INT ,MPI_INT };
	MPI_Aint disp[STRUCT_CLUSTER_SIZE];

	disp[0] = (char*)&cluster.clusterId - (char*)&cluster;
	disp[1] = (char*)&cluster.x - (char*)&cluster;
	disp[2] = (char*)&cluster.y - (char*)&cluster;
	disp[3] = (char*)&cluster.z - (char*)&cluster;
	disp[4] = (char*)&cluster.diameter - (char*)&cluster;
	disp[5] = (char*)&cluster.countPoints - (char*)&cluster;
	disp[6] = (char*)&cluster.indexsPointsInCluster - (char*)&cluster;

	MPI_Type_create_struct(STRUCT_CLUSTER_SIZE, blocklen, disp, type, clusterType);
	MPI_Type_commit(clusterType);
}
//send to slave LIMIT dT T
void sendKLimitTdTtoSlave(int numprocs,int LIMIT,double T, double dT,int numCluster) {
	for (int i = 1; i < numprocs; i++) {
		MPI_Send(&LIMIT, 1, MPI_INT, i, TAG, MPI_COMM_WORLD);
		MPI_Send(&T, 1, MPI_DOUBLE, i, TAG, MPI_COMM_WORLD);
		MPI_Send(&dT, 1, MPI_DOUBLE, i, TAG, MPI_COMM_WORLD);
		MPI_Send(&numCluster, 1, MPI_INT, i, TAG, MPI_COMM_WORLD);
	}

}
//send to slave num points ,num clusters,set of points,set of clusters
void sendPointsToSlaves(int numOfCluster, int numPoints, int numProcs, Point* allPoints, Cluster* allCluster, MPI_Datatype pointType, MPI_Datatype clusterType, int sizePointSet)
{
	for (int i = 0; i < numOfCluster; i++)
		allCluster[i].countPoints = 0;

	for (int i = 1; i < numProcs; i++) {
		int remainder = (numPoints - ((numProcs - 1)*sizePointSet));
		if (i == numProcs - 1 && numPoints % (numProcs - 1) != 0)
			sizePointSet = sizePointSet + remainder;
		MPI_Send(&sizePointSet, 1, MPI_INT, i, TAG, MPI_COMM_WORLD);
		MPI_Send(&allPoints[(i - 1) * (sizePointSet - remainder)], sizePointSet, pointType, i, TAG, MPI_COMM_WORLD);
		MPI_Send(allCluster, numOfCluster, clusterType, i, TAG, MPI_COMM_WORLD);
		
	}
}
//send to master set of match points,set of sum points in cluster,if point move
void sendResultToMaster(Point* slavePoints,Cluster* slaveClusters,int numPoints,int numOfClusters,int isClasterChange, MPI_Datatype pointType, MPI_Datatype clusterType) {
	MPI_Send(slavePoints, numPoints, pointType, MASTER, TAG, MPI_COMM_WORLD);
	MPI_Send(slaveClusters, numOfClusters, clusterType, MASTER, TAG, MPI_COMM_WORLD);
	MPI_Send(&isClasterChange, 1, MPI_INT, MASTER, TAG, MPI_COMM_WORLD);
}
// slave recv LIMIT dT T
void recvLimitTdtFromMaster(int* limit,double* T,double* dt,int* numCluster, MPI_Status* status) {
	MPI_Recv(limit, 1, MPI_INT, MASTER, TAG, MPI_COMM_WORLD, status);
	MPI_Recv(T, 1, MPI_DOUBLE, MASTER, TAG, MPI_COMM_WORLD, status);
	MPI_Recv(dt, 1, MPI_DOUBLE, MASTER, TAG, MPI_COMM_WORLD, status);
	MPI_Recv(numCluster, 1, MPI_INT, MASTER, TAG, MPI_COMM_WORLD, status);
}
