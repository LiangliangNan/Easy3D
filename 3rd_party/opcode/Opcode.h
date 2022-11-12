///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Main file for Opcode.dll.
 *	\file		Opcode.h
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __OPCODE_H__
#define __OPCODE_H__

// stddef.h and stdarg.h must be included before Opcode headers 
// as they latermay not compile being not able to find types in std::
#include <stddef.h>
#include <stdarg.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Things to help us compile on non-windows platforms

#if defined(__APPLE__) || defined(__MACOSX__)
#if __APPLE_CC__ < 1495
#define sqrtf sqrt
#define sinf sin
#define cosf cos
#define acosf acos
#define asinf asin
#endif
#endif

#ifndef _MSC_VER
#ifndef __int64
#define __int64 long long int
#endif
#ifndef __stdcall /* this is defined in MinGW and CygWin, so avoid the warning */
#define __stdcall /* */
#endif
#endif

#if defined(__GNUC__)
#define OPCODE_NORETURN __attribute__((noreturn))
#elif defined(_MSC_VER)
#define OPCODE_NORETURN __declspec(noreturn)
#else // #if !defined(_MSC_VER)
#define OPCODE_NORETURN
#endif // #if !defined(__GNUC__)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Preprocessor
#define OPCODE_API

	#include "OPC_Settings.h"
	#include "OPC_IceHook.h"

	namespace Opcode
	{
		// Bulk-of-the-work
		#include "OPC_Common.h"
		#include "OPC_MeshInterface.h"
		// Builders
		#include "OPC_TreeBuilders.h"
		// Trees
		#include "OPC_AABBTree.h"
		#include "OPC_OptimizedTree.h"
		// Models
		#include "OPC_BaseModel.h"
		#include "OPC_Model.h"
		#include "OPC_HybridModel.h"
		// Colliders
		#include "OPC_Collider.h"
		#include "OPC_VolumeCollider.h"
		#include "OPC_TreeCollider.h"
		#include "OPC_RayCollider.h"
		#include "OPC_SphereCollider.h"
		#include "OPC_OBBCollider.h"
		#include "OPC_AABBCollider.h"
		#include "OPC_LSSCollider.h"
		#include "OPC_PlanesCollider.h"
		// Usages
		#include "OPC_Picking.h"
		// Sweep-and-prune
		#include "OPC_BoxPruning.h"
		#include "OPC_SweepAndPrune.h"

		typedef void (*OPCODE_AbortHandler)();
		FUNCTION OPCODE_API bool InitOpcode(OPCODE_AbortHandler fnAbortHandler=NULL);
		FUNCTION OPCODE_API bool CloseOpcode();
	}

#endif // __OPCODE_H__
