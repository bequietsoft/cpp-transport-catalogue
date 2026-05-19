#pragma once

#include "json.h"

#include <vector>
#include <string>

namespace json {

    // Преварительные определения:
    class BaseContext;
    class DictKeyContext;
    class DictContext;
    class ArrayContext;


    // Основной класс
    class Builder {
    public:
        DictKeyContext Key(std::string key);
        BaseContext Value(Node::Value value);
        DictContext StartDict();
        BaseContext EndDict();
        ArrayContext StartArray();
        BaseContext EndArray();
        Document Build();

    private:
        std::string current_key_;
        std::vector<Node*> nodes_stack_;

        bool expect_key_ {false};
        bool expect_value_ {true};
        bool is_built_ {false};

        Builder& StartContainer(Node* container_ptr);
        Builder& EndContainer();

        Node* AddNode(Node* node);

        bool InDictContext() const;
        bool InArrayContext() const;

        void Error(const std::string& msg) const;
    };


    // Контексты:

    // Контекст общего случая 
    // разрешены все методы построения: Key, Value, StartDict/StartArray, EndDict/EndArray, Build 
    class BaseContext {
    public:
        BaseContext(Builder& builder) : builder_(builder) {}    
        DictKeyContext Key(std::string key);
        BaseContext Value(Node::Value value);
        DictContext StartDict();
        BaseContext EndDict();
        ArrayContext StartArray();
        BaseContext EndArray();
        Document Build();
    
    protected:
        Builder& builder_; 
    };

    // Контекст после Key в словаре 
    // разрешены: Value(переопределен), StartDict/StartArray
    // запрещены: Key, EndDict/EndArray, Build
    class DictKeyContext : public BaseContext {
    public:
        DictKeyContext(BaseContext context) : BaseContext(context) {}
        DictContext Value(Node::Value value);
        DictKeyContext Key(std::string key) = delete;
        BaseContext EndDict() = delete;
        BaseContext EndArray() = delete;
        Document Build() = delete;
    };

    // Контекст после Value() в словаре
    // разрешены: Key, EndDict
    // запрещены: Value, StartDict/StartArray, EndArray, Build)
    class DictContext : public BaseContext {
    public:
        DictContext(BaseContext context) : BaseContext(context) {}
        BaseContext Value(Node::Value value) = delete;
        DictContext StartDict() = delete;
        ArrayContext StartArray() = delete;
        BaseContext EndArray() = delete;
        Document Build() = delete;
    };

    // Контекст после Value в массиве
    // разрешены: Value(переопределен), StartDict/StartArray, EndArray
    // запрещены: Key, EndDict, Build)
    class ArrayContext : public BaseContext {
    public:
        ArrayContext(BaseContext context) : BaseContext(context) {}
        ArrayContext Value(Node::Value value);
        DictKeyContext Key(std::string key) = delete;
        BaseContext EndDict() = delete;
        Document Build() = delete;
    };

} // end of namespace json
