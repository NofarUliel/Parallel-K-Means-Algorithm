#include "kernel.h"
__global__ void updatePointsByTime(Point* allPoints, Point* result, int numOfPoints, double time)
{
	int index = (blockIdx.x*blockDim.x) + threadIdx.x;
	if (index < numOfPoints) {
		result[index].x = allPoints[index].orgx + allPoints[index].vx * time;
		result[index].y = allPoints[index].orgy + allPoints[index].vy * time;
		result[index].z = allPoints[index].orgz + allPoints[index].vz * time;

	}

}

// Helper function for using CUDA to add vectors in parallel.
cudaError_t updatePointByTime_CUDA(Point* allPoints, Point* result, int numOfPoints, double time)
{
	Point *dev_points = 0;
	Point *dev_result = 0;
	cudaError_t cudaStatus;
	int numOfblock = numOfPoints / MAX_BLOCK_SIZE_CUDA;
	if (numOfPoints%MAX_BLOCK_SIZE_CUDA != 0)
		numOfblock = numOfPoints / MAX_BLOCK_SIZE_CUDA + 1;

	// Choose which GPU to run on, change this on a multi-GPU system.
	cudaStatus = cudaSetDevice(0);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?");
		goto Error;
	}
	// Allocate GPU buffers for three vectors (one input, one output)    .
	cudaStatus = cudaMalloc((void**)&dev_points, numOfPoints * sizeof(Point));
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMalloc failed!");
		goto Error;
	}
	cudaStatus = cudaMalloc((void**)&dev_result, numOfPoints * sizeof(Point));
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMalloc failed!");
		goto Error;
	}
	// Copy input vectors from host memory to GPU buffers.
	cudaStatus = cudaMemcpy(dev_points, allPoints, numOfPoints * sizeof(Point), cudaMemcpyHostToDevice);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMemcpy failed!");
		goto Error;
	}
	// Copy input vectors from host memory to GPU buffers.
	cudaStatus = cudaMemcpy(dev_result, result, numOfPoints * sizeof(Point), cudaMemcpyHostToDevice);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMemcpy failed!");
		goto Error;
	}
	// Launch a kernel on the GPU with one thread for each element.
	updatePointsByTime << <numOfblock, MAX_BLOCK_SIZE_CUDA >> >(dev_points, dev_result, numOfPoints, time);

	// Check for any errors launching the kernel
	cudaStatus = cudaGetLastError();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "addKernel launch failed: %s\n", cudaGetErrorString(cudaStatus));
		goto Error;
	}

	// cudaDeviceSynchronize waits for the kernel to finish, and returns
	// any errors encountered during the launch.
	cudaStatus = cudaDeviceSynchronize();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching addKernel!\n", cudaStatus);
		goto Error;
	}

	// Copy output vector from GPU buffer to host memory.
	cudaStatus = cudaMemcpy(result, dev_result, numOfPoints * sizeof(Point), cudaMemcpyDeviceToHost);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMemcpy failed!");
		goto Error;
	}

Error:
	cudaFree(dev_result);
	cudaFree(dev_points);

	return cudaStatus;
}