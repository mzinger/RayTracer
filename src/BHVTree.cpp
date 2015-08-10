#include "BHVTree.h"

/****************************************************************
 *													   *
 *		    BHVTree                                            *
 *											   		   *
 ****************************************************************/

Box BVHNode::boundingBox() {
    return _boundingBox;
}

BVHNode::BVHNode(vector<Primitive*>& primitives, int AXIS) {
    _primitive = NULL;
    int N = primitives.size();
    assert (N >=1);
    if (N==1) {
        _primitive = primitives[0];
        _left_child = NULL;
        _right_child = NULL;
        _boundingBox = _primitive->boundingBox();
    } else {
        Box bb = Box(primitives);
        double midpoint = (bb.bottomLeft()[AXIS] + bb.topRight()[AXIS]) / 2;
        vector<Primitive*> left_list, right_list;
        for (Primitive* p : primitives) {
            if ((p->boundingBox().bottomLeft()[AXIS] - midpoint) *
                (p->boundingBox().topRight()[AXIS] - midpoint) >= 0) {
                // Completely on one side
                if (p->boundingBox().bottomLeft()[AXIS] - midpoint > 0) {
                    right_list.push_back(p);
                } else {
                    left_list.push_back(p);
                }
            } else {
                // randomly put on any side
                if (rand() % 2 == 0) {
                    right_list.push_back(p);
                } else {
                    left_list.push_back(p);
                }
            }
        }
        _left_child = NULL; _right_child = NULL;
        if (left_list.size() > 0 && right_list.size() > 0) {
            _left_child = new BVHNode(left_list, (AXIS + 1)%3);
            _right_child = new BVHNode(right_list, (AXIS + 1)%3);
            _boundingBox = _right_child->boundingBox();
            _boundingBox = _left_child->boundingBox().combine(_boundingBox);
        } else if (left_list.size() > 0 && right_list.size() == 0) {
            _left_child = new BVHNode(left_list, (AXIS + 1)%3);
            _boundingBox = _left_child->boundingBox();
        } else if (left_list.size() == 0 && right_list.size() > 0) {
            _right_child = new BVHNode(right_list, (AXIS + 1)%3);
            _boundingBox = _right_child->boundingBox();
        } else {
            // Cannnot happen.
            assert(false);
        }
    }
}

Primitive* BVHNode::intersect(Ray& ray, double& t) {
    if (_primitive != NULL) {
        // Leaf node
        double curr_t = _primitive->intersect(ray);
        if (curr_t < std::numeric_limits<double>::max() && curr_t > ray.getMinT()) {
            t = curr_t;
            return _primitive;
        }
    }
    if (!_boundingBox.intersect(ray)) return NULL;
    double t_l, t_r;
    Primitive* left_result = NULL;
    Primitive* right_result = NULL;
    if (_left_child != NULL) {
        left_result = _left_child->intersect(ray, t_l);
    }
    if (_right_child != NULL) {
        right_result = _right_child->intersect(ray, t_r);
    }
    if (left_result == NULL && right_result == NULL) {
        t = std::numeric_limits<double>::max();
        return NULL;
    } else if (left_result == NULL) {
        t = t_r;
        return right_result;
    } else if (right_result == NULL) {
        t = t_l;
        return left_result;
    }
    // Both subtrees have interection. Pick the one in front.
    t = MIN(t_l, t_r);
    return t_l < t_r ? left_result : right_result;
}