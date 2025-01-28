#include <typedefs.h>

#include <bounding_region.h>
#include <ogc/gx.h>

#include "common.h"

using namespace poyo;

BoundingRegion::BoundingRegion() :
    min_(0),
    max_(0),
    center_(0),
    extents_(0) {
    
}

BoundingRegion::BoundingRegion(const BoundingRegion& other):
    min_(other.min_),
    max_(other.max_),
    center_(other.center_),
    extents_(other.extents_) {
    
}

BoundingRegion::BoundingRegion(cFVec3& min, cFVec3& max) :
    min_(min), max_(max),
    center_{ (max + min) * 0.5f },
    extents_{ max.x - center_.x,
              max.y - center_.y,
              max.z - center_.z } {
    
}

BoundingRegion::BoundingRegion(cFVec3& inCenter, cfloat iI, cfloat iJ, cfloat iK) :
    min_(0), max_(0),
    center_{ inCenter }, extents_{ iI, iJ, iK } {
    
}

void BoundingRegion::draw(cUCVec3& color) {
    // FVec3 extents_ = extents_;
    // extents_ = FVec3{
    //     (max_.x - min_.x) * 0.5f,  // Distancia de centro a max (en el eje X)
    //     (max_.y - min_.y) * 0.5f,  // Distancia de centro a max (en el eje Y)
    //     (max_.z - min_.z) * 0.5f   // Distancia de centro a max (en el eje Z)
    // };
    FVec3 vertices[8] = {
        FVec3{ center_.x - extents_.x, center_.y + extents_.y, center_.z + extents_.z }, // v1
        FVec3{ center_.x - extents_.x, center_.y - extents_.y, center_.z + extents_.z }, // v2
        FVec3{ center_.x + extents_.x, center_.y - extents_.y, center_.z + extents_.z }, // v3
        FVec3{ center_.x + extents_.x, center_.y + extents_.y, center_.z + extents_.z }, // v4
        FVec3{ center_.x - extents_.x, center_.y + extents_.y, center_.z - extents_.z }, // v5
        FVec3{ center_.x + extents_.x, center_.y + extents_.y, center_.z - extents_.z }, // v6
        FVec3{ center_.x + extents_.x, center_.y - extents_.y, center_.z - extents_.z }, // v7
        FVec3{ center_.x - extents_.x, center_.y - extents_.y, center_.z - extents_.z }  // v8
    };
    
    GX_Begin(GX_LINESTRIP, GX_VTXFMT0, 16);
    GX_Position3f32(vertices[1].x, vertices[1].y, vertices[1].z);
    GX_Color4u8(color.x, color.y, color.z, 255);
    GX_Position3f32(vertices[0].x, vertices[0].y, vertices[0].z);
    GX_Color4u8(color.x, color.y, color.z, 255);
    GX_Position3f32(vertices[3].x, vertices[3].y, vertices[3].z);
    GX_Color4u8(color.x, color.y, color.z, 255);
    GX_Position3f32(vertices[2].x, vertices[2].y, vertices[2].z);
    GX_Color4u8(color.x, color.y, color.z, 255);
    GX_Position3f32(vertices[1].x, vertices[1].y, vertices[1].z);
    GX_Color4u8(color.x, color.y, color.z, 255);
    GX_Position3f32(vertices[7].x, vertices[7].y, vertices[7].z);
    GX_Color4u8(color.x, color.y, color.z, 255);
    GX_Position3f32(vertices[6].x, vertices[6].y, vertices[6].z);
    GX_Color4u8(color.x, color.y, color.z, 255);
    GX_Position3f32(vertices[2].x, vertices[2].y, vertices[2].z);
    GX_Color4u8(color.x, color.y, color.z, 255);
    GX_Position3f32(vertices[3].x, vertices[3].y, vertices[3].z);
    GX_Color4u8(color.x, color.y, color.z, 255);
    GX_Position3f32(vertices[5].x, vertices[5].y, vertices[5].z);
    GX_Color4u8(color.x, color.y, color.z, 255);
    GX_Position3f32(vertices[6].x, vertices[6].y, vertices[6].z);
    GX_Color4u8(color.x, color.y, color.z, 255);
    GX_Position3f32(vertices[7].x, vertices[7].y, vertices[7].z);
    GX_Color4u8(color.x, color.y, color.z, 255);
    GX_Position3f32(vertices[4].x, vertices[4].y, vertices[4].z);
    GX_Color4u8(color.x, color.y, color.z, 255);
    GX_Position3f32(vertices[5].x, vertices[5].y, vertices[5].z);
    GX_Color4u8(color.x, color.y, color.z, 255);
    GX_Position3f32(vertices[4].x, vertices[4].y, vertices[4].z);
    GX_Color4u8(color.x, color.y, color.z, 255);
    GX_Position3f32(vertices[0].x, vertices[0].y, vertices[0].z);
    GX_Color4u8(color.x, color.y, color.z, 255);
    GX_End();
    
}

void BoundingRegion::setMinMax(cFVec3& min, cFVec3& max) {
    min_ = min;
    max_ = max;
    center_ = (max + min) * 0.5f;
    extents_ =  FVec3{max.x - center_.x,
                   max.y - center_.y,
                   max.z - center_.z };
}

cFVec3& BoundingRegion::getMin() const {
    return min_;
}

cFVec3& BoundingRegion::getMax() const {
    return max_;
}

FVec3 BoundingRegion::calculateCenter() const {
    return (min_ + max_) * 0.5f;
}

FVec3 BoundingRegion::calculateDimensions() const {
    
    return max_ - min_;
}

bool BoundingRegion::operator==(const BoundingRegion& br) const {
    
    return min_ == br.min_ && max_ == br.max_ && center_ == br.center_;
}

bool BoundingRegion::isOnOrForwardPlane(const Plane& plane) const {
        // Compute the projection interval radius of b onto L(t) = b.c + t * p.n
        const float r = extents_.x * std::abs(plane.normal.x) +
                        extents_.y * std::abs(plane.normal.y) +
                        extents_.z * std::abs(plane.normal.z);

        return -r <= plane.getSignedDistanceToPlane(center_);
}

bool BoundingRegion::isOnFrustum(const Frustum& camFrustum, const Transform& transform) const {
    //Get global scale thanks to our transform
    cFMat4 currentTrans = glm::translate(FMat4(1.0f), transform.Position);

    auto globalCenter = center_;

    // Scaled orientation
    cFVec3 right = cFVec3(currentTrans[0] * extents_.x);
    cFVec3 up = cFVec3(currentTrans[1] * extents_.y);
    cFVec3 forward = cFVec3(-currentTrans[2] * extents_.z);

    cfloat newIi = std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, right)) +
        std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, up)) +
        std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, forward));

    cfloat newIj = std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, right)) +
        std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, up)) +
        std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, forward));

    cfloat newIk = std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, right)) +
        std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, up)) +
        std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, forward));

    const BoundingRegion globalAABB(globalCenter, newIi, newIj, newIk);

    return (globalAABB.isOnOrForwardPlane(camFrustum.leftFace) &&
            globalAABB.isOnOrForwardPlane(camFrustum.rightFace) &&
            globalAABB.isOnOrForwardPlane(camFrustum.topFace) &&
            globalAABB.isOnOrForwardPlane(camFrustum.bottomFace) &&
            globalAABB.isOnOrForwardPlane(camFrustum.nearFace) &&
            globalAABB.isOnOrForwardPlane(camFrustum.farFace));
}
