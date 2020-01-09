/**
* @file container/lru_set.h
* @brief lru_set implementation.
*/

#ifndef LIBANT_CONTAINER_LRU_SET_H_
#define LIBANT_CONTAINER_LRU_SET_H_

#include "linked_set.h"

namespace ant {

template<typename _Key, typename _Compare = std::less<_Key>, typename _Alloc = std::allocator<_Key> >
class lru_set {
public:
	using size_type = typename linked_set<_Key, _Compare, _Alloc>::size_type;

public:
	lru_set(size_t maxCachedKeys)
	{
		maxCachedKeys_ = maxCachedKeys;
	}

	/**
	 * @return 返回true表示插入成功，false表示key已存在
	 */
	bool emplace(_Key&& key)
	{
		auto ret = set_.emplace(key);
		if (ret.second) { // 插入成功
			if (set_.size() > maxCachedKeys_) {
				// 缓存个数太多，删除最老不被使用的缓存
				set_.erase(set_.link_begin());
			}
			return true;
		}
		// key已存在，将其移动到最后
		set_.move_to_last(ret.first);
		return false;
	}

	bool insert(const _Key& key)
	{
		return emplace(std::string(key));
	}

	size_type erase(const _Key& key)
	{
		return set_.erase(key);
	}

private:
	size_type							maxCachedKeys_;
	linked_set<_Key, _Compare, _Alloc>	set_;
};

}

#endif // LIBANT_CONTAINER_LRU_SET_H_
