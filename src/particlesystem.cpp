#include "ParticleSystem.h"

ParticleSystem::ParticleSystem() 
{
    generatedParticleColor = glm::vec3(0, 0, 1);
    loader = new LoadBeforeFrameLoader();
}

ParticleSystem::~ParticleSystem() 
{
    delete loader;
    deleteVelocityArray();
}

float ParticleSystem::getCubeLength()
{
    return cubeLength;
}

void ParticleSystem::deleteVelocityArray()
{
    if (velocityArray != NULL)
    {
        delete velocityArray;
        velocityArray = NULL;
    }
}

int ParticleSystem::getOriginalParticleCount()
{
    return loader->getParticleCountPerFrame(0);
}

int ParticleSystem::getGeneratedParitlceCount()
{
    return loader->getParticleCountPerFrame(0)*genPerParticleNotAntweakbar;
}

bool ParticleSystem::loadFiles(std::vector<std::string> fileNames)
{
    particles = std::vector<Particle>(0);
    generatedParticles = std::vector<Particle>(0);
    deleteVelocityArray();
    fileNumber = 0;
    isFileLoaded = false;

    if (onlyFileParticles) 
    {
        onlyFileParticlesRuntime = true;
    }
    else 
    {
        onlyFileParticlesRuntime = false;
    }

    delete loader;
    
    if (loadMode == 1)
    {
        loader = new LoadBeforeFrameLoader;
    }
    else
    {
        loader = new LoadRuntimeFrameLoader;
    }
    
    if (loader->loadFiles(fileNames))//If it succesfully loads something
    {
        isFileLoaded = true;
        
        if (!onlyFileParticlesRuntime) //If particles needs to be generated
        {
            velocityArray = new Array3d(glm::vec3(0, 0, 0), 
                cubeLength, velocityArrayParts);
            
            //Igy csak betolteskor modosulhat:
            genPerParticleNotAntweakbar = genPerParticle; 
            generatedParticles = std::vector<Particle>(
                        loader->getParticleCountPerFrame(0) *
                        genPerParticleNotAntweakbar);
        }
        
        return true;
    }
    return false;
}

void ParticleSystem::generateParticles(int number)
{
    std::vector<Particle> frameParticles;
    
    if (!loader->getFrame(number, frameParticles))
    {
        printf("Couldn't generate particles for number: %d\n", number);
        return;
    }
    
    for (unsigned int i = 0; i < frameParticles.size(); i++)
    {
        for (unsigned int j = 0; j < genPerParticleNotAntweakbar; j++)
        {
            float r = rand_FloatRange(0, atwR);
            float angle1 = rand_FloatRange(0, 360);
            float angle2 = rand_FloatRange(0, 360);
            
            glm::vec3 posVec = glm::rotateY(glm::vec3(r, 0, 0), angle1);
            glm::vec3 axisVec = glm::rotateY(glm::vec3(0, 0, 1), angle1);

            posVec = glm::rotate(posVec, angle2, axisVec);

            generatedParticles[i*genPerParticleNotAntweakbar + j].pos =
                    frameParticles[i].pos+posVec;
            generatedParticles[i*genPerParticleNotAntweakbar + j].color =
                    generatedParticleColor;
        }
    }
}

void ParticleSystem::initialize() 
{
    Animate(0);
}

void ParticleSystem::Animate(float t)
{
    if (!isFileLoaded || pause)
    {
        return;
    }

    secs += t;
    
    if (secs < 1.0f / maxFps)
    {
        return;
    }
    //else:

    secs = 0;
    
    if (onlyFileParticlesRuntime == false) //if there are particles generated
    {
        //generating the particles
        if (fileNumber == 0)
        {
            generateParticles(generatedParticleFile);
        }
        else//Moving the generated particles
        {
            for (unsigned int i = 0; i < generatedParticles.size(); i++)
            {
                generatedParticles[i].move(1);
            }
        }

        if (loader->getFrame(fileNumber,particles))
        {
            velocityArray->clear(); //Remove the previous velocities
            
            for (unsigned int i = 0; i < particles.size(); i++)
            {
                //Write the velocity to the 3D array:
                velocityArray->setVelocity(particles[i].pos, particles[i].v); 
            }
            
            velocityArray->calculateAvg();

            //Adding the new velocities into the generated particles
            for (unsigned int i = 0; i < generatedParticles.size(); i++)
            {
                glm::vec3 distanceFromParticle =
                        particles[i / genPerParticleNotAntweakbar].pos -
                        generatedParticles[i].pos;
                    
                float dist = glm::length(distanceFromParticle);
                
                if (dist >= triggerDistance && followControllPoints == true)
                {
                    //Experimental function. 
                    //If the particle too far from it's control point, 
                    //it jumps to there.
                    float r = rand_FloatRange(0, randomR);
                    float angle1 = rand_FloatRange(0, 360);
                    float angle2 = rand_FloatRange(0, 360);
                    
                    glm::vec3 randomPosVec =
                            glm::rotateY(glm::vec3(r, 0, 0), angle1);
                    glm::vec3 axisVec =
                            glm::rotateY(glm::vec3(0, 0, 1), angle1);
                    randomPosVec = glm::rotate(randomPosVec, angle2, axisVec);
                    generatedParticles[i].pos +=
                            distanceMultiplier *
                            dist *
                            glm::normalize(distanceFromParticle + randomPosVec);
                    generatedParticles[i].v =
                            velocityArray->getVelocity(
                                generatedParticles[i].pos, ignoreNullVelocity);
                }
                else
                {
                    //The default operation gets the velocity from the 3D array
                    generatedParticles[i].v =
                            velocityArray->getVelocity(
                                generatedParticles[i].pos, ignoreNullVelocity);
                }
            }

            fileNumber++;
        }
        else
        {
            fileNumber = 0;
        }
    }
    else //No generated particles
    {
        if (loader->getFrame(fileNumber, particles))
        {
            fileNumber++;
        }
        else
        {
            fileNumber = 0;
        }
    }
}

int ParticleSystem::getFileNumber()
{
    return fileNumber;
}

const std::vector<Particle>& ParticleSystem::getOriginalParticles()
{
    return particles;
}

const std::vector<Particle>& ParticleSystem::getGeneratedParticles()
{
    return generatedParticles;
}
