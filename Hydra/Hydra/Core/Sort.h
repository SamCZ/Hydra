#pragma once

#include <vector>
#include <functional>

template<typename T, typename A>
static void Swap(std::vector<T, A>& list, int i1, int i2)
{
	T p1 = list[i1];
	list[i1] = list[i2];
	list[i2] = p1;
}

template<typename T, typename A>
static void Arraycopy(std::vector<T, A> const& src, int srcPos, std::vector<T, A>& dest, int destPos, int length)
{
	while (length > 0)
	{
		dest[destPos] = src[srcPos];
		srcPos++;
		destPos++;
		length--;
	}
}

template<typename T, typename A>
static void MergeSort(std::vector<T, A>& src, std::vector<T, A>& dest, int low, int high, int off, std::function<int(const T, const T)> compare_function)
{
	int length = high - low;
	if (length < 7)
	{
		for (int i = low; i < high; i++)
		{
			for (int j = i; j > low && compare_function(dest[j - 1], dest[j]) > 0; j--)
			{
				Swap(dest, j, j - 1);
			}
		}
		return;
	}
	int destLow = low;
	int destHigh = high;
	low += off;
	high += off;
	int mid = (low + high) >> 1;
	MergeSort(dest, src, low, mid, -off, compare_function);
	MergeSort(dest, src, mid, high, -off, compare_function);

	if (compare_function(src[mid - 1], src[mid]) <= 0)
	{
		Arraycopy(src, low, dest, destLow, length);
		return;
	}

	// Merge sorted halves (now in src) into dest
	for (int i = destLow, p = low, q = mid; i < destHigh; i++)
	{
		if (q >= high || p < mid && compare_function(src[p], src[q]) <= 0)
			dest[i] = src[p++];
		else
			dest[i] = src[q++];
	}
}

template<typename T, typename A>
static void MergeSort(std::vector<T, A>& src, std::function<int(const T, const T)> compare_function)
{
	std::vector<T, A> copyOfsrc(src);
	MergeSort<T>(src, copyOfsrc, 0, src.size(), 0, compare_function);
	src = copyOfsrc;
}

static int SORT_MIN_MAX(float f1, float f2)
{
	if (f1 < f2)
	{
		return -1;
	}
	if (f1 > f2)
	{
		return 1;
	}

	return f1 == f2 ? 0 : (f1 < f2 ? -1 : 1);
}