#pragma once

namespace Hydra
{

	template<class UserClass, typename ReturnType, typename... ArgsTypes>
	struct MethodAction
	{
		typedef ReturnType(UserClass::*Type)(ArgsTypes...);
	};

	template<typename ReturnType, typename... ArgsTypes>
	class IDelegate
	{
	public:
		virtual ReturnType Invoke(ArgsTypes... params) = 0;
	};

	template<class UserClass, typename ReturnType, typename... ArgsTypes>
	class Delegate : public IDelegate<ReturnType, ArgsTypes...>
	{
	private:
		UserClass* _Class;
	public:
		typedef typename MethodAction<UserClass, ReturnType, ArgsTypes...>::Type MethodPtr;
	private:
		MethodPtr _Method;
	public:

		Delegate(UserClass* object, MethodPtr method) : _Class(object), _Method(method) { }

		virtual ReturnType Invoke(ArgsTypes... params) override
		{
			return std::invoke(_Method, *_Class, params...);
		}
	};

}