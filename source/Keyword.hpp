#ifndef KEYWORD_HPP
#define KEYWORD_HPP

#include <string>
#include <utility>


using namespace std;

enum class TypeType {
    VOID,
    INT,
    SHORT,
    CHAR,
    FLOAT,
    DOUBLE
};

class Type final {
    TypeType key;
public:
    explicit Type(const TypeType key) : key(key) {}

    int size() const {
        switch (key) {
            case TypeType::VOID:   return 0;
            case TypeType::INT:    return 4;
            case TypeType::SHORT:  return 2;
            case TypeType::CHAR:   return 1;
            case TypeType::FLOAT:  return 4;
            case TypeType::DOUBLE: return 8;
            default: throw string("TypeType type error in size()");
        }
    }

    string miType() {
        switch (key) {
            case TypeType::INT:    return "W";
            case TypeType::SHORT:  return "H";
            case TypeType::CHAR:   return "B";
            case TypeType::FLOAT:  return "F";
            case TypeType::DOUBLE: return "D";
            default: return "";
        }
    }
};


static Type convertStringToType(const string& name) {
    if (name == "void")  return Type(TypeType::VOID);
    if (name == "int")   return Type(TypeType::INT);
    if (name == "short") return Type(TypeType::SHORT);
    if (name == "char")  return Type(TypeType::CHAR);
    if (name == "float") return Type(TypeType::FLOAT);
    if (name == "double") return Type(TypeType::DOUBLE);
}

#endif