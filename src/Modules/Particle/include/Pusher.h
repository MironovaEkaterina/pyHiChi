#pragma once
#include "Constants.h"
#include "Species.h"
#include "FieldValue.h"
#include "Ensemble.h"

#include <array>
#include <vector>

#include <iostream>
namespace pfc
{
    class NoPusher {};

    class ParticlePusher
    {
    public:
        template<class T_Particle>
        inline void operator()(T_Particle* particle, ValueField& field, FP timeStep) {};

        template<class T_ParticleArray>
        inline void operator()(T_ParticleArray* particleArray, std::vector<ValueField>& fields, FP timeStep) { };
    };

    class BorisPusher : public ParticlePusher
    {
    public:
        template<class T_Particle>
        inline void operator()(T_Particle* particle, FP3& e, FP3& b, FP timeStep)
        {
            FP eCoeff = timeStep * particle->getCharge() / (2 * particle->getMass() * Constants<FP>::lightVelocity());
            FP3 eMomentum = e * eCoeff;
            FP3 um = particle->getP() + eMomentum;
            FP3 t = b * eCoeff / sqrt((FP)1 + um.norm2());
            FP3 uprime = um + cross(um, t);
            FP3 s = t * (FP)2 / ((FP)1 + t.norm2());
            particle->setP(eMomentum + um + cross(uprime, s));
            particle->setPosition(particle->getPosition() + timeStep * particle->getVelocity());
        }

        template<class T_Particle>
        inline void operator()(T_Particle* particle, ValueField& field, FP timeStep)
        {
            operator()(particle, field.getE(), field.getB(), timeStep);
        }

        template<class T_ParticleArray>
        inline void operator()(T_ParticleArray* particleArray, std::vector<ValueField>& fields, FP timeStep)
        {
            typedef typename T_ParticleArray::ParticleProxyType ParticleProxyType;

            OMP_FOR()
            OMP_SIMD()
            for (int i = 0; i < particleArray->size(); i++)
            {
                ParticleProxyType particle = (*particleArray)[i];
                operator()(&particle, fields[i], timeStep);
            }
        };

        template<class T_ParticleArray, class TGrid>
        inline void operator()(T_ParticleArray* particleArray, TGrid* grid, FP timeStep)
        {
            typedef typename T_ParticleArray::ParticleProxyType ParticleProxyType;

            OMP_FOR()
            OMP_SIMD()
            for (int i = 0; i < particleArray->size(); i++)
            {
                ParticleProxyType particle = (*particleArray)[i];
                FP3 pPos = particle.getPosition();
                operator()(&particle, grid->getE(pPos), grid->getB(pPos), timeStep);
            }
        };

        // need fixed? ensemble???
        template<class T_ParticleArray, class TGrid>
        inline void operator()(Ensemble<T_ParticleArray>* ensemble, TGrid* grid, FP timeStep)
        {
            for (int i = 0; i < pfc::sizeParticleTypes; i++)
                operator()(&ensemble->operator[](i), grid, timeStep);
        };
    };

    class RadiationReaction : public ParticlePusher
    {
    public:

        template<class T_Particle>
        inline void operator()(T_Particle* particle, ValueField& field, FP timeStep)
        {
            if (particle->getType() == Electron || particle->getType() == Positron)
            {

                FP3 e = field.getE();
                FP3 b = field.getB();
                FP3 v = particle->getVelocity();
                FP gamma = particle->getGamma();
                FP c = Constants<FP>::lightVelocity();
                FP electronCharge = Constants<FP>::electronCharge();
                FP electronMass = Constants<FP>::electronMass();
                FP3 dp = timeStep * (2.0 / 3.0) * sqr(sqr(electronCharge) / (electronMass * sqr(c))) *
                    (VP(e, b) + (1 / c) * (VP(b, VP(b, v)) + SP(v, e) * e) -
                        (1 / c) * sqr(gamma) * (sqr(e + (1 / c) * VP(v, b)) - sqr(SP(e, v) / c)) * v);

                particle->setMomentum(particle->getMomentum() + dp);
            }
        }

        template<class T_ParticleArray>
        inline void operator()(T_ParticleArray* particleArray, std::vector<ValueField>& fields, FP timeStep)
        {
            typedef typename T_ParticleArray::ParticleProxyType ParticleProxyType;

            OMP_FOR()
            OMP_SIMD()
            for (int i = 0; i < particleArray->size(); i++)
            {
                ParticleProxyType particle = (*particleArray)[i];
                operator()(&particle, fields[i], timeStep);
            }
        };
    };
}