#include "Core.h"
#include "MetaProcessor.h"
#include "FileProcessor.h"
#include "MeshProcessor.h"
#include "MaterialProcessor.h"

using namespace cm;

static CString ASSET_PATH = "F:/codes/SolarOmen/SolarOmen-2/Assets/Raw/";
int main()
{
	FileProcessor fileProcessor;

	MetaProcessor metaProcessor;
	metaProcessor.LoadAllMetaFiles(fileProcessor.GetFilePaths(ASSET_PATH, "slo"));

	std::vector<CString> modelPaths = fileProcessor.GetFilePaths(ASSET_PATH, "obj");
	std::vector<CString> missingMetaModels = metaProcessor.FindMissing(modelPaths);

	for (const CString& path : missingMetaModels)
	{
		ModelMetaFile metaData = {};
		metaData.scale = 2.0f;
		metaData.layout = VertexShaderLayoutType::Value::PNT;

		CString newPath = Util::StripFileExtension(path).Add(".slo");
		metaProcessor.SaveMetaData(newPath, metaData);
		LOG("Creating meta file for: " << path.GetCStr());
	}

	metaProcessor.LoadAllMetaFiles(fileProcessor.GetFilePaths(ASSET_PATH, "slo"));

	LOG("PROCESSING MODELS");
	ModelProcessor modelProcessor;
	std::vector<Model> models = modelProcessor.LoadModels(modelPaths, metaProcessor);
	LOG("COMPLETE");

	LOG("PROCESSING MATERIALS");
	MaterialProcessor materialProcessor;
	std::vector<Material> materials = materialProcessor.LoadMTLMaterials(fileProcessor.GetFilePaths(ASSET_PATH, "mtl"));
	LOG("COMPLETE");

	for (Model& model : models)
	{
		for (Mesh& mesh : model.meshes)
		{
			for (Material& mat : materials)
			{
				if (mat.asset.name == mesh.materialName)
				{
					for (int32 i = 0; i < mesh.vertices.size(); i++)
					{
						mesh.vertices.at(i).colours = Vec4f(mat.asset.colourKd, 1.0f);
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

	modelProcessor.SaveModels(models);
	materialProcessor.SaveMaterials(materials);

	return 0;
}