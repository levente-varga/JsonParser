/**
 * A JSON parser that supports all can parse all .json files that
 * follow the RFC4627 standard
 */

#include "headers/parser.h"
#include "headers/tokenizer.h"
#include <cmath>

Json::Parser::Parser()
{
	state = State::Undefined;
	lastState = State::Undefined;
}

bool Json::Parser::checkPreviousState(const std::vector<State>& allowedStates) const noexcept
{
	for (auto state : allowedStates)
	{
		if (lastState == state)
			return true;
	}
	return false;
}

void Json::Parser::requirePreviousState(const std::vector<State>& allowedStates) const
{
	if (!checkPreviousState(allowedStates))
	{
		throw Exception("Entered an illegal state. Parser's state:\n" + getFullStateAsString());
	}
}

std::string Json::Parser::getFullStateAsString() const noexcept
{
	std::string result = "last: ";
	result += stateToString(lastState);
	result += " \tnow: ";
	result += stateToString(state);
	result += " \tpar: ";
	result += getParentNode()->getTypeAsString();

	result += " \ttree: [";

	for (unsigned int i = 0; i < hierarchy.size(); i++)
	{
		result += hierarchy[i]->getTypeAsString();
		if (i + 1 != hierarchy.size())
			result += " > ";
	}

	result += "]";

	return result;
}

std::string Json::Parser::stateToString(State state) const noexcept
{
	switch (state)
	{
		case State::Value:
			return "Value      ";
		case State::Key:
			return "Key        ";
		case State::ListOpen:
			return "ListOpen   ";
		case State::ListClose:
			return "ListClose  ";
		case State::ObjectOpen:
			return "ObjectOpen ";
		case State::ObjectClose:
			return "ObjectClose";
		case State::Comma:
			return "Comma      ";
		case State::Colon:
			return "Colon      ";
		case State::End:
			return "End        ";
		case State::Start:
			return "Start      ";
		case State::Undefined:
		default:
			return "Undefined  ";
	}
}

std::shared_ptr<Json::Node> Json::Parser::parse(std::string jsonPath)
{
	Tokenizer tokenizer = Tokenizer(jsonPath);
	auto tokens = tokenizer.tokenize();

	hierarchy = { std::make_shared<Node>() };
	lastState = State::Undefined;
	state = State::Start;
	auto root = std::make_shared<Node>();

	for (Token token : tokens)
	{
		lastState = state;

		switch (token.getType())
		{
			case Token::Type::ObjectOpen:
			{
				state = State::ObjectOpen;
				requirePreviousState({ State::Start, State::Colon });

				if (lastState == State::Start)
				{
					root = std::make_shared<Node>(Object());
					hierarchy.push_back(root);
				}
				else
				{
					auto newNode = std::make_shared<Node>(Object());
					addChildNode(newNode);
					hierarchy.push_back(newNode);
				}
				break;
			}
			case Token::Type::ObjectClose:
			{
				state = State::ObjectClose;
				requirePreviousState({ State::Value, State::ObjectOpen, State::ObjectClose, State::ListClose });

				if (currentlyInAList())
					throw Exception("Found wrong closing bracket (object instead of list)");
				hierarchy.pop_back();
				break;
			}
			case Token::Type::ListOpen:
			{
				state = State::ListOpen;
				requirePreviousState({ State::Start, State::Colon });

				if (lastState == State::Start)
				{
					root = std::make_shared<Node>(List());
					hierarchy.push_back(root);
				}
				else
				{
					auto newNode = std::make_shared<Node>(List());
					addChildNode(newNode);
					hierarchy.push_back(newNode);
				}
				break;
			}
			case Token::Type::ListClose:
			{
				state = State::ListClose;
				requirePreviousState({ State::Value, State::ListOpen, State::ListClose, State::ObjectClose });

				if (currentlyInAnObject())
					throw Exception("Found wrong closing bracket (list instead of object)");
				hierarchy.pop_back();
				break;
			}
			case Token::Type::Comma:
			{
				state = State::Comma;
				requirePreviousState({ State::Value, State::ListClose, State::ObjectClose });
				break;
			}
			case Token::Type::Colon:
			{
				state = State::Colon;
				requirePreviousState({ State::Key });
				break;
			}
			case Token::Type::Boolean:
			{
				state = State::Value;
				requirePreviousState({ State::Colon, State::Comma, State::ListOpen });

				if (token.getValue() == "true")
				{
					addChildNode(std::make_shared<Node>(true));
				}
				else
				{
					addChildNode(std::make_shared<Node>(false));
				}
				break;
			}
			case Token::Type::Number:
			{
				state = State::Value;
				requirePreviousState({ State::Colon, State::Comma, State::ListOpen });

				double number = std::stod(token.getValue());
				double intPart;
				if (std::modf(number, &intPart) == 0.0)
				{
					addChildNode(std::make_shared<Node>((int)number));
				}
				else
				{
					addChildNode(std::make_shared<Node>(number));
				}
				break;
			}
			case Token::Type::String:
			{
				if (checkPreviousState({ State::ObjectOpen, State::Comma }) && currentlyInAnObject())
				{
					state = State::Key;
					lastKey = token.getValue();
				}
				else if ((checkPreviousState({ State::ListOpen, State::Comma }) && currentlyInAList()) || (checkPreviousState({ State::Colon }) && currentlyInAnObject()))
				{
					state = State::Value;
					addChildNode(std::make_shared<Node>(token.getValue()));
				}
				else
				{
					throw Exception("Wrong order of tokens");
				}
				requirePreviousState({ State::ObjectOpen, State::ListOpen, State::Colon, State::Comma });
				break;
			}
			case Token::Type::Null:
			{
				state = State::Value;
				requirePreviousState({ State::Colon, State::Comma, State::ListOpen });

				addChildNode(std::make_shared<Node>(nullptr));
				break;
			}
			case Token::Type::End:
			{
				state = State::End;
				requirePreviousState({ State::Start, State::ListClose, State::ObjectClose });
				break;
			}
		}
	}

	return root;
}

void Json::Parser::addChildNode(std::shared_ptr<Node> node)
{
	if (currentlyInAList())
	{
		getParentNode()->addChild(node);
	}
	else if (currentlyInAnObject())
	{
		getParentNode()->addChild({ lastKey, node });
	}
	else
		throw Exception("Hierarchy contains illegal node type (" + getParentNode()->getTypeAsString() + ")");
}

bool Json::Parser::currentlyInAList() const noexcept
{
	if (hierarchy.empty())
		return false;
	return hierarchy.back()->getType() == Node::Type::List;
}

bool Json::Parser::currentlyInAnObject() const noexcept
{
	if (hierarchy.empty())
		return false;
	return hierarchy.back()->getType() == Node::Type::Object;
}

std::shared_ptr<Json::Node> Json::Parser::getParentNode() const noexcept
{
	if (hierarchy.empty())
		return nullptr;
	return hierarchy.back();
}