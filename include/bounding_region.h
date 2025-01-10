#ifndef INCLUDE_BOUNDING_REGION_H_
#define INCLUDE_BOUNDING_REGION_H_ 1

namespace poyo {
    struct Plane;
    struct Frustum;
    struct Transform;
    struct BoundingRegion { //Axis-aligned bounding box
        BoundingRegion();
        BoundingRegion(const BoundingRegion& other);

        // initialize as AABB
        BoundingRegion(cFVec3& min, cFVec3& max);
        BoundingRegion(cFVec3& inCenter, cfloat iI, cfloat iJ, cfloat iK);

        void draw(cUCVec3& color);

        void setMinMax(cFVec3& min, cFVec3& max);
        cFVec3& getMin() const;
        cFVec3& getMax() const;
        
        FVec3 calculateCenter() const;

        // calculate dimensions
        FVec3 calculateDimensions() const;

        // operator overload
        bool operator==(const BoundingRegion& br) const;

        //Frustum 
        bool isOnOrForwardPlane(const Plane& plane) const;
        bool isOnFrustum(const Frustum& camFrustum, const Transform& transform) const;

        // bounding box values
    private:
        FVec3 min_;
        FVec3 max_;
        FVec3 center_;
        FVec3 extents_;
    };

    struct Plane {
        FVec3 normal = FVec3{ 0.f, 1.f, 0.f }; // unit vector
        float     distance = 0.f;        // Distance with origin

        Plane() = default;

        Plane(const glm::vec3& p1, const glm::vec3& norm)
            : normal(glm::normalize(norm)),
            distance(glm::dot(normal, p1))
        {}

        float getSignedDistanceToPlane(const glm::vec3& point) const {
            return glm::dot(normal, point) - distance;
        }           
    };
    
    struct Frustum {
        Plane topFace;
        Plane bottomFace;

        Plane rightFace;
        Plane leftFace;

        Plane farFace;
        Plane nearFace;
    };
    
};

#endif
