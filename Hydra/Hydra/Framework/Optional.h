#pragma once

template <typename T>
class TOptional
{
private:
	bool m_IsValueSet : 1;
	T m_Value;
public:
	inline TOptional() : m_IsValueSet(false) { }

	inline TOptional(T value) : m_IsValueSet(true), m_Value(value) { }

	inline bool IsValueSet()
	{
		return m_IsValueSet;
	}

	inline T GetValue()
	{
		return m_Value;
	}
};