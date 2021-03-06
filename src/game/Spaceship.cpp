/*==============================================================*
 | Implementation file Spaceship.cpp
 |        implements : Spaceship class
 |
 |
 | summary : Entity class that represents a spaceship.
 |           A spaceship can move,fire,collect bonus,receive damage.
 |           For more information about the damage, please consult the specification file.
 |
 | Creator : Diego Antognini
 | Creation date : 27/11/2012
 | Copyright: EIAJ, all rights reserved
 |
 |
 | Version of the file : 1.0.0
 |
 *==============================================================*/

#include "include/engine/DisplayEngine.h"
#include "include/menu/HUDWidget.h"
#include "include/engine/GameEngine.h"
#include "include/engine/SoundEngine.h"
#include "include/engine/UserControlsEngine.h"
#include "include/engine/wiimoteengine.h"
#include "include/game/Spaceship.h"
#include "include/game/ProjectileCross.h"
#include "include/game/ProjectileSimple.h"
#include "include/game/ProjectileV.h"
#include "include/game/BonusForceField.h"
#include "include/game/BonusHP.h"
#include "include/game/BonusProjectile.h"
#include "include/game/BonusInvicibility.h"
#include "include/game/SpecialBonus.h"
#include "include/game/SpecialBonusAntiGravity.h"
#include "include/game/SpecialBonusTracking.h"
#include "include/game/ProjectileTracking.h"
#include "include/game/ProjectileMulti.h"
#include "include/game/SpecialBonusMulti.h"
#include "include/game/SpecialBonusGuided.h"
#include "include/game/SpecialBonusFreeze.h"
#include "include/game/ProjectileGuided.h"
#include "include/config/Define.h"

// Temporary constant TO_REMOVE
#define dSpeed 8

Spaceship::Spaceship(qreal _dX, qreal _dY, Shooter _player, const QString& _playerName, qreal _dHealthPoint, qreal _dResistance, int _cooldown, TypeSpecialBonus _sp, GameEngine *_gameEngine)
    :Destroyable(_dHealthPoint,_dResistance),
      Displayable(_dX,_dY),
      gameEngine(_gameEngine),//GameEngine
      player(_player),//Kind of player (Player1 or Player2)
      playerName(_playerName),//Name
      bonus(0),//NULL pointer until the player receive an invicibility bonus
      timerProjectile(new QTimer(this)),//Initialize the timer for the projectile bonus, waiting to get one
      type(PROJ_SPACESHIP_DEF),//Default kind of projectile
      dHealthForceField(MAX_SPACESHIP_PV),//Health point of the force field
      dResistanceForceField(RESISTANCE_FORCE_FIELD),//Resistance of the forcefield
      score(0),//Number of point, only use in timerMode
      dAngleAttack(0),
      isInvicible(false),
      isFrozen(false),
      specialBonus(0),
      isGuided(false),
      projectileGuided(0),
      shield(PICTURE_SHIELD)
{

    switch(_sp)
    {
    case TypeSpecialBonusAntiGravity:
        specialBonus = new SpecialBonusAntiGravity(_cooldown, ANTIGRAVITY_ACTIVATIONS, player, gameEngine);
        break;
    case TypeSpecialBonusFreeze:
        specialBonus = new SpecialBonusFreeze(_cooldown, FREEZE_DURATION, (player == Player1 ? Player2 : Player1), gameEngine);
        break;
    case TypeSpecialBonusGuidedMissile:
        specialBonus = new SpecialBonusGuided(_cooldown, GUIDED_ACTIVATIONS, this, gameEngine);
        break;
    case TypeSpecialBonusOmnidirectionalShot:
        specialBonus = new SpecialBonusMulti(_cooldown, ROOTSHOT_ACTIVATIONS, this, gameEngine);
        break;
    case TypeSpecialBonusTrackingMissile:
        specialBonus = new SpecialBonusTracking(_cooldown, TRACKING_ACTIVATIONS, this, gameEngine);
        break;
    }

    if(_player == Player1)
        dAngle = 0;
    else if(_player == Player2)
        dAngle = M_PI;
    timerProjectile->setSingleShot(true);
    connect(timerProjectile,SIGNAL(timeout()),this,SLOT(removeProjectileBonus()));

    hud = gameEngine->displayEngine()->getHud();
    hud->setSpecialBonus(player, specialBonus);
}

void Spaceship::setPixmap(QPixmap *_pxmPixmap)
{
    Displayable::setPixmap(_pxmPixmap);
    if(player == Player2)
    {
        //Rotate the picture for the player2
        Displayable::setPixmap(new QPixmap(getPixmap()->transformed(QTransform().rotate(180))));
        setPos(pos().x()-getPixmap()->size().width(),pos().y());
    }
}

Spaceship::~Spaceship()
{
    if(bonus != 0)
        delete bonus;
    delete timerProjectile;
    delete specialBonus;
}

void Spaceship::setY(int _y)
{
	if(!isFrozen && _y+getPixmap()->height() < gameEngine->sceneSize().height() && _y > GameEngine::offset)
		setPos(pos().x(), _y);
}

QRectF Spaceship::boundingRect() const
{
    return QRectF(getPixmap()->rect());
}

QPainterPath Spaceship::shape() const
{
    QPainterPath l_path;
    l_path.addEllipse(boundingRect());
    return l_path;
}

void Spaceship::shotGuidedBonus()
{
    if(!isGuided)
    {
        projectileGuided = new ProjectileGuided(getXPositionFire(), getYPositionCenter(), player, this);
        gameEngine->addProjectile(projectileGuided);
        isGuided = true;
    }
}

void Spaceship::unfreeze()
{
    isFrozen = false;
}

void Spaceship::disableGuideBonus()
{
    isGuided = false;
}

void Spaceship::paint(QPainter *_painter,const QStyleOptionGraphicsItem *, QWidget *)
{
    _painter->drawPixmap(0 , 0, *getPixmap());
    if(isInvicible)
    {
        int l_x = shield.width()/2-getPixmap()->width()/2.0;
        int l_y = shield.height()/2-getPixmap()->height()/2.0;

        _painter->drawPixmap(-l_x, -l_y, shield);
    }
}

qreal Spaceship::getPercentageSpeed() const
{
    return 100.0;
}

void Spaceship::addBonus(Bonus *_bonus)
{
    if(bonus == 0)
    {
        bonus = _bonus;
        hud->setNormalBonus(player, bonus);
    }
}

void Spaceship::shotTrackingBonus()
{
    gameEngine->soundEngine()->playSound(ShootSound);
    gameEngine->addProjectile(new ProjectileTracking(getXPositionFire(), getYPositionFire(), player,(player == Player1) ? gameEngine->ship2() : gameEngine->ship1(),gameEngine));
}

void Spaceship::shotMultiBonus()
{
    gameEngine->soundEngine()->playSound(ShootSound);
    qreal l_x = getXPositionFire();
    qreal l_y = getYPositionFire();
    qreal angle = dAngleAttack*M_PI/180.0;

    gameEngine->addProjectile(new ProjectileMulti(l_x,l_y,player,1,0,angle));
    gameEngine->addProjectile(new ProjectileMulti(l_x,l_y,player,-1,0,angle));
    gameEngine->addProjectile(new ProjectileMulti(l_x,l_y,player,0.25,1.0/2.0,angle));
    gameEngine->addProjectile(new ProjectileMulti(l_x,l_y,player,0.6,1.0/3.0,angle));
    gameEngine->addProjectile(new ProjectileMulti(l_x,l_y,player,0.8,1.0/4.0,angle));
    gameEngine->addProjectile(new ProjectileMulti(l_x,l_y,player,0.8,1.0/5.0,angle));
    gameEngine->addProjectile(new ProjectileMulti(l_x,l_y,player,-0.25,1.0/2.0,angle));
    gameEngine->addProjectile(new ProjectileMulti(l_x,l_y,player,-0.6,1.0/3.0,angle));
    gameEngine->addProjectile(new ProjectileMulti(l_x,l_y,player,-0.8,1.0/4.0,angle));
    gameEngine->addProjectile(new ProjectileMulti(l_x,l_y,player,-0.8,1.0/5.0,angle));
}

void Spaceship::triggerBonus()
{
    if(bonus != 0 && !isInvicible && type == PROJ_SPACESHIP_DEF)
    {
        hud->activateBonus(player, NormalBonus);

        if(BonusHP* bhp = dynamic_cast<BonusHP*>(bonus))
        {
            dHealthPoint+=bhp->getHealthPoint();
            if(dHealthPoint>MAX_SPACESHIP_PV)
                dHealthPoint=MAX_SPACESHIP_PV;

            delete bonus;
            bonus = 0;
        }
        else if(BonusProjectile* bp = dynamic_cast<BonusProjectile*>(bonus))
        {
            //If a timer for projectile is already started, we need to stop it and start anothero ne
            if(timerProjectile->isActive())
                removeProjectileBonus();

            type = bp->getType();

            timerProjectile->start(bp->getExpiration());
        }
        else if(BonusForceField* bff = dynamic_cast<BonusForceField*>(bonus))
        {
            dHealthForceField = MAX_SPACESHIP_PV;
            delete bonus;
            bonus = 0;
        }
        else if(BonusInvicibility* bi = dynamic_cast<BonusInvicibility*>(bonus))
        {
            isInvicible = true;
            QTimer::singleShot(bi->getExpiration(),this,SLOT(removeBonusInvicibility()));
        }
    }
}

void Spaceship::removeProjectileBonus()
{
    timerProjectile->stop();
    type = PROJ_SPACESHIP_DEF;
    delete bonus;
    bonus = 0;
    gameEngine->displayEngine()->getHud()->deactivateBonus(player, NormalBonus);
}

void Spaceship::removeBonusInvicibility()
{
    isInvicible = false;
    delete bonus;
    bonus = 0;
}

void Spaceship::receiveAttack(qreal _dPower)
{
    if(!isInvicible)
    {
        gameEngine->wiimoteEngine()->rumble((player == Player1) ? 0 : 1, RUMBLE_TIME);

        double power1 = _dPower * (100.0 - dHealthForceField) / 100.0;
        double power2 = _dPower - power1;

        dHealthPoint -= power1 / dResistance;
        dHealthForceField -= power2 / dResistanceForceField;
        if(dHealthForceField < 0)
            dHealthForceField = 0;

        if(dHealthPoint<=0)
            dHealthPoint=0;
        isDead();
    }
}

void Spaceship::rotate(qreal pitch)
{
    resetTransform();
    qreal x = getPixmap()->width()/2.0;
    qreal y = getPixmap()->height()/2.0;
    qreal angle = -pitch;

    if(player == Player2)
        angle *= -1;

    setTransform(QTransform().translate(x,y).rotate(angle).translate(-x,-y));
    dAngleAttack = angle;
}

void Spaceship::attack()
{
    if(isFrozen)
        return;

    int l_x = getXPositionFire();
    int l_y = getYPositionFire();

	if(player == Player1)
		l_y -= 9;
	else
		l_y += 9;
    gameEngine->soundEngine()->playSound(ShootSound);

    qreal angle = dAngleAttack*M_PI/180.0;

    switch(type)
    {
        case ProjSimple:
            gameEngine->addProjectile(new ProjectileSimple(l_x,l_y,player,angle));
            break;

        case ProjCross:
            for(int i = 0;i<3;i++)
                gameEngine->addProjectile(new ProjectileCross(l_x,l_y,player,i-1,angle));
            break;

        case ProjV:
            gameEngine->addProjectile(new ProjectileV(l_x,l_y,player,AMPLI_SPACESHIP_PROJ_V,OMEGA_SPACESHIP_PROJ_V,angle));
            break;
        case ProjAlien:
            break;

        case Nothing:
            break;
    }
}

void Spaceship::top()
{
    if(isGuided)
        projectileGuided->top();
    else
        if(!isFrozen)
        {
            if((pos().y()-dSpeed)<=gameEngine->displayEngine()->sceneSize().y()+GameEngine::offset)
                setPos(pos().x(),gameEngine->displayEngine()->sceneSize().y()+GameEngine::offset);
            else
                setPos(pos().x(),pos().y()-dSpeed);
        }
}

void Spaceship::bottom()
{
    if(isGuided)
        projectileGuided->bottom();
    else
        if(!isFrozen)
        {
            if((pos().y()+getPixmap()->height()+dSpeed)>=gameEngine->displayEngine()->sceneSize().height())
                setPos(pos().x(),gameEngine->displayEngine()->sceneSize().height()-getPixmap()->height());
            else
                setPos(pos().x(),pos().y()+dSpeed);
        }
}

void Spaceship::advance(int _step)
{
    if (!_step)
        return;
}

void Spaceship::triggerSpecialAttack()
{
    if(hud->getBonusState(player, aSpecialBonus) == BonusStateReady)
    {
        specialBonus->trigger();
        hud->activateBonus(player, aSpecialBonus);
    }
}
