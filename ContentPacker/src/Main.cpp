#include "Core.h"
#include "MetaProcessor.h"
#include "FileProcessor.h"
#include "MeshProcessor.h"
#include "MaterialProcessor.h"
#include "TextureProcessor.h"
#include "ProgramProcessor.h"

#define PROCESS_MODELS 1
#define PROCESS_MATERIALS 1
#define PROCESS_TEXTURES 1

using namespace cm;

static CString ASSET_PATH = "F:/codes/SolarOmen/SolarOmen-2/Assets/Raw/";

static std::vector<Model> LoadAndProcessModels(FileProcessor& fileProcessor, MetaProcessor& metaProcessor)
{
	std::vector<CString> paths = fileProcessor.GetFilePaths(ASSET_PATH, "obj");
	std::vector<CString> missingMetaFiles = metaProcessor.FindMissing(paths);

	for (const CString& path : missingMetaFiles)
	{
		ModelMetaFile metaData = {};
		metaData.scale = 2.0f;
		metaData.layout = VertexLayoutType::Value::PNT;

		CString newPath = Util::StripFileExtension(path).Add(".slo");
		metaProcessor.SaveMetaData(newPath, metaData);
		LOG("Creating meta file for: " << path.GetCStr());
	}

	metaProcessor.LoadAllMetaFiles(fileProcessor.GetFilePaths(ASSET_PATH, "slo"));

	LOG("PROCESSING MODELS");
	ModelProcessor modelProcessor;
	std::vector<Model> models = modelProcessor.LoadModels(paths, metaProcessor);
	LOG("COMPLETE");

	return models;
}

static std::vector<Material> LoadAndProcessMaterials(FileProcessor& fileProcessor)
{
	LOG("PROCESSING MATERIALS");
	MaterialProcessor materialProcessor;
	std::vector<Material> materials = materialProcessor.LoadMTLMaterials(fileProcessor.GetFilePaths(ASSET_PATH, "mtl"));
	LOG("COMPLETE");

	return materials;
}

static std::vector<Texture> LoadAndProcessTextures(FileProcessor& fileProcessor, MetaProcessor& metaProcessor)
{
	std::vector<CString> paths = fileProcessor.GetFilePaths(ASSET_PATH, "png");
	std::vector<CString> missingMetaFiles = metaProcessor.FindMissing(paths);

	for (const CString& path : missingMetaFiles)
	{
		LOG("Creating meta file for: " << path.GetCStr());
	}

	metaProcessor.LoadAllMetaFiles(fileProcessor.GetFilePaths(ASSET_PATH, "slo"));

	LOG("PROCESSING TEXTURES");
	TextureProcessor textureProcessor;
	std::vector<Texture> textures = textureProcessor.LoadTextures(paths, metaProcessor);
	LOG("COMPLETE");

	return textures;
}

static std::vector<Program> LoadAndProcessPrograms(FileProcessor& fileProcessor)
{
	std::vector<CString> paths = CombineStdVectors(
		fileProcessor.GetFilePaths(ASSET_PATH, "vert.cso"),
		fileProcessor.GetFilePaths(ASSET_PATH, "pixl.cso"),
		fileProcessor.GetFilePaths(ASSET_PATH, "comp.cso")
	);

	LOG("PROCESSING SHADER PROGRAMS");
	ProgramProcessor textureProcessor;
	std::vector<Program> programs = textureProcessor.LoadPrograms(paths);
	LOG("COMPLETE");

	return programs;
}

template<typename T>
void SaveBinaryData(const std::vector<T>& data, const CString& path)
{
	BinaryFile file;
	file.Write((uint32)data.size());
	for (const T& d : data)
	{
		d.SaveBinaryData(&file);
	}
	file.SaveToDisk(path);
}

void CombineModels(std::vector<Model>& models, std::vector<Material>& materials);
int main()
{
	FileProcessor fileProcessor;

	MetaProcessor metaProcessor;
	metaProcessor.LoadAllMetaFiles(fileProcessor.GetFilePaths(ASSET_PATH, "slo"));

#if PROCESS_MODELS
	std::vector<Model> models = LoadAndProcessModels(fileProcessor, metaProcessor);
#endif
#if PROCESS_TEXTURES
	std::vector<Texture> textures = LoadAndProcessTextures(fileProcessor, metaProcessor);
#endif
	std::vector<Program> programs = LoadAndProcessPrograms(fileProcessor);
	std::vector<Material> materials = LoadAndProcessMaterials(fileProcessor);

#if PROCESS_MODELS 
	CombineModels(models, materials);
	LOG("SAVING MODELS");
	SaveBinaryData(models, "../Assets/Packed/models.bin");
	LOG("COMPLETE");
#endif

#if PROCESS_MATERIALS
	LOG("SAVING MATERIALS");
	SaveBinaryData(materials, "../Assets/Packed/materials.bin");
	LOG("COMPLETE");
#endif

#if PROCESS_TEXTURES
	LOG("SAVING TEXTURES");
	SaveBinaryData(textures, "../Assets/Packed/textures.bin");
	LOG("COMPLETE");
#endif

	LOG("SAVING SHADER PROGRAMS");
	SaveBinaryData(programs, "../Assets/Packed/programs.bin");
	LOG("COMPLETE");

	return 0;
}

void CombineModels(std::vector<Model>& models, std::vector<Material>& materials)
{
	for (Model& model : models)
	{
		for (Mesh& mesh : model.meshes)
		{
			for (Material& mat : materials)
			{
				if (mat.name == mesh.materialName)
				{
					for (int32 i = 0; i < mesh.vertices.size(); i++)
					{
						mesh.vertices.at(i).colours = Vec4f(mat.colourKd, 1.0f);
						mesh.hasColours = true;
					}

					break;
				}
			}
		}
	}

	for (Model& model : models)
	{
		if (model.meshes.size() > 1)
		{
			Mesh mesh = model.meshes[0];
			for (int32 i = 1; i < model.meshes.size(); i++)
			{
				Mesh& m = model.meshes.at(i);

				uint32 indexOffset = 0;
				for (int32 j = 0; j < mesh.indices.size(); j++)
				{
					indexOffset = Max(indexOffset, mesh.indices.at(j));
				}

				for (int32 j = 0; j < m.vertices.size(); j++)
				{
					mesh.vertices.push_back(m.vertices.at(j));
				}

				for (int32 j = 0; j < m.indices.size(); j++)
				{
					mesh.indices.push_back(indexOffset + m.indices.at(j) + 1);
				}
			}

			model.meshes.clear();
			model.meshes.push_back(mesh);
		}
	}
}