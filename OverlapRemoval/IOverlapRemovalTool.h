#ifndef OVERLAPREMOVAL_IOVERLAPREMOVALTOOL_H
#define OVERLAPREMOVAL_IOVERLAPREMOVALTOOL_H

// Framework includes
#include "AsgTools/IAsgTool.h"

// Put the tool in a namespace?

/// Interface for the overlap removal tool
///
/// @author Steve Farrell <steven.farrell@cern.ch>
///
class IOverlapRemovalTool : public virtual asg::IAsgTool
{

    /// Declare the interface
    ASG_TOOL_INTERFACE(IOverlapRemovalTool)

  public:

    // overlap interface methods go here

}; // class IOverlapRemovalTool

#endif
