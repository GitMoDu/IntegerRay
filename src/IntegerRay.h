#ifndef _INTEGER_RAY_INCLUDE_h
#define _INTEGER_RAY_INCLUDE_h

// IntegerRay depends on IntegerSignal https://github.com/GitMoDu/IntegerSignal
#include "Model/ColorModel.h"

// Library interfaces for drivers, buffers, and animators.
#include "Model/Interface.h"


// Engine for managing buffers, animators, and drivers.
#include "Engine/RayBuffer.h"
#include "Engine/RayEngine.h"

// Animation framework.
#include "Framework/Animation/Model.h"
#include "Framework/Animation/Curves.h"
// Shared extension point for custom animator implementations.
#include "Framework/Animation/AnimatorCore.h"
#include "Framework/Animation/Animator.h"
#include "Framework/Animation/LambdaAnimator.h"
#include "Framework/Animation/FieldAnimator.h"

// Constants for reference white points and color grading.
#include "Model/ReferenceWhites.h"

// Color grading stages.
#include "Model/Grading/Model.h"
#include "Model/Grading/HueShift.h"
#include "Model/Grading/Brightness.h"
#include "Model/Grading/Contrast.h"
#include "Model/Grading/Gamma.h"
#include "Model/Grading/RedBlueBalance.h"

#endif