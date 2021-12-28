#include "Core.h"
#include "FileProcessor.h"
#include "MeshProcessor.h"

using namespace cm;

static CString ASSET_PATH = "F:/codes/SolarOmen/SolarOmen-2/Assets/Raw/";
int main()
{
	FileProcessor fileProcessor;

	std::vector<CString> modelPaths = fileProcessor.GetFilePaths(ASSET_PATH, "obj");
	MetaFileProcessor metaProcessor = MetaFileProcessor(fileProcessor.GetFilePaths(ASSET_PATH, "slo"));

	CString modelPath = modelPaths[0];
	Model m(modelPath);

	ModelProcessor modelProcessor;
	modelProcessor.LoadModels(modelPaths, metaProcessor);

	return 0;
}