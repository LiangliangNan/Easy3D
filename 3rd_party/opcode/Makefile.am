EXTRA_DIST = COPYING \
             ReadMe.txt \
             README-ODE.txt \
             TemporalCoherence.txt

SUBDIRS = Ice

noinst_LTLIBRARIES = libOPCODE.la
AM_CPPFLAGS= -I$(top_srcdir)/include \
             -I$(top_builddir)/include

libOPCODE_la_SOURCES =  OPC_AABBCollider.cpp    OPC_AABBCollider.h \
                        OPC_AABBTree.cpp        OPC_AABBTree.h \
                        OPC_BaseModel.cpp       OPC_BaseModel.h \
                        OPC_Collider.cpp        OPC_Collider.h \
                        OPC_Common.cpp          OPC_Common.h \
                        OPC_HybridModel.cpp     OPC_HybridModel.h \
                        OPC_LSSCollider.cpp     OPC_LSSCollider.h \
                        OPC_MeshInterface.cpp   OPC_MeshInterface.h \
                        OPC_Model.cpp           OPC_Model.h \
                        OPC_OBBCollider.cpp     OPC_OBBCollider.h \
                        Opcode.cpp              Opcode.h \
                        OPC_OptimizedTree.cpp   OPC_OptimizedTree.h \
                        OPC_Picking.cpp         OPC_Picking.h \
                        OPC_PlanesCollider.cpp  OPC_PlanesCollider.h \
                        OPC_RayCollider.cpp     OPC_RayCollider.h \
                        OPC_SphereCollider.cpp  OPC_SphereCollider.h \
                        OPC_TreeBuilders.cpp    OPC_TreeBuilders.h \
                        OPC_TreeCollider.cpp    OPC_TreeCollider.h \
                        OPC_VolumeCollider.cpp  OPC_VolumeCollider.h \
                        OPC_Settings.h \
                        OPC_SphereAABBOverlap.h \
                        OPC_BoxBoxOverlap.h \
                        OPC_SphereTriOverlap.h \
                        OPC_PlanesAABBOverlap.h \
                        OPC_TriBoxOverlap.h \
                        OPC_IceHook.h \
                        OPC_PlanesTriOverlap.h \
                        OPC_TriTriOverlap.h \
                        OPC_LSSAABBOverlap.h \
                        OPC_RayAABBOverlap.h \
                        Stdafx.h \
                        OPC_LSSTriOverlap.h \
                        OPC_RayTriOverlap.h 

