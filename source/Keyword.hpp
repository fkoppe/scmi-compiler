#ifndef KEYWORD_HPP
#define KEYWORD_HPP

#include <string>

using namespace std;

enum class Keyword {
    VOID,
    INT,
    SHORT,
    CHAR,
    FLOAT,
    DOUBLE
};

class Type final {
    Keyword key;
public:
    explicit Type(const Keyword key) : key(key) {}

    int size() const {
        switch (key) {
            case Keyword::VOID:   return 0;
            case Keyword::INT:    return 4;
            case Keyword::SHORT:  return 2;
            case Keyword::CHAR:   return 1;
            case Keyword::FLOAT:  return 4;
            case Keyword::DOUBLE: return 8;
            default: throw runtime_error("Keyword type error in size()");
        }
    }

    string miType() {
        switch (key) {
            case Keyword::INT:    return "W";
            case Keyword::SHORT:  return "H";
            case Keyword::CHAR:   return "B";
            case Keyword::FLOAT:  return "F";
            case Keyword::DOUBLE: return "D";
            default: throw runtime_error("no miType error");
        }
    }
};


static Type convertStringToType(const string& name) {
    if (name == "void")  return Type(Keyword::VOID);
    if (name == "int")   return Type(Keyword::INT);
    if (name == "short") return Type(Keyword::SHORT);
    if (name == "char")  return Type(Keyword::CHAR);
    if (name == "float") return Type(Keyword::FLOAT);
    if (name == "double") return Type(Keyword::DOUBLE);
    throw runtime_error(name + " is not a valid type name");
}

#endif