#include "asserts.hpp"
#include "btinterface.hpp"

namespace bullet
{
	manager::manager(const node::node& world)
		: in_use_(true)
	{
		if(world.is_null() || !world.has_key("has_physics") || !world["has_physics"].as_bool()) {
			std::cerr << "World is empty: Physics disabled" << std::endl;
			in_use_ = false;
			return;
		}

		if(world.has_key("broadphase") && world["broadphase"] == "sap") {
			ASSERT_LOG(world.has_key("world") && world["world"].has_key("min") && world["world"].has_key("max"), 
				"broadphase was specified as sap, but no minimum (world.min) or maximum (world.max) boundaries where defined.");
			btVector3 wmin(world["world"]["min"][0].as_float(),
				world["world"]["min"][1].as_float(),
				world["world"]["min"][2].as_float());
			btVector3 wmax(world["world"]["max"][0].as_float(),
				world["world"]["max"][1].as_float(),
				world["world"]["max"][2].as_float());
			broadphase_ = boost::shared_ptr<btBroadphaseInterface>(new btAxisSweep3(wmin, wmax));
		} else {
			broadphase_ = boost::shared_ptr<btBroadphaseInterface>(new btDbvtBroadphase());
		}
 
		collision_cfg_ = boost::shared_ptr<btDefaultCollisionConfiguration>(new btDefaultCollisionConfiguration());
		dispatcher_ = boost::shared_ptr<btCollisionDispatcher>(new btCollisionDispatcher(collision_cfg_.get()));
 
		solver_ = boost::shared_ptr<btSequentialImpulseConstraintSolver>(new btSequentialImpulseConstraintSolver());
 
		dynamics_world_ = boost::shared_ptr<btDiscreteDynamicsWorld>(new btDiscreteDynamicsWorld(dispatcher_.get(),broadphase_.get(),solver_.get(),collision_cfg_.get()));
		ASSERT_LOG(world.has_key("world") && world["world"].has_key("gravity"), "No node \"world.gravity\" found.");
		const node::node& gravity = world["world"]["gravity"];
		dynamics_world_->setGravity(btVector3(
			gravity[0].as_float(),
			gravity[1].as_float(),
			gravity[2].as_float()));
 
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
		if(!in_use_) {
			return;
		}
		for(int i = 0; i != rigid_bodies_.size(); ++i) {
			dynamics_world_->removeRigidBody(rigid_bodies_[i].get());
		}
	}

	int manager::step()
	{
		ASSERT_LOG(in_use_, "bullet::manager::step() called when physics not in use.");
		return dynamics_world_->stepSimulation(1/60.0f,10);
	}

	boost::shared_ptr<btRigidBody> manager::get_rigid_body(size_t n)
	{
		ASSERT_LOG(in_use_, "bullet::manager::get_rigid_body() called when physics not in use.");
		ASSERT_LOG(n < rigid_bodies_.size(), "Invalid rigid body index");
		return rigid_bodies_[n];
	}
}
