#include <camera_calibration_with_circles/GraphBoxFinder.hpp>

typedef graph_traits<Graph>::vertex_descriptor Vertex;
typedef graph_traits<Graph>::edge_descriptor Edge;

typedef exterior_vertex_property<Graph, int> DistanceProperty;
typedef DistanceProperty::matrix_type DistanceMatrix;
typedef DistanceProperty::matrix_map_type DistanceMatrixMap;
typedef property_traits<DistanceMatrixMap>::value_type DistanceMap;
typedef constant_property_map<Edge, int> WeightMap;

GraphBoxFinderParameters::GraphBoxFinderParameters()
{
  densityNeighborhoodSize = Size2f(4, 4);
  minDensity = 16;
  kmeansAttempts = 100;
  maxBasisDistance = 8;
  minDistanceToAddKeypoint = 15;
  keypointScale = 1;
  minBestPathLength = 6;
  minGraphConfidence = 9;
  vertexGain = 2;
  vertexPenalty = -5;
  edgeGain = 1;
  edgePenalty = -5;
  briefShift = 4;
}

GraphBoxFinder::GraphBoxFinder(int _boxWidth, int _boxHeight, Mat &_testImage,
                               const vector<KeyPoint> &testKeypoints, const GraphBoxFinderParameters &_parameters) :
  boxWidth(_boxWidth), boxHeight(_boxHeight)
{
  keypoints = testKeypoints;
  testImage = _testImage;

  //descriptorExtractor = new BriefDescriptorExtractor();
  //descriptorMatcher = new BruteForceMatcher<Hamming> ();
  //descriptorExtractor = new SurfDescriptorExtractor();
  //descriptorMatcher = new BruteForceMatcher<L2<float> > ();


  parameters = _parameters;
  srand(time(0));
}

void GraphBoxFinder::findHoles()
{
  vector<Point2f> vectors, filteredVectors, basis;
  //computeEdgeVectors(vectors);
  computeEdgeVectorsOfRNG(vectors);
  filterOutliersByDensity(vectors, filteredVectors);
  findBasis(filteredVectors, basis);
  vector<Graph> basisGraphs;
  computeBasisGraphs(basis, basisGraphs);
  findMCS(basis, basisGraphs);
  findRemainingHoles(basis);
}

void GraphBoxFinder::findMCS(const vector<Point2f> &basis, vector<Graph> &basisGraphs)
{
  Path longestPath;
  findLongestPath(basisGraphs, longestPath);

  vector<int> holesRow;
  computePointsInPath(basis[0], longestPath, holesRow);

  int confidenceBackup = parameters.minGraphConfidence;
  while( holesRow.size() > boxWidth )
  {
    holesRow.pop_back();
    holesRow.erase( holesRow.begin() );
  }

  holes.push_back(holesRow);

  //Mat longestPathImage;
  //drawHoles( testImage, longestPathImage );
  //Mat resized;
  //resize( longestPathImage, resized, Size() ,0.3, 0.3);
  //imshow("longestPath", resized );
  //imshow("longestPath", longestPathImage );

  int w = holes[0].size();
  int h = holes.size();

  parameters.minGraphConfidence = holes[0].size() * parameters.vertexGain + (holes[0].size() - 1) * parameters.edgeGain;
  for (int i = h; i < boxHeight; i++)
  {
    addHolesByGraph(basisGraphs, true, basis[1]);
  }

  parameters.minGraphConfidence = confidenceBackup;

  for (int i = w; i < boxWidth; i++)
  {
    addHolesByGraph(basisGraphs, false, basis[0]);
  }
}

void GraphBoxFinder::findRemainingHoles(const vector<Point2f> &basis)
{
  int w = holes[0].size();
  int h = holes.size();

  while (w != boxWidth || h != boxHeight)
  {
    bool addRow = true;
    if (h == boxHeight)
      addRow = false;

    addHolesByBrief(addRow, basis[addRow]);

    if (addRow)
      h++;
    else
      w++;
  }
}

void GraphBoxFinder::indices2keypoints(const vector<KeyPoint> &keypoints, const vector<int> &points,
                                       vector<KeyPoint> &result)
{
  result.clear();
  for (size_t i = 0; i < points.size(); i++)
  {
    result.push_back(keypoints[points[i]]);
  }
}

int GraphBoxFinder::findNearestKeypoint(Point2f pt) const
{
  int bestIdx = -1;
  float minDist = std::numeric_limits<float>::max();
  for (size_t i = 0; i < keypoints.size(); i++)
  {
    float dist = norm(pt - keypoints[i].pt);
    if (dist < minDist)
    {
      minDist = dist;
      bestIdx = i;
    }
  }
  return bestIdx;
}

void GraphBoxFinder::addPoint(Point2f pt, vector<int> &points)
{
  int ptIdx = findNearestKeypoint(pt);
  if (norm(keypoints[ptIdx].pt - pt) > parameters.minDistanceToAddKeypoint)
  {
    KeyPoint kpt = KeyPoint(pt, parameters.keypointScale);
    keypoints.push_back(kpt);
    points.push_back(keypoints.size() - 1);
  }
  else
  {
    points.push_back(ptIdx);
  }
}

float GraphBoxFinder::computeBriefConfidence(const vector<int> &indices)
{
  vector<KeyPoint> points;
  indices2keypoints(keypoints, indices, points);

  Mat descriptors;
  vector<DMatch> matches;

  int shift = parameters.briefShift;
  float result = 0.1;
  for (size_t kpIdx = 0; kpIdx < points.size(); kpIdx++)
  {
    vector<KeyPoint> shiftedPoints;
    for (int x = -shift; x <= shift; x++)
    {
      for (int y = -shift; y <= shift; y++)
      {
        shiftedPoints.push_back(KeyPoint(points[kpIdx].pt + Point2f(x, y), parameters.keypointScale));
      }
    }
    //descriptorExtractor->compute(testImage, shiftedPoints, descriptors);
    //descriptorMatcher->match(descriptors, trainDescriptors, matches);
    float minDist = std::numeric_limits<float>::max();
    for (size_t k = 0; k < matches.size(); k++)
    {
      if (matches[k].distance < minDist)
      {
        minDist = matches[k].distance;
      }
    }
    result += minDist;
  }
  return 1. / result;
}

void GraphBoxFinder::findCandidateLine(vector<int> &line, int seedLineIdx, bool addRow, Point2f basisVec,
                                       vector<int> &seeds)
{
  line.clear();
  seeds.clear();

  if (addRow)
  {
    for (size_t i = 0; i < holes[seedLineIdx].size(); i++)
    {
      Point2f pt = keypoints[holes[seedLineIdx][i]].pt + basisVec;
      addPoint(pt, line);
      seeds.push_back(holes[seedLineIdx][i]);
    }
  }
  else
  {
    for (size_t i = 0; i < holes.size(); i++)
    {
      Point2f pt = keypoints[holes[i][seedLineIdx]].pt + basisVec;
      addPoint(pt, line);
      seeds.push_back(holes[i][seedLineIdx]);
    }
  }

  assert( line.size() == seeds.size() );
}

void GraphBoxFinder::findCandidateHoles(vector<int> &above, vector<int> &below, bool addRow, Point2f basisVec, vector<
    int> &aboveSeeds, vector<int> &belowSeeds)
{
  above.clear();
  below.clear();
  aboveSeeds.clear();
  belowSeeds.clear();

  findCandidateLine(above, 0, addRow, -basisVec, aboveSeeds);
  int lastIdx = addRow ? holes.size() - 1 : holes[0].size() - 1;
  findCandidateLine(below, lastIdx, addRow, basisVec, belowSeeds);

  assert( below.size() == above.size() );
  assert( belowSeeds.size() == aboveSeeds.size() );
  assert( below.size() == belowSeeds.size() );
}

void GraphBoxFinder::insertWinner(float aboveConfidence, float belowConfidence, float minConfidence, bool addRow,
                                  const vector<int> &above, const vector<int> &below, vector<vector<int> > &holes)
{
  if (aboveConfidence < minConfidence && belowConfidence < minConfidence)
    return;

  if (addRow)
  {
    if (aboveConfidence >= belowConfidence)
    {
      holes.insert(holes.begin(), above);
    }
    else
    {
      holes.insert(holes.end(), below);
    }
  }
  else
  {
    if (aboveConfidence >= belowConfidence)
    {
      for (size_t i = 0; i < holes.size(); i++)
      {
        holes[i].insert(holes[i].begin(), above[i]);
      }
    }
    else
    {
      for (size_t i = 0; i < holes.size(); i++)
      {
        holes[i].insert(holes[i].end(), below[i]);
      }
    }
  }
}

void GraphBoxFinder::addHolesByBrief(bool addRow, Point2f basisVec)
{
  vector<int> aboveIndices, belowIndices;
  vector<int> aboveSeeds, belowSeeds;
  findCandidateHoles(aboveIndices, belowIndices, addRow, basisVec, aboveSeeds, belowSeeds);

  float aboveConfidence = computeBriefConfidence(aboveIndices);
  float belowConfidence = computeBriefConfidence(belowIndices);
  insertWinner(aboveConfidence, belowConfidence, -1, addRow, aboveIndices, belowIndices, holes);
}

bool GraphBoxFinder::areVerticesAdjacent(const Graph &graph, int vertex1, int vertex2)
{
  property_map<Graph, vertex_index_t>::type index = get(vertex_index, graph);

  bool areAdjacent = false;
  graph_traits<Graph>::adjacency_iterator ai;
  graph_traits<Graph>::adjacency_iterator ai_end;

  for (tie(ai, ai_end) = adjacent_vertices(vertex1, graph); ai != ai_end; ++ai)
  {
    if (*ai == index[vertex2])
      areAdjacent = true;
  }

  return areAdjacent;
}

float GraphBoxFinder::computeGraphConfidence(const vector<Graph> &basisGraphs, bool addRow, const vector<int> &points,
                                             const vector<int> &seeds)
{
  assert( points.size() == seeds.size() );
  float confidence = 0;
  const int vCount = num_vertices(basisGraphs[0]);

  for (size_t i = 0; i < seeds.size(); i++)
  {
    if (seeds[i] < vCount && points[i] < vCount)
    {
      if (!areVerticesAdjacent(basisGraphs[addRow], seeds[i], points[i]))
      {
        confidence += parameters.vertexPenalty;
      }
      else
      {
        confidence += parameters.vertexGain;
      }
    }
  }

  for (size_t i = 1; i < points.size(); i++)
  {
    if (points[i - 1] < vCount && points[i] < vCount)
    {
      if (!areVerticesAdjacent(basisGraphs[!addRow], points[i - 1], points[i]))
      {
        confidence += parameters.edgePenalty;
      }
      else
      {

        confidence += parameters.edgeGain;
      }
    }
  }
  return confidence;

}

void GraphBoxFinder::addHolesByGraph(const vector<Graph> &basisGraphs, bool addRow, Point2f basisVec)
{
  vector<int> above, below, aboveSeeds, belowSeeds;
  findCandidateHoles(above, below, addRow, basisVec, aboveSeeds, belowSeeds);
  float aboveConfidence = computeGraphConfidence(basisGraphs, addRow, above, aboveSeeds);
  float belowConfidence = computeGraphConfidence(basisGraphs, addRow, below, belowSeeds);

  insertWinner(aboveConfidence, belowConfidence, parameters.minGraphConfidence, addRow, above, below, holes);
}

void GraphBoxFinder::filterOutliersByDensity(const vector<Point2f> &samples, vector<Point2f> &filteredSamples)
{
  if( samples.empty() )
    CV_Error( 0, "samples is empty" );

  filteredSamples.clear();

  for (size_t i = 0; i < samples.size(); i++)
  {
    Rect_<float> rect(samples[i] - Point2f(parameters.densityNeighborhoodSize) * 0.5,
                      parameters.densityNeighborhoodSize);
    int neighborsCount = 0;
    for (size_t j = 0; j < samples.size(); j++)
    {
      if (rect.contains(samples[j]))
        neighborsCount++;
    }
    if (neighborsCount >= parameters.minDensity)
      filteredSamples.push_back(samples[i]);
  }

  if( filteredSamples.empty() )
    CV_Error( 0, "filteredSamples is empty" );
}

void GraphBoxFinder::readBestDescriptors(string trainKeypointsFilename)
{
  std::ifstream fin(trainKeypointsFilename.c_str());
  assert( fin.is_open() );

  float x, y;
  int holesCount;
  fin >> holesCount;
  string imagesPath;
  fin >> imagesPath;
  //trainDescriptors.create(holesCount, descriptorExtractor->descriptorSize(), descriptorExtractor->descriptorType());
  for (int holeIdx = 0; holeIdx < holesCount; holeIdx++)
  {
    fin >> x >> y;
    string imageName;
    fin >> imageName;
    Mat image; //= imread(imagesPath + "/" + imageName, CV_LOAD_IMAGE_GRAYSCALE);
    assert( !image.empty() );

    vector<KeyPoint> keypoints;
    const int defaultScale = 1;
    keypoints.push_back(KeyPoint(Point2f(x, y), defaultScale));

    Mat descriptors;
    //descriptorExtractor->compute(image, keypoints, descriptors);
    Mat row = trainDescriptors.row(holeIdx);
    descriptors.copyTo(row);
  }

  fin.close();
}

void GraphBoxFinder::findBasis(const vector<Point2f> &samples, vector<Point2f> &basis)
{
  basis.clear();
  Mat bestLabels;
  TermCriteria termCriteria;
  Mat centers;
  int clusters = 4;
  kmeans(Mat(samples).reshape(1, 0), clusters, bestLabels, termCriteria, parameters.kmeansAttempts,
         KMEANS_RANDOM_CENTERS, &centers);
  assert( centers.type() == CV_32FC1 );

  //TODO: only remove duplicate
  for (int i = 0; i < clusters; i++)
  {
    int maxIdx = (fabs(centers.at<float> (i, 0)) < fabs(centers.at<float> (i, 1)));
    if (centers.at<float> (i, maxIdx) > 0)
    {
      Point2f vec(centers.at<float> (i, 0), centers.at<float> (i, 1));
      basis.push_back(vec);
    }
  }
  if( basis.size() != 2 )
    CV_Error( 0, "");

  if (basis[1].x > basis[0].x)
  {
    std::swap(basis[0], basis[1]);
  }

  const float minBasisDif = 2;
  if( norm( basis[0] - basis[1] ) < minBasisDif )
    CV_Error( 0, "degenerate basis" );
}

void GraphBoxFinder::computeEdgeVectors(vector<Point2f> &vectors, float maxEdgeLength, Mat *drawImage) const
{
  vectors.clear();
  for (size_t i = 0; i < keypoints.size(); i++)
  {
    for (size_t j = 0; j < keypoints.size(); j++)
    {
      if (i == j)
        continue;
      Point2f vec = keypoints[i].pt - keypoints[j].pt;
      if (norm(vec) < maxEdgeLength)
      {
        vectors.push_back(keypoints[i].pt - keypoints[j].pt);
        if( drawImage != 0 )
          line( *drawImage, keypoints[i].pt, keypoints[j].pt, Scalar( 255, 0,0 ),1 );
      }
    }
  }
}

void GraphBoxFinder::computeEdgeVectorsOfRNG (vector<Point2f> &vectors, Mat *drawImage) const
{
  vectors.clear();

  //TODO: use more fast algorithm instead of naive N^3
  for (size_t i = 0; i < keypoints.size(); i++)
  {
    for (size_t j = 0; j < keypoints.size(); j++)
    {
      if (i == j)
        continue;

      Point2f vec = keypoints[i].pt - keypoints[j].pt;
      float dist = norm(vec);

      bool isNeighbors = true;
      for( size_t k=0;k<keypoints.size();k++ )
      {
        if( k == i || k == j )
          continue;

        float dist1 = norm( keypoints[i].pt - keypoints[k].pt );
        float dist2 = norm( keypoints[j].pt - keypoints[k].pt );
        if( dist1 < dist && dist2 < dist )
        {
          isNeighbors = false;
          break;
        }
      }

      if( isNeighbors )
      {
        vectors.push_back(keypoints[i].pt - keypoints[j].pt);
        if( drawImage != 0 )
          line( *drawImage, keypoints[i].pt, keypoints[j].pt, Scalar( 255, 0,0 ),1 );
      }
    }
  }
}

void GraphBoxFinder::computeBasisGraphs(const vector<Point2f> &basis, vector<Graph> &basisGraphs)
{
  basisGraphs.resize(basis.size(), Graph(keypoints.size()));
  for (size_t i = 0; i < keypoints.size(); i++)
  {
    for (size_t j = 0; j < keypoints.size(); j++)
    {
      if (i == j)
        continue;

      Point2f vec = keypoints[i].pt - keypoints[j].pt;

      for (size_t k = 0; k < basis.size(); k++)
      {
        if (norm(vec - basis[k]) < parameters.maxBasisDistance)
        {
          add_edge(i, j, basisGraphs[k]);
        }
      }
    }
  }
}

void GraphBoxFinder::findLongestPath(vector<Graph> &basisGraphs, Path &bestPath)
{
  vector<Path> longestPaths(1);

  int bestGraphIdx = -1;
  size_t graphIdx = 0;
  //for (size_t graphIdx = 0; graphIdx < basisGraphs.size(); graphIdx++)
  {
    const Graph &g = basisGraphs[graphIdx];
    DistanceMatrix distances(num_vertices(g));
    DistanceMatrixMap dm(distances, g);
    WeightMap wm(1);
    floyd_warshall_all_pairs_shortest_paths(g, dm, weight_map(wm));

    graph_traits<Graph>::vertex_iterator i, end, i2, end2;
    //TODO: fix this by passing inf to floyd_warhsall_...;
    int inf = 1000000;
    for (boost::tie(i, end) = vertices(g); i != end; ++i)
    {
      DistanceMap distancesMap = get(dm, *i);
      for (boost::tie(i2, end2) = vertices(g); i2 != end2; ++i2)
      {
        int dist = get(distancesMap, *i2);
        //std::cout << dist << std::endl;
        if (dist >= inf)
          continue;

        if (dist > longestPaths[0].length)
        {
          longestPaths.clear();
          bestGraphIdx = graphIdx;
        }
        if (longestPaths.empty() || ( dist == longestPaths[0].length && graphIdx == bestGraphIdx ) )
        {
          longestPaths.push_back(Path(*i, *i2, dist));
        }
      }
    }
  }
  if( bestGraphIdx != 0 )
    CV_Error( 0, "" );

  int bestPathIdx = rand() % longestPaths.size();
  bestPath = longestPaths.at(bestPathIdx);
  if (keypoints[bestPath.lastVertex].pt.x < keypoints[bestPath.firstVertex].pt.x)
  {
    std::swap(bestPath.lastVertex, bestPath.firstVertex);
  }
  //std::cout << bestPath.length << std::endl;
  if( bestPath.length < parameters.minBestPathLength )
    CV_Error( 0, "best path is too short" );


}

void GraphBoxFinder::computePointsInPath(Point2f basisVec, const Path &path, vector<int> &points) const
{
  points.clear();
  points.push_back(path.firstVertex);
  Point2f curPoint = keypoints[points[0]].pt;
  /*
  Mat image = testImage;
  circle( image, curPoint, 7, Scalar( 255, 0, 255 ), -1);
  circle( image, keypoints[ path.lastVertex ].pt, 7, Scalar( 255, 0, 255 ), -1);
  Mat resized;
  resize( image, resized, Size(), 0.3,0.3);
  imshow("path", resized );
  waitKey();*/
  for (int i = 1; i <= path.length; i++)
  {
    curPoint += basisVec;
    int kpIdx = findNearestKeypoint(curPoint);
    curPoint = keypoints[kpIdx].pt;
    points.push_back(kpIdx);

    /*circle( image, curPoint, 7, Scalar( 255, 0, 255 ), -1);
    Mat resized;
    resize( image, resized, Size(), 0.3,0.3);
    imshow("path", resized );
    waitKey();*/

  }
  assert( path.length >= 0 );
  assert( points.size() == (size_t) path.length + 1 );

  const size_t maxLen = boxWidth;
  while (points.size() > maxLen)
  {
    points.pop_back();
    points.erase(points.begin());
  }
}

void GraphBoxFinder::drawBasis(const vector<Point2f> &basis, Point2f origin, Mat &drawImg) const
{
  for (size_t i = 0; i < basis.size(); i++)
  {
    Point2f pt(basis[i]);
    line(drawImg, origin, origin + pt, Scalar(0, i*255, 0), 2);
  }
}

void GraphBoxFinder::drawBasisGraphs(const vector<Graph> &basisGraphs, Mat &drawImage, bool drawEdges, bool drawVertices) const
{
  if (testImage.channels() == 1)
    cvtColor(testImage, drawImage, CV_GRAY2RGB);
  else
    testImage.copyTo(drawImage);

  const int vertexRadius = 1;
  const Scalar vertexColor = Scalar(0, 0, 255);
  const int vertexThickness = -1;

  const Scalar edgeColor = Scalar(255, 0, 0);
  const int edgeThickness = 1;

  if (drawEdges)
  {
    for (size_t i = 0; i < basisGraphs.size(); i++)
    {
      for (size_t v = 0; v < num_vertices(basisGraphs[i]); v++)
      {
        typedef graph_traits<Graph> GraphTraits;
        property_map<Graph, vertex_index_t>::type index = get(vertex_index, basisGraphs[i]);

        GraphTraits::out_edge_iterator out_i, out_end;
        GraphTraits::edge_descriptor e;
        for (tie(out_i, out_end) = out_edges(v, basisGraphs[i]); out_i != out_end; ++out_i)
        {
          e = *out_i;
          Vertex src = source(e, basisGraphs[i]), targ = target(e, basisGraphs[i]);

          line(drawImage, keypoints[index[src]].pt, keypoints[index[targ]].pt, edgeColor, edgeThickness);
        }
      }
    }
  }
  if (drawVertices)
  {
    for (size_t v = 0; v < num_vertices(basisGraphs[0]); v++)
    {
      circle(drawImage, keypoints[v].pt, vertexRadius, vertexColor, vertexThickness);
    }
  }
}

void GraphBoxFinder::drawHoles(const Mat &srcImage, Mat &drawImage) const
{
  //const int holeRadius = 4;
  const int holeRadius = 2;
  const int holeThickness = 1;
  const Scalar holeColor = Scalar(0, 0, 255);

  if (srcImage.channels() == 1)
    cvtColor(srcImage, drawImage, CV_GRAY2RGB);
  else
    srcImage.copyTo(drawImage);

  for (size_t i = 0; i < holes.size(); i++)
  {
    for (size_t j = 0; j < holes[i].size(); j++)
    {
      if( j != holes[i].size() - 1 )
        line( drawImage, keypoints[holes[i][j]].pt, keypoints[holes[i][j+1]].pt, Scalar(255,0,0), 1 );
      if( i != holes.size() - 1 )
        line( drawImage, keypoints[holes[i][j]].pt, keypoints[holes[i+1][j]].pt, Scalar(255,0,0), 1 );

      //circle(drawImage, keypoints[holes[i][j]].pt, holeRadius, holeColor, holeThickness);
      circle(drawImage, keypoints[holes[i][j]].pt, holeRadius, holeColor, holeThickness);
    }
  }
}

void GraphBoxFinder::getHoles( vector<Point2f> &outHoles )
{
  outHoles.clear();

  for( size_t i=0;i<holes.size();i++ )
  {
    for( size_t j=0;j<holes[i].size();j++ )
    {
      outHoles.push_back( keypoints[ holes[i][j] ].pt );
    }
  }
}
