-----------------------------------------K-MEANS ALGORITHM---------------------------------------------- 
Nofar Uliel 312428410

*********you need to change the path of input.txt and output.txt in Headers->defintion.h**********

--Problem Definition---
Given a set of points in 3-dimensional space.
Initial position (xi, yi, zi) and velocity (vxi, vyi, vzi) are known for each point Pi. 
Its position at the given time t can be calculated as follows:
xi(t) = xi + t*vxi
yi(t) = yi + t*vyi
zi(t) = zi + t*vzi
Implement simplified K-Means algorithm to find K clusters.
Find a first occurrence during given time interval [0, T] when a system of K clusters has a Quality Measure q that is less than given value QM.

--K-MEANS ALGORITHM--
MPI-->master split the points between the slaves.
	  master send the points to slaves and recv the results from them.
OMP-->the slaves match points to clusters and summarizes the points in cluster.
CUDA--> update by time location of points.

--MASTER--
    1. read points from file.
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

--SLAVES--

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