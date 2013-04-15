#pragma once

#include <vector>
#include <boost/shared_ptr.hpp>
#include <btBulletDynamicsCommon.h>

#include "node.hpp"

namespace bullet
{
	class manager
	{
	public:
		manager(const node::node& world, bool enable=true);
		virtual ~manager();
		int step();
	private:
		bool in_use_;

		boost::shared_ptr<btBroadphaseInterface> broadphase_;
		boost::shared_ptr<btDefaultCollisionConfiguration> collision_cfg_;
		boost::shared_ptr<btCollisionDispatcher> dispatcher_;
		boost::shared_ptr<btSequentialImpulseConstraintSolver> solver_;

		boost::shared_ptr<btDiscreteDynamicsWorld> dynamics_world_;

		std::vector<boost::shared_ptr<btCollisionShape> > collision_shapes_;
		std::vector<boost::shared_ptr<btRigidBody> > rigid_bodies_;

		std::vector<boost::shared_ptr<btDefaultMotionState> > motion_states_;

		boost::shared_ptr<btRigidBody> fallRigidBody

	};
}
