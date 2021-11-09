#pragma once
#if 0
#include "platform/SolarPlatform.h"

namespace cm
{
	struct GameState;
	struct RenderState;
	struct AssetState;
	struct TransientState;
	struct MemoryArena;

	void SaveWorldPrefabToTextFile(GameState* gs, RenderState* rs, AssetState* as, TransientState* ts, CString path);
	bool32 LoadWorldPrefabToTextFile(GameState* gs, RenderState* rs, AssetState* as, CString path);

	void SaveWorldToTextFile(GameState* gs, RenderState* rs, AssetState* as, TransientState* ts, CString path);
	bool32 LoadWorldFromTextFile(GameState* gs, AssetState* as, RenderState* rs, CString path);

	void EditorPreprocessAllTextures(CString folderPath, TransientState* ts);
	void EditorLoadProcessedTextures(RenderState* rs, AssetState* as, CString path);

	void EditorPreprocessAllMeshes(CString folderPath, TransientState* ts);
	void EditorPreprocessOutOfDateMeshes(CString folderPath, TransientState* ts);
	void EditorLoadProcessedAssets(RenderState* rs, AssetState* as, MemoryArena* mem, CString folderPath);

	void EditorLoadFontFile(RenderState* rs, AssetState* as, CString path);

	void DEBUGCreateAllMeshes(RenderState* rs, AssetState* as, MemoryArena* mem, CString folderPath);
	int32 DEBUGCreateMesh(RenderState* rs, AssetState* as, MemoryArena* mem, PlatformFile objFile, PlatformFile voxFile);
	int32 DEBUGCreateMesh(RenderState* rs, AssetState* as, MemoryArena* mem, CString objPath, CString voxPath);

	int32 DEBUGCreateOBJMesh(RenderState* rs, AssetState* as, MemoryArena* mem, CString path);
	int32 DEBUGCreateOBJMesh(RenderState* rs, AssetState* as, MemoryArena* mem, PlatformFile objFile);
	int32 DEBUGCreateTexture(RenderState* rs, AssetState* as, CString path, bool flip = true, bool mips = false);
}
#endif