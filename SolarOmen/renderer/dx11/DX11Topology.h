#pragma once

namespace cm
{
	// @TODO: Strings 
	class Topology
	{
	public:
		enum class Value
		{
			TRIANGLE_LIST,
			LINE_LIST,
		};

		Topology(Value value)
		{
			this->value = value;
		}

		inline D3D_PRIMITIVE_TOPOLOGY GetDXFormat() const
		{
			switch (value)
			{
			case Value::TRIANGLE_LIST: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			case Value::LINE_LIST: return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
			}

			return {};
		}

	private:
		Value value;
	};

}
