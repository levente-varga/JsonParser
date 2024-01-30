#include <stdexcept>
#include "headers/node.h"

Json::Node::Node()
{
	type = Json::Node::Type::Root;
}

Json::Node::Node(Value value) : value(value)
{
	type = Type::Root;

	if (std::holds_alternative<bool>(value))
		type = Type::Boolean;
	if (std::holds_alternative<int>(value))
		type = Type::Number;
	if (std::holds_alternative<double>(value))
		type = Type::Number;
	if (std::holds_alternative<std::nullptr_t>(value))
		type = Type::Null;
	if (std::holds_alternative<std::string>(value))
		type = Type::String;
	if (std::holds_alternative<List>(value))
		type = Type::List;
	if (std::holds_alternative<Object>(value))
		type = Type::Object;
}

std::shared_ptr<Json::Node> Json::Node::at(unsigned index)
{
	return (*this)[index];
}

std::shared_ptr<Json::Node> Json::Node::at(std::string key)
{
	return (*this)[key.c_str()];
}

std::shared_ptr<Json::Node> Json::Node::operator[](const unsigned int index)
{
	if (std::holds_alternative<List>(value))
	{
		auto list = std::get<List>(value);
		if (index < list.size())
		{
			return list[index];
		}
		else
		{
			throw Exception("List type JSON node indexed out of range (index: " + std::to_string(index) + ", size: " + std::to_string(list.size()) + ")");
		}
	}
	else throw Exception("Requested vector-like indexing on " + getTypeAsString() + " type JSON node");
}

std::shared_ptr<Json::Node> Json::Node::operator[](const char* key)
{
	if (std::holds_alternative<Object>(value))
	{
		auto map = std::get<Object>(value);
		auto pos = map.find(key);
		if (pos != map.end())
		{
			return pos->second;
		}
		else
		{
			throw Exception("Key \"" + std::string(key) + "\" does not exists in indexed JSON object");
		}
	}
	else throw Exception("Requested map-like indexing on " + getTypeAsString() + " type JSON node");
}

Json::Node::operator bool() const
{
	if (std::holds_alternative<bool>(value))
	{
		return std::get<bool>(value);
	}
	else throw Exception("Cannot convert " + getTypeAsString() + " node to bool");
}

Json::Node::operator int() const
{
	if (std::holds_alternative<int>(value))
	{
		return std::get<int>(value);
	}
	else if (std::holds_alternative<double>(value))
	{
		return (int)std::get<double>(value);
	}
	else throw Exception("Cannot convert " + getTypeAsString() + " node to int");
}

Json::Node::operator double() const
{
	if (std::holds_alternative<double>(value))
	{
		return std::get<double>(value);
	}
	else if (std::holds_alternative<int>(value))
	{
		return (double)std::get<int>(value);
	}
	else throw Exception("Cannot convert " + getTypeAsString() + " node to double");
}

Json::Node::operator std::string() const
{
	if (std::holds_alternative<std::string>(value))
	{
		return std::get<std::string>(value);
	}
	else throw Exception("Cannot convert " + getTypeAsString() + " node to std::string");
}

Json::Node::operator Json::List() const
{
	if (std::holds_alternative<List>(value))
	{
		return std::get<List>(value);
	}
	else throw Exception("Cannot convert " + getTypeAsString() + " node to std::vector");
}

Json::Node::operator Json::Object() const
{
	if (std::holds_alternative<Object>(value))
	{
		return std::get<Object>(value);
	}
	else throw Exception("Cannot convert " + getTypeAsString() + " node to std::map");
}

void Json::Node::addChild(std::pair<std::string, std::shared_ptr<Node>> child)
{
	if (std::holds_alternative<Object>(value))
	{
		Object& map = std::get<Object>(value);
		map.insert(child);
	}
	else throw Exception("Requested map-like insert on " + getTypeAsString() + " type node");
}

void Json::Node::addChild(std::shared_ptr<Node> child)
{
	if (std::holds_alternative<List>(value))
	{
		List& list = std::get<List>(value);
		list.push_back(child);
	}
	else throw Exception("Requested list-like insert on " + getTypeAsString() + " type node");
}

Json::Value Json::Node::getRawValue() const noexcept
{
	return value;
}

std::string Json::Node::getName() const noexcept
{
	return name;
}

Json::Node::Type Json::Node::getType() const noexcept
{
	return type;
}

std::string Json::Node::getTypeAsString() const noexcept
{
	if (type == Type::Root)
		return "Root";
	if (std::holds_alternative<bool>(value))
		return "Boolean";
	if (std::holds_alternative<int>(value))
		return "Number (int)";
	if (std::holds_alternative<double>(value))
		return "Number (double)";
	if (std::holds_alternative<std::nullptr_t>(value))
		return "Null";
	if (std::holds_alternative<std::string>(value))
		return "String";
	if (std::holds_alternative<List>(value))
		return "List";
	if (std::holds_alternative<Object>(value))
		return "Object";
	return "Undefined";
}

std::string Json::Node::toString(unsigned indentation) const
{
	std::string tabs;
	std::string output = "";

	for (unsigned i = 0; i < indentation; i++)
		tabs += "  ";

	switch (type)
	{
		case Type::String:
		{
			output += "\"" + std::get<std::string>(value) + "\"";
			break;
		}
		case Type::Number:
		{
			if (std::holds_alternative<double>(value))
			{
				output += std::to_string(std::get<double>(value));
			}
			else if (std::holds_alternative<int>(value))
			{
				output += std::to_string(std::get<int>(value));
			}
			break;
		}
		case Type::Boolean:
		{
			output += (std::get<bool>(value) ? "true" : "false");
			break;
		}
		case Type::Null:
		{
			output += "null";
			break;
		}
		case Type::List:
		{
			output += "[\n";
			unsigned int index = 0;
			for (auto node : std::get<List>(value))
			{
				output += tabs + "  " + node->toString(indentation + 1);
				if (index < std::get<List>(value).size() - 1)
				{
					output += ",\n";
				}
				index++;
			}
			output += "\n" + tabs + "]";
			break;
		}
		case Type::Object:
		{
			output += "{\n";
			for (auto iter = std::get<Object>(value).begin(); iter != std::get<Object>(value).end(); iter++)
			{
				output += tabs + "  " + "\"" + iter->first + "\": ";
				output += iter->second->toString(indentation + 1);
				auto next = iter;
				if ((++next) != std::get<Object>(value).end())
				{
					output += ",";
				}
				output += "\n";
			}
			output += tabs + "}";
			break;
		}
	}
	return output;
}