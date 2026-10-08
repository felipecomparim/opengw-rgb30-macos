#include "particle.h"
#include "game.h"
#include "entityblackhole.h"
#include <vector>
#include "platform/profile.h"

#define VIRTUAL_SCREEN_WIDTH 800 // FIX ME
#define VIRTUAL_SCREEN_HEIGHT 600 // FIX ME

#define _DISTANCE 320
#define _MIN_DISTANCE .001
#define _MAX_DISTANCE 800


static BOOL renderParticles = TRUE;
static float brightness = 1.0f;
static float particleLineRadius = 3;


static SDL_Thread* mRunThread = NULL;
static bool mRunFlag = false;


particle::PARTICLE* particle::mParticles = NULL;
int particle::mNumParticles = 0;
int particle::mIndex = 0;


static int runThread(void *ptr)
{
    int x,y;

    #ifdef _WIN32
    while(1)
#else
    if (true)
#endif
    {
#ifdef _WIN32
        while (!mRunFlag)
        {
            Sleep(1);
        };
        mRunFlag = false;

#endif

        // The same active black holes affect every particle in this tick.
        // Scan the enemy pool once, instead of ~1,400 enemies per particle.
        entityBlackHole* activeHoles[numEnemyBlackHole];
        int holeCount = 0;
        for (int e = 0; e < NUM_ENEMIES; ++e) {
            entity* candidate = game::mEnemies.mEnemies[e];
            if (candidate->getType() == entity::ENTITY_TYPE_BLACKHOLE &&
                candidate->getState() == entity::ENTITY_STATE_RUNNING) {
                entityBlackHole* hole = static_cast<entityBlackHole*>(candidate);
                if (hole->mActivated && holeCount < numEnemyBlackHole) activeHoles[holeCount++] = hole;
            }
        }

        int i;
        if (particle::mParticles)
        {
            for (i=0; i<particle::mNumParticles; i++)
            {
                if (particle::mParticles[i].timeToLive > 0)
                {
                    // This particle is active

                    particle::PARTICLE* particle = &particle::mParticles[i];

                    --particle->timeToLive;
                    if (particle->timeToLive <= 0)
                    {
                        // This particle died
                        particle->timeToLive = 0;
                    }
                    else
                    {
                        // Evaluate against attractors
                        Point3d ppoint = particle->posStream[0];
                        if (particle->gravity)
                        {
                            Point3d speed = game::mAttractors.evaluateParticle(particle);
                            particle->speedX += speed.x;
                            particle->speedY += speed.y;
                        }

                        // Evaluate only the active black holes.
                        for (int h = 0; h < holeCount; ++h) {
                            entityBlackHole* hole = activeHoles[h];
                            const Point3d delta = particle->posStream[0] - hole->getPos();
                            const float radius = hole->getRadius() * 1.01f;
                            if (delta.x*delta.x + delta.y*delta.y < radius*radius)
                                particle->timeToLive *= .7;
                        }

                        // Add drag
                        particle->speedX *= particle->drag;
                        particle->speedY *= particle->drag;
                
                        Point3d speedClamp(particle->speedX, particle->speedY, 0);
                        speedClamp = mathutils::clamp2dVector(speedClamp, 2);
                        particle->speedX = speedClamp.x;
                        particle->speedY = speedClamp.y;

                        // Move the particle
                        particle->posStream[0].x += particle->speedX;
                        particle->posStream[0].y += particle->speedY;
                        particle->posStream[0].z = 0;

                        if (particle->gridBound)
                        {
                            // Keep it within the grid

						    Point3d hitPoint;
                            Point3d speed(particle->speedX, particle->speedY);
						    if (game::mGrid.hitTest(particle->posStream[0], 0, &hitPoint, &speed))
						    {
                                particle->hitGrid = true;
							    particle->posStream[0] = hitPoint;

                                particle->speedX = speed.x;
                                particle->speedY = speed.y;
						    }

                        }

                        // Shift the position stream
                        for (int p=NUM_POS_STREAM_ITEMS-2; p >= 0; p--)
                        {
                            particle->posStream[p+1] = particle->posStream[p];
                        }

                    }
                }
            }
        }

    }

    return 0;
}

particle::particle()
{
    mNumParticles = 3000; // PERFORMANCE: The larger this number is, the larger the performance hit!
    mIndex = 0;

    mParticles = new PARTICLE[mNumParticles];
    if (mParticles)
    {
        for (int i=0; i<mNumParticles; i++)
        {
            mParticles[i].timeToLive = 0;
        }
    }

#ifdef _WIN32
    // Thread stuff
    mRunThread = SDL_CreateThread(runThread, "runThread");
    if (!mRunThread)
    {
        OutputDebugString(L"Couldn't create particle run thread\n");
    }

#endif
}

particle::~particle()
{
    if (mParticles)
    {
        for (int i=0; i<mNumParticles; i++)
        {
            mParticles[i].timeToLive = 0;
        }

        delete[] mParticles;
    }
    mParticles = NULL;
}


void particle::emitter(Point3d* position, Point3d* angle, float speed, float spread, int num, vector::pen* color, int timeToLive,
                       BOOL gravity, BOOL gridBound, float drag, BOOL glowPass)
{

    // Emit a number of random thrust particles from the nozzle
    for (int p=0; p<num; p++)
    {
        Point3d speedVertex, speedVector;

        speedVertex.x = 0;
        speedVertex.y = speed * mathutils::frandFrom0To1();
        speedVertex.z = 0;

        matrix mat;
        mat.Identity();
        mat.Rotate(0,
                   0,
                   angle->y + ((mathutils::frandFrom0To1()*spread)-spread/2));

        mat.TransformVertex(speedVertex, &speedVector);

        assignParticle(position,
                       speedVector.x, speedVector.y, speedVector.z,
                       timeToLive, color, gravity, gridBound, drag, glowPass);

    }
}


void particle::assignParticle(Point3d* position,
                              float aSpeedX, float aSpeedY, float aSpeedZ,
                              int aTime, vector::pen* aColor, BOOL gravity, BOOL gridBound, float drag, BOOL glowPass)
{
    int i;

    PARTICLE* particle = &mParticles[mIndex++];
    if (mIndex >= mNumParticles) mIndex = 0;

    if (particle)
    {
        Point3d pos = *position;

        if (gridBound)
        {
			Point3d hitPoint;
			if (game::mGrid.hitTest(pos, 0, &hitPoint))
			{
				pos = hitPoint;
			}
        }

        for (int p=0; p<NUM_POS_STREAM_ITEMS; p++)
        {
            particle->posStream[p] = pos;
        }

        particle->speedX = aSpeedX;
        particle->speedY = aSpeedY;
        particle->speedZ = aSpeedZ;
        particle->color = *aColor;
        particle->timeToLive = aTime * mathutils::frandFrom0To1();
        particle->fadeStep = 1.0 / particle->timeToLive;
        particle->gravity = gravity;
        particle->gridBound = gridBound;
        particle->drag = drag;
        particle->glowPass = glowPass;
        particle->hitGrid = false;
    }

}


void particle::draw()
{
#ifndef _WIN32
    profile::Scope timer(profile::PARTICLE_DRAW);
    // Preserve trail colors while drawing at most three batches, instead of
    // one driver draw call for each live particle.
    struct TrailVertex { GLfloat x, y, z, r, g, b, a; };
    static std::vector<TrailVertex> batches[3];
    for (auto& batch : batches) batch.clear();
    for (int i = 0; i < mNumParticles; ++i) {
        PARTICLE& part = mParticles[i];
        if (part.timeToLive <= 0) continue;
        const float speed = std::sqrt(part.speedX*part.speedX + part.speedY*part.speedY);
        float alpha = part.color.a * speed * .8f;
        if (alpha < .05f) { part.timeToLive = 0; continue; }
        int width = static_cast<int>(speed * 8 + .5f);
        width = std::max(2, std::min(4, width));
        auto& vertices = batches[width - 2];
        alpha = std::min(alpha, 1.f);
        for (int p = 0; p < NUM_POS_STREAM_ITEMS - 1 && alpha > 0; ++p) {
            Point3d from = part.posStream[p], to = part.posStream[p+1];
            if (from.x == to.x && from.y == to.y) { to.x += .1; to.y += .1; }
            vertices.push_back({from.x, from.y, 0, part.color.r, part.color.g, part.color.b, alpha});
            alpha -= .1f;
            vertices.push_back({to.x, to.y, 0, part.color.r, part.color.g, part.color.b, std::max(alpha, 0.f)});
            alpha -= .1f;
        }
    }
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    for (int b = 0; b < 3; ++b) {
        auto& vertices = batches[b];
        if (vertices.empty()) continue;
        glLineWidth(b + 2);
        glVertexPointer(3, GL_FLOAT, sizeof(TrailVertex), &vertices[0].x);
        glColorPointer(4, GL_FLOAT, sizeof(TrailVertex), &vertices[0].r);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vertices.size()));
    }
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    return;
#endif

    if (mParticles)
    {
        for (int i=0; i<mNumParticles; i++)
        {
            if (mParticles[i].timeToLive > 0)
            {
                // This particle is active
                PARTICLE* particle = &mParticles[i];

                float a = particle->color.a;
                float speedNormal = mathutils::calculate2dDistance(Point3d(0,0,0), Point3d(particle->speedX, particle->speedY, particle->speedZ));
                a = a * (speedNormal * .8);

                if (a < .05)
                {
                    // This particle died
                    particle->timeToLive = 0;
                    continue;
                }

                float width = speedNormal * 8;
                if (width > 4) width = 4;
                else if (width < 1) width = 1;

                if (scene::mPass == scene::RENDERPASS_BLUR)
                {
                    width *= 4;
                }

                if (width < 2) width = 2;

                glLineWidth(width);

                // This is SO inefficient
                // I really should be doing all the particles in one GL_LINES block but the
                // width of the lines needs to change per-particle, and you can't do a call to
                // glLineWidth() inside a GL_LINES block :-(

                glBegin(GL_LINES);

                float aa = a;
                if (aa > 1) aa = 1;
                for (int p=0; p<NUM_POS_STREAM_ITEMS-1; p++)
                {
                    if (aa <= 0) break;

                    glColor4f(particle->color.r, particle->color.g, particle->color.b, aa); // RGBA

                    Point3d from = particle->posStream[p];
                    Point3d to = particle->posStream[p+1];

                    if ((from.x == to.x) && (from.y == to.y))
                    {
                        to.x += .1;
                        to.y += .1;
                    }

                    glColor4f(particle->color.r, particle->color.g, particle->color.b, aa); // RGBA
                    glVertex3d(from.x, from.y, 0);
                    aa-=.1;

                    glColor4f(particle->color.r, particle->color.g, particle->color.b, aa); // RGBA
                    glVertex3d(to.x, to.y, 0);
                    aa-=.1;

                }

        	    glEnd();

            }
        }


    }

}


void particle::run()
{
#ifndef _WIN32
    profile::Scope timer(profile::PARTICLE_STEP);
#endif
#ifdef _WIN32
    mRunFlag = true;
#else
    runThread(NULL);
#endif
}

void particle::killAll()
{
    if (mParticles)
    {
        for (int i=0; i<mNumParticles; i++)
        {
            mParticles[i].timeToLive = 0;
        }
    }
}



