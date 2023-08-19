#import <Foundation/Foundation.h>
#import "WorldManagerShim.h"
#import "WorldManagerReverseShim.h"

WorldManagerReverseShim::WorldManagerReverseShim( WorldManagerShim* shim )
    :m_shim{ shim }
{

}

WorldManagerReverseShim::~WorldManagerReverseShim() {
    ;
}

