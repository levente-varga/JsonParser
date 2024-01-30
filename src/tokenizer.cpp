#include "headers/tokenizer.h"

Json::Tokenizer::Tokenizer(std::string fileName)
{
	if (file.is_open())
	{
		file.close();
	}

	file.open(fileName);

	if (!file.good())
	{
		throw std::logic_error("[JSON Tokenizer Error] Failed to open JSON file: \"" + fileName + "\"");
	}
}

Json::Tokenizer::~Tokenizer()
{
	file.close();
}

std::vector<Json::Token> Json::Tokenizer::tokenize()
{
	std::vector<Json::Token> tokens;

	file.seekg(0);

	while (hasMoreTokens())
	{
		Token token = getToken();
		tokens.push_back(token);
	}

	return tokens;
}

/**
 * Reads from Tokenizer::file until a character inside characters is read
 *
 * This function expects that the input pointer is set to be directly
 * in front of the string value, after the starting double quote.
 *
 * @param characters a string containing characters that when read
 * stop the reading of the file (i.e. '"' for json strings).
 * @param inclusive whether the input pointer should stop before
 * (exclusive) or after (inclusive) the closing character
 * @returns the string value as an std::string
 * @throws a logic error if the file cannot be read
 */
std::string Json::Tokenizer::readUntil(std::string characters, bool inclusive)
{
	std::string result = "";
	char c = file.get();

	while (characters.find(c) == std::string::npos)
	{
		result += c;

		if (!file.good())
		{
			throw Exception("Function readUntil() could not find closing character(s) while reading the json file");
		}

		c = file.get();
	}

	if (!inclusive)
	{
		rollBackCharacter();
	}

	return result;
}

std::string Json::Tokenizer::readWhile(std::string characters, bool inclusive)
{
	std::string negatedCharacters = "";
	for (unsigned int i = 0; i <= 255; i++)
	{
		char c = i;
		if (characters.find(c) == std::string::npos)
		{
			negatedCharacters += c;
		}
	}
	return readUntil(negatedCharacters, !inclusive);
}

char Json::Tokenizer::getNextNonWhiteSpaceCharacter()
{
	char c = ' ';

	while (c == ' ' || c == '\n' || c == '\t')
	{
		c = file.get();

		if (file.eof())
		{
			return -1;
		}
		if (!file.good())
		{
			throw Exception("Can't read more characters from file!");
		}
	}

	return c;
}

bool Json::Tokenizer::hasMoreTokens() noexcept
{
	return !file.eof() && !file.fail();
}

void Json::Tokenizer::rollBackToken()
{
	if (file.eof())
	{
		file.clear();
	}
	file.seekg(previousReaderPosition);
}

void Json::Tokenizer::rollBackCharacter()
{
	if (file.eof())
	{
		file.clear();
	}
	file.unget();
}

void Json::Tokenizer::moveReader(int distance)
{
	file.seekg(distance, file.cur);
}

bool Json::Tokenizer::checkNextNCharacters(unsigned int n, std::string expected)
{
	if (expected.size() < n)
		return false;

	for (unsigned int i = 0; i < n; i++)
	{
		if (file.get() != expected[i])
			return false;
	}

	return true;
}

/**
 * Returns the next token in the given json file
 *
 * @returns the next token as a Token
 * @throws a logic error if the file cannot be read
 */
Json::Token Json::Tokenizer::getToken()
{
	if (file.eof())
	{
		throw Exception("Ran out of tokens!");
	}

	previousReaderPosition = file.tellg();
	char c = getNextNonWhiteSpaceCharacter();
	Token token;

	if (('0' <= c && c <= '9') || c == '-' || c == '.')
	{
		moveReader(-1);
		token.type = Token::Type::Number;
		token.value = readUntil(",]}", false); // readWhile("0123456789.-eE", false);

		double number;

		try
		{
			number = std::stod(token.value);
		}
		catch (...)
		{
			throw Exception("Could not convert \"" + token.value + "\" to a number");
		}
	}
	else if (c == '"')
	{
		token.type = Token::Type::String;
		token.value = readUntil("\"", true);
	}
	else if (c == 't')
	{
		moveReader(-1);
		token.type = Token::Type::Boolean;
		token.value = readUntil(",]}", false);
		if (token.value != "true")
		{
			throw Exception("Misspelled Boolean Token value: found \"" + token.value + "\" instead of \"true\"");
		}
	}
	else if (c == 'f')
	{
		moveReader(-1);
		token.type = Token::Type::Boolean;
		token.value = readUntil(",]}", false);
		if (token.value != "false")
		{
			throw Exception("Misspelled Boolean Token value: found \"" + token.value + "\" instead of \"false\"");
		}
	}
	else if (c == 'n')
	{
		moveReader(-1);
		token.type = Token::Type::Null;
		token.value = readUntil(",]}", false);
		if (token.value != "null")
		{
			throw Exception("Misspelled Null Token value: found \"" + token.value + "\" instead of \"null\"");
		}
	}
	else if (c == '{')
	{
		token.type = Token::Type::ObjectOpen;
	}
	else if (c == '}')
	{
		token.type = Token::Type::ObjectClose;
	}
	else if (c == '[')
	{
		token.type = Token::Type::ListOpen;
	}
	else if (c == ']')
	{
		token.type = Token::Type::ListClose;
	}
	else if (c == ',')
	{
		token.type = Token::Type::Comma;
	}
	else if (c == ':')
	{
		token.type = Token::Type::Colon;
	}
	else if (c == -1)
	{
		token.type = Token::Type::End;
	}
	else
	{
		throw Exception("Could not parse token starting with \"" + std::string(1, c) + "\"");
	}

	return token;
}

Json::Token::Type Json::Token::getType() const noexcept
{
	return type;
}

std::string Json::Token::getValue() const noexcept
{
	return value;
}

std::string Json::Token::toString() const noexcept
{
	switch (type)
	{
		case Token::Type::Null:
			return "Null";
		case Token::Type::Boolean:
			return "Boolean: " + value;
		case Token::Type::Number:
			return "Number: " + value;
		case Token::Type::String:
			return "String: \"" + value + "\"";
		case Token::Type::ListOpen:
			return "ListOpen";
		case Token::Type::ListClose:
			return "ListClose";
		case Token::Type::ObjectOpen:
			return "ObjectOpen";
		case Token::Type::ObjectClose:
			return "ObjectClose";
		case Token::Type::Comma:
			return "Comma";
		case Token::Type::Colon:
			return "Colon";
		case Token::Type::End:
			return "End";
		default:
			return "Undefined";
	}
}