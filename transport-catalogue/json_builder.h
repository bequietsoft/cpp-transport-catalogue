#pragma once

#include "json.h"

#include <vector>
#include <string>

namespace json {

class Builder;

    // Предварительные объявления классов
    class Builder;
    class RootContext;
    class DictContext;
    class ArrayContext;
    class KeyContext;
    class DictValueContext;
    class ArrayValueContext;
    class ValueContext;

    // Контексты
    class ContextBase {
    protected:
        explicit ContextBase(Builder& builder) : builder_(builder) {}
        Builder& builder_;
    };

    class RootContext : public ContextBase {
    public:
        explicit RootContext(Builder& builder) : ContextBase(builder) {}
        DictContext StartDict();
        ArrayContext StartArray();
        ValueContext Value(Node::Value value);
        Document Build();
    };

    class DictContext : public ContextBase {
    public:
        explicit DictContext(Builder& builder) : ContextBase(builder) {}
        KeyContext Key(std::string key);
        Builder& EndDict();
    };

    class ArrayContext : public ContextBase {
    public:
        explicit ArrayContext(Builder& builder) : ContextBase(builder) {}
        ArrayValueContext Value(Node::Value value);
        DictContext StartDict();
        ArrayContext StartArray();
        Builder& EndArray();
    };

    class KeyContext : public ContextBase {
    public:
        explicit KeyContext(Builder& builder) : ContextBase(builder) {}
        DictValueContext Value(Node::Value value);
        DictContext StartDict();
        ArrayContext StartArray();
    };

    class DictValueContext : public ContextBase {
    public:
        explicit DictValueContext(Builder& builder) : ContextBase(builder) {}
        KeyContext Key(std::string key);
        Builder& EndDict();
    };

    class ArrayValueContext : public ContextBase {
    public:
        explicit ArrayValueContext(Builder& builder) : ContextBase(builder) {}
        ArrayValueContext Value(Node::Value value);
        DictContext StartDict();
        ArrayContext StartArray();
        Builder& EndArray();
    };

    class ValueContext : public ContextBase {
    public:
        explicit ValueContext(Builder& builder) : ContextBase(builder) {}
        Builder& EndArray();
        Document Build();
    };

    // Основной класс
    class Builder {
    public:
        Document Build();
        KeyContext Key(std::string key);
        ValueContext Value(Node::Value value);
        DictContext StartDict();
        Builder& EndDict();
        ArrayContext StartArray();
        Builder& EndArray();

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

} // end of namespace json
