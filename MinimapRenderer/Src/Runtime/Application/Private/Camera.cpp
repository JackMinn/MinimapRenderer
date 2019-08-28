#include "Camera.h"
#include "Input.h"

Camera::Camera(const uint32_t& screenWidth, const uint32_t& screenHeight) : m_Transform(Transform(DirectX::SimpleMath::Vector3::Zero, DirectX::SimpleMath::Quaternion::Identity,
													DirectX::SimpleMath::Vector3::One)), m_ClearColor { 0.04f, 0.04f, 0.04f, 1.0f }
{
	m_EulerAngles = m_Transform.GetEulerAngles();

	m_NearClip = 0.3f;
	m_FarClip = 100000.0f;
	//m_FarClip = 1000000.0f;
	m_FieldOfView = DirectX::XM_2PI / 8.0f;
	m_ScreenAspect = (float)screenWidth / (float)screenHeight;
	m_ViewPort.Width = (float)screenWidth;
	m_ViewPort.Height = (float)screenHeight;
	m_ViewPort.MinDepth = 0.0f;
	m_ViewPort.MaxDepth = 1.0f;
	m_ViewPort.TopLeftX = 0.0f;
	m_ViewPort.TopLeftY = 0.0f;

	Camera::BuildProjectionMatrix();
	Camera::BuildViewMatrix();
}

Camera::Camera(const Camera&)
{
}

Camera::~Camera() 
{
}

// It seems the call to update is currently frame rate dependent, so move speed and rotate speed are fps based
// Maybe later change this to a FixedUpdate similar to Unity
void Camera::Update() 
{
	using namespace DirectX::SimpleMath;

	//float moveSpeed = (1.0f / 120.f);
	float moveSpeed = 20.f;
	const float rotateSpeed = 0.02f;

	if (Input::IsKeyDown(VK_SHIFT)) {
		moveSpeed *= 50;
	}

	Vector3 movementVector = Vector3{ 0.0f, 0.0f, 0.0f };

	// Directions are modified to account for the rotated left handed space the NIFs exist in
	if (Input::IsKeyDown(VK_UP)) {
		//movementVector.z += moveSpeed;
		movementVector.y += moveSpeed;
	}
	if (Input::IsKeyDown(VK_DOWN)) {
		//movementVector.z -= moveSpeed;
		movementVector.y -= moveSpeed;
	}
	if (Input::IsKeyDown(VK_RIGHT)) {
		//movementVector.x += moveSpeed;
		movementVector.x += moveSpeed;
	}
	if (Input::IsKeyDown(VK_LEFT)) {
		//movementVector.x -= moveSpeed;
		movementVector.x -= moveSpeed;
	}

	/*  Roll is a rotation around the Forward Axis
		Pitch is a rotation around the Right Axis
		Yaw is a rotation around the Up Axis
		The rotation should be done in the order Roll->Pitch->Yaw
	*/
	
	// This is change in the y axis of the screen, which for both coordinate systems, we store as a rotation around the x
	m_EulerAngles.x += Input::GetMouseDelta().second * rotateSpeed;
	// This is change in the x axis of the screen, which for a RH coordinate system, we store as rotation around the y
	m_EulerAngles.y += Input::GetMouseDelta().first * rotateSpeed; 
	m_EulerAngles.x = std::fmod(m_EulerAngles.x, 360.f);
	m_EulerAngles.y = std::fmod(m_EulerAngles.y, 360.f);

	// Correct way to do it, respect the quaternion sequence order, then fix axis to match CS and negate to match LH
	Quaternion rotation = Quaternion::CreateFromPitchYawRoll(m_EulerAngles.x, m_EulerAngles.y, 0.0f); 
	std::swap(rotation.y, rotation.z);
	rotation.x = -rotation.x;
	rotation.y = -rotation.y;
	rotation.z = -rotation.z;
		
	m_Transform.SetRotation(rotation);

	Vector3 deltaPosition = Vector3::Transform(movementVector, m_Transform.GetRotation());
	m_Transform.SetPosition(deltaPosition + m_Transform.GetPosition());
}

void Camera::BuildProjectionMatrix()
{
	using namespace DirectX::SimpleMath;

	// Maybe for LH matrix, my shaders need to swap the order of matrix multiplication?
	//m_ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(m_FieldOfView, m_ScreenAspect, m_NearClip, m_FarClip); // Why does this completely break?
	m_ProjectionMatrix = DirectX::XMMatrixPerspectiveFovRH(m_FieldOfView, m_ScreenAspect, m_NearClip, m_FarClip); // Why RH?
	m_InvProjectionMatrix = m_ProjectionMatrix.Invert();
	m_ProjectionMatrixGPU = DirectX::XMMatrixTranspose(m_ProjectionMatrix);
	m_InvProjectionMatrixGPU = DirectX::XMMatrixTranspose(m_InvProjectionMatrix);
}

void Camera::BuildViewMatrix()
{
	using namespace DirectX::SimpleMath;

	//Vector3 lookAtPoint = { 0, 0, 1 };
	//Vector3 upVector = { 0, 1, 0 };

	// Lets try coordinate system change
	Vector3 lookAtPoint = { 0, -1, 0 };
	Vector3 upVector = { 0, 0, 1 };

	//rotate the point and vector by the camera's rotation quaternion
	lookAtPoint = DirectX::XMVector3Rotate(lookAtPoint, m_Transform.GetRotation());
	upVector = DirectX::XMVector3Rotate(upVector, m_Transform.GetRotation());

	//translate the look at point so that it is relative to the cameras actual position
	lookAtPoint = DirectX::XMVectorAdd(m_Transform.GetPosition(), lookAtPoint);

	//create a left handed look at matrix
	//m_ViewMatrix = Matrix::CreateLookAt(m_Position, lookAtPoint, upVector);
	m_ViewMatrix = Matrix::CreateLookAt(m_Transform.GetPosition(), lookAtPoint, upVector);
	m_ViewMatrixGPU = DirectX::XMMatrixTranspose(m_ViewMatrix);
}
