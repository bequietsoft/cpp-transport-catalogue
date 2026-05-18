#include "json_builder.h"

namespace json {

    // Контекстные методы

    DictContext RootContext::StartDict() { return builder_.StartDict(); }
    ArrayContext RootContext::StartArray() { return builder_.StartArray(); }
    ValueContext RootContext::Value(Node::Value value) { return builder_.Value(value); }
    Document RootContext::Build() { return builder_.Build(); }

    KeyContext DictContext::Key(std::string key) { return builder_.Key(key); }
    Builder& DictContext::EndDict() { return builder_.EndDict(); }

    ArrayValueContext ArrayContext::Value(Node::Value value) { 
        builder_.Value(value); return ArrayValueContext(builder_); }
    DictContext ArrayContext::StartDict() { return builder_.StartDict(); }
    ArrayContext ArrayContext::StartArray() { return builder_.StartArray(); }
    Builder& ArrayContext::EndArray() { return builder_.EndArray(); }

    DictValueContext KeyContext::Value(Node::Value value) { 
        builder_.Value(value); return DictValueContext(builder_); }
    DictContext KeyContext::StartDict() { return builder_.StartDict(); }
    ArrayContext KeyContext::StartArray() { return builder_.StartArray(); }

    KeyContext DictValueContext::Key(std::string key) { return builder_.Key(key); }
    Builder& DictValueContext::EndDict() { return builder_.EndDict(); }

    ArrayValueContext ArrayValueContext::Value(Node::Value value) { 
        builder_.Value(value); return ArrayValueContext(builder_); }
    DictContext ArrayValueContext::StartDict() { return builder_.StartDict(); }
    ArrayContext ArrayValueContext::StartArray() { return builder_.StartArray(); }
    Builder& ArrayValueContext::EndArray() { return builder_.EndArray(); }

    Document ValueContext::Build() { return builder_.Build(); }
    Builder& ValueContext::EndArray() { return builder_.EndArray(); }


    // Методы основного класса

    Document Builder::Build() {
        if (nodes_stack_.size() != 1) 
            Error("Build: Error JSON");
        
        is_built_ = true;
        return Document(std::move(*nodes_stack_[0]));
    }

    KeyContext Builder::Key(std::string key) {
        if (is_built_) 
            Error("Key: Build already called");

        if (!expect_key_) 
            Error("Key: Key is not expected at this position");

        current_key_ = std::move(key);
        expect_key_ = false;
        expect_value_ = true;
        
        return KeyContext(*this);
    }

    ValueContext Builder::Value(Node::Value value) {
        if (is_built_) 
            Error("Value: Build already called");

        if (!expect_value_) 
            Error("Value: Value is not expected at this position");

        // Извлекаем значение из variant
        Node* node_ptr = std::visit([](auto&& val) -> Node* {
            return new Node(std::forward<decltype(val)>(val));
        }, value);

        AddNode(node_ptr);
        expect_key_ = InDictContext();
        expect_value_ = InArrayContext(); 

        return ValueContext(*this);
    }

    Builder& Builder::StartContainer(Node* container_ptr) {
        if (is_built_) 
            Error("StartContext: Build already called");

        if (InDictContext() && expect_key_) 
            Error("StartContext: Key is expected at this position");

        Node* context_ptr = AddNode(container_ptr);
        if (context_ptr) 
            nodes_stack_.emplace_back(context_ptr);
        expect_key_ = true;
        expect_value_ = InArrayContext(); 

        return *this;
    }

    Builder& Builder::EndContainer() {
        if (is_built_) 
            Error("EndContainer: Build already called");

        if (nodes_stack_.size() > 1) 
            nodes_stack_.pop_back();

        expect_key_ = InDictContext();
        expect_value_ = InArrayContext(); 

        return *this;
    }

    DictContext Builder::StartDict() {
        return DictContext(StartContainer(new Node(Dict{})));
    }

    Builder& Builder::EndDict() {
        if (!InDictContext()) 
            Error("EndDict: EndDict in wrong context");

        return EndContainer();
    }

    ArrayContext Builder::StartArray() {
        return ArrayContext(StartContainer(new Node(Array{})));
    }

    Builder& Builder::EndArray() {
        if (!InArrayContext()) 
            Error("EndArray: EndArray in wrong context");

        return EndContainer();
    }

    // Добавляет узел или контейнер в текущий контекст и возвращает текущий контекст
    Node* Builder::AddNode(Node* node_ptr) {

        // Корневой узел или контейнер
        if (nodes_stack_.empty()) {
            nodes_stack_.emplace_back(node_ptr);
            return nullptr;
        }
        
        Node* context_ptr = nodes_stack_.back();
        
        // Массив
        if (context_ptr->IsArray()) {
            Array& array = const_cast<Array&>(context_ptr->AsArray());
            array.emplace_back(std::move(*node_ptr));
            if (array.back().IsArray() || array.back().IsDict()) {
                context_ptr = &array.back();
            }
        } 
        else
        // Словарь
        if (context_ptr->IsDict()) {
            Dict& dict = const_cast<Dict&>(context_ptr->AsDict());
            auto [it, inserted] = dict.try_emplace(std::move(current_key_), std::move(*node_ptr));
            if (!inserted) 
                Error("AddNode: Insertion error in dictionary");

            if (it->second.IsArray() || it->second.IsDict()) {
                context_ptr = &it->second;
            }
        } 
        else {
        // Контекст не определен
            Error("AddNode: Undefined context");
        }

        return context_ptr;
    }

    bool Builder::InDictContext() const {
        if (nodes_stack_.empty()) return false;
        return nodes_stack_.back()->IsDict();
    }

    bool Builder::InArrayContext() const {
        if (nodes_stack_.empty()) return false;
        return nodes_stack_.back()->IsArray();
    }

    void Builder::Error(const std::string& msg) const {
        throw std::logic_error(msg);
    }

} // end of namespace json
