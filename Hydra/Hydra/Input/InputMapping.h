#pragma once

#include "Hydra/Core/Common.h"
#include "Hydra/Input/InputCoreTypes.h"

namespace Hydra
{
	struct InputActionKeyMapping
	{
		String ActionName;
		Key KeyType;
		uint8 bShift : 1;
		uint8 bCtrl : 1;
		uint8 bAlt : 1;
		uint8 bCmd : 1;

		bool operator==(const InputActionKeyMapping& Other) const
		{
			return (ActionName == Other.ActionName
				&& KeyType == Other.KeyType
				&& bShift == Other.bShift
				&& bCtrl == Other.bCtrl
				&& bAlt == Other.bAlt
				&& bCmd == Other.bCmd);
		}

		InputActionKeyMapping(const String InActionName = String_None, const Key InKey = Keys::Invalid, const bool bInShift = false, const bool bInCtrl = false, const bool bInAlt = false, const bool bInCmd = false)
			: ActionName(InActionName)
			, KeyType(InKey)
			, bShift(bInShift)
			, bCtrl(bInCtrl)
			, bAlt(bInAlt)
			, bCmd(bInCmd)
		{}
	};

	struct InputAxisKeyMapping
	{
		String AxisName;
		Key KeyType;
		float Scale;

		bool operator==(const InputAxisKeyMapping& Other) const
		{
			return (AxisName == Other.AxisName
				&& KeyType == Other.KeyType
				&& Scale == Other.Scale);
		}

		InputAxisKeyMapping(const String InAxisName = String_None, const Key InKey = Keys::Invalid, const float InScale = 1.f)
			: AxisName(InAxisName)
			, KeyType(InKey)
			, Scale(InScale)
		{
		}
	};
}