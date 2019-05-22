#pragma once

#include "Fixture.h"
#include "Transform.h"

class FixtureInstance {
public:
	FixtureInstance(const std::string& fixtureName) :
		m_FixtureName(fixtureName),
		m_Transform()
	{}

	~FixtureInstance()
	{}

	NO_COPY_CONSTRUCT_OR_COPY_ASSIGN(FixtureInstance);

	std::string GetFixtureName() const { return m_FixtureName; }

public:
	Transform m_Transform;

private:
	std::string m_FixtureName;
};
