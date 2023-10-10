#pragma once
#include <DirectXMath.h>
class Camera {
public:
	Camera() {
		position = DirectX::XMVectorSet(1.0f, 1.0f, -5.0f, 1.0f);
		focusPoint = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		upDirection = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	}

	void Move(DirectX::XMVECTOR delta) {
		float moveSpeed = 0.1;
		DirectX::XMVECTOR scaledDelta = DirectX::XMVectorScale(delta, moveSpeed);
		position = DirectX::XMVectorAdd(position, scaledDelta);
		focusPoint = DirectX::XMVectorAdd(focusPoint, scaledDelta);
	}
	void Rotate(int deltaX, int deltaY) {
		float rotationSpeed = 0.005f;
		float pitch = deltaY * rotationSpeed;
		float yaw = deltaX * rotationSpeed;

		DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, 0);

		DirectX::XMVECTOR currentDirection = GetForwardDirection();
		DirectX::XMVECTOR newdirection = DirectX::XMVector3Transform(currentDirection, rotationMatrix);
		focusPoint = DirectX::XMVectorAdd(position, newdirection);
	}

	void UpdateViewMatrix() {
		viewMatrix = DirectX::XMMatrixLookAtLH(position, focusPoint, upDirection);
	}

	DirectX::XMMATRIX GetViewMatrix() const {
		return viewMatrix;
	}
	DirectX::XMVECTOR GetForwardDirection() const {
		return DirectX::XMVectorSubtract(focusPoint, position);
	}
	DirectX::XMVECTOR GetLeftDirection() const {
		return DirectX::XMVector4Normalize(DirectX::XMVector3Cross(upDirection, GetForwardDirection()));
	}

private:
	DirectX::XMVECTOR position;
	DirectX::XMVECTOR focusPoint;
	DirectX::XMVECTOR upDirection;
	DirectX::XMMATRIX viewMatrix;
	DirectX::XMVECTOR direction;
};