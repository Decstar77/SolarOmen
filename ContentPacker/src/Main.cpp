#include "Core.h"
#include "FileProcessor.h"
#include "MeshProcessor.h"
#include "MaterialProcessor.h"

using namespace cm;

static CString ASSET_PATH = "F:/codes/SolarOmen/SolarOmen-2/Assets/Raw/";
int main()
{
	FileProcessor fileProcessor;

	std::vector<CString> modelPaths = fileProcessor.GetFilePaths(ASSET_PATH, "obj");
	MetaFileProcessor metaProcessor = MetaFileProcessor(fileProcessor.GetFilePaths(ASSET_PATH, "slo"));

	LOG("PROCESSING MODELS");
	ModelProcessor modelProcessor;
	std::vector<Model> models = modelProcessor.LoadModels(modelPaths, metaProcessor);
	LOG("COMPLETE");

	LOG("PROCESSING MATERIALS");
	MaterialProcessor materialProcessor;
	std::vector<Material> materials = materialProcessor.LoadMTLMaterials(fileProcessor.GetFilePaths(ASSET_PATH, "mtl"));
	LOG("COMPLETE");



	modelProcessor.SaveModels(models);
	materialProcessor.SaveMaterials(materials);

	return 0;
}