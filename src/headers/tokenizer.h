#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <exception>

namespace Json
{
	class Token
	{
		friend class Tokenizer;

	public:
		enum class Type
		{
			Null,        // null
			Boolean,     // true
			Number,      // 1
			String,      // "text"
			ListOpen,    // [
			ListClose,   // ]
			ObjectOpen,  // {
			ObjectClose, // }
			Comma,       // ,
			Colon,       // :
			End,         // EOF
		};

		Type getType() const noexcept;
		std::string getValue() const noexcept;
		std::string toString() const noexcept;

	private:
		std::string value;
		Type type;
	};

	class Tokenizer
	{
	private:
		bool checkNextNCharacters(unsigned int n, std::string expected);
		void moveReader(int distance);

		class Exception : public std::exception
		{
		private:
			std::string whatBuffer;

		public:
			Exception(std::string description)
			{
				std::ostringstream oss;
				oss << "[JSON Tokenizer Error] " << description;
				whatBuffer = oss.str();
			}
			const char* what() const noexcept override
			{
				return whatBuffer.c_str();
			}
		};

	public:
		unsigned int previousReaderPosition;
		std::fstream file;

		char getNextNonWhiteSpaceCharacter();
		void rollBackToken();
		void rollBackCharacter();
		Token getToken();
		bool hasMoreTokens() noexcept;
		std::string readUntil(std::string characters, bool inclusive);
		std::string readWhile(std::string characters, bool inclusive);

		Tokenizer(std::string fileName);
		~Tokenizer();
		std::vector<Token> tokenize();
	};
}

#endif