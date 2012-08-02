/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2011, Willow Garage, Inc.
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
 *   * Neither the name of Willow Garage, Inc. nor the names of its
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


#ifndef FCL_COLLISION_OBJECT_BASE_H
#define FCL_COLLISION_OBJECT_BASE_H

#include "fcl/BV/AABB.h"
#include "fcl/transform.h"
#include <boost/shared_ptr.hpp>

namespace fcl
{

/// @brief object type: BVH (mesh, points), basic geometry, octree
enum OBJECT_TYPE {OT_UNKNOWN, OT_BVH, OT_GEOM, OT_OCTREE, OT_COUNT};

/// @brief traversal node type: bounding volume (AABB, OBB, RSS, kIOS, OBBRSS, KDOP16, KDOP18, kDOP24), basic shape (box, sphere, capsule, cone, cylinder, convex, plane, triangle), and octree
enum NODE_TYPE {BV_UNKNOWN, BV_AABB, BV_OBB, BV_RSS, BV_kIOS, BV_OBBRSS, BV_KDOP16, BV_KDOP18, BV_KDOP24,
                GEOM_BOX, GEOM_SPHERE, GEOM_CAPSULE, GEOM_CONE, GEOM_CYLINDER, GEOM_CONVEX, GEOM_PLANE, GEOM_TRIANGLE, GEOM_OCTREE, NODE_COUNT};

/// @brief The geometry for the object for collision or distance computation
class CollisionGeometry
{
public:
  CollisionGeometry() : cost_density(1),
                        threshold_occupied(1),
                        threshold_free(0)
  {
  }

  virtual ~CollisionGeometry() {}

  /// @brief get the type of the object
  virtual OBJECT_TYPE getObjectType() const { return OT_UNKNOWN; }

  /// @brief get the node type 
  virtual NODE_TYPE getNodeType() const { return BV_UNKNOWN; }

  /// @brief compute the AABB for object in local coordinate
  virtual void computeLocalAABB() = 0;

  /// @brief get user data in geometry
  void* getUserData() const
  {
    return user_data;
  }

  /// @brief set user data in geometry
  void setUserData(void *data)
  {
    user_data = data;
  }

  /// @brief whether the object is completely occupied
  inline bool isOccupied() const { return cost_density >= threshold_occupied; }

  /// @brief whether the object is completely free
  inline bool isFree() const { return cost_density <= threshold_free; }

  /// @brief whether the object has some uncertainty
  inline bool isUncertain() const { return !isOccupied() && !isFree(); }

  /// @brief AABB center in local coordinate
  Vec3f aabb_center;

  /// @brief AABB radius
  FCL_REAL aabb_radius;

  /// @brief AABB in local coordinate, used for tight AABB when only translation transform
  AABB aabb_local;

  /// @brief pointer to user defined data specific to this object
  void *user_data;

  /// @brief collision cost for unit volume
  FCL_REAL cost_density;

  /// @brief threshold for occupied ( >= is occupied)
  FCL_REAL threshold_occupied;

  /// @brief threshold for free (<= is free)
  FCL_REAL threshold_free;
};

/// @brief the object for collision or distance computation, contains the geometry and the transform information
class CollisionObject
{
public:
  CollisionObject(const boost::shared_ptr<CollisionGeometry> &cgeom_) : cgeom(cgeom_)
  {
    cgeom->computeLocalAABB();
    computeAABB();
  }

  CollisionObject(const boost::shared_ptr<CollisionGeometry> &cgeom_, const Transform3f& tf) : cgeom(cgeom_), t(tf)
  {
    cgeom->computeLocalAABB();
    computeAABB();
  }

  CollisionObject(const boost::shared_ptr<CollisionGeometry> &cgeom_, const Matrix3f& R, const Vec3f& T):
      cgeom(cgeom_), t(Transform3f(R, T))
  {
    cgeom->computeLocalAABB();
    computeAABB();
  }

  CollisionObject()
  {
  }

  ~CollisionObject()
  {
  }

  /// @brief get the type of the object
  OBJECT_TYPE getObjectType() const
  {
    return cgeom->getObjectType();
  }

  /// @brief get the node type
  NODE_TYPE getNodeType() const
  {
    return cgeom->getNodeType();
  }

  /// @brief get the AABB in world space
  inline const AABB& getAABB() const
  {
    return aabb;
  }

  /// @brief compute the AABB in world space
  inline void computeAABB()
  {
    if(t.getQuatRotation().isIdentity())
    {
      aabb = cgeom->aabb_local;
    }
    else
    {
      Vec3f center = t.transform(cgeom->aabb_center);
      Vec3f delta(cgeom->aabb_radius, cgeom->aabb_radius, cgeom->aabb_radius);
      aabb.min_ = center - delta;
      aabb.max_ = center + delta;
    }
  }

  /// @brief get user data in object
  void* getUserData() const
  {
    return user_data;
  }

  /// @brief set user data in object
  void setUserData(void *data)
  {
    user_data = data;
  }

  /// @brief get translation of the object
  inline const Vec3f& getTranslation() const
  {
    return t.getTranslation();
  }

  /// @brief get matrix rotation of the object
  inline const Matrix3f& getRotation() const
  {
    return t.getRotation();
  }

  /// @brief get quaternion rotation of the object
  inline const Quaternion3f& getQuatRotation() const
  {
    return t.getQuatRotation();
  }

  /// @brief get object's transform
  inline const Transform3f& getTransform() const
  {
    return t;
  }

  /// @brief set object's rotation matrix
  void setRotation(const Matrix3f& R)
  {
    t.setRotation(R);
  }

  /// @brief set object's translation
  void setTranslation(const Vec3f& T)
  {
    t.setTranslation(T);
  }

  /// @brief set object's quatenrion rotation
  void setQuatRotation(const Quaternion3f& q)
  {
    t.setQuatRotation(q);
  }

  /// @brief set object's transform
  void setTransform(const Matrix3f& R, const Vec3f& T)
  {
    t.setTransform(R, T);
  }

  /// @brief set object's transform
  void setTransform(const Quaternion3f& q, const Vec3f& T)
  {
    t.setTransform(q, T);
  }

  /// @brief set object's transform
  void setTransform(const Transform3f& tf)
  {
    t = tf;
  }

  /// @brief whether the object is in local coordinate
  bool isIdentityTransform() const
  {
    return t.isIdentity();
  }

  /// @brief set the object in local coordinate
  void setIdentityTransform()
  {
    t.setIdentity();
  }

  /// @brief get geometry from the object instance
  const CollisionGeometry* getCollisionGeometry() const
  {
    return cgeom.get();
  }

  /// @brief get object's cost density
  FCL_REAL getCostDensity() const
  {
    return cgeom->cost_density;
  }

  /// @brief set object's cost density
  void setCostDensity(FCL_REAL c)
  {
    cgeom->cost_density = c;
  }

  /// @brief whether the object is completely occupied
  inline bool isOccupied() const
  {
    return cgeom->isOccupied();
  }

  /// @brief whether the object is completely free
  inline bool isFree() const
  {
    return cgeom->isFree();
  }

  /// @brief whether the object is uncertain
  inline bool isUncertain() const
  {
    return cgeom->isUncertain();
  }

protected:

  boost::shared_ptr<CollisionGeometry> cgeom;

  Transform3f t;

  /// @brief AABB in global coordinate
  mutable AABB aabb;

  /// @brief pointer to user defined data specific to this object
  void *user_data;
};

}

#endif
