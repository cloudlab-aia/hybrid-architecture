#include <game/sys/collision.hpp>
#include <game/sys/render.hpp>
#include <game\util\log.hpp>
#include <game\util\opencl-utils.hpp>

namespace GM {
CollisionSystem_t::CollisionSystem_t()
{
#ifdef COLLISION
	if (ocl == NULL) {
		Log::log("ERROR OpenCLArgs (ocl) in System_t has to be set before initializing the systems");
		Log::flush();
		exit(-1);
	}
	createAndBuildProgram(ocl, program, "game/ocl/collision_ocl.cl");
	createKernelFromProgram(*ocl, program, kernel, "update");
	//deltaTimeBuffer = createFloatParam(op->ocl, RenderSystem_t::deltaTime);
#endif
}

/*
	Gets the phy index in the vecPhy array.
*/
unsigned _int64 CollisionSystem_t::findPhyIndex(const std::vector<PhysicsComponent_t>& vecPhy, const PhysicsComponent_t& phy) const {
	auto* ptrPhy = &phy;
	auto* ptrVecPhy = vecPhy.data();
	//Defined by the c++ standart, substraction between two memory adresses results in the diference of elements between them in the arrray they are 
	return (ptrPhy - ptrVecPhy);
}

/*
	Creates a vector of the index of the corresponding PhysicsComponent_t for every BoxCollider_t
*/
std::vector<unsigned _int64> CollisionSystem_t::getPhyIndexFromBx(const ECS::EntityManager_t& em) const {
	std::vector<unsigned _int64> vecIndex;
	const auto& vecBx = em.getComponents<BoxCollider_t>();
	const auto& vecPhy = em.getComponents<PhysicsComponent_t>();

	for (const auto& bx : vecBx) {
		auto& entity = em.getEntity(bx.entityID);
		auto* phy = entity.getComponent<PhysicsComponent_t>();
		if (phy == nullptr) {
			Log::log("ERROR: getPhyIndexFromBX(...) Box collider without Physics Component");
			exit(-1);
		}
		vecIndex.push_back(findPhyIndex(vecPhy, *phy));
	}
	return vecIndex;
}

void CollisionSystem_t::update(ECS::EntityManager_t& g) {
#ifdef COLLISION
#ifdef TIMEMEASURE
	tm.StartCounter();
#endif
	//GPU Implementation
	std::vector<PhysicsComponent_t>& vecPhy = g.getComponents<PhysicsComponent_t>();
	std::vector<BoxCollider_t>& vecBx = g.getComponents<BoxCollider_t>();

	if (vecPhy.size() != lastPhysicsVectorSize) { //TODO liberar buffer anterior antes de reservar
		createBuffer(*ocl, phyBuffer, true, vecPhy);
		lastPhysicsVectorSize = vecPhy.size();
	}
	if (vecBx.size() != lastCollisionsVectorSize) { //TODO liberar buffer anterior antes de reservar
		createBuffer(*ocl, bxBuffer, true, vecBx);
		vecIndex = getPhyIndexFromBx(g);
		createBuffer(*ocl, indexBuffer, false, vecIndex);
		lastCollisionsVectorSize = vecBx.size();
	}

	copyParameter(*ocl, kernel, 0, phyBuffer, vecPhy);
	copyParameter(*ocl, kernel, 1, bxBuffer, vecBx);
	copyParameter(*ocl, kernel, 2, indexBuffer, vecIndex);

	//copyFloatParam(op->ocl, kernel, 3, deltaTimeBuffer, RenderSystem_t::deltaTime);
	cl_float dt = RenderSystem_t::deltaTime;
	copySimpleParameter<cl_float>(kernel, 3, dt);
	cl_int vecSize = (int)vecBx.size();
	copySimpleParameter<cl_int>(kernel, 4, vecSize);
	int amount_x, amount_y;
	if (vecBx.size() % 8 == 0) {
		amount_x = vecBx.size();
		amount_y = amount_x;
	}
	else {
		amount_x = vecBx.size() + 8 - (vecBx.size() % 8);
		amount_y = amount_x;
	}
	unsigned int globalDimensionSizes[] = { amount_x, amount_y}; //Definimos el tama�o de las dos dimensiones. Los convertimos a multiplo de 8
	unsigned int localSizes[] = { 8, 8 };
	executeKernel(*ocl, kernel, 2, globalDimensionSizes, localSizes); // Null local dimensions lets OpenCL decide the optimal size
	clFinish(ocl->commandQueue);
	readVectorBuffer(*ocl, phyBuffer, vecPhy);

	//CPU Implementation
	//auto& vec = g.getComponents<BoxCollider_t>();
 //   for (size_t i = 0; i < vec.size(); ++i) {
	//	auto& coll1 = vec[i];
	//	//TODO ELIMINATE DUPLICATE
	//	for (size_t j = i+1; j < vec.size(); ++j) {
	//		auto& coll2 = vec[j];
	//		auto* phy1 = g.getEntity(coll1.entityID).getComponent<PhysicsComponent_t>();
	//		auto* phy2 = g.getEntity(coll2.entityID).getComponent<PhysicsComponent_t>();
	//		if (phy1 == nullptr || phy2 == nullptr) {
	//			GM::Log::log("CollisionSystem_t::collide ERROR A collider entity MUST have a physics component");
	//			exit(-1);
	//		}
	//		if (collide(*phy1, *phy2, coll1, coll2)) {
	//			////Check speed correction for obj 1
	//			glm::vec3 speed1 = { phy1->speed };
	//			glm::vec3 speed2 = { phy2->speed };
	//			modifySpeedAndVelocityOnCollision(*phy1, *phy2, coll1, coll2); //Comprobar los dos objetos, por eso se le da la vuelta a los parametros
	//			modifySpeedAndVelocityOnCollision(*phy2, *phy1, coll2, coll1);
	//			phy1->position -= speed1 * RenderSystem_t::deltaTime;
	//			phy2->position -= speed2 * RenderSystem_t::deltaTime;
	//		}
 //       }
 //   }
	////auto& vec = g.getComponents<BoxCollider_t>();
 //   for (size_t i = 0; i < vec.size(); ++i) {
	//	auto& coll1 = vec[i];
	//	//TODO ELIMINATE DUPLICATE
	//	for (size_t j = i+1; j < vec.size(); ++j) {
	//		auto& coll2 = vec[j];
	//		auto* phy1 = g.getEntity(coll1.entityID).getComponent<PhysicsComponent_t>();
	//		auto* phy2 = g.getEntity(coll2.entityID).getComponent<PhysicsComponent_t>();
	//		if (phy1 == nullptr || phy2 == nullptr) {
	//			GM::Log::log("CollisionSystem_t::collide ERROR A collider entity MUST have a physics component");
	//			exit(-1);
	//		}
	//		if (collide(*phy1, *phy2, coll1, coll2)) {
	//			////Check speed correction for obj 1
	//			glm::vec3 speed1 = { phy1->speed };
	//			glm::vec3 speed2 = { phy2->speed };
	//			modifySpeedAndVelocityOnCollision(*phy1, *phy2, coll1, coll2); //Comprobar los dos objetos, por eso se le da la vuelta a los parametros
	//			modifySpeedAndVelocityOnCollision(*phy2, *phy1, coll2, coll1);
	//			phy1->position -= speed1 * RenderSystem_t::deltaTime;
	//			phy2->position -= speed2 * RenderSystem_t::deltaTime;
	//		}
 //       }
 //   }
#ifdef TIMEMEASURE
	Log::log("Collisions: " + std::to_string(tm.GetCounter()));
#endif
#endif
}

bool CollisionSystem_t::collide(const PhysicsComponent_t& phy1, const PhysicsComponent_t& phy2,
	const BoxCollider_t& c1, const BoxCollider_t& c2) const
{
	//Add the offset to the position
	glm::vec3 pos1 = calculatePosition(phy1.position, c1.length, c1.offset);
	glm::vec3 pos2 = calculatePosition(phy2.position, c2.length, c2.offset);

	//TODO ELIMINATE DUPLICATE
	if (linearOverlap(pos1.x, c1.length.x,
		pos2.x, c2.length.x)) {
		if (linearOverlap(pos1.y, c1.length.y,
			pos2.y, c2.length.y)) {
			if (linearOverlap(pos1.z, c1.length.z,
				pos2.z, c2.length.z)) {
				return true;
			}
		}
	}
	return false;
}
bool CollisionSystem_t::linearOverlap(float x1, float w1, float x2, float w2) const
{
	return (x1 + w1 >= x2 && x1 + w1 <= x2 + w2) || (x2 + w2 >= x1 && x2 + w2 <= x1 + w1);
}

glm::vec3 CollisionSystem_t::calculatePosition(const glm::vec3& position, const glm::vec3& length, const glm::vec3& offset) const {
	glm::vec3 result = { position.x - (length.x / 2) + offset.x,
		position.y - (length.y / 2) + offset.y,  // Length / 2 because thats how the center of the box is defined,
		position.z - (length.z / 2) + offset.z	 // where lenght is the side of the box
	};
	return result;
}

//Modifies the speed of phy1 checking the collision of phy1 with phy2
void CollisionSystem_t::modifySpeedAndVelocityOnCollision(PhysicsComponent_t& phy1, PhysicsComponent_t& phy2,
	const BoxCollider_t& coll1, const BoxCollider_t& coll2) const
{
	bool oneCollides = false;
	phy1.position -= phy1.speed * RenderSystem_t::deltaTime;

	//Check X
	float auxspeed = phy1.speed.x;
	phy1.position.x += phy1.speed.x;
	if (collide(phy1, phy2, coll1, coll2)) {
		oneCollides = true;
		phy1.speed.x = 0;
	}
	phy1.position.x -= auxspeed;

	//Check Y
	auxspeed = phy1.speed.y;
	phy1.position.y += phy1.speed.y;
	if (collide(phy1, phy2, coll1, coll2)) {
		oneCollides = true;
		phy1.speed.y = 0;
	}
	phy1.position.y -= auxspeed;

	//Check Z
	auxspeed = phy1.speed.z;
	phy1.position.z += phy1.speed.z;
	if (collide(phy1, phy2, coll1, coll2)) {
		oneCollides = true;
		phy1.speed.z = 0;
	}
	phy1.position.z -= auxspeed;

	if (!oneCollides) {
		phy1.speed = { 0, 0, 0 };
	}
	phy1.position += phy1.speed * RenderSystem_t::deltaTime;
}
}