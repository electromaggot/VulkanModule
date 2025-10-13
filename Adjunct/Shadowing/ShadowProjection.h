//
// ShadowProjection.h
//	VulkanModule Adjunct - Shadow Mapping Utilities
//
// Provides helper functions for calculating light-space projection matrices
//	for shadow mapping.  Supports both orthographic (directional light) and
//	perspective (point light) projections.
//
// Created 3 Oct 2025 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef ShadowProjection_h
#define ShadowProjection_h

#include "VulkanMath.h"
#include "ShadowMappingTypes.h"


class ShadowProjection		// Helper class for calculating shadow projection matrices.
{
public:
	// Calculate recommended shadow map resolution based on FOV and camera mode.
	// Wider FOV requires higher resolution to maintain shadow quality.
	// Returns recommended resolution (width and height are always equal for square shadow maps).
	static uint32_t calculateRecommendedResolution(
		float fovRadians,
		ShadowCameraMode cameraMode,
		ShadowProjectionMode projectionMode)
	{
		// Base resolution for standard 90° FOV.
		const uint32_t BASE_RESOLUTION = 2048;

		// Straight-down and custom cameras with wide FOV need higher resolution
		//	to cover the larger area without losing detail.
		if (cameraMode == SHADOW_CAMERA_STRAIGHT_DOWN || cameraMode == SHADOW_CAMERA_CUSTOM_DIRECTION) {
			float fovDegrees = glm::degrees(fovRadians);

			if (fovDegrees > 150.0f) {
				return 4096;  // Very wide FOV (150°+): 4K shadow map
			} else if (fovDegrees > 120.0f) {
				return 3072;  // Wide FOV (120-150°): 3K shadow map
			} else if (fovDegrees > 90.0f) {
				return 2048;  // Standard-wide FOV (90-120°): 2K shadow map
			} else {
				return 2048;  // Narrow FOV (< 90°): Standard 2K
			}
		}

		// Look-at-origin mode with narrower effective FOV can use lower resolution:
		if (cameraMode == SHADOW_CAMERA_LOOK_AT_ORIGIN) {
			float fovDegrees = glm::degrees(fovRadians);

			if (fovDegrees > 120.0f) {
				return 2048;  // Wide: 2K
			} else {
				return 2048;  // Standard: 2K (original default)
			}
		}

		return BASE_RESOLUTION;
	}

	// Calculate light-space matrix for shadow mapping.
	// Parameters:
	//   lightPosition: World-space position of the light source.
	//   sceneCenter: World-space center point the light looks at (used for SHADOW_CAMERA_LOOK_AT_ORIGIN mode).
	//   projectionMode: SHADOW_ORTHOGRAPHIC or SHADOW_PERSPECTIVE
	//   cameraMode: How shadow camera is oriented (straight down, custom direction, or look at origin).
	//   customDirection: Direction vector for SHADOW_CAMERA_CUSTOM_DIRECTION mode (ignored otherwise).
	//   orthoSize: Half-width/height for orthographic projection (ignored for perspective).
	//   fov: Field of view in radians for perspective projection (ignored for orthographic).
	//   nearPlane: Near clipping plane distance.
	//   farPlane: Far clipping plane distance.
	static glm::mat4 calculateLightSpaceMatrix(
		const glm::vec3& lightPosition,
		const glm::vec3& sceneCenter,
		ShadowProjectionMode projectionMode,
		ShadowCameraMode cameraMode = SHADOW_CAMERA_STRAIGHT_DOWN,
		const glm::vec3& customDirection = glm::vec3(0.0f, -1.0f, 0.0f),
		float orthoSize = 15.0f,
		float fov = glm::radians(90.0f),
		float nearPlane = 0.1f,
		float farPlane = 40.0f)
	{
		// Calculate light's view direction based on camera mode.
		glm::vec3 lightDir;
		glm::vec3 targetPosition;
		glm::vec3 up;

		switch (cameraMode) {
			case SHADOW_CAMERA_STRAIGHT_DOWN:
				// Point straight down (-Y axis), like a lamp with top reflector.
				lightDir = glm::vec3(0.0f, -1.0f, 0.0f);
				targetPosition = lightPosition + lightDir;
				break;

			case SHADOW_CAMERA_CUSTOM_DIRECTION:
				// Use custom direction vector:
				lightDir = glm::normalize(customDirection);
				targetPosition = lightPosition + lightDir;
				break;

			case SHADOW_CAMERA_LOOK_AT_ORIGIN:
			default:
				// Look at scene center from light position (original behavior).
				lightDir = glm::normalize(sceneCenter - lightPosition);
				targetPosition = sceneCenter;
				break;
		}

		// Choose stable "up" vector to avoid gimbal lock when light crosses axes:
		if (abs(lightDir.y) > 0.99f) {
			up = glm::vec3(0.0f, 0.0f, 1.0f);  // Light nearly vertical, use Z-axis as up.
		} else {
			up = glm::vec3(0.0f, 1.0f, 0.0f);  // Normal case, use Y-axis as up.
		}

		glm::mat4 lightView = glm::lookAt(lightPosition, targetPosition, up);

		// Choose projection based on shadow mode:
		glm::mat4 lightProj;
		if (projectionMode == SHADOW_ORTHOGRAPHIC) {
			// Orthographic: Parallel light rays (sun/directional light),
			//	uniform shadow quality across entire scene.
			lightProj = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, nearPlane, farPlane);
		} else {
			// Perspective: Radial light rays (point light source),
			//	matches Phong lighting for accurate shadows.
			float aspect = 1.0f;  // Square shadow map
			lightProj = glm::perspective(fov, aspect, nearPlane, farPlane);
		}

		return lightProj * lightView;
	}

};

#endif // ShadowProjection_h
