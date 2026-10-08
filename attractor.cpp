#include "defines.h"
#include "attractor.h"
#include "mathutils.h"

attractor::attractor()
{
    mNumAttractors = 50; // PERFORMANCE: If you set this too high things will start to slow down in grid::run()!!

    mAttractors = new Attractor[mNumAttractors];
    if (mAttractors)
    {
        clearAll();
    }
}

attractor::~attractor()
{
    if (mAttractors)
        delete[] mAttractors;
}

attractor::Attractor* attractor::getAttractor()
{
    for (int i=0; i<mNumAttractors; i++)
    {
        if (!mAttractors[i].enabled)
        {
            return &mAttractors[i];
        }
    }

    return NULL;
}

void attractor::clearAll()
{
    for (int i=0; i<mNumAttractors; i++)
    {
        mAttractors[i].enabled = false;
    }
}

Point3d attractor::evaluateParticle(particle::PARTICLE* p)
{
    Point3d speed(0,0,0);

    for (int i=0; i<mNumAttractors; i++)
    {
        Attractor att = mAttractors[i];
        if (att.enabled && att.attractsParticles)
        {
            const Point3d delta = att.pos - p->posStream[0];
            const float actualDistance = std::sqrt(delta.x*delta.x + delta.y*delta.y);
            const float distance = std::max(actualDistance, att.radius);
            if (distance <= 0) continue;
            const float directionX = actualDistance > 0 ? delta.x / actualDistance : 1.f;
            const float directionY = actualDistance > 0 ? delta.y / actualDistance : 0.f;
            // Rotate the normalized radial direction by the original .25 rad.
            // A fixed rotation avoids atan and lookup-table work per particle.
            const float rotationCos = 0.9689124217f;
            const float rotationSin = 0.2474039593f;
            const float force = -att.strength * .5f / (distance * distance);
            speed.x += (directionX*rotationCos - directionY*rotationSin) * force;
            speed.y += (directionX*rotationSin + directionY*rotationCos) * force;

        }
    }

    return speed;
}


