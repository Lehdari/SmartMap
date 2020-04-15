//
// Project: SmartMap
// File: SmartMap.cpp
//
// Copyright (c) 2020 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include "SmartMap.hpp"


// Initializations for SmartMap static member variables
SmartMap::TypeId SmartMap::typeIdCounter = 0;


// Member functions of SmartMap
SmartMap::SmartMap(const SmartMap& other) :
    _typeHelpers    (other._typeHelpers)
{
    copyData(&other, this);
}

SmartMap::SmartMap(SmartMap&& other) noexcept :
    _typeHelpers    (std::move(other._typeHelpers))
{
    moveData(&other, this);
}

SmartMap& SmartMap::operator=(const SmartMap& other)
{
    // Delete the previous data
    moveData(this);
    _typeHelpers = other._typeHelpers;
    copyData(&other, this);

    return *this;
}

SmartMap& SmartMap::operator=(SmartMap&& other) noexcept
{
    // Delete the previous data
    moveData(this);
    _typeHelpers = std::move(other._typeHelpers);
    moveData(&other, this);

    return *this;
}

SmartMap::~SmartMap()
{
    moveData(this);
}

SmartMap::TypeHelper::TypeHelper() noexcept :
    pointerMapDataUpdater   (nullptr),
    poolMover               (nullptr),
    pointerPoolMover        (nullptr)
{
}

void SmartMap::moveData(const SmartMap* oldMap, SmartMap* newMap)
{
    for (auto& m : _typeHelpers) {
        if (m.pointerMapDataUpdater == nullptr)
            continue;

        m.pointerMapDataUpdater(oldMap, newMap);
        m.poolMover(oldMap, newMap);
        m.pointerPoolMover(oldMap, newMap);

        for (auto& idMapMover : m.idMapMovers) {
            if (idMapMover != nullptr)
                idMapMover(oldMap, newMap);
        }
    }
}

void SmartMap::copyData(const SmartMap* oldMap, const SmartMap* newMap)
{
    if (newMap == nullptr)
        return;

    for (auto& m : _typeHelpers) {
        if (m.pointerMapDataUpdater == nullptr)
            continue;

        m.poolCopier(oldMap, newMap);
        m.pointerPoolCopier(oldMap, newMap);

        for (auto& idMapCopier : m.idMapCopiers) {
            if (idMapCopier != nullptr)
                idMapCopier(oldMap, newMap);
        }
    }
}
