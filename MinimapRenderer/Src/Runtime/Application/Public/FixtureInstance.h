#pragma once

#include "Fixture.h"
#include "Transform.h"

class FixtureInstance {
public:
	FixtureInstance(const std::string& fixtureName, const DirectX::SimpleMath::Vector3& position, DirectX::SimpleMath::Quaternion& rotation, DirectX::SimpleMath::Vector3& scale) :
		m_FixtureName(fixtureName),
		m_Transform(position, rotation, scale * 0.01f)
	{}

	~FixtureInstance()
	{}

	//NO_COPY_CONSTRUCT_OR_COPY_ASSIGN(FixtureInstance);

	const std::string& GetFixtureName() const { return m_FixtureName; }

public:
	Transform m_Transform;

private:
	std::string m_FixtureName;
};
