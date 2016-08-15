/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2011-2014, Willow Garage, Inc.
 *  Copyright (c) 2014-2016, Open Source Robotics Foundation
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Open Source Robotics Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */ 

/** \author Jia Pan */

#ifndef FCL_BROAD_PHASE_BRUTE_FORCE_H
#define FCL_BROAD_PHASE_BRUTE_FORCE_H

#include "fcl/broadphase/broadphase.h"
#include <list>


namespace fcl
{

/// @brief Brute force N-body collision manager
template <typename S>
class NaiveCollisionManager : public BroadPhaseCollisionManager<S>
{
public:
  NaiveCollisionManager() {}

  /// @brief add objects to the manager
  void registerObjects(const std::vector<CollisionObject<S>*>& other_objs);

  /// @brief add one object to the manager
  void registerObject(CollisionObject<S>* obj);

  /// @brief remove one object from the manager
  void unregisterObject(CollisionObject<S>* obj);

  /// @brief initialize the manager, related with the specific type of manager
  void setup();

  /// @brief update the condition of manager
  void update();

  /// @brief clear the manager
  void clear();

  /// @brief return the objects managed by the manager
  void getObjects(std::vector<CollisionObject<S>*>& objs) const;

  /// @brief perform collision test between one object and all the objects belonging to the manager
  void collide(CollisionObject<S>* obj, void* cdata, CollisionCallBack<S> callback) const;

  /// @brief perform distance computation between one object and all the objects belonging to the manager
  void distance(CollisionObject<S>* obj, void* cdata, DistanceCallBack<S> callback) const;

  /// @brief perform collision test for the objects belonging to the manager (i.e., N^2 self collision)
  void collide(void* cdata, CollisionCallBack<S> callback) const;

  /// @brief perform distance test for the objects belonging to the manager (i.e., N^2 self distance)
  void distance(void* cdata, DistanceCallBack<S> callback) const;

  /// @brief perform collision test with objects belonging to another manager
  void collide(BroadPhaseCollisionManager<S>* other_manager, void* cdata, CollisionCallBack<S> callback) const;

  /// @brief perform distance test with objects belonging to another manager
  void distance(BroadPhaseCollisionManager<S>* other_manager, void* cdata, DistanceCallBack<S> callback) const;

  /// @brief whether the manager is empty
  bool empty() const;
  
  /// @brief the number of objects managed by the manager
  inline size_t size() const { return objs.size(); }

protected:

  /// @brief objects belonging to the manager are stored in a list structure
  std::list<CollisionObject<S>*> objs;
};

using NaiveCollisionManagerf = NaiveCollisionManager<float>;
using NaiveCollisionManagerd = NaiveCollisionManager<double>;

//============================================================================//
//                                                                            //
//                              Implementations                               //
//                                                                            //
//============================================================================//

//==============================================================================
template <typename S>
void NaiveCollisionManager<S>::registerObjects(const std::vector<CollisionObject<S>*>& other_objs)
{
  std::copy(other_objs.begin(), other_objs.end(), std::back_inserter(objs));
}

//==============================================================================
template <typename S>
void NaiveCollisionManager<S>::unregisterObject(CollisionObject<S>* obj)
{
  objs.remove(obj);
}

//==============================================================================
template <typename S>
void NaiveCollisionManager<S>::registerObject(CollisionObject<S>* obj)
{
  objs.push_back(obj);
}

//==============================================================================
template <typename S>
void NaiveCollisionManager<S>::setup()
{

}

//==============================================================================
template <typename S>
void NaiveCollisionManager<S>::update()
{

}

//==============================================================================
template <typename S>
void NaiveCollisionManager<S>::clear()
{
  objs.clear();
}

//==============================================================================
template <typename S>
void NaiveCollisionManager<S>::getObjects(std::vector<CollisionObject<S>*>& objs_) const
{
  objs_.resize(objs.size());
  std::copy(objs.begin(), objs.end(), objs_.begin());
}

//==============================================================================
template <typename S>
void NaiveCollisionManager<S>::collide(CollisionObject<S>* obj, void* cdata, CollisionCallBack<S> callback) const
{
  if(size() == 0) return;

  for(auto* obj2 : objs)
  {
    if(callback(obj, obj2, cdata))
      return;
  }
}

//==============================================================================
template <typename S>
void NaiveCollisionManager<S>::distance(CollisionObject<S>* obj, void* cdata, DistanceCallBack<S> callback) const
{
  if(size() == 0) return;

  S min_dist = std::numeric_limits<S>::max();
  for(auto* obj2 : objs)
  {
    if(obj->getAABB().distance(obj2->getAABB()) < min_dist)
    {
      if(callback(obj, obj2, cdata, min_dist))
        return;
    }
  }
}

//==============================================================================
template <typename S>
void NaiveCollisionManager<S>::collide(void* cdata, CollisionCallBack<S> callback) const
{
  if(size() == 0) return;

  for(typename std::list<CollisionObject<S>*>::const_iterator it1 = objs.begin(), end = objs.end();
      it1 != end; ++it1)
  {
    typename std::list<CollisionObject<S>*>::const_iterator it2 = it1; it2++;
    for(; it2 != end; ++it2)
    {
      if((*it1)->getAABB().overlap((*it2)->getAABB()))
      {
        if(callback(*it1, *it2, cdata))
          return;
      }
    }
  }
}

//==============================================================================
template <typename S>
void NaiveCollisionManager<S>::distance(void* cdata, DistanceCallBack<S> callback) const
{
  if(size() == 0) return;

  S min_dist = std::numeric_limits<S>::max();
  for(typename std::list<CollisionObject<S>*>::const_iterator it1 = objs.begin(), end = objs.end(); it1 != end; ++it1)
  {
    typename std::list<CollisionObject<S>*>::const_iterator it2 = it1; it2++;
    for(; it2 != end; ++it2)
    {
      if((*it1)->getAABB().distance((*it2)->getAABB()) < min_dist)
      {
        if(callback(*it1, *it2, cdata, min_dist))
          return;
      }
    }
  }
}

//==============================================================================
template <typename S>
void NaiveCollisionManager<S>::collide(BroadPhaseCollisionManager<S>* other_manager_, void* cdata, CollisionCallBack<S> callback) const
{
  NaiveCollisionManager* other_manager = static_cast<NaiveCollisionManager*>(other_manager_);

  if((size() == 0) || (other_manager->size() == 0)) return;

  if(this == other_manager)
  {
    collide(cdata, callback);
    return;
  }

  for(auto* obj1 : objs)
  {
    for(auto* obj2 : other_manager->objs)
    {
      if(obj1->getAABB().overlap(obj2->getAABB()))
      {
        if(callback(obj1, obj2, cdata))
          return;
      }
    }
  }
}

//==============================================================================
template <typename S>
void NaiveCollisionManager<S>::distance(BroadPhaseCollisionManager<S>* other_manager_, void* cdata, DistanceCallBack<S> callback) const
{
  NaiveCollisionManager* other_manager = static_cast<NaiveCollisionManager*>(other_manager_);

  if((size() == 0) || (other_manager->size() == 0)) return;

  if(this == other_manager)
  {
    distance(cdata, callback);
    return;
  }

  S min_dist = std::numeric_limits<S>::max();
  for(auto* obj1 : objs)
  {
    for(auto* obj2 : other_manager->objs)
    {
      if(obj1->getAABB().distance(obj2->getAABB()) < min_dist)
      {
        if(callback(obj1, obj2, cdata, min_dist))
          return;
      }
    }
  }
}

//==============================================================================
template <typename S>
bool NaiveCollisionManager<S>::empty() const
{
  return objs.empty();
}

} // namespace

#endif
