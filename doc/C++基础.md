```c++
STL:
	find_if的使用
	template<class InputIterator, class UnaryPredicate>
  InputIterator find_if (InputIterator first, InputIterator last, UnaryPredicate pred)
{
  while (first!=last) {
    if (pred(*first)) return first;//pred 是前缀的意思
    ++first;
  }
  return last;
}
```

