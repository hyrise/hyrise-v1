#include <iostream>
#include <stdlib.h>
#include <sys/time.h>

using namespace std;

typedef struct timeval TIMEVAL_TYPE;

long time_diff(TIMEVAL_TYPE& begin, TIMEVAL_TYPE& end)
{
	return (end.tv_sec - begin.tv_sec) * 1000000 + (end.tv_usec - begin.tv_usec);
}

int main(int argc, char *argv[])
{
	TIMEVAL_TYPE begin, end;
	unsigned long a1M = sizeof(int) * 256 * 1000;
	unsigned long a10M = sizeof(int) * 256 * 10000;
	unsigned long a100M = sizeof(int) * 256 * 100000;
	unsigned long a200M = sizeof(int) * 256 * 200000;
	unsigned long a400M = sizeof(int) * 256 * 400000;
	unsigned long a600M = sizeof(int) * 256 * 600000;
	unsigned long a1000M = sizeof(int) * 256 * 1000000;
	
	char c;
	
	int* data;
	
	gettimeofday(&begin, 0);
	data = (int*) malloc(a1M);
	gettimeofday(&end, 0);
	free(data);
	cout << "Allocation of 1M took " << time_diff(begin, end) << endl;
	
	gettimeofday(&begin, 0);
	data = (int*) malloc(a10M);
	gettimeofday(&end, 0);
	free(data);
	cout << "Allocation of 10M took " << time_diff(begin, end) << endl;
	
	gettimeofday(&begin, 0);
	data = (int*) malloc(a100M);
	gettimeofday(&end, 0);
	free(data);
	cout << "Allocation of 100M took " << time_diff(begin, end) << endl;
	
	gettimeofday(&begin, 0);
	data = (int*) malloc(a200M);
	gettimeofday(&end, 0);
	free(data);
	cout << "Allocation of 200M took " << time_diff(begin, end) << endl;
	
	gettimeofday(&begin, 0);
	data = (int*) malloc(a400M);
	gettimeofday(&end, 0);
	free(data);
	cout << "Allocation of 400M took " << time_diff(begin, end) << endl;
	
	gettimeofday(&begin, 0);
	data = (int*) malloc(a600M);
	gettimeofday(&end, 0);
	free(data);
	cout << "Allocation of 600M took " << time_diff(begin, end) << endl;
	
	gettimeofday(&begin, 0);
	data = (int*) calloc(a1000M, 1);
	gettimeofday(&end, 0);
	
	free(data);
	cout << "Allocation of 1000M took " << time_diff(begin, end) << endl;
	
	return 0;
}
