namespace mu {

    template<typename T>
    inline std::vector<uint8_t> Mesh::getRawVertexData(const std::vector<T> &vertices) {
        const uint8_t *data = reinterpret_cast<const uint8_t *>(vertices.data());
        return std::vector<uint8_t>(data, data + vertices.size() * sizeof(T));
    }

}
