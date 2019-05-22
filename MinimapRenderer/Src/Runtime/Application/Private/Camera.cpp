#include "Camera.h"
#include "Input.h"

Camera::Camera(const uint32_t& screenWidth, const uint32_t& screenHeight) : m_Transform(Transform(DirectX::SimpleMath::Vector3::Zero, DirectX::SimpleMath::Quaternion::Identity,
													DirectX::SimpleMath::Vector3::One)), m_ClearColor { 0.04f, 0.04f, 0.04f, 1.0f }
{
	m_EulerAngles = m_Transform.GetEulerAngles();

	m_NearClip = 0.3f;
	m_FarClip = 100000.0f;
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

	//const float moveSpeed = (1.0f / 60.0f);
	//const float rotateSpeed = 0.1f;
	float moveSpeed = (1.0f / 120.f);
	const float rotateSpeed = 0.02f;
	//const float rotateSpeed = 0.001f; 

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

	m_Transform.SetRotation(Quaternion::CreateFromPitchYawRoll(m_EulerAngles.x, m_EulerAngles.y, 0.0f));

	// Changed to axis to match the rotated LH coordinate system of the NIFs
	// However this does not work because it builds the quaternion in the wrong order of rotation, quaternions are intrinsic rotations
	// and thus applying the axis rotations in the wrong order produces the wrong total rotation
	//m_Transform.SetRotation(Quaternion::CreateFromPitchYawRoll(-m_EulerAngles.x, 0.0f, -m_EulerAngles.y)); 

	Quaternion temp = Quaternion::CreateFromPitchYawRoll(-m_EulerAngles.x, 0.0f, -m_EulerAngles.y); // I still dont understand why this doesnt work
	temp.y = -temp.y; // The components here are in the right place, but still need negating
	
	temp = Quaternion::CreateFromPitchYawRoll(m_EulerAngles.x, m_EulerAngles.y, 0.0f); // CORRECT WAY TO DO IT, respect the quaternion sequence order, then fix axis to match CS and negate to match LH
	temp.x = -temp.x;
	temp.y = -temp.y;
	temp.z = -temp.z;
	std::swap(temp.y, temp.z);

	// Having our own set of quaternions made from axis angles around the basis axis allows us to build the intrinsic rotation order as we desire
	Quaternion zRot = Quaternion::CreateFromAxisAngle(Vector3::UnitZ, DirectX::XMConvertToRadians(-m_EulerAngles.y));
	Quaternion xRot = Quaternion::CreateFromAxisAngle(Vector3::UnitX, DirectX::XMConvertToRadians(-m_EulerAngles.x));
	Quaternion rot = xRot * zRot; // Suggests that we are doing zRot first, then xRot next.... this is the opposite order of what we should do for Roll->Pitch->Yaw
	//Quaternion xRot = Quaternion::CreateFromAxisAngle(Vector3::UnitX, DirectX::XMConvertToRadians(m_EulerAngles.x));
	//Quaternion yRot = Quaternion::CreateFromAxisAngle(Vector3::UnitY, DirectX::XMConvertToRadians(m_EulerAngles.y));
	//Quaternion rot = xRot * yRot; // This matches what CreateFromPitchYawRoll is doing, and so this must apply xRot first then yRot
	//rot.Normalize();

	Quaternion q1 = Quaternion::CreateFromPitchYawRoll(m_EulerAngles.x, m_EulerAngles.y, 0.0f);
	Quaternion q2X = Quaternion::CreateFromAxisAngle(Vector3::UnitX, DirectX::XMConvertToRadians(m_EulerAngles.x));
	Quaternion q2Y = Quaternion::CreateFromAxisAngle(Vector3::UnitY, DirectX::XMConvertToRadians(m_EulerAngles.y));
	Quaternion q2 = q2X * q2Y; // This matches what CreateFromPitchYawRoll is doing, and so this must apply xRot first then yRot
	q2.Normalize();

	// This produces differing quaternions, but I dont know why, and the issue is q1 being incorrect
	// It will rotate by roll then pitch then yaw in terms of the argument angles, 
	// however in this case the value passed into "Roll" is actually our Yaw (our rotation by the Up Axis), while the Pitch is correct.
	// So we have made a quaternion that is applying our Yaw rotation first, and then our Pitch. 
	q1 = Quaternion::CreateFromPitchYawRoll(-m_EulerAngles.x, 0.0f, -m_EulerAngles.y); 
	//q1 = Quaternion::CreateFromPitchYawRoll(m_EulerAngles.x, 0.0f, m_EulerAngles.y); // THIS WORKS....
	//q1.x = -q1.x; q1.y = -q1.y; q1.z = -q1.z;
	q2X = Quaternion::CreateFromAxisAngle(Vector3::UnitX, DirectX::XMConvertToRadians(-m_EulerAngles.x));
	q2Y = Quaternion::CreateFromAxisAngle(Vector3::UnitZ, DirectX::XMConvertToRadians(-m_EulerAngles.y));
	q2 = q2Y * q2X; // LHS then RHS
	q2.Normalize();

	// What we need is to pass the angles in the correct order (negate for coordinate system change from RH to LH) aka:
	Quaternion test = Quaternion::CreateFromPitchYawRoll(-m_EulerAngles.x, -m_EulerAngles.y, 0.0f);
	// Now the values inside the quaternion represent a rotation around an axis, we just need to swap the axis components to match our coordinate system
	// Namely our right axis is unchanged, but our forward and up axis have swapped (y is forward, z is up) so we do:
	//test.x = -test.x; test.y = -test.y; test.z = -test.z;
	std::swap(test.y, test.z);

	// FIGURE OUT THE BELOW - The math for multiplying quaternions very clearly illustrates why changing order is flipping a sign, rather than giving the same
	// quaternion but with re-arranged values
	q2X = Quaternion::CreateFromAxisAngle(Vector3::UnitX, DirectX::XMConvertToRadians(-m_EulerAngles.x));
	q2Y = Quaternion::CreateFromAxisAngle(Vector3::UnitZ, DirectX::XMConvertToRadians(-m_EulerAngles.y));
	q2 = q2Y * q2X; // LHS then RHS
	q2.Normalize();
	q1 = q2X * q2Y;
	q1.Normalize();
	// WHY ARE THESE TWO THE SAME BUT THE ABOVE TWO DIFFERENT? Order is Roll->Pitch->Yaw
	// The quaternion will always hold the SAME 4 floats, regardless of the quaternion sequence, the only thing that differs is WHICH component each float becomes in the quaternion
	// (Hence why swapping values is enough to retrieve them). Yes, different sequences produce different rotations, but you can make them the same rotation by changing what axis
	// the quaternion encodes by swapping around the components that make up the axis. 
	float a1 = 57.f, a2 = 196.f, a3 = 134.f;
	// (0, a1, a3) and (a3, 0, a1) produced some interesting results, one of the components was negated so swapping around alone was not sufficient
	// same with (0, a3, a1) and (a3, 0, a1), you would assume I could just swap x and y to get the same result but no
	q1 = Quaternion::CreateFromPitchYawRoll(0, a3, a1); // a1->a3->a2 doesnt work but a2->a1->a3 does work
	std::swap(q1.x, q1.y);
	q2 = Quaternion::CreateFromPitchYawRoll(a3, 0, a1); // a3->a1->a2
	//std::swap(q2.y, q2.z);

	// So the math adds up, for a quaternion comprised of a rotation around 2 axis only (not 3), then all that changes for axis-angle quaternions order change is a negative sign in 1 component.
	// However when building the quaternion with the function, no matter what order I give the euler angles in, the components will be the same (?? confirm this - SEEMS ONLY IF WE HAVE TWO EULER ANGLES)
	// Adding a 3rd angle in causes the quaternions to diverge. 

	//DebugLog("sfsfsfsfs", "<", test.x, ", ", test.y, ", ", test.z, ", ", test.w, ">");
	DebugLog("sfsfsfsfs", "<", q1.x, ", ", q1.y, ", ", q1.z, ", ", q1.w, ">");
	DebugLog("sfsfsfsfs", "<", q2.x, ", ", q2.y, ", ", q2.z, ", ", q2.w, ">");
	//if (test == q1 && test == q2) {
	//	DebugLog("s", "ALL THE SAME");
	//}

	DirectX::XMVECTOR quat = DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(45.f), DirectX::XMConvertToRadians(45.f), 0.f);
	float angle2 = acosf(quat.m128_f32[3]) * 2;
	DirectX::XMVECTOR axis;
	float angle;
	DirectX::XMQuaternionToAxisAngle(&axis, &angle, quat);
	axis = DirectX::XMVector3Normalize(axis);
	//DebugLog("sfsfsfsfsfs", "<", axis.m128_f32[0], ", ", axis.m128_f32[1], ", ", axis.m128_f32[2], ", ", DirectX::XMConvertToDegrees(angle), ", ", DirectX::XMConvertToDegrees(angle2), ">");

	Quaternion trial = Quaternion::CreateFromPitchYawRoll(m_EulerAngles.x, 0.0f, m_EulerAngles.y);
	trial.x = -trial.x; trial.y = -trial.y; trial.z = -trial.z; // This is tempting to do, but wouldnt work too hot if all 3D was being used
	m_Transform.SetRotation(trial);

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
	//lookAtPoint = DirectX::XMVector3Rotate(lookAtPoint, m_Rotation);
	//upVector = DirectX::XMVector3Rotate(upVector, m_Rotation);

	lookAtPoint = DirectX::XMVector3Rotate(lookAtPoint, m_Transform.GetRotation());
	upVector = DirectX::XMVector3Rotate(upVector, m_Transform.GetRotation());


	//translate the look at point so that it is relative to the cameras actual position
	//lookAtPoint = DirectX::XMVectorAdd(m_Position, lookAtPoint);
	lookAtPoint = DirectX::XMVectorAdd(m_Transform.GetPosition(), lookAtPoint);

	//create a left handed look at matrix
	//m_ViewMatrix = Matrix::CreateLookAt(m_Position, lookAtPoint, upVector);
	m_ViewMatrix = Matrix::CreateLookAt(m_Transform.GetPosition(), lookAtPoint, upVector);
	m_ViewMatrixGPU = DirectX::XMMatrixTranspose(m_ViewMatrix);
}
