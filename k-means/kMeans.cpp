
#include "kMeans.h"
#include "MPI.h"
#include "OMP.h"
#include "kernel.h"

// K-Means algorithm to find K clusters
void kMeans(MPI_Status* status, int numprocs, MPI_Datatype pointType, MPI_Datatype clusterType) {
	/*
	1. read points from file to pointer- allPoints .
	2. initalize first K points as centers of clusters.
	3. send to slaves LIMIT ,dT,T,K (MPI).
	4. send to slaves number of points,set of points,set of clusters(MPI).
	5. recv from slave pointer of match points,pointer of sum points cluster and if the cluster change (MPI).
	6. claculate the new center of cluster -avg of points (OMP).
	7. send to slave if run again (MPI).
	8. check the termination condition – no points move to other clusters or maximum iteration LIMIT was made.
	9. repeat from 4 till the termination condition fulfills
	10.evaluate the Quality of the clusters found, Calculate q.
	11.send to slaves if the cluster found.
	12.if no cluster found - update points by time with CUDA
	13.check the termination condition - if t<T and no cluster was found go step 4 till the termination condition fulfills.
	14.write to file clusters.
	*/

	int N;     //			num of points
	int K;     //			num of clusters to find
	int LIMIT;  //			the maximum number of iterations for K-MEAN algorithm.
	double T;  //			defines the end of time interval[0, T]
	double dT; //			defines moments t = n*dT, n = { 0, 1, 2, … , T / dT } for which calculate the clusters and the quality
	double QM; //			quality measure to stop
	int  sizeOfBlocks, isClasterChange = 0, isFoundCluster = 0, toRunSlave = 0;
	double q, t;
	Point* allPoints;  //pointer to all set of points read from file
	Point* slavePoints;
	Cluster* allCluster = NULL; 
	Cluster* slavesClusters = NULL;

/* 1. */ allPoints = readFromFile(&N, &K, &T, &dT, &LIMIT, &QM);
		 allCluster = (Cluster*)malloc(sizeof(Cluster)*K);
		 slavesClusters = (Cluster*)malloc(sizeof(Cluster)*(K*(numprocs - 1)));
	
/* 2. */ initializeCluster(allCluster, allPoints, K);   
/* 3. */ sendKLimitTdTtoSlave(numprocs, LIMIT, T, dT,K);  

		for (t = 0; t < T && isFoundCluster == 0; t += dT) {
			toRunSlave = 1;
			for ( int l = 1; l <= LIMIT && toRunSlave == 1; l++) {
				printf("iteration %d\n", l);
				fflush(stdout);
				toRunSlave = 0;
				sizeOfBlocks = N / (numprocs - 1);
				int remainder = (N - ((numprocs - 1)*sizeOfBlocks));

/* 4. */		sendPointsToSlaves(K, N, numprocs, allPoints, allCluster, pointType, clusterType, sizeOfBlocks);

/* 5. */        for (int j = 1; j < numprocs; j++) {  
					if (j == numprocs - 1 && N % (numprocs - 1) != 0)
						sizeOfBlocks = sizeOfBlocks + remainder;
					MPI_Recv(&allPoints[(j - 1) * (sizeOfBlocks - remainder)], sizeOfBlocks, pointType, j, TAG, MPI_COMM_WORLD, status);
					MPI_Recv(&slavesClusters[(j - 1)*K], K, clusterType, j, TAG, MPI_COMM_WORLD, status);
					MPI_Recv(&isClasterChange, 1, MPI_INT, j, TAG, MPI_COMM_WORLD, status);
					if (isClasterChange == 1)
						toRunSlave += 1;
				}
				
/* 6. */        allCluster = mergeSlavesCenterClusters(slavesClusters, K, numprocs - 1);
			
				if (toRunSlave > 0) //not found cluster yet
					toRunSlave = 1;
				
				if (l==LIMIT)//last iteration stop the slaves
					toRunSlave = 0;
			
/* 7. */	    for (int i = 1; i < numprocs; i++)  
					MPI_Send(&toRunSlave, 1, MPI_INT, i, TAG, MPI_COMM_WORLD);

/* 8. */     }//end limit loop   
				updatePointsInCluster(K, N, allPoints, allCluster);//update to cluster his points.
				calculateDiameterOfCluster(N, K, allPoints, allCluster);
/* 10.*/		calcilateQuality(N, K, allPoints, allCluster, &q); 

				if (q < QM) 
					isFoundCluster = 1;//found cluser because all points not moved clusters

/* 12. */		for (int i = 1; i < numprocs; i++)       
					MPI_Send(&isFoundCluster, 1, MPI_INT, i, TAG, MPI_COMM_WORLD);

/* 13. */		if (isFoundCluster == 0 && (t + dT)<T) {
					cudaError_t cudaStatus = updatePointByTime_CUDA(allPoints, allPoints, N, t + dT);
					if (cudaStatus != cudaSuccess) 
						fprintf(stderr, "updatePointByTime_CUDA failed!");
				}


/* 14. */	}  //end time loop 
		
/* 11. */writeClusterToFile(allCluster, K, t-dT, q);
		free(allPoints);
		for(int i=0;i<K;i++)
			free(allCluster[i].indexsPointsInCluster);
		free(allCluster);
		free(slavesClusters);
	}

// read points from input file
Point* readFromFile(int* numOfPoints, int* numOfClusters, double* time, double* dtime, int* limit, double* qm)
{
	FILE* file; 
	fopen_s(&file,INPUT_FILE_NAME, "r");
	if (!file)
	{
		printf("the file not exist or incorrect path!\n");
		exit(1);
	}
	fscanf_s(file, "%d %d %lf %lf %d %lf", numOfPoints, numOfClusters, time, dtime, limit, qm);
	Point* points = (Point*)malloc(sizeof(Point)*(*numOfPoints));
	for (int i = 0; i < *numOfPoints; i++)
	{
		fscanf_s(file, "%lf %lf %lf %lf %lf %lf\n", &points[i].x, &points[i].y, &points[i].z, &points[i].vx, &points[i].vy, &points[i].vz);
		points[i].clusterId = -1;
		points[i].orgx = points[i].x;
		points[i].orgy = points[i].y;
		points[i].orgz = points[i].z;
	}
	fclose(file);
	return points;
}
//write cluster to output file
void writeClusterToFile(Cluster* allClusters, int numOfClusters, double time, double q) {
	FILE* file;
	fopen_s(&file,OUTPUT_FILE_NAME, "w");
	if (!file)
	{
		printf("the file not exist or incorrect path!\n");
		exit(1);
	}
	fprintf_s(file, "First occurrence at t = %lf with q = %lf \n\n", time, q);
	fprintf_s(file, "Centers of the clusters:\n\n");
	for (int i = 0; i < numOfClusters; i++) {
		fprintf_s(file, "%lf    %lf    %lf\n", allClusters[i].x, allClusters[i].y, allClusters[i].z);
	}
	fclose(file);
}
//initalize the first K clusters
void initializeCluster(Cluster* allCluster, Point* allPoints, int numOfClusters)
{
	for (int i = 0; i < numOfClusters; i++) {
		allCluster[i].clusterId = i;
		allCluster[i].x = allPoints[i].x;
		allCluster[i].y = allPoints[i].y;
		allCluster[i].z = allPoints[i].z;
		allCluster[i].diameter = 0;
		allCluster[i].countPoints = 0;
	}
}
//calculate distance between two points
double calculateDistance(double x1, double y1, double z1, double x2, double y2, double z2)
{
	return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2) + pow(z1 - z2, 2));
}
void resetCluster(Cluster* allCluster, int numOfClusters) {
	for (int i = 0; i < numOfClusters; i++)
	{
		allCluster[i].countPoints = 0;
		allCluster[i].diameter = 0;
	}
}
//update to cluster his points
void updatePointsInCluster(int numOfClusters,int numOfPoints,Point* allPoints, Cluster* allCluster) {
	for (int cluster = 0; cluster < numOfClusters; cluster++) {
		int* pointsInCluster = (int*)(malloc(sizeof(int)*allCluster[cluster].countPoints));
		int index = 0;
		for (int point = 0; point < numOfPoints; point++) {
			if (allPoints[point].clusterId == cluster) {
				pointsInCluster[index++] = point;
			}
		}
		allCluster[cluster].indexsPointsInCluster = pointsInCluster;
	}
}
//calculate the max distance between to points in cluster
void calculateDiameterOfCluster(int numOfPoints, int numOfClusters, Point* allPoints, Cluster* allCluster) {
	
	for (int c = 0; c < numOfClusters; c++) {
		double diameter = 0, currentDistance;
		for (int i = 0; i < allCluster[c].countPoints; i++) {
			for (int j = i + 1; j < allCluster[c].countPoints; j++) {
				int indexPoint1 = allCluster[c].indexsPointsInCluster[i];
				int indexPoint2= allCluster[c].indexsPointsInCluster[j];
				currentDistance = calculateDistance(allPoints[indexPoint1].x, allPoints[indexPoint1].y, allPoints[indexPoint1].z, allPoints[indexPoint2].x, allPoints[indexPoint2].y, allPoints[indexPoint2].z);
				if (currentDistance > diameter)
					diameter = currentDistance;
			}
		}
		
		allCluster[c].diameter = diameter;
	}
}


void calcilateQuality(int numOfPoints, int numOfClusters, Point* allPoints, Cluster* allCluster, double* q) {
	double quality = 0, distance;
	for (int i = 0; i < numOfClusters; i++) {
		for (int j = 0; j < numOfClusters; j++) {
			if (i != j) {
				distance = calculateDistance(allCluster[i].x, allCluster[i].y, allCluster[i].z, allCluster[j].x, allCluster[j].y, allCluster[j].z);
				quality += (allCluster[i].diameter / distance);
			}
		}
	}
	*q = quality / (numOfClusters*(numOfClusters - 1));
}

//merge the sum points in cluster from slaves
Cluster* mergeSlavesCenterClusters(Cluster* slavesClusters, int numClusters, int numprocs) {
	Cluster* mergeCenter = (Cluster*)malloc(sizeof(Cluster)*numClusters);
	for (int i = 0; i < numClusters; i++)
	{
		mergeCenter[i].clusterId = i;
		mergeCenter[i].x = 0;
		mergeCenter[i].y = 0;
		mergeCenter[i].z = 0;
		mergeCenter[i].diameter = 0;
		mergeCenter[i].countPoints = 0;
	}

	for (int i = 0; i<numClusters; i++) {
		for (int j = i; j <(numClusters*numprocs); j += numClusters)
		{
			mergeCenter[i].x += slavesClusters[j].x;
			mergeCenter[i].y += slavesClusters[j].y;
			mergeCenter[i].z += slavesClusters[j].z;
			mergeCenter[i].countPoints += slavesClusters[j].countPoints;

		}
		if (mergeCenter[i].countPoints > 0) {
			
			mergeCenter[i].x = mergeCenter[i].x / mergeCenter[i].countPoints;
			mergeCenter[i].y = mergeCenter[i].y / mergeCenter[i].countPoints;
			mergeCenter[i].z = mergeCenter[i].z / mergeCenter[i].countPoints;

		}

	}

	return mergeCenter;
}