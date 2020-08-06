#ifndef KMEANS_H
#define KMEANS_H

#include "Point.h"
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <cfloat>
#include <iomanip>

using namespace std;

class Kmeans : public Point
{
private:
	int K, I, D, numRun, totPoints;
	double threshold;
	vector<Point> centroids;

public:
	/* Default Constructor */
	Kmeans()
	{
		// Default Constructor
		K = 0;
		I = 0;
		threshold = 0.0;
		numRun = 0;
		totPoints = 0;
		D = 0;

	}

	/* Constructor */
	Kmeans(int K, int iterations, double threshold, int numRun, int N, int dimensions)
	{
		this->K = K;
		this->I = iterations;
		this->threshold = threshold;
		this->numRun = numRun;
		this->totPoints = N;
		this->D = dimensions;

	}

	/* Calculates the centeroid of a cluster based on the mean of all coordinate values 
	   of points within the cluster */
	void calculateCentroids(vector<Point>* allPoints) {
		vector<int> nPoints(K, 0);
		vector<Point> resetCentroids;
		
		// Create point vector of empty points
		for (int i = 0; i < K; i++) {
			Point pt(D);
			resetCentroids.push_back(pt);
		}
		
		// Swap empty point vector with centroid vector to reset values
		centroids.swap(resetCentroids);

		// Sum up values for each coordinate and save in centroids vector
		for (int i = 0; i < totPoints; i++) {
			int clusterId = (*allPoints)[i].getCluster();
			nPoints[clusterId] += 1;
			for (int j = 0; j < D; j++) {
				double sum = centroids[clusterId].getValue(j) + (*allPoints)[i].getValue(j);
				centroids[clusterId].setValueByPos(j, sum);
			}

			(*allPoints)[i].setMinDist(DBL_MAX); // reset distance
		}

		// Divide values of centroid coordinates by # of points in cluster to obtain mean
		for (int i = 0; i < K; i++) {
			for (int j = 0; j < D; j++) {
				double mean = centroids[i].getValue(j) / nPoints[i];
				centroids[i].setValueByPos(j, mean);
			}
		}
	}

	/* The batch (Lloyd's) K-mean algorithm 
	   returns the Sum of Squared Error */
	double run(vector<Point>* allPoints)
	{

		calculateCentroids(allPoints);

		cout << "clusters initialized = " << centroids.size() << endl << endl;

		cout << "Running K-Means Clustering... " << endl;

		int currentIter = 1;
		double prevSSE = INT_MAX;
		double currentSSE = 0.0;
		bool belowThreshold = false;

		cout << "\nRun " << numRun << endl;
		cout << "-----" << endl;

		do {
			cout << "Iteration " << currentIter << ": ";

			// Add all points to their nearest cluster
			for (vector<Point>::iterator it = allPoints->begin();
				it != allPoints->end(); ++it) {
				Point& p = *it;

				for (vector<Point>::iterator c = begin(centroids);
					c != end(centroids); ++c) {
					int clusterId = c - begin(centroids);
					double dist = c->distance(p);
					if (dist <= p.getMinDist()) {
						p.setMinDist(dist);
						p.setCluster(clusterId);
					}
				}

				// Add min distance to SSE
				currentSSE += it->getMinDist();
			}

			// Print total SSE
			cout << "SSE = " << std::setprecision(7) << currentSSE << endl;

			// Check for threshold
			if ((abs(currentSSE - prevSSE) / prevSSE) < threshold) {
				belowThreshold = true;
			}
			
			prevSSE = currentSSE; // Save SSE for next iteration
			currentSSE = 0.0;	  // Reset currentSSE

			// Recalculate the centroid of each cluster
			calculateCentroids(allPoints);

			currentIter++;
		} while (currentIter <= I && !belowThreshold);

		return prevSSE;
	}
};

#endif
