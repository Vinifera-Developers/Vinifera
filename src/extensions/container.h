/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          CONTAINER.H
 *
 *  @author        CCHyper
 *
 *  @brief         Wrapper to unordered map for use with extended classes.
 *
 *  @license       Vinifera is free software: you can redistribute it and/or
 *                 modify it under the terms of the GNU General Public License
 *                 as published by the Free Software Foundation, either version
 *                 3 of the License, or (at your option) any later version.
 *
 *                 Vinifera is distributed in the hope that it will be
 *                 useful, but WITHOUT ANY WARRANTY; without even the implied
 *                 warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *                 PURPOSE. See the GNU General Public License for more details.
 *
 *                 You should have received a copy of the GNU General Public
 *                 License along with this program.
 *                 If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/
#pragma once

#include "always.h"
#include <unordered_map>
#include <typeinfo>

#include "debughandler.h"
#include "asserthandler.h"


/**
 *  Templated wrapper class around std::unordered_map.
 */
template<typename Key, typename Value>
class ContainerMap
{
    public:
        using map_type = std::unordered_map<const Key *, Value *>;
        using iterator = typename map_type::iterator;
        using const_iterator = typename map_type::const_iterator;

    public:
        ContainerMap() : Items() {}
        ~ContainerMap() {}
        
        /**
         *  Finds element with specific key.
         */
        Value * find(const Key * key) const
        {
            const auto it = Items.find(key);
            if(it != Items.end()) {
                return it->second;
            }
            return nullptr;
        }
        
        /**
         *  Insert a new element instance into the map.
         */
        Value * insert(const Key * key, Value * value)
        {
            Items.emplace(key, value);
            return value;
        }
        
        /**
         *  Remove an element from the map.
         */
        Value * remove(const Key * key)
        {
            const auto it = Items.find(key);
            if(it != Items.cend()) {
                Value * value = it->second;
                Items.erase(it);
                return value;
            }
            return nullptr;
        }
        
        /**
         *  Checks whether the container is empty.
         */
        bool empty() const
        {
            return size() == 0;
        }
        
        /**
         *  Clears the contents of the map.
         */
        void clear()
        {
            Items.clear();
        }
        
        /**
         *  Returns the number of elements in the map.
         */
        size_t size() const
        {
            return Items.size();
        }
        
        /**
         *  Returns an iterator to the beginning.
         */
        iterator begin() const
        {
            auto item = Items.begin();
            return reinterpret_cast<iterator &>(item);
        }
        
        /**
         *  Returns an iterator to the end.
         */
        iterator end() const
        {
            auto item = Items.end();
            return reinterpret_cast<iterator &>(item);
        }

    private:
        /**
         *  The unordered map of items.
         */
        map_type Items;

    private:
        //ContainerMap() = default;
        ContainerMap(const ContainerMap &) = delete;
        ContainerMap & operator = (const ContainerMap &) = delete;
        ContainerMap & operator = (ContainerMap &&) = delete;
};


/**
 *  Templated wrapper class around ContainerMap that implements
 *  interfaces required by the extended classes for game use.
 * 
 *  This hides the direct access to the map without accessing
 *  it directly, which is not recommended.
 */
template<class Base, class Extension>
class ExtensionMap final
{
    public:
        ExtensionMap() : Map() {}
        ~ExtensionMap() {}

        /**
         *  Find the extension instance for this type.
         */
        Extension * find(const Base * key, bool log_failure = true) const
        {
            if (key == nullptr) {
                DEBUG_ERROR("Attempted to find \"%s\" instance a NULL base pointer!\n", typeid(Extension).name());
                return nullptr;
            }
            const auto ptr = Map.find(key);
            if (!ptr) {
                if (log_failure) {
                    DEBUG_WARNING("Failed to find extension instance \"%s\"!\n", typeid(Extension).name());
                }
                return nullptr;
            }
            //DEBUG_WARNING("Found extension instance \"%s\"!\n", typeid(Extension).name());
            return ptr;
        }

        /**
         *  Find the extension instance for this type or create a default instance.
         */
        Extension * find_or_create(Base * key)
        {
            if (key == nullptr) {
                DEBUG_ERROR("Find or create of \"%s\" instance attempted with a NULL base pointer!\n", typeid(Extension).name());
                return nullptr;
            }

            /**
             *  Find and return the instance in the map, if it exists.
             */
            if (const auto ptr = Map.find(key)) {
                //DEBUG_WARNING("Found extension instance \"%s\"!\n", typeid(Extension).name());
                return ptr;
            }

            /**
             *  Create an instance of the extended class. This expects the extended class
             *  constructor takes a pointer to the base class type!
             */
            //DEBUG_WARNING("Creating new \"%s\" extension instance!\n", typeid(Extension).name());
            auto val = new Extension(key);
            return Map.insert(key, val);
        }

        /**
         *  Create a default instance.
         * 
         *  @warning: Only use this if you really need to!
         */
        Extension * create(Base * key)
        {
            if (key == nullptr) {
                DEBUG_ERROR("Creation of \"%s\" instance attempted with a NULL base pointer!\n", typeid(Extension).name());
                return nullptr;
            }

            /**
             *  Create an instance of the extended class. This expects the extended class
             *  constructor takes a pointer to the base class type!
             */
            //DEBUG_WARNING("Creating new \"%s\" extension instance!\n", typeid(Extension).name());
            auto val = new Extension(key);
            return Map.insert(key, val);
        }
        
        /**
         *  Remove an element from the map.
         */
        void remove(const Base * key)
        {
            delete Map.remove(key);
        }

        /**
         *  Checks whether the container is empty.
         */
        bool empty() const
        {
            return Map.size() == 0;
        }

        /**
         *  Clears the contents of the map.
         */
        void clear()
        {
            if (!empty()) {
                int sz = Map.size();
                Map.clear();
                DEBUG_WARNING("Cleared %u items from \"%s\".\n", sz, typeid(Extension).name());
            }
        }

    public:
        /**
         *  The container map of items.
         */
        ContainerMap<Base, Extension> Map;

    private:
        ExtensionMap(const ExtensionMap &) = delete;
        ExtensionMap & operator = (const ExtensionMap &) = delete;
        ExtensionMap & operator = (ExtensionMap &&) = delete;
};
