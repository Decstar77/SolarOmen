#include "NodeWindow.h"

#include "../vendor/imgui_node/imgui_node_editor.h"

namespace cm
{
	namespace ed = ax::NodeEditor;
	static ed::EditorContext* context = nullptr;

	void NodeWindow::Initialize()
	{
		context = ed::CreateEditor();
	}

	void NodeWindow::Shutdown()
	{
		ed::DestroyEditor(context);
	}

	void NodeWindow::Show()
	{
		ed::SetCurrentEditor(context);

		ImGui::Begin("Brain");

		ed::Begin("My Editor");

		int32 uniqueId = 1;

		// Start drawing nodes.
		ed::BeginNode(uniqueId++);
		ImGui::Text("Node A");
		ed::BeginPin(uniqueId++, ed::PinKind::Input);
		ImGui::Text("-> In");
		ed::EndPin();
		ImGui::SameLine();
		ed::BeginPin(uniqueId++, ed::PinKind::Output);
		ImGui::Text("Out ->");
		ed::EndPin();
		ed::EndNode();

		ed::End();

		ImGui::End();
	}
}