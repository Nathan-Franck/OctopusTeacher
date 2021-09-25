
#include "Game.h"
#include "TypeSet.h"
#include <Model.h>
#include <MadnessModel.h>

void Game::Update(float dt)
{
	time += dt;

	octopusComponent.Update(time);

	TransformComponent* transform = Utils::GetMutableForEntity<TransformComponent>(octopusComponent.octopusScene);
	const Entity parentEnt = Utils::GetForEntity<HierarchyComponent>(octopusComponent.octopusScene)->parentID;
	const TransformComponent* parentTransform = Utils::GetForEntity<TransformComponent>(parentEnt);
	const XMMATRIX localToGlobal = Utils::LocalToGlobalMatrix(Utils::GetAncestryForEntity(parentEnt));
	const XMMATRIX globalToLocal = XMMatrixInverse(nullptr, localToGlobal);
	const XMFLOAT3 local = transform->translation_local;
	const XMVECTOR position = XMVector4Transform({ local.x, local.y, local.z, 1 }, localToGlobal);
	const XMVECTOR direction = position - particle.position;
	const XMFLOAT3 resFl = particle.MoveTowards(direction);
	const XMVECTOR result = { resFl.x, resFl.y, resFl.z, 1 };
	particle.position = result;
	XMStoreFloat3(&transform->translation_local, XMVector4Transform(result, globalToLocal));
	transform->UpdateTransform_Parented(*parentTransform);

	TypeSetTester::Test();
	ModelTester::Test();
	MadnessModelTester::Test();
}
