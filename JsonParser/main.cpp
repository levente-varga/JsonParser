#include <iostream>
#include "headers/parser.h"

int main(int argc, char* argv[])
{
	// Instantiate a parser:
	Json::Parser parser = Json::Parser();


	// Parse a JSON file:
	auto json = parser.parse("example.json");


	// Print the JSON object's content to the console:
	std::cout << json->toString() << std::endl;


	// Navigate through the JSON hierarchy:
	auto node = json->at("Image");


	// Convert nodes to C++ types:
	bool isAnimated = json->at("Image")->at("Animated")->getAs<bool>();
	std::cout << "Is the image animated: ";
	std::cout << (isAnimated ? "true" : "false") << std::endl; // false


	// Access a specific element in a list:
	int thirdID = json->at("Image")->at("IDs")->at(2)->getAs<int>();
	std::cout << "The third ID in the list: ";
	std::cout << thirdID << std::endl; // 234


	// Iterate through the elements of a list:
	auto IDs = json->at("Image")->at("IDs")->getAs<Json::List>();
	for (auto element : IDs)
	{
		int currentID = element->getAs<int>();

		/* ... */
	}


	// Iterate through the children nodes of an object:
	auto thumbnail = json->at("Image")->at("Thumbnail")->getAs<Json::Object>();
	for (auto node : thumbnail)
	{
		/* ... */
	}
}