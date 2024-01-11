# JsonParser for C++

This project is a full-fledged JSON parser that is capable of parsing any .json file following the ```RFC4627``` standard.

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
auto age = node->getAs<double>();
```