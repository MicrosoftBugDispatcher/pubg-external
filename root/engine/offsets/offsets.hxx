#pragma once
#include <cstdint>

namespace engine::offsets
{
	// CORE RVAs
	constexpr uint64_t XenuineDecrypt = 0x10F23628;
	constexpr uint64_t UWorld = 0x12A06DE8;
	constexpr uint64_t GNames = 0x12C9CC20;
	constexpr uint64_t GNamesPtr = 0x10;
	constexpr uint64_t ChunkSize = 0x3E14;

	// UWORLD / LEVEL / GAMEINSTANCE
	constexpr uint64_t CurrentLevel = 0x410;
	constexpr uint64_t GameInstance = 0x238;
	constexpr uint64_t AActors = 0x190;
	constexpr uint64_t ALocalPlayer = 0x60;

	// PLAYER CONTROLLER / CAMERA
	constexpr uint64_t PlayerController = 0x38;
	constexpr uint64_t AcknowledgedPawn = 0x4C8;
	constexpr uint64_t PlayerCameraManager = 0x4F0;
	constexpr uint64_t CameraCacheFOV = 0xAC8;
	constexpr uint64_t CameraCacheLocation = 0xACC;
	constexpr uint64_t CameraCacheRotation = 0xABC;

	// PHYSICS
	constexpr uint64_t bAlwaysCreatePhysicsState = 0x498;

	// CHARACTER / MESH / BONES
	constexpr uint64_t RootComponent = 0x2E8;
	constexpr uint64_t Mesh = 0x640;
	constexpr uint64_t ComponentLocation = 0x270;
	constexpr uint64_t ComponentToWorld = 0x260;
	constexpr uint64_t StaticMesh = 0xAE8;
	constexpr uint64_t Gender = 0xB58;
	constexpr uint64_t GroggyHealth = 0x1430;
	constexpr uint64_t SpectatedCount = 0x2B68;

	// CHARACTER CONTROLLER / RECOIL / LEAN
	constexpr uint64_t AnimScriptInstance = 0xE30;
	constexpr uint64_t LastTeamNumber = 0x1370;
	constexpr uint64_t CharacterName = 0x2B98;

	// WEAPON / RECOIL
	constexpr uint64_t bIsScoping_CP = 0x865;
	constexpr uint64_t WeaponProcessor = 0xA10;
	constexpr uint64_t EquippedWeapons = 0x210;
	constexpr uint64_t InvertedRecoilVectorOfEquippedWeapon = 0x11A0;
	constexpr uint64_t VerticalRecovery = 0x1174;

	// DROPPED ITEMS
	constexpr uint64_t DroppedItem = 0x488;
	constexpr uint64_t ItemTable = 0xB0;
	constexpr uint64_t ItemID = 0x274;

	// SPAWNED ITEMS
	constexpr uint64_t DroppedItemGroup = 0x3C0;
	constexpr uint64_t DroppedItemGroupUItem = 0x888;
	constexpr uint64_t DroppedItemGroupStride = 0x10;

	// OBJECT ID
	constexpr uint64_t ObjID_Offset_1 = 0x14;
	constexpr uint64_t GObjects = 0x129C7B60;
	constexpr uint64_t GObjectsCount = 0x129C7B68;
}
