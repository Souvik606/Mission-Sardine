#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <algorithm>

template <typename T>
class CowVector {
public:
    std::shared_ptr<std::vector<T>> data;

    CowVector() : data(std::make_shared<std::vector<T>>()) {}
    explicit CowVector(std::vector<T> v) : data(std::make_shared<std::vector<T>>(std::move(v))) {}
    CowVector(const CowVector& other) : data(other.data) {}
    CowVector(CowVector&& other) noexcept : data(std::move(other.data)) {}

    CowVector& operator=(const CowVector& other) {
        if (this != &other) {
            data = other.data;
        }
        return *this;
    }
    CowVector& operator=(CowVector&& other) noexcept {
        if (this != &other) {
            data = std::move(other.data);
        }
        return *this;
    }

    void detach() {
        if (data.use_count() > 1) {
            data = std::make_shared<std::vector<T>>(*data);
        }
    }

    operator const std::vector<T>&() const { return *data; }

    [[nodiscard]] size_t size() const { return data->size(); }
    [[nodiscard]] bool empty() const { return data->empty(); }
    void reserve(size_t n) { detach(); data->reserve(n); }
    void resize(size_t n) { detach(); data->resize(n); }

    const T& operator[](size_t index) const { return (*data)[index]; }
    const T& at(size_t index) const { return data->at(index); }
    const T& back() const { return data->back(); }
    const T& front() const { return data->front(); }

    auto begin() const { return data->begin(); }
    auto end() const { return data->end(); }
    auto rbegin() const { return data->rbegin(); }
    auto rend() const { return data->rend(); }

    T& operator[](size_t index) { detach(); return (*data)[index]; }
    T& at(size_t index) { detach(); return data->at(index); }
    T& back() { detach(); return data->back(); }
    T& front() { detach(); return data->front(); }

    auto begin() { detach(); return data->begin(); }
    auto end() { detach(); return data->end(); }

    void push_back(const T& val) { detach(); data->push_back(val); }
    void push_back(T&& val) { detach(); data->push_back(std::move(val)); }
    void pop_back() { detach(); data->pop_back(); }
    void clear() { detach(); data->clear(); }

    template <typename... Args>
    auto insert(Args&&... args) {
        detach();
        return data->insert(std::forward<Args>(args)...);
    }

    template <typename... Args>
    auto erase(Args&&... args) {
        detach();
        return data->erase(std::forward<Args>(args)...);
    }
};

template <typename K, typename V>
class CowMap {
public:
    std::shared_ptr<std::unordered_map<K, V>> data;

    CowMap() : data(std::make_shared<std::unordered_map<K, V>>()) {}
    explicit CowMap(std::unordered_map<K, V> m) : data(std::make_shared<std::unordered_map<K, V>>(std::move(m))) {}
    CowMap(const CowMap& other) : data(other.data) {}
    CowMap(CowMap&& other) noexcept : data(std::move(other.data)) {}

    CowMap& operator=(const CowMap& other) {
        if (this != &other) data = other.data;
        return *this;
    }
    CowMap& operator=(CowMap&& other) noexcept {
        if (this != &other) data = std::move(other.data);
        return *this;
    }

    void detach() {
        if (data.use_count() > 1) {
            data = std::make_shared<std::unordered_map<K, V>>(*data);
        }
    }

    operator const std::unordered_map<K, V>&() const { return *data; }

    [[nodiscard]] size_t size() const { return data->size(); }
    [[nodiscard]] bool empty() const { return data->empty(); }

    auto find(const K& key) const { return data->find(key); }
    const V& at(const K& key) const { return data->at(key); }
    size_t count(const K& key) const { return data->count(key); }

    auto begin() const { return data->begin(); }
    auto end() const { return data->end(); }

    V& operator[](const K& key) { detach(); return (*data)[key]; }
    V& at(const K& key) { detach(); return data->at(key); }

    auto find(const K& key) { detach(); return data->find(key); }

    auto begin() { detach(); return data->begin(); }
    auto end() { detach(); return data->end(); }

    template <typename... Args>
    auto erase(Args&&... args) {
        detach();
        return data->erase(std::forward<Args>(args)...);
    }
    void clear() { detach(); data->clear(); }
};
