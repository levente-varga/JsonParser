#ifndef JSON_NODE_H
#define JSON_NODE_H

#include <string>
#include <sstream>
#include <variant>
#include <vector>
#include <map>
#include <memory>
#include <exception>

namespace Json
{
	class Node;

	using List = std::vector<std::shared_ptr<Node>>;
	using Object = std::map<std::string, std::shared_ptr<Node>>;
	using Value = std::variant<
		bool,           // Boolean
		int,            // Number
		double,         // Number
		std::nullptr_t, // Null
		std::string,    // String
		List,           // List
		Object          // Object
	>;

	class Node
	{
		friend class Parser;

	public:
		enum class Type
		{
			Null,
			Boolean,
			Number,
			String,
			List,
			Object,
			Root
		};

		Node();
		Node(Value value);

		std::shared_ptr<Node> operator[](const unsigned int index);
		std::shared_ptr<Node> operator[](const char* key);
		operator bool() const;
		operator int() const;
		operator double() const;
		operator std::string() const;

		std::shared_ptr<Node> at(unsigned index);
		std::shared_ptr<Node> at(std::string key);

		template <typename T>
		T getAs() { return (*this); }

		std::string getTypeAsString() const noexcept;
		std::string toString(unsigned indentation = 0) const;
		Value getRawValue() const noexcept;
		std::string getName() const noexcept;
		Type getType() const noexcept;

	private:
		class Exception : public std::exception
		{
		private:
			std::string whatBuffer;

		public:
			Exception(std::string description)
			{
				std::ostringstream oss;
				oss << "[JSON Node Error] " << description;
				whatBuffer = oss.str();
			}
			const char* what() const noexcept override
			{
				return whatBuffer.c_str();
			}
		};

		std::string name;
		Type type;
		Value value;

		operator Json::List() const;
		operator Json::Object() const;

		void addChild(std::pair<std::string, std::shared_ptr<Node>> child);
		void addChild(std::shared_ptr<Node> child);
	};
}

#endif