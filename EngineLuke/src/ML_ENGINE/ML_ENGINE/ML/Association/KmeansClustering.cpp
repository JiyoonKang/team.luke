/******************************************************************************/
/*!
\file KmeansClustering.cpp
\project CS399_TeamLuke
\author Hanbyul Jeon, Deok-Hwa (DK) Seo

Copyright (C) 2016 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
*/
/******************************************************************************/
#include "KMeansClustering.h"
#include "../Dataframe/Dataframe.h"
#include "../Sampling/RandomSampler.h"

static double Sq(double v)
{
  return v * v;
}

KMeansClustering::KMeansClustering(Dataframe & dataframe,
  std::vector<std::string> & ignores)
  : _dataframe(dataframe),
  _clusters(std::vector<std::vector<const Instance*>>()),
  _o(nullptr)
{
  _ignores.resize(ignores.size());
  for (auto & i : ignores)
    _ignores.push_back(_dataframe.GetAttributeIndex(i));

  CalculateLimits();

  if (_ignores.size() != 0)
    qsort(_ignores.data(), _ignores.size(),
      sizeof(_ignores.front()), compareMyType);
}

void KMeansClustering::AddIgnore(std::string & ignore)
{
  int att = _dataframe.GetAttributeIndex(ignore);
  auto result = std::find(_ignores.begin(), _ignores.end(), att);
  if(result == _ignores.end())
    _ignores.push_back(att);

  if (_ignores.size() != 0)
    qsort(_ignores.data(), _ignores.size(),
      sizeof(_ignores.front()), compareMyType);
}

void KMeansClustering::RemoveIgnore(std::string & ignores)
{
  int att = _dataframe.GetAttributeIndex(ignores);
  auto result = std::find(_ignores.begin(), _ignores.end(), att);
  if (result != _ignores.end())
    _ignores.erase(result);
}

void KMeansClustering::SetDebugOutput(std::ofstream* o)
{
  _o = o;
}

const std::vector<const Instance*>& KMeansClustering::Get(int i) const
{
  return _clusters[i];
}

std::vector<DataPoint> KMeansClustering::SampleDataPoints(const int n)
{
  std::vector<DataPoint> points;
  points.resize(n);

  // sample n instances
  RandomSampler sampler;
  sampler.Sample(_dataframe, n);

  // push them into output container
  //const std::vector<const Instance*>& samples = sampler.GetSamples();
  auto samples = sampler.GetSamples();
  int pointIndex = 0;
  for (const auto& sample : samples)
  {
    int size = sample->GetAttributeCount();
    points[pointIndex].mDataPoints.reserve(size);
    auto ignore = _ignores.cbegin();
    for (int i = 0;
      i < size; ++i)
    {
      if (ignore == _ignores.end() || i != *ignore)
        points[pointIndex].mDataPoints.push_back(sample->GetAttribute(i).AsDouble());
      else points[pointIndex].mDataPoints.push_back(-1.0);
      if (ignore != _ignores.end()) ++ignore;
    }
    ++pointIndex;
  }

  return std::move(points);
}

DataPoint KMeansClustering::ToDataPoint(const Instance* instance)
{
  DataPoint point;
  int attSize = instance->GetAttributeCount(),
    dataSize = attSize;// -static_cast<int>(_ignores.size());
  point.mDataPoints.resize(dataSize);
  auto ignoreIndex = _ignores.begin();
  for (int instIndex = 0, dataIndex = 0;
    instIndex < attSize; ++instIndex)
  {
    if (ignoreIndex == _ignores.end() ||
      instIndex != *ignoreIndex)
    {
      point.mDataPoints[dataIndex++] =
        instance->GetAttribute(instIndex).AsDouble();
    }
    if (ignoreIndex != _ignores.end())
    {
      if (instIndex == *ignoreIndex)
        point.mDataPoints[dataIndex++] = 0.0;
      ++ignoreIndex;
    }
  }
  return std::move(point);
}

const std::vector<std::pair<double, double>>&
KMeansClustering::GetLimits() const
{
  return _limits;
}

const std::vector<double>& KMeansClustering::GetDiff() const
{
  return _diffs;
}

DataPoint KMeansClustering::CalculateCentroid(const std::vector<const Instance*>& instances)
{
  DataPoint centroid;
  if (instances.size() == 0) return centroid;
  centroid.mDataPoints.resize(instances.front()->GetAttributeCount());

  // sums all the given points
  for (auto & instance : instances)
  {
    DataPoint point = ToDataPoint(instance);
    auto ignore = _ignores.cbegin();
    for (int i = 0; i < centroid.mDataPoints.size() &&
      i < point.mDataPoints.size(); ++i)
    {
      if (ignore == _ignores.end() ||
        i != *ignore)
        centroid.mDataPoints[i] += point.mDataPoints[i];
      if (ignore != _ignores.end()) ++ignore;
    }
  }

  // divide the sum by the number of instances
  double divisor = static_cast<double>(instances.size() - _ignores.size());
  if (divisor != 0.0) // smooth error handling
  {
    for (auto & i : centroid.mDataPoints)
      i /= divisor;
  }
  // return it
  return std::move(centroid);
}

void KMeansClustering::PrintClusters(std::ofstream& o)
{
  for (int i = 0; i < int(_clusters.size()); ++i)
  {
    DataPoint centroid = CalculateCentroid(_clusters[i]);
    PrintClusters(o, i + 1, centroid);
  }
}

void KMeansClustering::PrintClusters(std::ofstream& o, const int group, const DataPoint& centroid)
{
  // print group info
  o << "Group " << group << " :" << "n=" << _clusters[group - 1].size() << ", " << "centroid=(";

  auto & points = centroid.mDataPoints;
  auto ignore = _ignores.cbegin();
  for (int i = 0, size = static_cast<int>(points.size());
    i < size; )
  {
    if (ignore == _ignores.end() || *ignore != i)
      o << points[i];
    else o << "NULL";
    if (ignore != _ignores.end()) ++ignore;
    if (++i != size) *_o << ", ";
  }
  o << ")" << std::endl;

  o << "---------------------" << std::endl;
  // print info of instances
  for (auto& instance : _clusters[group - 1])
  {
    //o << instance->GetAttribute(0).AsString() << " : ";
    o << "Tuple[" << group << "] : ";
    auto ignore = _ignores.begin();
    for (int attIndex = 0; attIndex < _dataframe.GetAttributeCount(); ++attIndex)
    {
      o << _dataframe.GetAttributeName(attIndex) << "=";

      if (ignore == _ignores.end() || attIndex != *ignore)
        o << instance->GetAttribute(attIndex).AsString();
      else o << "NULL";

      if (ignore != _ignores.end()) ++ignore;

      if (attIndex + 1 != _dataframe.GetAttributeCount())
        o << ",";
    }
    o << std::endl;
  }
  o << std::endl;
}

ClusterData KMeansClustering::Cluster(const int k)
{
  // randomly select k points from instances
  ClusterData centroids = std::move(SampleDataPoints(k));

  // prints initial centroids
  if (_o)
  {
    for (int i = 0; i < k; ++i)
    {
      (*_o) << "Initial centroid for group " << i + 1 << " : ";
      auto & points = centroids[i].mDataPoints;

      auto ignore = _ignores.cbegin();
      for (int i = 0, size = static_cast<int>(points.size());
        i < size; )
      {
        if (ignore == _ignores.end() || *ignore != i)
          *_o << points[i];
        else {
          *_o << "NULL";
        }
        if (ignore != _ignores.end()) ++ignore;
        if (++i != size) *_o << ", ";
      }

      *_o << std::endl;
    }

    (*_o) << std::endl;
  }

  // prepare k clusters
  _clusters.clear();
  _clusters.resize(k);

  int iteration = 0;
  bool centroidsChanged = true;
  double sumDist;
  while (centroidsChanged) // if centroid is not changed, exit loop
  {
    // clear up clusters and sumDist to regroup instances
    for (auto& cluster : _clusters)
      cluster.clear();
    sumDist = 0.0;

    // assign each instance to a cluster whose centroid is closest to it
    auto& instances = _dataframe.GetInstances();
    for (auto instance = instances.cbegin();
      instance != instances.end(); ++instance)
    {
      DataPoint point = ToDataPoint(*instance);

      // find the closest cluster-group
      double minDistSq = std::numeric_limits<double>::max();
      int minDistCluster = 0;
      for (int i = 0; i < k; ++i)
      {
        double distSq = DistSq(centroids[i], point);
        if (distSq < minDistSq)
        {
          minDistSq = distSq;
          minDistCluster = i;
        }
      }

      // add instance to the group
      _clusters[minDistCluster].emplace_back(*instance);
      if (_o)
        sumDist += sqrtl(minDistSq);
    }

    // debug output : iteration info
    if (_o)
    {
      (*_o) << "-----------------------" << std::endl;
      (*_o) << "iteration " << iteration << std::endl << std::endl;
    }

    // update centroids of clusters with their elements
    centroidsChanged = false;
    for (int i = 0; i < k; ++i)
    {
      DataPoint centroid = CalculateCentroid(_clusters[i]);
      auto ignore = _ignores.cbegin();
      for (int index = 0, size = static_cast<int>(centroid.mDataPoints.size());
        index < size; ++index)
      {
        if (ignore == _ignores.end() || *ignore != index)
        {
          if (centroids[i].mDataPoints.size() > 0 &&
            centroid.mDataPoints[index] != centroids[i].mDataPoints[index])
            centroidsChanged = true;
        }
        if (ignore != _ignores.end()) ++ignore;
      }

      centroids[i] = centroid;

      if (_o)
        PrintClusters(*_o, i + 1, centroid);
    }
    if (_o)
      (*_o) << "Sum Distance : " << sumDist << std::endl;

    ++iteration;
  }

  return std::move(centroids);
}

double KMeansClustering::DistSq(const DataPoint & p1, DataPoint & p2)
{
  double dist = 0.0;
  auto ignore = _ignores.cbegin();
  for (int i = 0; i < static_cast<int>(p1.mDataPoints.size()) &&
    i < static_cast<int>(p2.mDataPoints.size()); ++i)
  {
    if (ignore == _ignores.end() || i != *ignore)
    {
      double diff = p1.mDataPoints[i] - p2.mDataPoints[i];
      dist += sqrt(diff * diff);
    }
    if (ignore != _ignores.end()) ++ignore;
  }

  return dist;
}

void KMeansClustering::CalculateLimits()
{
  _limits.resize(_dataframe.GetAttributeCount(),
    std::make_pair(std::numeric_limits<double>().max(),
      std::numeric_limits<double>().min()));

  auto& instances = _dataframe.GetInstances();
  for (auto instance = instances.cbegin();
    instance != instances.end(); ++instance)
  {
    DataPoint point = ToDataPoint(*instance);

    for (int i = 0; i < point.mDataPoints.size(); ++i)
    {
      if (_limits[i].first > point.mDataPoints[i])
        _limits[i].first = point.mDataPoints[i];
      if (_limits[i].second < point.mDataPoints[i])
        _limits[i].second = point.mDataPoints[i];
    }
  }

  _diffs.resize(_limits.size());

  for (int i = 0, size = static_cast<int>(_limits.size());
    i < size; ++i)
    _diffs[i] = _limits[i].second - _limits[i].first;

}

double KMeansClustering::Dot(const DataPoint & p1,
  const DataPoint & p2,
  const std::vector<int> & ignores,
  const std::vector<double> & diffs)
{
  if (p1.mDataPoints.size() != p2.mDataPoints.size())
    return 0.0;

  bool isDiffs = true;
  if (diffs.size() != p1.mDataPoints.size())
    isDiffs = false;

  double dot = 0.0;
  auto ignore = ignores.cbegin();
  for (int i = 0, size = static_cast<int>(p1.mDataPoints.size());
    i < size; ++i)
  {
    if (ignore == ignores.end() || *ignore != i)
    {
      double temp = p1.mDataPoints[i] * p2.mDataPoints[i];
      if (isDiffs) temp /= (diffs[i] * diffs[i]);
      dot += temp;
    }

    if (ignore != ignores.end()) ++ignore;
  }

  return dot;
}

double KMeansClustering::Length(const DataPoint & p,
  const std::vector<int> & ignores,
  const std::vector<double> & diffs,
  const bool Sqrt)
{
  double length = 0.0;

  bool isDiffs = true;
  if (diffs.size() != p.mDataPoints.size())
    isDiffs = false;

  auto ignore = ignores.cbegin();
  for (int i = 0; i < p.mDataPoints.size(); ++i)
  {
    if (ignore == ignores.end() ||
      *ignore != i)
    {
      double temp = (p.mDataPoints[i] * p.mDataPoints[i]);
      if (isDiffs) temp /= (diffs[i] * diffs[i]);
      length += temp;
    }
    if (ignore != ignores.end()) ++ignore;
  }

  return (Sqrt) ? sqrtl(length) : length;
}
