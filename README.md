# JsonParser for C++

This project is a full-fledged JSON parser that is capable of parsing any ```.json``` file following the ```RFC4627``` standard into an easily traversable C++ data structure.

## How to use

1. Parse a JSON file.

```C++
auto parser = new Json::Parser();
auto json = parser.parse("path_to_json");
```

2. Traverse the returned JSON structure

```C++
auto node = json->at("users")->at(0)->at("age");
```

3. Convert node to value

```C++
int age = node->getAs<int>();
```

4. Iterate through a list or an object

```C++
for (auto user : json->at("users"))
{
	std::string userName = user->at("name")->getAs<std::string>();
}
```

## Notes

- The ```getAs<T>()``` method only accepts types that can be stored in a JSON node, such types are: ```bool```, ```int```, ```double```, ```std::string```, ```std::nullptr_t```, ```Json::List``` and ```Json::Object```.
- ```Json::List``` and ```Json::Object``` hide an ```std::vector``` and an ```std::map``` of ```Json::Node``` shared pointers respectively.
- Trying to perform an unsupported conversion using the ```getAs<T>()``` function (i.e the user tries to convert a string node to ```int```) throws an exception.
- Trying to use the ```at(int)``` function on a non-list node, as well as trying to use the ```at(std::string)``` function on a non-object node throws an expression.
- The parser will throw an exception if the JSON object found in the provided .json file contains serious formatting errors.