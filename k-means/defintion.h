#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <mpi.h>
#include <omp.h>
#define MASTER 0
#define TAG 0
#define STRUCT_POINT_SIZE 10
#define STRUCT_CLUSTER_SIZE 7
#define MAX_BLOCK_SIZE_CUDA 1024
#define INPUT_FILE_NAME "D:\\k-means\\k-means\\input.txt"
#define OUTPUT_FILE_NAME "D:\\k-means\\k-means\\output.txt"
struct Point
{
	int clusterId;
	double x;
	double y;
	double z;
	double orgx;
	double orgy;
	double orgz;
	double vx;
	double vy;
	double vz;
};

struct Cluster
{
	int clusterId;
	double x;
	double y;
	double z;
	double diameter;
	int countPoints;
	int* indexsPointsInCluster;
};
