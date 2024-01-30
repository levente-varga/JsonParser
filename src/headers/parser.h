#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <string>
#include <fstream>
#include <sstream>
#include "node.h"

namespace Json
{
	class Parser
	{
	private:
		class Exception : public std::exception
		{
		private:
			std::string whatBuffer;

		public:
			Exception(std::string description)
			{
				std::ostringstream oss;
				oss << "[JSON Parser Error] " << description;
				whatBuffer = oss.str();
			}
			const char* what() const noexcept override
			{
				return whatBuffer.c_str();
			}
		};

		enum class State
		{
			Start,
			ObjectOpen,
			ObjectClose,
			ListOpen,
			ListClose,
			Key,
			Value,
			Colon,
			Comma,
			End,
			Undefined
		};

		enum class Hierarchy
		{
			Object,
			List
		};

		std::string getFullStateAsString() const noexcept;
		std::string stateToString(State state) const noexcept;
		bool checkPreviousState(const std::vector<State>& allowedStates) const noexcept;
		void requirePreviousState(const std::vector<State>& allowedStates) const;
		void addChildNode(std::shared_ptr<Node> node);
		bool currentlyInAList() const noexcept;
		bool currentlyInAnObject() const noexcept;
		std::shared_ptr<Node> getParentNode() const noexcept;

		std::vector<std::shared_ptr<Node>> hierarchy;
		State lastState;
		State state;
		std::string lastKey;

	public:
		Parser();

		std::shared_ptr<Json::Node> parse(std::string jsonPath);
	};
}

#endif
