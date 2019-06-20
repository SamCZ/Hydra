#pragma once

#include "Hydra/Core/Common.h"

namespace ETaskbarProgressState
{
	enum Type
	{
		//Stops displaying progress and returns the button to its normal state.
		NoProgress = 0x0,

		//The progress indicator does not grow in size, but cycles repeatedly along the 
		//length of the task bar button. This indicates activity without specifying what 
		//proportion of the progress is complete. Progress is taking place, but there is 
		//no prediction as to how long the operation will take.
		Indeterminate = 0x1,

		//The progress indicator grows in size from left to right in proportion to the 
		//estimated amount of the operation completed. This is a determinate progress 
		//indicator; a prediction is being made as to the duration of the operation.
		Normal = 0x2,

		//The progress indicator turns red to show that an error has occurred in one of 
		//the windows that is broadcasting progress. This is a determinate state. If the 
		//progress indicator is in the indeterminate state, it switches to a red determinate 
		//display of a generic percentage not indicative of actual progress.
		Error = 0x4,

		//The progress indicator turns yellow to show that progress is currently stopped in 
		//one of the windows but can be resumed by the user. No error condition exists and 
		//nothing is preventing the progress from continuing. This is a determinate state. 
		//If the progress indicator is in the indeterminate state, it switches to a yellow 
		//determinate display of a generic percentage not indicative of actual progress.
		Paused = 0x8,
	};
}

class FWindow;
class FIcon;

class HYDRA_API FTaskbar
{
public:
	FTaskbar();
	virtual ~FTaskbar();

	virtual void SetOverlayIcon(const SharedPtr<FWindow>& NativeWindow, const FIcon& Icon, const String& Description);

	virtual void SetProgressState(const SharedPtr<FWindow>& NativeWindow, ETaskbarProgressState::Type State);

	virtual void SetProgressValue(const SharedPtr<FWindow>& NativeWindow, uint64 Current, uint64 Total);
};