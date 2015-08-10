#ifndef BHVTREE_H_
#define BHVTREE_H_

#include "global.h"
#include "Primitives.h"
#include <vector>

class BVHNode {
public:
    BVHNode(vector<Primitive*>& primitives, int AXIS);
    
    // Returns the primitive in the subtree rooted at this node that fist intersects a ray or NULL if no intersection.
    Primitive* intersect(Ray& r, double& t);
    
    Box boundingBox();

    BVHNode* _left_child;
    BVHNode* _right_child;
    Primitive* _primitive;  // if this is a leaf node.
    Box _boundingBox;
};

#endif /* BHVTREE_H_ */