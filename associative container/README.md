# 关联式容器

## 概念

容器里的每一个元素都有一个键值(key)和一个实值(value)

## stl里的关联式容器

标准的 STL 关联式容器分为 set(集合) 和 map(映射表) 两大类。衍生的还有 multiset(多键集合) 和 multimap(多键映射表)。这些容器的底层机制都是 `RB-tree` (红黑树原理) 完成。

散列表是以 hash table 为底层机制而完成的 hash_set、hash_map、hash_multiset、hash_multimap。 书上说没进入stl标准，但是c++11 应该是添加了unordered_map，事实上就是hash



