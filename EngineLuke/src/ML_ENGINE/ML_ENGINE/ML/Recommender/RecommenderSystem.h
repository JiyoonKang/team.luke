/******************************************************************************/
/*!
\file RecommenderSystem.h
\project CS399_TeamLuke
\author Hanbyul Jeon, Deok-Hwa (DK) Seo

Copyright (C) 2016 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
*/
/******************************************************************************/
#pragma once
#include <vector>
#include <ostream>
#include <string>

class Dataframe;
class Instance;

// rename type of vector layout of unsigned integer
typedef std::vector<int> IndexList;

class RecommenderSystem
{
public:
  // initialize with critics dataframe to be used for recommendation
  RecommenderSystem(Dataframe& critics);
  ~RecommenderSystem();

  // show process detail if stream is given
  void SetDebugOutput(std::ostream* o);

  // recommend for user and return scores for each product based on critics
  std::vector<double> RecommendFor(Instance* user);

  // recommend for users and return user-score table based on critics
  std::vector<std::vector<double>>
    RecommenderSystem::RecommendFor(const Dataframe& users);

private:
  // return double vector that consists only with attributes of given indices
  std::vector<double> GetFeatureVector(
    Instance* instance, const std::vector<int>& indices);

  // print attributes only with attributes of given indoces
  void PrintAttributes(const std::vector<int>& attIndices);

private:
  Dataframe& _critics; // reference of critics dataframe
  std::ostream* _o; // debug output stream
};

// dummy object
namespace Dummy { static IndexList indexDummy = IndexList(); }

#include "../Association/KmeansClustering.h"

typedef std::pair<unsigned, double>  AnswerForm;
typedef std::vector<AnswerForm>      Answer;
typedef std::vector<Answer>          Answers;

// recommender implement class
template <typename Cluster = KMeansClustering, typename Data = ClusterData>
class Recommender
{
public:
  Recommender(Dataframe & data);
  ~Recommender();

  // change data frame
  void ChangeDataFrame(Dataframe & data);

  // show process detail if stream is given
  void SetDebugOutput(std::ostream* o);

  // set the number of classified groups ( 1 by default )
  void SetGroupNumber(const int k);

  // get current number of classified groups
  int GetGroupNumber() const;

  // set precision of similiarity ( 50 % by default )
  void SetPrecision(const float percent);

  // get current precision of similiarity
  float GetPrecision() const;

  // add favorite items
  void SetFavoriteList(std::vector<int> & favoritelist);

  // recommend possible choices
  Answers & Recommend(IndexList & favorList = Dummy::indexDummy);

  // add ignore attribute name
  void AddIgnoreAttribute(std::string & attribute);

  // revmoe ignore attribute name
  void RemoveIgnoreAttribute(std::string & attribute);

  // display attributes on ignore list
  void DisplayIgnoreAttribute(std::ostream * o);

  // check if a specific attribute is on ignore list
  bool IsInIgnoreList(const std::string & attribute);

private:
  // return double vector that consists only with attributes of given indices
  std::vector<double> GetFeatureVector(
    Instance* instance, const std::vector<int>& indices);

  // print attributes only with attributes of given indoces
  void PrintAttributes(const std::vector<int>& attIndices);

private:
#  define DO_CENTROID   0001
#  define DO_CLUSTER    0002
#  define DO_RECOMMEND  0004
private:
  Dataframe & mTable;
  Cluster* pCluster;
  std::vector<std::string> mIgnores;
  std::ostream * pOStream;
  int   mClusterGroup;
  float mPrecision;
  unsigned int mFlag;
  Answers mRecommendList;
  Data clustered;
  Dataframe favorites;
  std::vector<int> mfavoriteList;
};

#include "RecommenderSystem.inl"

typedef Recommender<> WineRecommender;