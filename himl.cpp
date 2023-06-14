#include "himl.h"

using namespace himl;

static void makesp(std::string& s, int d) {
    --d;
    if (d < 0) return;
    while (d--) s += "    ";
}

static std::string strTrim(const std::string& str) {
    std::string::size_type pos = str.find_first_not_of(' ');
    if (pos == std::string::npos) {
        return str;
    }
    std::string::size_type pos2 = str.find_last_not_of(' ');
    if (pos2 != std::string::npos)
        return str.substr(pos, pos2 - pos + 1);
    return str.substr(pos);
}

static void strEscape(std::string& json, const char* raw) {
    const char* c = raw;
    if (!c) return;
    while (*c) {
        switch (*c) {
        case '\b': json += ('\\'); json += ('b'); break;
        case '\f': json += ('\\'); json += ('f'); break;
        case '\n': json += ('\\'); json += ('n'); break;
        case '\r': json += ('\\'); json += ('r'); break;
        case '\t': json += ('\\'); json += ('t'); break;
        case '\\': json += ('\\'); json += ('\\'); break;
        case '"':  json += ('\\'); json += ('"'); break;
        default: json += *c;
        }
        ++c;
    }
}

static bool strNeedEscape(const char* raw) {
    const char* c = raw;
    if (!c) return false;
    while (*c) {
        switch (*c) {
        case '\b': return true;
        case '\f': return true;
        case '\n': return true;
        case '\r': return true;
        case '\t': return true;
        case '\\': return true;
        case '"':  return true;
        }
        ++c;
    }
    return false;
}

ObjValue* Value::get_obj() {
    return dynamic_cast<ObjValue*>(this);
}
StrValue* Value::get_str() {
    return dynamic_cast<StrValue*>(this);
}

StrValue::~StrValue(){
}

const char* StrValue::get_text() {
    return text.c_str();
}
int StrValue::get_int() {
    return atoi(text.c_str());
}
double StrValue::get_double() {
    return atof(text.c_str());
}

void StrValue::set_str(const char* value) {
    text = value;
}
void StrValue::set_int(int i) {
    char buf[128];
    snprintf(buf, 128, "%ld", i);
    text = buf;
}
void StrValue::set_double(double v) {
    char buf[128];
    snprintf(buf, 128, "%f", v);
    text = buf;
}

void StrValue::to_string(std::string &json, int level) {
    bool escape = strNeedEscape(text.c_str());
    if (escape) json += ("\"");
    strEscape(json, text.c_str());
    if (escape) json += ("\"");
}

ObjValue::~ObjValue() {
    for (Value* v : children) {
        delete v;
    }
    children.clear();
    for (std::pair<std::string, Value*>& item : attributes) {
        delete item.second;
    }
    attributes.clear();
}

int ObjValue::child_count() {
    return children.size();
}

void ObjValue::add(Value* v) {
    children.push_back(v);
}

void ObjValue::set(const char* name, Value* p) {
    Value* it = get_by(name);
    if (it) {
        delete it;
    }
    attributes.push_back(std::pair<std::string, Value*>(name, p));
}

Value* ObjValue::get_at(int i) {
    return children.at(i);
}
Value* ObjValue::get_by(const char* name) {
    for (std::pair<std::string, Value*>& item : attributes) {
        if (item.first == name) {
            return item.second;
        }
    }
    return NULL;
}

void ObjValue::to_string(std::string &json, int level) {
    if (level > 0 || name.size() > 0) {
        if (name.size() > 0) {
            json += name + " ";
        }
        json += ("{\n");
    }
    for (int i = 0, n = (int)attributes.size(); i < n; i++) {
        std::pair<std::string, Value*>& item = attributes[i];
        makesp(json, level + 1);
        json += item.first+" = ";
        item.second->to_string(json, level + 1);
        json += ("\n");
    }

    bool sameLine = false;
    for (int i = 0, n = (int)children.size(); i < n; i++) {
        if (children[i]->get_obj() && sameLine) {
            json += ("\n");
            makesp(json, level + 1);
        }
        else if (!sameLine) {
            makesp(json, level + 1);
        }
        children[i]->to_string(json, level + 1);
        if (children[i]->get_obj()) {
            json += ("\n");
            sameLine = false;
        }
        else {
            if (i == n-1) json += ("\n");
            else {
                json += (",");
                sameLine = true;
            }
        }
    }
    if (level > 0 || name.size() > 0) {
        makesp(json, level);
        json += ("}");
    }
}




ObjValue* HimlParser::parseObj(bool isRoot) {
    ObjValue* obj = new ObjValue();
    skipWhitespace();
    if (!isRoot) expect((int)JsonToken::objectStart);
    while (true)
    {
        skipWhitespace();
        if (maybe((int)JsonToken::objectEnd)) return obj;
        parsePair(obj);
        maybe((int)JsonToken::comma);
        if (cur < 0) break;
        if (error.size() > 0) break;
    }
    return obj;
}

void HimlParser::parsePair(ObjValue* obj) {
    skipComment();

    std::string str;
    parseStr(str);

    skipWhitespace();

    if (maybe((int)'=')) {
        skipWhitespace();

        Value *val = parseVal();
        skipWhitespace();
        obj->set(str.c_str(), val);
    }
    else if (cur == (int)JsonToken::objectStart) {
        ObjValue* aobj = parseObj();
        aobj->name.swap(str);
        obj->add(aobj);
    }
    else {
        maybe(',');
        StrValue* value = new StrValue();
        value->text.swap(str);
        obj->add(value);
    }
}

Value* HimlParser::parseVal() {
    skipComment();

    if (cur == (int)JsonToken::objectStart) {
        return parseObj();
    }
    else {
        std::string str;
        parseStr(str);
        skipWhitespace();
        if (cur == (int)JsonToken::objectStart) {
            ObjValue* obj = parseObj();
            obj->name.swap(str);
            return obj;
        }
        else {
            StrValue* value = new StrValue();
            value->text.swap(str);
            return value;
        }
    }
}

void HimlParser::parseStr(std::string &str) {
    bool hasQuote = maybe((char)JsonToken::quote);
    while (true) {
        if (hasQuote && cur != (int)JsonToken::quote) break;
        if (!hasQuote && !isalpha(cur) && !isdigit(cur) && cur != '_' && cur < 128) break;
        if (cur < 0) {
            break;
        }
        if (cur == '\\') {
            str += escape();
        }
        else {
            str += (cur);
            consume();
        }
    }

    if (hasQuote) {
        expect((int)JsonToken::quote);
    }
    else {
        strTrim(str);
    }
}

std::string HimlParser::escape()
{
    // consume slash
    expect('\\');
    std::string str;

    // check basics
    switch (cur) {
    case 'b':   consume(); str += '\b'; break;
    case 'f':   consume(); str += '\f'; break;
    case 'n':   consume(); str += '\n'; break;
    case 'r':   consume(); str += '\r'; break;
    case 't':   consume(); str += '\t'; break;
    case '"':   consume(); str += '"'; break;
    case '\\':  consume(); str += '\\'; break;
    case '/':   consume(); str += '/'; break;
    }
    return str;
}
