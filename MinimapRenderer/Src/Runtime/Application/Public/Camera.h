#pragma once

#include <d3d11.h>
#include "SimpleMath.h"
#include "Transform.h"
#include "Misc/RefCounting.h"

class Camera : public RefCountedObject
{
	
public:
	Camera(const UINT32& screenWidth, const UINT32& screenHeight);
	Camera(const Camera&);
	~Camera();

	void Update();
	void BuildProjectionMatrix();
	void BuildViewMatrix();
	inline void GetProjectionMatrix(DirectX::SimpleMath::Matrix& m) { m = m_ProjectionMatrix; }
	inline void GetGPUProjectionMatrix(DirectX::SimpleMath::Matrix& m) { m = m_ProjectionMatrixGPU; }
	inline void GetInvProjectionMatrix(DirectX::SimpleMath::Matrix& m) { m = m_InvProjectionMatrix; }
	inline void GetGPUInvProjectionMatrix(DirectX::SimpleMath::Matrix& m) { m = m_InvProjectionMatrixGPU; }
	inline void GetViewMatrix(DirectX::SimpleMath::Matrix& m) { m = m_ViewMatrix; }
	inline void GetGPUViewMatrix(DirectX::SimpleMath::Matrix& m) { m = m_ViewMatrixGPU; }

	Transform m_Transform;
	DirectX::SimpleMath::Vector3 m_EulerAngles;
	DirectX::SimpleMath::Color m_ClearColor;
	float m_NearClip, m_FarClip;
	float m_FieldOfView, m_ScreenAspect;
	D3D11_VIEWPORT m_ViewPort;

private:
	DirectX::SimpleMath::Matrix m_ProjectionMatrix;
	DirectX::SimpleMath::Matrix m_InvProjectionMatrix;
	DirectX::SimpleMath::Matrix m_ViewMatrix;
	DirectX::SimpleMath::Matrix m_ProjectionMatrixGPU;
	DirectX::SimpleMath::Matrix m_InvProjectionMatrixGPU;
	DirectX::SimpleMath::Matrix m_ViewMatrixGPU;

};
