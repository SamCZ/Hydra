#pragma once

#include "Hydra/Core/Log.h"
#include "Hydra/Core/Container.h"

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



	template<typename ReturnType, typename... ArgsType>
	struct NammedDelegate
	{
		String Name;
		IDelegate<ReturnType, ArgsType...>* Delegate;
	};

	template<typename ReturnType, typename... ArgsType>
	class DelegateEvent
	{
	private:
		Map<String, IDelegate<ReturnType, ArgsType...>*> _Events;
	public:
		~DelegateEvent()
		{
			for (auto& it : _Events)
			{
				delete it.second;
			}
		}

		DelegateEvent& operator+=(const NammedDelegate<ReturnType, ArgsType...>& d)
		{
			if (_Events.find(d.Name) != _Events.end())
			{
				LogError("DelegateEvent +=", d.Name, "Cannot add ! Same event already exists.");
				delete d.Delegate;

				return *this;
			}

			_Events[d.Name] = d.Delegate;

			return *this;
		}

		DelegateEvent& operator-=(const String evtName)
		{
			if (_Events.find(evtName) != _Events.end())
			{
				delete _Events[evtName];

				_Events.erase(evtName);
			}

			return *this;
		}

		void Invoke(ArgsType... args)
		{
			for (auto& it : _Events)
			{
				it.second->Invoke(args...);
			}
		}
	};

	#define EVENT_NAME(ClassName, MethodName) (String(#ClassName) + ":" + String(#MethodName))
	#define EVENT(ClassName, MethodName) NammedDelegate<void> { EVENT_NAME(ClassName, MethodName), new Delegate<ClassName, void>(this, &ClassName::MethodName) }
	#define EVENT_ARGS(ClassName, MethodName, ...) NammedDelegate<void, __VA_ARGS__> { EVENT_NAME(ClassName, MethodName), new Delegate<ClassName, void, __VA_ARGS__>(this, &ClassName::MethodName) }

}