#pragma once

#include "Hydra/Core/SmartPointer.h"
#include "Hydra/Core/Library.h"

template<typename T>
struct HYDRA_API UIBaseNammedArgs
{

};

template<typename WidgetType>
class HYDRA_API WidgetDecl
{
public:
	inline SharedPtr<WidgetType> operator<<=(const typename WidgetType::FArguments& arguments)
	{
		SharedPtr<WidgetType>& widget = MakeShared<WidgetType>();

		widget->Initialize(arguments);

		return widget;
	}
};

#define UI_BEGIN_ARGS(WidgetType) \
		struct HYDRA_API FArguments : UIBaseNammedArgs<WidgetType> \
		{ \
		public: \
			FArguments()

#define UI_END_ARGS }


#define UI_ARGUMENT(DataType, VarName) \
		DataType _##VarName; \
		FArguments& VarName(DataType arg) \
		{ \
			_##VarName = arg; \
			return *this; \
		}

#define UI_DEFAULT_SLOT(DataType, SlotName) \
		FArguments& operator[](const SharedPtr<UIWidget>& widget) \
		{ \
			return *this; \
		}

#define UINew(WidgetType) WidgetDecl<WidgetType>() <<= WidgetType::FArguments()