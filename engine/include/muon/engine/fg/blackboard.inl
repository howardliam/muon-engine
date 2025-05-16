namespace muon::engine::fg {

    template<typename T, typename ...Args>
    T &Blackboard::add(Args &&...args) {
        return storage[typeid(T)].emplace<T>(T{std::forward<Args>(args)...});
    }

    template<typename T>
    const T &Blackboard::get() const {
        return std::any_cast<const T &>(storage.at(typeid(T)));
    }

}
