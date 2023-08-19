#ifndef __WORLD_MANAGER_REVERSE_SHIM_H__
#define __WORLD_MANAGER_REVERSE_SHIM_H__

// This header file is included from WorldManagerCppEnd.cpp and it must be ObjC-free.

#include <vector>
#include <string>

// This class is for the call chains in the direciton of C++ -> Swift.
class WorldManagerReverseShim
{
public:
    WorldManagerReverseShim( WorldManagerShim* shim );
    ~WorldManagerReverseShim();
    
private:
    WorldManagerShim* m_shim;
};

#endif /*__WORLD_MANAGER_REVERSE_SHIM_H__*/

