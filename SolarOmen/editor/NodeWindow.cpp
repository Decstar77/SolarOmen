#include "NodeWindow.h"


namespace cm
{
	void NodeWindow::Initialize()
	{
		nodes = std::vector<Node>();
		links = std::vector<Link>();

		idCounter = 0;
		{
			Node& node = nodes.emplace_back();
			node.name = "Ouput node";
			node.pos = Vec2f(0, 0);
			node.id = GetNextId();

			Pin inputPin = {};
			inputPin.id = GetNextId();
			inputPin.node = node.id;

			node.outputPins.push_back(inputPin);
		}

		{
			Node& node = nodes.emplace_back();
			node.name = "Input node";
			node.pos = Vec2f(0, 0);
			node.id = GetNextId();


			Pin inputPin = {};
			inputPin.id = GetNextId();
			inputPin.node = node.id;

			node.inputPins.push_back(inputPin);
		}

		{
			Node& node = nodes.emplace_back();
			node.name = "Input node";
			node.pos = Vec2f(0, 0);
			node.id = GetNextId();


			Pin inputPin = {};
			inputPin.id = GetNextId();
			inputPin.node = node.id;

			node.inputPins.push_back(inputPin);
		}
	}

	void NodeWindow::Show(Input* input)
	{
		ImGui::Begin("Node editor");

		ImNodes::BeginNodeEditor();
#if 1
		for (Node& node : nodes)
		{
			ImNodes::BeginNode(node.id);

			ImNodes::BeginNodeTitleBar();
			ImGui::TextUnformatted(node.name.GetCStr());
			ImNodes::EndNodeTitleBar();

			for (Pin& pin : node.outputPins)
			{
				ImNodes::BeginOutputAttribute(pin.id);
				ImGui::Text("output pin");
				ImNodes::EndOutputAttribute();
			}

			for (Pin& pin : node.inputPins)
			{
				ImNodes::BeginInputAttribute(pin.id);
				ImGui::Text("input pin");
				ImNodes::EndInputAttribute();
			}

			ImNodes::EndNode();
		}


		for (uint32 i = 0; i < links.size(); ++i)
		{
			Link& link = links.at(i);
			ImNodes::Link(i, link.outputPin, link.inputPin);
		}

		ImNodes::MiniMap();
		ImNodes::EndNodeEditor();

		Link createdLink = {};
		if (ImNodes::IsLinkCreated(&createdLink.outputPin, &createdLink.inputPin))
		{
			//LOG("Input" << createdLink.inputPin);
			//LOG("Output" << createdLink.outputPin);

			for (Node& node : nodes)
			{
				for (Pin& pin : node.inputPins)
				{
					if (pin.id == createdLink.inputPin)
					{
						createdLink.inputNode = node.id;
						//LOG("Input Node" << createdLink.inputNode);
						break;
					}
				}

				for (Pin& pin : node.outputPins)
				{
					if (pin.id == createdLink.outputPin)
					{
						createdLink.outputNode = node.id;
						//LOG("Output Node" << createdLink.outputNode);
						break;
					}
				}
			}

			links.push_back(createdLink);
		}

		int32 destroyedLink = -1;
		if (ImNodes::IsLinkDestroyed(&destroyedLink))
		{
			links.erase(links.begin() + destroyedLink);
		}

		int32 droppedPin = -1;
		if (ImNodes::IsLinkDropped(&droppedPin))
		{
			LOG("Pin" << droppedPin);

			int32 index = -1;
			for (int32 i = 0; i < links.size(); i++)
			{
				Link& link = links.at(i);
				if (link.inputPin == droppedPin)
				{
					index = i;
					break;
				}
			}

			if (index >= 0)
			{
				LOG(index);
				links.erase(links.begin() + index);
			}
		}

		int32 hoveredLink = -1;
		if (ImNodes::IsLinkHovered(&hoveredLink))
		{
			if (input->alt && IsKeyJustDown(input, mb1))
			{
				links.erase(links.begin() + hoveredLink);
			}
		}
#else
		idCounter = 0;
		ImNodes::BeginNode(GetNextId());

		ImNodes::BeginNodeTitleBar();
		ImGui::TextUnformatted("output node");
		ImNodes::EndNodeTitleBar();


		ImNodes::BeginOutputAttribute(GetNextId());
		// in between Begin|EndAttribute calls, you can call ImGui
		// UI functions
		ImGui::Text("output pin");
		ImNodes::EndOutputAttribute();

		ImNodes::EndNode();



		ImNodes::BeginNode(GetNextId());

		ImNodes::BeginNodeTitleBar();
		ImGui::TextUnformatted("input node");
		ImNodes::EndNodeTitleBar();


		ImNodes::BeginInputAttribute(GetNextId());
		// in between Begin|EndAttribute calls, you can call ImGui
		// UI functions
		ImGui::Text("input pin");
		ImNodes::EndInputAttribute();

		ImNodes::EndNode();

		for (int i = 0; i < links.size(); ++i)
		{
			const std::pair<int, int> p = links[i];
			// in this case, we just use the array index of the link
			// as the unique identifier
			ImNodes::Link(i, p.first, p.second);
		}

		int start_attr, end_attr;
		if (ImNodes::IsLinkCreated(&start_attr, &end_attr))
		{
			links.push_back(std::make_pair(start_attr, end_attr));
		}
#endif





		ImGui::End();
	}

	int32 NodeWindow::GetNextId()
	{
		int32 id = idCounter;
		idCounter++;
		return id;
	}

}