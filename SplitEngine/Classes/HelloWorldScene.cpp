//
//  HelloWorldScene.cpp
//  SplitEngine
//
//  Created by 子龙山人 on 12-3-26.
//  Copyright __MyCompanyName__ 2012年. All rights reserved.
//
#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"

#include "GLES-Render.h"

using namespace cocos2d;
using namespace CocosDenshion;

#define PTM_RATIO 32
enum 
{
	kTagTileMap = 1,
	kTagSpriteManager = 1,
	kTagAnimation1 = 1,
}; 



HelloWorld::HelloWorld()
{
	setIsTouchEnabled( true );
	setIsAccelerometerEnabled( true );
    
	CCSize screenSize = CCDirector::sharedDirector()->getWinSize();
	//UXLOG(L"Screen width %0.2f screen height %0.2f",screenSize.width,screenSize.height);
    
	// Define the gravity vector.
	b2Vec2 gravity;
	gravity.Set(0.0f, -10.0f);
	
	// Do we want to let bodies sleep?
	bool doSleep = true;
    
	// Construct a world object, which will hold and simulate the rigid bodies.
	world = new b2World(gravity);
    world->SetAllowSleeping(doSleep);    
	world->SetContinuousPhysics(true);
    
    
     m_debugDraw = new GLESDebugDraw( PTM_RATIO );
     world->SetDebugDraw(m_debugDraw);
     
     int flags = 0;
     flags += b2Draw::e_shapeBit;
//     flags += b2Draw::e_jointBit;
//     flags += b2Draw::e_aabbBit;
//     flags += b2Draw::e_pairBit;
//     flags += b2Draw::e_centerOfMassBit;
     m_debugDraw->SetFlags(flags);		
     
	
	b2BodyDef groundBodyDef;
	groundBodyDef.position.Set(screenSize.width/2/PTM_RATIO, 
                               screenSize.height/2/PTM_RATIO); // bottom-left corner
	
	// Call the body factory which allocates memory for the ground body
	// from a pool and creates the ground box shape (also from a pool).
	// The body is also added to the world.
	m_groundBody = world->CreateBody(&groundBodyDef);
    
	
	// Define the ground box shape.
	b2PolygonShape groundBox;		
	
    // bottom
    groundBox.SetAsBox(screenSize.width/2/PTM_RATIO, 40 / PTM_RATIO,
                       b2Vec2(0, -screenSize.height/2/PTM_RATIO), 0);
 	m_groundBody->CreateFixture(&groundBox, 0);
	
    // top
//    groundBox.SetAsBox(screenSize.width/2/PTM_RATIO, 0, b2Vec2(0, screenSize.height/2/PTM_RATIO), 0);
//    groundBody->CreateFixture(&groundBox, 0);
    
    // left
    groundBox.SetAsBox(0, screenSize.height/2/PTM_RATIO, b2Vec2(-screenSize.width/2/PTM_RATIO, 0), 0);
    m_groundBody->CreateFixture(&groundBox, 0);
    
    // right
    groundBox.SetAsBox(0, screenSize.height/2/PTM_RATIO, b2Vec2(screenSize.width/2/PTM_RATIO, 0), 0);
    m_groundBody->CreateFixture(&groundBox, 0);
    
    
    
    
	
	//Set up sprite
	CCSpriteBatchNode *mgr = CCSpriteBatchNode::batchNodeWithFile("blocks.png", 150);
	addChild(mgr, -1, kTagSpriteManager);
	
	addNewSpriteWithCoords( CCPointMake(screenSize.width/2, screenSize.height / 2) );
    
//    srand(time(NULL));
//    m_body->ApplyLinearImpulse(b2Vec2(rand() % 10 + 1,rand() % 40 + 13), m_body->GetWorldCenter());
	
    m_bIsDraw = false;
    laserSegment = NULL;
	
	schedule( schedule_selector(HelloWorld::tick) );
}

HelloWorld::~HelloWorld()
{
	delete world;
	world = NULL;
	
	delete m_debugDraw;
    m_debugDraw = NULL;
}

b2Vec2 HelloWorld::toBox2dPoint(cocos2d::CCPoint pt)
{
    return b2Vec2(pt.x / PTM_RATIO, pt.y / PTM_RATIO);
}

CCPoint HelloWorld::toCocos2dPoint(b2Vec2 &pt)
{
    return ccp(pt.x * PTM_RATIO, pt.y * PTM_RATIO);
}



void HelloWorld::draw()
{
	// Default GL states: GL_TEXTURE_2D, GL_VERTEX_ARRAY, GL_COLOR_ARRAY, GL_TEXTURE_COORD_ARRAY
	// Needed states:  GL_VERTEX_ARRAY, 
	// Unneeded states: GL_TEXTURE_2D, GL_COLOR_ARRAY, GL_TEXTURE_COORD_ARRAY
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
	
    world->DrawDebugData();
    
    glClearColor(0.5, 0.5, 0.5, 0.5);
    
    //画红色的激光
    glColor4f(1, 0, 0, 1);
    if (m_bIsDraw) {
        ccDrawLine(m_startPt, m_endPoint);
    }
//    
//    //画激光的入口点
//    if (m_bIsDrawEnter) {
//        b2Vec2 center = b2Vec2_zero;
//        for (int i=0; i < entryPoint.size(); ++i) {
//            b2Vec2 entryPt = entryPoint[i];
//            center += 0.5 * entryPt;
//            CCPoint pt = ccp( entryPt.x * PTM_RATIO, entryPt.y * PTM_RATIO);
//            ccDrawCircle(pt, 7.0f, 0, 40, false);
//        }
//        
//        if (entryPoint.size() == 2) {
//            CCPoint pt = ccp( center.x * PTM_RATIO, center.y * PTM_RATIO);
//            ccDrawCircle(pt, 7.0f, 0, 40, false);
//        }
//    }


	
	// restore default GL states
	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);	
}

float32 HelloWorld::ReportFixture(b2Fixture *fixture, const b2Vec2 &point, const b2Vec2 &normal, float32 fraction)
{
    b2Body *affectedBody = fixture->GetBody();
    
    if(m_groundBody == affectedBody){
        return -1;
    }
    
//    vector<b2Body*>::iterator fixtureIndex = std::find(affectedByLaser.begin(),
//                                                       affectedByLaser.end(), affectedBody);

    int fixtureIndex = -1;
    for (int index = 0; index < affectedByLaser.size(); ++index) {
        if (affectedBody == affectedByLaser[index]) {
            fixtureIndex = index;
            cout<<"fixtureIndex = "<<fixtureIndex<<endl;
            break;
        }
    }
    
    
     b2PolygonShape* affectedPolygon = (b2PolygonShape*)fixture->GetShape();
    if (-1 == fixtureIndex) {
        cout<<"first point\n";
        affectedByLaser.push_back(affectedBody);
        entryPoint.push_back(point);
    }else{
//        m_bIsDrawEnter = true;
        
        cout<<"print laser vertices\n";
        this->printVertices(entryPoint);
        
        b2Vec2 rayCenter = b2Vec2( (point.x + entryPoint[fixtureIndex].x) / 2.0f,
                                  (point.y + entryPoint[fixtureIndex].y) / 2.0f);
        
        cout<<"previous.x= "<<entryPoint[fixtureIndex].x * PTM_RATIO<<", y = "<<entryPoint[fixtureIndex].y * PTM_RATIO<<endl;
        cout<<"point.x= "<<point.x * PTM_RATIO<<", y = "<<point.y * PTM_RATIO<<endl;
        cout<<"rayCenter.x= "<<rayCenter.x * PTM_RATIO<<", y = "<<rayCenter.y * PTM_RATIO<<endl;
        
        float rayAngle = b2Atan2(entryPoint[fixtureIndex].y - point.y, 
                                 entryPoint[fixtureIndex].x - point.x);
        
        //得到此多边形的顶点
        vector<b2Vec2> polyVertices;
        b2Transform xf = affectedBody->GetTransform();
        int32 vertexCount = affectedPolygon->m_vertexCount;
        b2Assert(vertexCount <= b2_maxPolygonVertices);
        for (int32 i = 0; i < vertexCount; ++i)
        {
            b2Vec2 vertice = b2Mul(xf, affectedPolygon->m_vertices[i]);
            polyVertices.push_back(vertice);
        }
        
        cout<<"print polygon vertices\n";
        this->printVertices(polyVertices);
        
        vector<b2Vec2> newPolyVertices1;
        vector<b2Vec2> newPolyVertices2;
        int currentPoly = 0;
        bool cutPlaced1 = false;
        bool cutPlaced2 = false;
        
        for (int i=0; i<polyVertices.size(); i++) {
//            b2Vec2 worldPoint = affectedBody->GetWorldPoint(polyVertices[i]);
            b2Vec2 worldPoint = polyVertices[i];
            float cutAngle = b2Atan2(worldPoint.y - rayCenter.y, 
                                     worldPoint.x - rayCenter.x) - rayAngle;

            if (cutAngle < M_PI * -1) {
                cutAngle += 2 * M_PI;
            }
            if (cutAngle > 0 && cutAngle <= M_PI) {
                if (currentPoly==2) {
                    cutPlaced1=true;
                    newPolyVertices1.push_back(point);
                    newPolyVertices1.push_back(entryPoint[fixtureIndex]);
                }
                newPolyVertices1.push_back(worldPoint);
                currentPoly=1;
            } else {
                if (currentPoly==1) {
                    cutPlaced2=true;
                    newPolyVertices2.push_back(entryPoint[fixtureIndex]);
                    newPolyVertices2.push_back(point);
                }
                newPolyVertices2.push_back(worldPoint);
                currentPoly=2;
                
            }
        }
        if (! cutPlaced1) {
            newPolyVertices1.push_back(point);
            newPolyVertices1.push_back(entryPoint[fixtureIndex]);
        }
        if (! cutPlaced2) {
            newPolyVertices2.push_back(entryPoint[fixtureIndex]);
            newPolyVertices2.push_back(point);
        }
        
        cout<<"print new polygon1:\n";
        this->printVertices(newPolyVertices1);
        cout<<"print new plygon2:\n";
        this->printVertices(newPolyVertices2);
        
        
        this->createSlice(newPolyVertices1,newPolyVertices1.size());
        this->createSlice(newPolyVertices2,newPolyVertices2.size());
        world->DestroyBody(affectedBody);
        
    }
    
    return 1;

}


void HelloWorld::printVertices(vector<b2Vec2> &vertices)
{
    for (int i = 0; i < vertices.size(); ++i) {
        cout<<"x="<<vertices[i].x * PTM_RATIO<<",y="<<vertices[i].y * PTM_RATIO<<endl;
    }
    cout<<"end of print"<<endl;
}

b2Vec2 HelloWorld::findCenter(vector<b2Vec2>& vertices, int count)
{
    b2Vec2 c;
    float area = 0.0f;
    float p1x = 0.0f;
    float p1y = 0.0f;
    float inv3 = 1.0f / 3.0f;
    for (int i=0; i < count; ++i) {
        b2Vec2 p2 = vertices[i];
        b2Vec2 p3 = (i + 1) < count ? vertices[i+1] : vertices[0];
        float e1x = p2.x - p1x;
        float e1y = p2.y - p1y;
        float e2x = p3.x - p1x;
        float e2y = p3.y - p1y;
        float d = (e1x * e2y - e1y * e2x);
        float triangleArea = 0.5f * d;
        area += triangleArea;
        c.x += triangleArea * inv3 * (p1x + p2.x + p3.x);
        c.y += triangleArea * inv3 * (p1y + p2.y + p3.y);
    }
    c.x *= 1.0 / area;
    c.y *= 1.0 / area;
    return c;
}

void HelloWorld::createSlice(vector<b2Vec2> &vertices, int count)
{
    //找到刚体的中心点
    b2Vec2 centre = this->findCenter(vertices, vertices.size());
    
    cout<<"centre:  x = "<<centre.x<<" y = "<<centre.y<<endl;
    
    //if y != y is true, then y's value is nan
    if (  centre.x < -50 || centre.x > 50 || centre.y > 50 || centre.y < -50 || centre.y != centre.y
        || count > b2_maxPolygonVertices) {
        cout<<"slice too small!"<<endl;
        return;
    }
    
    for (int i=0; i< count; ++i) {
        vertices[i] -= centre;
    }
    
    b2BodyDef sliceBody;
    sliceBody.position.Set(centre.x, centre.y);
    sliceBody.type = b2_dynamicBody;
        
    
    b2PolygonShape slicePoly;
    b2Vec2 arrVertices[b2_maxPolygonVertices];
//    cout<<sizeof(arrVertices) / sizeof(arrVertices[0])<<endl;
    cout<<"create slice count = "<<count<<endl;
    std::copy(vertices.begin(), vertices.end(), arrVertices);
    slicePoly.Set(arrVertices, count);
    b2FixtureDef sliceFixture;
    sliceFixture.shape = &slicePoly;
    sliceFixture.density = 1;
    b2Body *pSliceBody = world->CreateBody(&sliceBody);
        
    pSliceBody->CreateFixture(&sliceFixture);
    for (int i=0; i < count; ++i) {
        vertices[i] += centre;
    }
}




void HelloWorld::tick(ccTime dt)
{
	//It is recommended that a fixed time step is used with Box2D for stability
	//of the simulation, however, we are using a variable time step here.
	//You need to make an informed choice, the following URL is useful
	//http://gafferongames.com/game-physics/fix-your-timestep/
	
	int velocityIterations = 8;
	int positionIterations = 1;
    
	// Instruct the world to perform a single step of simulation. It is
	// generally best to keep the time step and iterations fixed.
	world->Step(dt, velocityIterations, positionIterations);
    
    //防止dynamic tree error
    if (m_bIsRayCast && NULL != laserSegment && (laserSegment->p2 - laserSegment->p1).LengthSquared() > 0) {
        
        entryPoint.clear();
        affectedByLaser.clear();
        
        world->RayCast(this, laserSegment->p1,laserSegment->p2);
        
        world->RayCast(this, laserSegment->p2,laserSegment->p1);
        
        delete laserSegment;
        laserSegment = NULL;
        
        
    }  	
    
	//Iterate over the bodies in the physics world
	for (b2Body* b = world->GetBodyList(); b; b = b->GetNext())
	{
		if (b->GetUserData() != NULL) {
			//Synchronize the AtlasSprites position and rotation with the corresponding body
//			CCSprite* myActor = (CCSprite*)b->GetUserData();
//            
//			myActor->setPosition( CCPointMake( b->GetPosition().x * PTM_RATIO, b->GetPosition().y * PTM_RATIO) );
//			myActor->setRotation( -1 * CC_RADIANS_TO_DEGREES(b->GetAngle()) );
		}	
	}
}


void HelloWorld::ccTouchesBegan(cocos2d::CCSet *touches, cocos2d::CCEvent *event){
     m_bSliceObject = false;
    
    cout<<"touch begin"<<endl;
    laserSegment = new Segment;
    m_bIsRayCast = false;
    m_bIsDraw = false;
    m_bIsDrawEnter = false;

    
    CCTouch *touch = (CCTouch*)touches->anyObject();
    m_startPt = CCDirector::sharedDirector()->convertToGL(touch->locationInView());
    m_endPoint = m_startPt;
    
    laserSegment->p1 = b2Vec2(m_startPt.x / PTM_RATIO,m_startPt.y / PTM_RATIO);
    
//    m_laserFired1.isDrawCircle = false;
//    m_laserFired2.isDrawCircle = false;
}

void HelloWorld::ccTouchesMoved(cocos2d::CCSet *touches, cocos2d::CCEvent *event){
    CCTouch *touch = (CCTouch*)touches->anyObject();
    m_endPoint = CCDirector::sharedDirector()->convertToGL(touch->locationInView());
    m_bIsDraw = true;
}

void HelloWorld::ccTouchesEnded(CCSet* touches, CCEvent* event)
{
	//Add a new body/atlas sprite at the touched location
	CCSetIterator it;
	CCTouch* touch;
    
	for( it = touches->begin(); it != touches->end(); it++) 
	{
		touch = (CCTouch*)(*it);
        
		if(!touch)
			break;
        
		CCPoint location = touch->locationInView();
		
		location = CCDirector::sharedDirector()->convertToGL(location);
        m_endPoint = location;
        
        laserSegment->p2 = b2Vec2(m_endPoint.x / PTM_RATIO, m_endPoint.y / PTM_RATIO);
        
        m_bIsDraw = false;
        m_bIsRayCast = true;
//        m_laserFired1.isRayCasted = false;
//        m_laserFired2.isRayCasted = false;
//        m_laserFired1.isDrawCircle = false;
//        m_laserFired2.isDrawCircle = false;
    }
}

CCScene* HelloWorld::scene()
{
    // 'scene' is an autorelease object
    CCScene *scene = CCScene::node();
    
    // add layer as a child to scene
    CCLayer* layer = new HelloWorld();
    scene->addChild(layer);
    layer->release();
    
    return scene;
}


void HelloWorld::addNewSpriteWithCoords(CCPoint p)
{
	//UXLOG(L"Add sprite %0.2f x %02.f",p.x,p.y);
	CCSpriteBatchNode* sheet = (CCSpriteBatchNode*)getChildByTag(kTagSpriteManager);
	
	//We have a 64x64 sprite sheet with 4 different 32x32 images.  The following code is
	//just randomly picking one of the images
    //	int idx = (CCRANDOM_0_1() > .5 ? 0:1);
    //	int idy = (CCRANDOM_0_1() > .5 ? 0:1);
    
    const int spriteSize = 64;
	CCSprite *sprite = CCSprite::spriteWithBatchNode(sheet, CCRectMake(0,0,spriteSize,spriteSize));
    sprite->setTag(101);
	sheet->addChild(sprite);
    sprite->setIsVisible(false);
	
	sprite->setPosition( CCPointMake( p.x, p.y) );
	
	// Define the dynamic body.
	//Set up a 1m squared box in the physics world
	b2BodyDef bodyDef;
//	bodyDef.type = b2_dynamicBody;
    bodyDef.type = b2_staticBody;
	bodyDef.position.Set(p.x/PTM_RATIO, p.y/PTM_RATIO);
    //	bodyDef.userData = sprite;
    bodyDef.allowSleep = true;
	m_body = world->CreateBody(&bodyDef);
    
    
	
	// Define another box shape for our dynamic body.
	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox(2.0f, 2.0f);//These are mid points for our 1m box
	
	// Define the dynamic body fixture.
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &dynamicBox;	
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.3f;
	m_body->CreateFixture(&fixtureDef);
}
