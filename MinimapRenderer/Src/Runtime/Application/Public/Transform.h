#pragma once

#include <d3d11.h>
#include "SimpleMath.h"

// WARNING, THESE FUNCTIONS WERE BUILT FOR THE DEFAULT RH COORDINATE SYSTEM DIRECTX USES

class Transform
{
public:
	Transform(const Transform& T) {
		memcpy_s(this, sizeof(Transform), &T, sizeof(Transform));
	}
	~Transform() 
	{}

	inline DirectX::SimpleMath::Vector3 GetPosition() const { return m_Position; }
	inline DirectX::SimpleMath::Quaternion GetRotation() const { return m_Rotation; }
	inline DirectX::SimpleMath::Vector3 GetScale() const { return m_Scale; }
	inline DirectX::SimpleMath::Vector3 GetEulerAngles() const { return DirectX::SimpleMath::Quaternion::EulerAngles(m_Rotation); }

	inline void SetPosition(const DirectX::SimpleMath::Vector3& position) { m_Position = position; }
	inline void SetRotation(const DirectX::SimpleMath::Quaternion& rotation) { m_Rotation = rotation; }
	inline void SetScale(const DirectX::SimpleMath::Vector3& scale) { m_Scale = scale; }
	inline void SetEulerAngles(const DirectX::SimpleMath::Vector3& eulerAngles) { m_Rotation = DirectX::SimpleMath::Quaternion::CreateFromPitchYawRoll(
																				eulerAngles.x, eulerAngles.y, eulerAngles.z); }

	inline void SetEulerAngles(float pitch, float yaw, float roll) { m_Rotation = DirectX::SimpleMath::Quaternion::CreateFromPitchYawRoll(
																	pitch, yaw, roll); }

	// First apply rotation, then apply m_Rotation, which transforms the base of rotation to local basis causing it to become a local rotation
	inline void RotateLocal(const DirectX::SimpleMath::Vector3& eulerAngles) { 
		DirectX::SimpleMath::Quaternion rotation = DirectX::SimpleMath::Quaternion::CreateFromPitchYawRoll(eulerAngles.x, eulerAngles.y, eulerAngles.z);
		m_Rotation = rotation * m_Rotation;
		m_Rotation.Normalize();
	}

	inline void RotateLocal(float pitch, float yaw, float roll) {
		DirectX::SimpleMath::Quaternion rotation = DirectX::SimpleMath::Quaternion::CreateFromPitchYawRoll(pitch, yaw, roll);
		m_Rotation = rotation * m_Rotation;
		m_Rotation.Normalize();
	}

	inline void RotateWorld(const DirectX::SimpleMath::Vector3& eulerAngles) {
		DirectX::SimpleMath::Quaternion rotation = DirectX::SimpleMath::Quaternion::CreateFromPitchYawRoll(eulerAngles.x, eulerAngles.y, eulerAngles.z);
		m_Rotation = m_Rotation * rotation;
		m_Rotation.Normalize();
	}

	inline void RotateWorld(float pitch, float yaw, float roll) {
		DirectX::SimpleMath::Quaternion rotation = DirectX::SimpleMath::Quaternion::CreateFromPitchYawRoll(pitch, yaw, roll);
		m_Rotation =  m_Rotation * rotation;
		m_Rotation.Normalize();
	}

	// This is very inefficient, its better to build these once a frame, or even only
	// once everytime the transform is modified, so this can remain on TODO for optimization
	inline DirectX::SimpleMath::Matrix GetLocalToWorldMatrix() 
	{
		return DirectX::SimpleMath::Matrix::SRT(m_Scale, m_Rotation, m_Position);
	}

	inline DirectX::SimpleMath::Matrix GetLocalToWorldMatrixGPU()
	{
		return DirectX::XMMatrixTranspose(GetLocalToWorldMatrix());
	}

	inline DirectX::SimpleMath::Matrix GetWorldToLocalMatrix()
	{
		return DirectX::XMMatrixInverse(nullptr, GetLocalToWorldMatrix());
	}

	inline DirectX::SimpleMath::Matrix GetWorldToLocalMatrixGPU()
	{
		return DirectX::XMMatrixTranspose(GetWorldToLocalMatrix());
	}

private:
	friend class AssetManager;
	friend class FixtureInstance;
	friend class Camera;

	DirectX::SimpleMath::Vector3 m_Position;
	DirectX::SimpleMath::Quaternion m_Rotation;
	DirectX::SimpleMath::Vector3 m_Scale;

	Transform() : m_Position(DirectX::SimpleMath::Vector3::Zero), m_Rotation(DirectX::SimpleMath::Quaternion::Identity), 
						m_Scale(DirectX::SimpleMath::Vector3::One) {
		DebugLog("s", "Made Transform with default constructor.");
	}

	explicit Transform(const DirectX::SimpleMath::Vector3& p, const DirectX::SimpleMath::Quaternion& q, const DirectX::SimpleMath::Vector3& s) :
		m_Position(p), m_Rotation(q), m_Scale(s) {
		DebugLog("s", "Made Transform with explicit constructor.");
	}

};


