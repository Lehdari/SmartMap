//
// Project: SmartMap
// File: SmartMap.hpp
//
// Copyright (c) 2020 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#ifndef SMARTMAP_SMARTMAP_HPP
#define SMARTMAP_SMARTMAP_HPP


#include <vector>
#include <unordered_map>


class SmartMap {
private:
    template <typename T>
    struct ObjectPool;

    // Type alias for indexing ObjectPool vector
    template <typename T>
    using Id = typename std::vector<typename ObjectPool<T>::Wrapper>::size_type;

    // Helper type for pooling objects to avoid unnecessary (de-)allocations
    template <typename T>
    struct ObjectPool {
        struct Wrapper {
            T       o;
            bool    active;

            Wrapper(bool active = false);
        };

        // Return ID of first inactive object, quarantees that object with the returned
        // ID exists after the call. Can set the invalidated flag.
        Id<T> firstInactiveId();

        // Direct object access
        inline T& operator[](Id<T> id) __attribute__((always_inline));

        std::vector<Wrapper>    data;
        bool                    invalidated = false; // true if container pointers and iterators are invalidated
    };


public:
    template <typename T>
    class Pointer {
    public:
        friend class SmartMap;

        Pointer();

        Pointer(const Pointer<T>&);
        Pointer(Pointer<T>&&) noexcept;
        Pointer<T>& operator=(const Pointer<T>&);
        Pointer<T>& operator=(Pointer<T>&&) noexcept;

        ~Pointer();

        T& operator*();

    private:
        Pointer(SmartMap* m, Id<T> objectId, T* objectPtr);
        SmartMap*       _map; // Pointer to parent SmartMap, required for syncing
        Id<T>           _objectId; // ID of the object in the pool obtained from accessPool, required for syncing
        T*              _objectPtr; // Pointer to the object
        Id<Pointer<T>*> _pointerId; // ID of the pointer in the pool obtained from accessPointerPool
    };

    SmartMap() = default;

    SmartMap(const SmartMap&);
    SmartMap(SmartMap&&) noexcept;
    SmartMap& operator=(const SmartMap&);
    SmartMap& operator=(SmartMap&&) noexcept;

    ~SmartMap();

    /// Get a pointer to object of specific type
    /// T: Data type
    /// K: Key type
    template <typename T, typename K>
    Pointer<T> getPointer(const K& key);

    /// Overload for string literal -> std::string mapping
    template <typename T>
    Pointer<T> getPointer(const char* key);

    /// TypeId is used to assign an id for each type stored in SmartMaps
    using TypeId = unsigned;

    /// Get TypeId of a specific type
    template <typename T>
    static TypeId getTypeId();

private:
    // Struct for pointers to functions required when copying or moving a SmartMap
    // object. Since a SmartMap instance does not contain type information, type
    // erasure mechanism provided by this struct is required.
    struct TypeHelper {
        // Pointer to updatePointerMapData
        void (*pointerMapDataUpdater)(const SmartMap* oldMap, SmartMap* newMap);

        // Pointer to movePool
        void (*poolMover)(const SmartMap* oldMap, const SmartMap* newMap);
        // Pointer to movePointerPool
        void (*pointerPoolMover)(const SmartMap* oldMap, const SmartMap* newMap);
        // Pointers to moveIdMaps (required for each key type K)
        std::vector<void(*)(const SmartMap* oldMap, const SmartMap* newMap)> idMapMovers;

        // Pointer to copyPool
        void (*poolCopier)(const SmartMap* oldMap, const SmartMap* newMap);
        // Pointer to copyPointerPool
        void (*pointerPoolCopier)(const SmartMap* oldMap, const SmartMap* newMap);
        // Pointers to copyIdMaps (required for each key type K)
        std::vector<void(*)(const SmartMap* oldMap, const SmartMap* newMap)> idMapCopiers;

        TypeHelper() noexcept;

        // Initialize TypeHelper for specified type
        template <typename T>
        inline void init() noexcept __attribute((always_inline));

        // Add idMapMover for key type K
        // Note: Make sure that object type T matches the other function pointers!
        template <typename T, typename K>
        inline void addIdMapFunctions() __attribute__((always_inline));
    };

    friend struct TypeHelper;

    // All TypeHelper objects required to move the SmartMap instance. Each object is
    // stored at index specified by the respective typeId (see getTypeId).
    std::vector<TypeHelper>   _typeHelpers;

    // Helper for assigning unique TypeId for each type
    static TypeId typeIdCounter;

    // Static variable template storing the actual data and providing mapping
    // from object type and SmartMap pointer to an ObjectPool instance. This enables
    // heterogenous storage through templated interface.
    template <typename T>
    static std::unordered_map<const SmartMap*, ObjectPool<T>> poolMap;

    // Static template variable providing mapping from object type and SmartMap
    // pointer to ObjectPool of pointers of corresponding type. This essentially
    // stores pointers to Pointer objects, which are required for container <->
    // pointer syncing purposes. (See updatePointer*Data functions)
    template <typename T>
    static std::unordered_map<const SmartMap*, ObjectPool<Pointer<T>*>> pointerPoolMap;

    // In similar fashion to template variables above, this function provides mapping
    // from key type and SmartMap pointer to ObjectPool Id:s. This enables usage of
    // any type of key.
    template <typename T, typename K>
    static std::unordered_map<const SmartMap*, std::unordered_map<K, Id<T>>> idMapMap;

    // As SmartMap instances get moved/destructed, the data in *Map static variable
    // templates is required to be reassigned to new key / erased from the maps.
    // This function performs the reassign(use nullprt as newMap for erasure) for
    // poolMap. Pointers to this function are stored in TypeHelper objects.
    template <typename T>
    static void movePool(const SmartMap* oldMap, const SmartMap* newMap);

    // Similar to function above, performs reassign/erase for pointerPoolMap.
    template <typename T>
    static void movePointerPool(const SmartMap* oldMap, const SmartMap* newMap);

    // Similar to function above, performs reassign/erase for idMapMap.
    template <typename T, typename K>
    static void moveIdMap(const SmartMap* oldMap, const SmartMap* newMap);

    // Similar to movePool but performs a copy instead of move.
    template <typename T>
    static void copyPool(const SmartMap* oldMap, const SmartMap* newMap);

    // Similar to movePointerPool but performs a copy instead of move.
    template <typename T>
    static void copyPointerPool(const SmartMap* oldMap, const SmartMap* newMap);

    // Similar to moveIdMap but performs a copy instead of move.
    template <typename T, typename K>
    static void copyIdMap(const SmartMap* oldMap, const SmartMap* newMap);

    // Move data from old map to new map using the _typeHelpers
    void moveData(const SmartMap* oldMap, SmartMap* newMap = nullptr);

    // Copy data from old map to new map using the _typeHelpers
    void copyData(const SmartMap* oldMap, const SmartMap* newMap = nullptr);

    // Inform the SmartMap about construction of a new pointer
    template <typename T>
    Id<Pointer<T>*> registerPointer(Pointer<T>* p);

    // Inform the SmartMap about destruction of a new pointer
    template <typename T>
    void unregisterPointer(Id<Pointer<T>*> pId);

    // Update object data in Pointers, probably due to ObjectPool invalidation
    template <typename T>
    void updatePointerObjectData();

    // Update map data in Pointers, due to SmartMap move or destruction
    template <typename T>
    static void updatePointerMapData(const SmartMap* oldMap, SmartMap* newMap = nullptr);
};


#include "SmartMap.inl"


#endif //SMARTMAP_SMARTMAP_HPP
