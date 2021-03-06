#ifndef SPAWN_ENGINE_H
#define SPAWN_ENGINE_H

#include "include/config/Define.h"

class GameEngine;
class DisplayEngine;
class QTimer;
class Bonus;

class SpawnEngine : public QObject
{
    Q_OBJECT

public:
    SpawnEngine(int difficulty, GameEngine*, bool _isDemo = false);
    ~SpawnEngine();

    Bonus* generateBonus();
	void start();

public slots:
    void pause(bool);

private slots:
    void timerFired();

private:
    GameEngine *ge;
    DisplayEngine *de;
    bool spawnAsteroids;
    bool spawnAlienMothership;
    bool spawnSatellites;
    bool spawnSupernovae;
    bool spawnBlackSquadron;

    QTimer *timer;

    const static int kProbAsteroid = PROB_ASTEROID;
    const static int kProbAlien = PROB_ALIEN;
    const static int kProbSat = PROB_SAT;
    const static int kProbSupernova = PROB_SUPERNOVA;
    const static int kProbBlackSquadron = PROB_BLACKSQUADRON;

    int totalProba;
    int intervalAsteroid;
    int intervalAlien;
    int intervalSat;
    int intervalSupernova;
    int intervalBlackSquadron;
    bool isDemo;

    static int irand(int min, int max);

    static double proba(double t);
};
#endif
