#include "kMeans.h"
#include "defintion.h"
#include "MPI.h"
#include "OMP.h"

int main(int argc, char *argv[])
{

	int numprocs, myid;
	
	MPI_Status status;
	MPI_Datatype pointType, clusterType;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	createMpiPoint(&pointType);
	createMpiCluster(&clusterType);

	if (numprocs == 1) {
		printf("to run this program need at lest two processes");
		MPI_Abort(MPI_COMM_WORLD, 0);
	}

	if (myid == MASTER)
	{
		double startTime = MPI_Wtime();
		kMeans(&status, numprocs, pointType, clusterType);
		double endTime = MPI_Wtime();
		printf("----------------------done!!!!!!!--------------------\n");
		fflush(stdout);
		printf("running time = %lf\n", endTime - startTime);
		fflush(stdout);
	}
	else {//slave
		  /*
		  1. recv from master LIMIT,dT,T ,K (MPI).
		  2. recv from master number of points,set of points,set of cluster (MPI).
		  3. match point to cluster (OMP).
		  4. sum all points in cluster (OMP).
		  5. send to master set of match points,set of sum point,if point move (MPI).
		  6. recv if run again slave (MPI).
		  7. check the termination condition – no points move to other clusters or maximum iteration LIMIT was made.
		  8. repeat from 2 till the termination condition fulfills.
		  9. recv if cluster found (MPI).
		  10.check the termination condition - if t<T and no cluster was found .
		  11.repeat from 2 till the termination condition fulfills.
		  */
		int numPoints, numOfClusters, isClasterChange, toRunAgain, limit, isFoundCluster = 0;
		double T, dt;
		Point* slavePoints = NULL;
		Cluster* slaveClusters = NULL;

/* 1. */recvLimitTdtFromMaster(&limit, &T, &dt,&numOfClusters, &status);
		slaveClusters = (Cluster*)malloc(sizeof(Cluster)*(numOfClusters));

		for (double t = 0; t < T && isFoundCluster == 0; t += 0.1) {
			toRunAgain = 1;
			for (int l = 1; l<= limit && toRunAgain == 1; l++) {
				isClasterChange = 0;
				
/* 2. */		MPI_Recv(&numPoints, 1, MPI_INT, MASTER, TAG, MPI_COMM_WORLD, &status);
	
				slavePoints = (Point*)malloc(sizeof(Point)* (numPoints));
			
				MPI_Recv(slavePoints, numPoints, pointType, MASTER, TAG, MPI_COMM_WORLD, &status);
				MPI_Recv(slaveClusters, numOfClusters, clusterType, MASTER, TAG, MPI_COMM_WORLD, &status);
				
/* 3. */		matchPointToCluster(numOfClusters, numPoints, slavePoints, slaveClusters, &isClasterChange);
/* 4. */		calculateCenterCluster(slavePoints, slaveClusters, numPoints, numOfClusters);
				
/* 5. */		sendResultToMaster(slavePoints,slaveClusters,numPoints,numOfClusters,isClasterChange,pointType,clusterType);
/* 6. */		MPI_Recv(&toRunAgain, 1, MPI_INT, MASTER, TAG, MPI_COMM_WORLD, &status);
			
/*7.+8.*/	  }//end limit loop 

/* 9. */	MPI_Recv(&isFoundCluster, 1, MPI_INT, MASTER, TAG, MPI_COMM_WORLD, &status);

/* 10.+11.*/}  //end time loop

		free(slavePoints);
		free(slaveClusters);

	}//end slave

	MPI_Finalize();
	return 0;
}


