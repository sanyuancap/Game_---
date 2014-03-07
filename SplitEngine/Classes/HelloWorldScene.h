//
//  HelloWorldScene.h
//  SplitEngine
//
//  Created by 子龙山人  on 12-3-26.
//  Copyright __MyCompanyName__ 2012年. All rights reserved.
//
#ifndef __HELLO_WORLD_H__
#define __HELLO_WORLD_H__

// When you import this file, you import all the cocos2d classes
#include "cocos2d.h"
#include "Box2D.h"



using namespace cocos2d;


class GLESDebugDraw;

struct Segment
{
    b2Vec2 p1;
    b2Vec2 p2;
};


class HelloWorld : public cocos2d::CCLayer ,public b2RayCastCallback{
public:
    ~HelloWorld();
    HelloWorld();
    
    // returns a Scene that contains the HelloWorld as the only child
    static cocos2d::CCScene* scene();
    
    // adds a new sprite at a given coordinate
    void addNewSpriteWithCoords(cocos2d::CCPoint p);
    virtual void draw();
    virtual void ccTouchesEnded(cocos2d::CCSet* touches, cocos2d::CCEvent* event);
    virtual void ccTouchesBegan(CCSet* touches, CCEvent* event);
    virtual void ccTouchesMoved(CCSet* touches, CCEvent* event);
    virtual float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point,
                                  const b2Vec2& normal, float32 fraction);
    
    void tick(cocos2d::ccTime dt);
    
    b2Vec2 findCenter(vector<b2Vec2>& vertices,int count);
    void createSlice(vector<b2Vec2>& vertices, int count);
    void printVertices(vector<b2Vec2>& vertices);
    
    b2Vec2 toBox2dPoint(CCPoint pt);
    CCPoint toCocos2dPoint(b2Vec2& pt);
    
private:
    GLESDebugDraw *m_debugDraw;
    
    b2World* world;
    b2Body* m_body;
    
    b2Body *m_groundBody;
    
    bool    m_bIsDraw;
    CCPoint m_startPt;
    CCPoint m_endPoint;
    
    bool    m_bIsRayCast;
    Segment* laserSegment;
    
    
    vector<b2Vec2> entryPoint;
    vector<b2Body*> affectedByLaser;
    bool m_bSliceObject;
    
    
    //temp
    bool m_bIsDrawEnter;
    b2Vec2 enterPoint;
};

#endif // __HELLO_WORLD_H__
