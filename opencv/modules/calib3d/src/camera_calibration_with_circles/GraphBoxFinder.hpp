#ifndef GRAPHBOXFINDER_HPP_
#define GRAPHBOXFINDER_HPP_

#include "precomp.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <set>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/floyd_warshall_shortest.hpp>
#include <boost/graph/exterior_property.hpp>
#include <boost/graph/undirected_graph.hpp>
#include <boost/graph/graph_utility.hpp>

#include "../../../features2d/include/opencv2/features2d/features2d.hpp"


using namespace cv;
using namespace boost;

typedef adjacency_list<vecS, vecS, undirectedS> Graph;

struct Path
{
  int firstVertex;
  int lastVertex;
  int length;

  Path(int first = -1, int last = -1, int len = -1)
  {
    firstVertex = first;
    lastVertex = last;
    length = len;
  }
};

struct GraphBoxFinderParameters
{
  GraphBoxFinderParameters();
  Size2f densityNeighborhoodSize;
  float minDensity;
  int kmeansAttempts;
  float maxBasisDistance;
  int minDistanceToAddKeypoint;
  int keypointScale;
  int minBestPathLength;
  int minGraphConfidence;
  float vertexGain;
  float vertexPenalty;
  float edgeGain;
  float edgePenalty;
  int briefShift;
};

class GraphBoxFinder
{
public:
  GraphBoxFinder(int boxWidth, int boxHeight, Mat &testImage,
                 const vector<KeyPoint> &testKeypoints, const GraphBoxFinderParameters &parameters =
                     GraphBoxFinderParameters());
  void findHoles();

  void drawBasis(const vector<Point2f> &basis, Point2f origin, Mat &drawImg) const;
  void drawBasisGraphs(const vector<Graph> &basisGraphs, Mat &drawImg, bool drawEdges = true, bool drawVertices = true) const;
  void drawHoles(const Mat &srcImage, Mat &drawImage) const;



  void computeEdgeVectors(vector<Point2f> &vectors, float maxEdgeLength, Mat *drawImage=0) const;
  void computeEdgeVectorsOfRNG (vector<Point2f> &vectors, Mat *drawImage=0) const;
  void filterOutliersByDensity(const vector<Point2f> &samples, vector<Point2f> &filteredSamples);
  void findBasis(const vector<Point2f> &samples, vector<Point2f> &basis);
  void computeBasisGraphs(const vector<Point2f> &basis, vector<Graph> &basisGraphs);
  void findMCS(const vector<Point2f> &basis, vector<Graph> &basisGraphs);

  void getHoles( vector<Point2f> &holes );
  vector<vector<int> > holes;
private:
  void readBestDescriptors(string trainKeypointsFilename);
  //void computeEdgeVectors(vector<Point2f> &vectors) const;
  //void filterOutliersByDensity(const vector<Point2f> &samples, vector<Point2f> &filteredSamples);
  //void findBasis(const vector<Point2f> &samples, vector<Point2f> &basis);
  //void computeBasisGraphs(const vector<Point2f> &basis, vector<Graph> &basisGraphs);
  //void findMCS(const vector<Point2f> &basis, vector<Graph> &basisGraphs);
  void findLongestPath(vector<Graph> &basisGraphs, Path &bestPath);
  void computePointsInPath(Point2f basisVec, const Path &path, vector<int> &points) const;
  float computeGraphConfidence(const vector<Graph> &basisGraphs, bool addRow, const vector<int> &points, const vector<
      int> &seeds);
  void addHolesByGraph(const vector<Graph> &basisGraphs, bool addRow, Point2f basisVec);
  void findRemainingHoles(const vector<Point2f> &basis);
  float computeBriefConfidence(const vector<int> &indices);
  void addHolesByBrief(bool addRow, Point2f basisVec);

  int findNearestKeypoint(Point2f pt) const;
  void addPoint(Point2f pt, vector<int> &points);
  void findCandidateLine(vector<int> &line, int seedLineIdx, bool addRow, Point2f basisVec, vector<int> &seeds);
  void findCandidateHoles(vector<int> &above, vector<int> &below, bool addRow, Point2f basisVec,
                          vector<int> &aboveSeeds, vector<int> &belowSeeds);

  static void indices2keypoints(const vector<KeyPoint> &keypoints, const vector<int> &points, vector<KeyPoint> &result);
  static void insertWinner(float aboveConfidence, float belowConfidence, float minConfidence,
                           bool addRow,
                           const vector<int> &above, const vector<int> &below, vector<vector<int> > &holes);
  static bool areVerticesAdjacent(const Graph &graph, int vertex1, int vertex2);

  vector<KeyPoint> keypoints;
  Mat testImage;
  Ptr<DescriptorExtractor> descriptorExtractor;
  Ptr<DescriptorMatcher> descriptorMatcher;
  Mat trainDescriptors;
  //vector<vector<int> > holes;

  const int boxWidth;
  const int boxHeight;
  GraphBoxFinderParameters parameters;
};

#endif /* GRAPHBOXFINDER_HPP_ */
