//
// Project: SmartMap
// File: SmartMap.inl
//
// Copyright (c) 2020 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

// Definitions of SmartMap static member variable templates
template <typename T>
std::unordered_map<const SmartMap*, SmartMap::ObjectPool<T>> SmartMap::poolMap;

template <typename T>
std::unordered_map<const SmartMap*, SmartMap::ObjectPool<SmartMap::Pointer<T>*>> SmartMap::pointerPoolMap;

template <typename T, typename K>
std::unordered_map<const SmartMap*, std::unordered_map<K, SmartMap::Id<T>>> SmartMap::idMapMap;


template <typename T>
SmartMap::ObjectPool<T>::Wrapper::Wrapper(bool active) :
    active  (active)
{
}

template <typename T>
SmartMap::Id<T> SmartMap::ObjectPool<T>::firstInactiveId()
{
    // TODO implement caching for the first inactive object
    // Using naive linear search for now
    for (Id<T> i=0; i<data.size(); ++i) {
        if (!data[i].active) {
            // Activate the object and return its ID
            data[i].active = true;
            return i;
        }
    }

    data.emplace_back(true);
    invalidated = true;
    return data.size()-1;
}

template <typename T>
T& SmartMap::ObjectPool<T>::operator[](Id<T> id)
{
    return data[id].o;
}

template <typename T>
SmartMap::Pointer<T>::Pointer() :
    _map        (nullptr),
    _objectId   (0),
    _objectPtr  (nullptr),
    _pointerId  (0)
{
}

template <typename T>
SmartMap::Pointer<T>::Pointer(const SmartMap::Pointer<T>& other) :
    _map        (other._map),
    _objectId   (other._objectId),
    _objectPtr  (other._objectPtr),
    _pointerId  (_map != nullptr ? _map->registerPointer(this) : 0)
{
}

template <typename T>
SmartMap::Pointer<T>::Pointer(SmartMap::Pointer<T>&& other) noexcept :
    _map        (other._map),
    _objectId   (other._objectId),
    _objectPtr  (other._objectPtr),
    _pointerId  (_map != nullptr ? _map->registerPointer(this) : 0)
{
    // Unregister the other pointer and set it to moved-from state
    if (other._map != nullptr)
        other._map->unregisterPointer<T>(other._pointerId);
    other._map = nullptr;
    other._objectPtr = nullptr;
}

template <typename T>
SmartMap::Pointer<T>& SmartMap::Pointer<T>::operator=(const SmartMap::Pointer<T>& other)
{
    // Reregister the pointer in case the other pointer uses different SmartMap instance
    if (_map != other._map && _map != nullptr) {
        _map->unregisterPointer<T>(_pointerId);
        _map = other._map;
        _pointerId = _map->registerPointer<T>(this);
    }

    _objectId = other._objectId;
    _objectPtr = other._objectPtr;

    return *this;
}

template <typename T>
SmartMap::Pointer<T>& SmartMap::Pointer<T>::operator=(SmartMap::Pointer<T>&& other) noexcept
{
    // Reregister the pointer in case the other pointer uses different SmartMap instance
    if (_map != other._map && _map != nullptr) {
        _map->unregisterPointer<T>(_pointerId);
        _map = other._map;
        _pointerId = _map->registerPointer<T>(this);
    }

    _objectId = other._objectId;
    _objectPtr = other._objectPtr;

    // Unregister the other pointer and set it to moved-from state
    if (other._map != nullptr)
        other._map->unregisterPointer<T>(other._pointerId);
    other._map = nullptr;
    other._objectPtr = nullptr;

    return *this;
}

template <typename T>
SmartMap::Pointer<T>::~Pointer()
{
    if (_map != nullptr)
        _map->unregisterPointer<T>(_pointerId);
}

template <typename T>
T& SmartMap::Pointer<T>::operator*()
{
    return *_objectPtr;
}

template <typename T>
SmartMap::Pointer<T>::Pointer(SmartMap* m, Id<T> objectId, T *objectPtr) :
    _map        (m),
    _objectId   (objectId),
    _objectPtr  (objectPtr),
    _pointerId  (m->registerPointer(this))
{
}

template <typename T, typename K>
typename SmartMap::Pointer<T> SmartMap::getPointer(const K& key)
{
    static const auto typeId = getTypeId<T>(); // object type id

    // Access the id map and object pool designated to this SmartMap object
    auto& idMap = idMapMap<T, K>[this];
    auto& pool = poolMap<T>[this];

    // If the key doesn't exist, create new key -> id mapping
    if (idMap.find(key) == idMap.end()) {
        // Resize the _typeHelpers vector if necessary (every entry stored to index specified by type id)
        if (_typeHelpers.size() <= typeId)
            _typeHelpers.resize(typeId+1);

        // Add the TypeHelper for the type if it is uninitialized
        if (_typeHelpers[typeId].pointerMapDataUpdater == nullptr)
            _typeHelpers[typeId].template init<T>();

        // Add idMapMover for the key type
        _typeHelpers[typeId].template addIdMapFunctions<T, K>();

        auto newId = pool.firstInactiveId();
        // The pool might have invalidated all pointers and references, forcing a Pointer update
        if (pool.invalidated)
            updatePointerObjectData<T>();

        idMap[key] = newId;
        return Pointer<T>(this, newId, &(pool[newId]));
    }

    // Return pointer for existing key
    auto id = idMap.at(key);
    return Pointer<T>(this, id, &(pool[id]));
}

template <typename T>
typename SmartMap::Pointer<T> SmartMap::getPointer(const char* key)
{
    return getPointer<T, std::string>(key);
}

template <typename T>
SmartMap::TypeId SmartMap::getTypeId()
{
    static TypeId typeIdCounter = 0;
    return typeIdCounter++;
}

template <typename T>
void SmartMap::TypeHelper::init() noexcept
{
    pointerMapDataUpdater = &updatePointerMapData<T>;
    poolMover = &movePool<T>;
    pointerPoolMover = &movePointerPool<T>;
    poolCopier = &copyPool<T>;
    pointerPoolCopier = &copyPointerPool<T>;
}

template <typename T, typename K>
void SmartMap::TypeHelper::addIdMapFunctions()
{
    static const auto typeId = getTypeId<K>(); // key type id

    // Resize the idMapMovers vector if necessary
    if (idMapMovers.size() <= typeId)
        idMapMovers.resize(typeId+1, nullptr);

    // Add the idMapMover for the type if it doesn't exist
    if (idMapMovers[typeId] == nullptr)
        idMapMovers[typeId] = &SmartMap::moveIdMap<T, K>;

    // Resize the idMapCopiers vector if necessary
    if (idMapCopiers.size() <= typeId)
        idMapCopiers.resize(typeId+1, nullptr);

    // Add the idMapCopier for the type if it doesn't exist
    if (idMapCopiers[typeId] == nullptr)
        idMapCopiers[typeId] = &SmartMap::copyIdMap<T, K>;
}

template <typename T>
void SmartMap::movePool(const SmartMap* oldMap, const SmartMap* newMap)
{
    if (newMap == nullptr) {
        poolMap<T>.erase(oldMap);
    }
    else {
        auto poolNode = poolMap<T>.extract(oldMap);
        poolNode.key() = newMap;
        poolMap<T>.insert(std::move(poolNode));
    }
}

template <typename T>
void SmartMap::movePointerPool(const SmartMap* oldMap, const SmartMap* newMap)
{
    if (newMap == nullptr) {
        pointerPoolMap<T>.erase(oldMap);
    }
    else {
        auto pointerPoolNode = pointerPoolMap<T>.extract(oldMap);
        pointerPoolNode.key() = newMap;
        pointerPoolMap<T>.insert(std::move(pointerPoolNode));
    }
}

template <typename T, typename K>
void SmartMap::moveIdMap(const SmartMap* oldMap, const SmartMap* newMap)
{
    if (newMap == nullptr) {
        idMapMap<T,K>.erase(oldMap);
    }
    else {
        auto idMapNode = idMapMap<T,K>.extract(oldMap);
        idMapNode.key() = newMap;
        idMapMap<T,K>.insert(std::move(idMapNode));
    }
}

template<typename T>
void SmartMap::copyPool(const SmartMap* oldMap, const SmartMap* newMap)
{
    poolMap<T>[newMap] = poolMap<T>[oldMap];
}

template<typename T>
void SmartMap::copyPointerPool(const SmartMap* oldMap, const SmartMap* newMap)
{
    pointerPoolMap<T>[newMap] = pointerPoolMap<T>[oldMap];
}

template<typename T, typename K>
void SmartMap::copyIdMap(const SmartMap* oldMap, const SmartMap* newMap)
{
    idMapMap<T,K>[newMap] = idMapMap<T,K>[oldMap];
}

template <typename T>
SmartMap::Id<SmartMap::Pointer<T>*>
SmartMap::registerPointer(SmartMap::Pointer<T>* p)
{
    auto& pointerPool = pointerPoolMap<T>[this];

    // Add the new pointer to the pool
    Id<Pointer<T>*> id = pointerPool.firstInactiveId();
    pointerPool[id] = p;

    return id;
}

template <typename T>
void SmartMap::unregisterPointer(SmartMap::Id<SmartMap::Pointer<T>*> pId)
{
    pointerPoolMap<T>[this].data[pId].active = false;
}

template <typename T>
void SmartMap::updatePointerObjectData()
{
    auto& pool = poolMap<T>[this];

    // Fetch new addresses of the objects and update the Pointers
    for (auto& p : pointerPoolMap<T>[this].data)
        if (p.active)
            p.o->_objectPtr = &pool[p.o->_objectId];

    pool.invalidated = false;
}

template <typename T>
void SmartMap::updatePointerMapData(const SmartMap* oldMap, SmartMap* newMap)
{
    // Update the _map pointers of the Pointers
    for (auto& p : pointerPoolMap<T>[oldMap].data)
        if (p.active)
            p.o->_map = newMap;
}
