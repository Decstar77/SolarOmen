#pragma once

#include "core/SolarCore.h"

namespace cm
{
	//namespace ed = ax::NodeEditor;

	//enum class PinKind
	//{
	//	Output,
	//	Input
	//};

	//enum class NodeType
	//{
	//	Blueprint,
	//	Simple,
	//	Tree,
	//	Comment,
	//	Houdini
	//};

	//struct Pin
	//{
	//	ed::PinId   ID;
	//	class Node* Node;
	//	std::string Name;
	//	PinType     Type;
	//	PinKind     Kind;

	//	Pin(int id, const char* name, PinType type) :
	//		ID(id), Node(nullptr), Name(name), Type(type), Kind(PinKind::Input)
	//	{
	//	}
	//};

	//class Node
	//{
	//public:
	//	ed::NodeId ID;
	//	std::string Name;
	//	std::vector<Pin> Inputs;
	//	std::vector<Pin> Outputs;
	//	ImColor Color;
	//	NodeType Type;
	//	ImVec2 Size;

	//	std::string State;
	//	std::string SavedState;

	//	Node(int id, const char* name, ImColor color = ImColor(255, 255, 255)) :
	//		ID(id), Name(name), Color(color), Type(NodeType::Blueprint), Size(0, 0)
	//	{
	//	}


	//};

	enum class PinType
	{
		Flow,
		Bool,
		Int,
		Float,
		String,
		Object,
		Function,
		Delegate,
	};

	struct Link
	{
		int32 inputNode;
		int32 inputPin;
		int32 outputPin;
		int32 outputNode;
	};

	struct Pin
	{
		PinType type;
		// @NOTE: This pin id
		int32 id = -1;
		// @NOTE: The id of the node that this pin is part of
		int32 node = -1;
		// @NOTE: The pin id that this node is connected to
		int32 connected = -1;
	};

	struct Node
	{
		int32 id = -1;
		CString name;
		Vec2f pos;

		std::vector<Pin> outputPins;
		std::vector<Pin> inputPins;
	};


	class NodeWindow
	{
	public:
		int32 idCounter;
		std::vector<Node> nodes;
		std::vector<Link> links;

	public:

		void Initialize();
		void Show(Input* input);

	private:
		void AddInputPinToNode();
		void AddOuputPinToNode();

		int32 GetNextId();

	};
}