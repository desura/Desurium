template <typename T>
bool isInVector(const T&, const std::vector<T>&);

#include <iostream>

template <typename T>
bool isInVector(const T& t, const std::vector<T>& vector)
{
	bool b = false;
	std::for_each(vector.begin(), vector.end(), [&b, &t](const T& i)
	{
		if (i == t)
		{
			b = true;
			return;
		}
	});
	return b;
}
