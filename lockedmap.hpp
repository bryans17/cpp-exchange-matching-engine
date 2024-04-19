#ifndef LOCKEDMAP_HPP
#define LOCKEDMAP_HPP

#include <unordered_map>
#include <shared_mutex>
template<class Key, class T>
class LockedMap {
  private:
    std::unordered_map<Key, T> lockedmap;
    mutable std::shared_mutex mut;
  public:
    using value_type = std::pair<const Key, T>;
    using iterator = typename std::unordered_map<Key, T>::iterator;

    size_t count(const Key& key) {
      std::shared_lock l{mut};
      return lockedmap.count(key);
    }

    T& operator[](const Key& key) {
      {
        std::shared_lock l{mut};
        if (lockedmap.count(key))
          return lockedmap[key];
      }
      std::lock_guard l{mut};
      return lockedmap[key];
    }
    LockedMap() = default;
    LockedMap& operator=(const LockedMap&) = delete;
    LockedMap(const LockedMap&) = delete;
};

#endif
