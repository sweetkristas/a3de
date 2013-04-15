#include "btinterface.hpp"

namespace bullet
{
	manager::manager(const node::node& world, bool enable)
		: in_use_(enable)
	{
		if(!enable) {
			return;
		}
		broadphase_ = boost::shared_ptr<btBroadphaseInterface>(new btDbvtBroadphase());
 
		collision_cfg_ = boost::shared_ptr<btDefaultCollisionConfiguration>(new btDefaultCollisionConfiguration());
		dispatcher_ = boost::shared_ptr<btCollisionDispatcher>(new btCollisionDispatcher(collision_cfg_.get()));
 
		solver_ = boost::shared_ptr<btSequentialImpulseConstraintSolver>(new btSequentialImpulseConstraintSolver());
 
		dynamics_world_ = boost::shared_ptr<btDiscreteDynamicsWorld>(new btDiscreteDynamicsWorld(dispatcher_.get(),broadphase_.get(),solver_.get(),collision_cfg_.get()));
		dynamics_world_->setGravity(btVector3(0,-10,0));
 
		collision_shapes_.push_back(boost::shared_ptr<btCollisionShape>(new btStaticPlaneShape(btVector3(0,1,0),1)));
		collision_shapes_.push_back(boost::shared_ptr<btCollisionShape>(new btSphereShape(1)));
 
		motion_states_.push_back(boost::shared_ptr<btDefaultMotionState>(new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,-1,0)))));
		btRigidBody::btRigidBodyConstructionInfo
				groundRigidBodyCI(0,motion_states_.back().get(),collision_shapes_[0].get(),btVector3(0,0,0));
		rigid_bodies_.push_back(boost::shared_ptr<btRigidBody>(new btRigidBody(groundRigidBodyCI)));
		dynamics_world_->addRigidBody(rigid_bodies_.back().get());
 
 
		motion_states_.push_back(boost::shared_ptr<btDefaultMotionState>(new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,50,0)))));
		btScalar mass = 1;
		btVector3 fallInertia(0,0,0);
		collision_shapes_[1]->calculateLocalInertia(mass,fallInertia);
		btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass,motion_states_.back().get(),collision_shapes_[1].get(),fallInertia);
		rigid_bodies_.push_back(boost::shared_ptr<btRigidBody>(new btRigidBody(fallRigidBodyCI)));
		dynamics_world_->addRigidBody(rigid_bodies_.back().get());
	}

	manager::~manager()
	{
		for(int i = 0; i < rigid_bodies_.size(); ++i) {
			dynamics_world_->removeRigidBody(rigid_bodies_[i].get());
		}
	}

	int manager::step()
	{
		return dynamics_world_->stepSimulation(1/60.0f,10);
	}
}
