#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <iomanip>
#include <vector>
#include <numeric>
#include <cmath>
#include <sstream>
#include <random>
#include "Point.h"
#include "Kmeans.h"
#include "ExternalValidation.h"

using namespace std;

/* Normalize data column-wise by coordinate setting the maximum value
   to 1 and minimum value to zero */
void minMaxNormalize(vector<Point>* allPoints, int totPoints, int dimensions) {
	double min, max;

	for (int i = 0; i < dimensions; i++) {
		// Holds the ith value from each point. Initialized to zero
		vector<double> column(totPoints, 0);

		// Fill column vector with ith value from each point
		for (int j = 0; j < totPoints; j++) {
			column[j] = (*allPoints)[j].getValue(i);
		}
		
		// Find min and max value
		auto result = minmax_element(column.begin(), column.end());
		min = *result.first;
		max = *result.second;

		if (max - min == 0) {
			max = 1;
			min = 0;
		}
		// Normalize values to be between 0 and 1 using the formula:
		// v' = (v - min) / (max - min)
		for (int f = 0; f < totPoints; f++) {
			double normalized = ((*allPoints)[f].getValue(i) - min) / (max - min);
			(*allPoints)[f].setValueByPos(i, normalized);
		}
	}
}

/* Driver. Takes in command-line arguments in the following order:
   [dataFilename] [maxIterations] [threshold] [maxRuns]
   
   Calculates the best run based on lowest Sum of Squeared Error */
int main(int argc, char** argv) {
	int K, I, R, N, D, bestRun, A1;
	double T, rand,jaccard,fowlkes;
	double SSE, minSSE;				// Hold return SSE value from kmeans run
	string F;
	string line;					// Stores data from input file
	vector<Point> all_points;		// Store all created points
	vector<double>	allSSEs,		// Store all SSEs from all runs
					allRand,		// Store all Rand Indices
					allJaccard,		// Store all Jaccard Coefficients
					allFowlkes;		// Store all Fowlkes-Mallows Indices

	// Need 4 arguments (except executable filename) to run, else exit
	if (argc != 5) {
		cout << "Error: command-line argument count mismatch.";
		return 1;
	}

	// Initializing Variables
	I = atoi(argv[2]); // I: maximum number of iterations in a run
	T = atof(argv[3]); // T: convergence threshold
	R = atoi(argv[4]); // R: number of runs

	// Open File for reading
	F = argv[1];		// F: filename
	fstream in_file("C:/Users/rossm/source/repos/ExternalValidation/ExternalValidation/phase4_data_sets/" + F, ios::in);

	// Test for open
	if (!in_file) {
		cout << "Cannot open " << F << " for input" << endl;
		return 1;
	}

	// Read first line
	getline(in_file, line);
	istringstream(line) >> N >> A1 >> K;// First line contains N: number of points,
										// A1: # of attributes + 1, and K: # of clusters
	D = A1 - 1; // dimensions equals number of attributes

	// Read from file and store points
	while (getline(in_file, line)) {
		Point point(line,D);
		all_points.push_back(point);
	}
	in_file.close();
	
	// Check all points imported
	if (all_points.size() != N) {
		cout << "\nError: incorrect number of data points imported." << endl << endl;
	}
	else
	{
		cout << "\nData Imported sucessfully" << endl << endl;
	}

	// Return if number of clusters > number of points
	if ((int)all_points.size() < K) {
		cout << "Error: Number of clusters greater than number of points." << endl;
	}
	
	// Normalize data
	minMaxNormalize(&all_points, N, D);

	// Update filename for writing
	F.erase(F.end() - 4,F.end());
	F += "Output.txt";

	// Run k-means R times 
	for (int i = 0; i < R; i++) {

		// initialize random number generator
		random_device rd;
		mt19937 rng(rd());
		uniform_int_distribution<int> uni(0,K-1);

		// Random Partition Initialization: assign each point to a cluster 
		// uniformly at random
		for (int j = 0; j < N; j++) {
			auto random_integer = uni(rng);
			all_points[j].setCluster(random_integer);
		}

		Kmeans kmeans(K, I, T, (i + 1), N, D);
		SSE = kmeans.run(&all_points);
		allSSEs.push_back(SSE);

		// Compute External validation measures for this run
		ExternalValidation external(K, N);
		external.calculateTable(&all_points);
		external.calculateMeasures();
		rand = external.randIndex();
		jaccard = external.jaccardCoefficient();
		fowlkes = external.fowlkesMallows();

		// Save external validation measures for this run
		allRand.push_back(rand);
		allJaccard.push_back(jaccard);
		allFowlkes.push_back(fowlkes);
	}

	// Open file for writing
	fstream out_file(F, ios::out | ios::app);
	if (!out_file) {
		cout << "Error: Cannot open " << F << " for writing" << endl;
		return 1;
	}

	// Find and print lowest SSE
	bestRun = min_element(allSSEs.begin(), allSSEs.end()) - allSSEs.begin() + 1;
	minSSE = *min_element(allSSEs.begin(), allSSEs.end());

	cout << setprecision(7) << "Lowest SSE at Run " << bestRun << ": " << minSSE << endl;
	out_file << setprecision(7) << "Lowest SSE at Run " << bestRun << ": " << minSSE << endl;

	// Find and print highest Rand Index
	bestRun = max_element(allRand.begin(), allRand.end()) - allRand.begin() + 1;
	rand = *max_element(allRand.begin(), allRand.end());

	cout << setprecision(7) << "Highest Rand Index at Run " << bestRun << ": " << rand << endl;
	out_file << setprecision(7) << "Highest Rand Index at Run " << bestRun << ": " << rand << endl;

	// Find and print highest Jaccard Coefficient
	bestRun = max_element(allJaccard.begin(), allJaccard.end()) - allJaccard.begin() + 1;
	jaccard = *max_element(allJaccard.begin(), allJaccard.end());

	cout << setprecision(7) << "Highest Jaccard Coefficient at Run " << bestRun << ": " << jaccard << endl;
	out_file << setprecision(7) << "Highest Jaccard Coefficient at Run " << bestRun << ": " << jaccard << endl;

	// Find and print highest Fowlkes-Mallows Index
	bestRun = max_element(allFowlkes.begin(), allFowlkes.end()) - allFowlkes.begin() + 1;
	fowlkes = *max_element(allFowlkes.begin(), allFowlkes.end());

	cout << setprecision(7) << "Highest Fowlkes-Mallows Index at Run " << bestRun << ": " << fowlkes << endl;
	out_file << setprecision(7) << "Highest Fowlkes-Mallows Index at Run " << bestRun << ": " << fowlkes << endl;

	out_file.close();

	return 0;
}