#pragma once

#include "Widgets/SCompoundWidget.h"

/**
 * Loading screen base theme
 */
class SLoadingScreenBase : public SCompoundWidget
{
public:	
	static float PointSizeToSlateUnits(float PointSize);
protected:
	float GetDPIScale() const;	
};
