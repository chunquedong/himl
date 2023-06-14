#ifndef HIML_H
#define HIML_H

#include <vector>
#include <utility>
#include <string>

namespace himl {
    struct ObjValue;
    struct StrValue;

    struct Value {
        ObjValue* get_obj();
        StrValue* get_str();

        virtual ~Value() {}

        virtual void to_string(std::string &json, int level = 0) = 0;
    };

    struct StrValue : public Value {
        std::string text;

        ~StrValue();

        const char* get_text();
        int get_int();
        double get_double();

        void set_str(const char* value);
        void set_int(int i);
        void set_double(double v);

        virtual void to_string(std::string &json, int level = 0);
    };

    struct ObjValue: public Value {
        std::vector<Value*> children;
        std::vector<std::pair<std::string, Value*> > attributes;
        std::string name;

        ~ObjValue();

        int child_count();
        void add(Value* v);
        void set(const char* name, Value* p);
        Value* get_at(int i);
        Value* get_by(const char* name);

        virtual void to_string(std::string &json, int level = 0);
    };

    enum class JsonToken {
        objectStart = '{',
        objectEnd = '}',
        comma = ',',
        quote = '"',
    };

    class HimlParser {
        std::string str;
        std::string &input;

        int cur;
        int pos;
        std::string error;
    public:
        HimlParser() : input(str), cur(-1), pos(-1) {}

        Value* parse(std::string &str) {
            input = str;
            pos = -1;
            cur = -1;
            consume();
            skipWhitespace();
            skipComment();
            return parseObj(true);
        }

        const std::string& getError() { return error; }

    private:
        ObjValue* parseObj(bool isRoot=false);
        void parsePair(ObjValue* obj);
        Value* parseVal();
        void parseStr(std::string &str);
        std::string escape();
    private:

        void consume(int i = 1) {
            pos += i;
            if (pos < input.size()) {
                cur = (unsigned char)input[pos];
            }
            else {
                cur = -1;
            }
        }

        bool maybe(int tt) {
            if (cur != tt) return false;
            consume();
            return true;
        }

        void expect(int tt) {
            if (cur < 0) {
                error = ("Unexpected end of stream");
                return;
            }
            else if (cur != tt) {
                char buf[128];
                snprintf(buf, 128, "'%c' Expected, got '%c' at %d", tt, cur, pos);
                error = buf;
                return;
            }
            consume();
        }

        void skipWhitespace() {
            if (cur < 0) return;
            while (isspace(cur))
                consume();
        }

        void skipComment() {
            if (cur == '/') {
                if (maybe('/')) {
                    while (cur != '\r' && cur != '\n' && cur > 0)
                        consume();
                    skipWhitespace();
                }
            }
        }
    };
}

#endif