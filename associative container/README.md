# 关联式容器

## 概念

容器里的每一个元素都有一个键值(key)和一个实值(value)

## stl里的关联式容器

标准的 STL 关联式容器分为 set(集合) 和 map(映射表) 两大类。衍生的还有 multiset(多键集合) 和 multimap(多键映射表)。这些容器的底层机制都是 `RB-tree` (红黑树原理) 完成。

散列表是以 hash table 为底层机制而完成的 hash_set、hash_map、hash_multiset、hash_multimap。 书上说没进入stl标准，但是c++11 应该是添加了unordered_map，事实上就是hash



## 红黑树和哈希表的对比

1. map始终保证遍历的时候是按key的大小顺序的，hashmap是无序的。
2. map可以做范围查找，而hashmap不可以。原因和上面相同
3. map的iterator可以双向遍历，这样可以很容易查找到当前map中刚好大于这个key的值，或者刚好小于这个key的值（lower_bound , upper_bound）
4. hashmap的内存效率比map差，无效内存多
5. hashmap查找快,O(1) 对 O(log n)
6. hashmap查找时候要算hash，这个最坏时间复杂度是O(M)（M是key字符串的长度）
7. hashmap实现简单。
8. 多线程的时候，hashmap采用开链式的话可以针对一个桶进行加锁，红黑树有线程安全的实现吗？
9. map的查找效率比hashmap稳定。很多场合是需要实时性，比如各种嵌入式设备，这时候必须保证最坏复杂度（zhihu看到的）