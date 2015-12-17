/*
-----------------------------------------------------------------------------
Filename:    TutorialApplication.cpp
-----------------------------------------------------------------------------

This source file is part of the
   ___                 __    __ _ _    _
  /___\__ _ _ __ ___  / / /\ \ (_) | _(_)
 //  // _` | '__/ _ \ \ \/  \/ / | |/ / |
/ \_// (_| | | |  __/  \  /\  /| |   <| |
\___/ \__, |_|  \___|   \/  \/ |_|_|\_\_|
      |___/
Tutorial Framework (for Ogre 1.9)
http://www.ogre3d.org/wiki/
-----------------------------------------------------------------------------
*/

#include "TutorialApplication.h"
#include <irrKlang.h>

using namespace irrklang;
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "irrKlang.lib") // link with irrKlang.dll
//---------------------------------------------------------------------------
TutorialApplication::TutorialApplication(void): mTerrainGroup(0), mTerrainGlobals(0), mInfoLabel(0)
{
}
//---------------------------------------------------------------------------
TutorialApplication::~TutorialApplication(void)
{
}

//---------------------------------------------------------------------------
void TutorialApplication::createScene(void)
{
    // Create your scene here :)
	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));

	//간단 조명설정
	//Ogre::Light* light = mSceneMgr->createLight("MainLight");
	//light->setPosition(20, 80, 50);


	ISoundEngine* engine = createIrrKlangDevice();
	engine->play2D("../../media/getout.ogg", true); //왜 재생이 안되지...

	

	Ogre::Entity* ogreEntity = mSceneMgr->createEntity("ninja.mesh");
	Ogre::SceneNode* ogreNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(Ogre::Vector3(1684, 40, 2116));
	ogreNode->attachObject(ogreEntity);


	Ogre::Entity* ogreEntity2 = mSceneMgr->createEntity("ogrehead.mesh");
	Ogre::SceneNode* ogreNode2 = ogreNode->createChildSceneNode(Ogre::Vector3(1683, 50, 2116));
	ogreNode2->attachObject(ogreEntity2);


	Ogre::Entity* ninjaEntity = mSceneMgr->createEntity("ninja.mesh");
	Ogre::SceneNode* ninjaNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("NinjaNode");
	ninjaNode->attachObject(ninjaEntity);

	////플레인생성
	//Ogre::Plane plane(Ogre::Vector3::UNIT_Y, 0);
	//Ogre::MeshManager::getSingleton().createPlane(
	//	"ground",
	//	Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
	//	plane, 
	//	1500, 1500, 20, 20, 
	//	true, 
	//	1, 5, 5, 
	//	Ogre::Vector3::UNIT_Z);
	//Ogre::Entity* groundEntity = mSceneMgr->createEntity("ground");
	//mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(groundEntity);
	//groundEntity->setCastShadows(false);
	//groundEntity->setMaterialName("Examples/Rockwall");

	

	//그냥 따라쓴 코드
	bool infiniteClip =
		mRoot->getRenderSystem()->getCapabilities()->hasCapability(
		Ogre::RSC_INFINITE_FAR_PLANE);

	if (infiniteClip)
		mCamera->setFarClipDistance(0);
	else
		mCamera->setFarClipDistance(50000);




	//터레인을 위한 조명설정
	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.2f, 0.2f, 0.2f));

	//Ogre::Vector3 lightdir(0.55f, -0.3f, 0.75f);
	//lightdir.normalise();

	Ogre::Light* light = mSceneMgr->createLight("TestLight");
	light->setType(Ogre::Light::LT_POINT);
	light->setPosition(250, 150, 250);
	//light->setDirection(lightdir);
	light->setDiffuseColour(Ogre::ColourValue::White);
	light->setSpecularColour(Ogre::ColourValue(0.4f, 0.4f, 0.4f));



	//그림자 주기
	mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_MODULATIVE);


	
	//안개
	Ogre::ColourValue fadeColour(0.8, 0.3, 0.3);
	mWindow->getViewport(0)->setBackgroundColour(fadeColour);
	//mSceneMgr->setFog(Ogre::FOG_LINEAR, fadeColour, 0, 600, 900); //선형 안개
	mSceneMgr->setFog(Ogre::FOG_EXP, fadeColour, 0.0003); //기하급수적 안개

	

	//터레인 작성
	mTerrainGlobals = OGRE_NEW Ogre::TerrainGlobalOptions();

	mTerrainGroup = OGRE_NEW Ogre::TerrainGroup(
		mSceneMgr, 
		Ogre::Terrain::ALIGN_X_Z, 
		513, 12000.0);
	mTerrainGroup->setFilenameConvention(Ogre::String("terrain"), Ogre::String("dat"));
	mTerrainGroup->setOrigin(Ogre::Vector3::ZERO);

	configureTerrainDefaults(light);

	for (long x = 0; x <= 0; ++x)
		for (long y = 0; y <= 0; ++y)
			defineTerrain(x, y);

	mTerrainGroup->loadAllTerrains(true);


	if (mTerrainsImported)
	{
		Ogre::TerrainGroup::TerrainIterator ti = mTerrainGroup->getTerrainIterator();

		while (ti.hasMoreElements())
		{
			Ogre::Terrain* t = ti.getNext()->instance;
			initBlendMaps(t);
		}
	}

	mTerrainGroup->freeTemporaryResources();


	//스카이박스
	mSceneMgr->setSkyBox(true, "Examples/SpaceSkyBox", 3000, false);
	engine->drop();
}
//---------------------------------------------------------------------------

void TutorialApplication::createFrameListener()
{
	BaseApplication::createFrameListener();
}

void TutorialApplication::destroyScene()
{
	OGRE_DELETE mTerrainGroup;
	OGRE_DELETE mTerrainGlobals;
}


void getTerrainImage(bool flipX, bool flipY, Ogre::Image& img)
{
	img.load("terrain.png", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	if (flipX)
		img.flipAroundY();
	if (flipY)
		img.flipAroundX();

}

void TutorialApplication::defineTerrain(long x, long y)
{
	Ogre::String filename = mTerrainGroup->generateFilename(x, y);

	bool exists =
		Ogre::ResourceGroupManager::getSingleton().resourceExists(
		mTerrainGroup->getResourceGroup(),
		filename);

	if (exists)
		mTerrainGroup->defineTerrain(x, y);
	else
	{
		Ogre::Image img;
		getTerrainImage(x % 2 != 0, y % 2 != 0, img);
		mTerrainGroup->defineTerrain(x, y, &img);

		mTerrainsImported = true;
	}


}

void TutorialApplication::initBlendMaps(Ogre::Terrain* terrain)
{
	Ogre::Real minHeight0 = 70;
	Ogre::Real fadeDist0 = 40;
	Ogre::Real minHeight1 = 70;
	Ogre::Real fadeDist1 = 15;

	Ogre::TerrainLayerBlendMap* blendMap0 = terrain->getLayerBlendMap(1);
	Ogre::TerrainLayerBlendMap* blendMap1 = terrain->getLayerBlendMap(2);

	float* pBlend0 = blendMap0->getBlendPointer();
	float* pBlend1 = blendMap1->getBlendPointer();

	for (Ogre::uint16 y = 0; y < terrain->getLayerBlendMapSize(); ++y)
	{
		for (Ogre::uint16 x = 0; x < terrain->getLayerBlendMapSize(); ++x)
		{
			Ogre::Real tx, ty;

			blendMap0->convertImageToTerrainSpace(x, y, &tx, &ty);
			Ogre::Real height = terrain->getHeightAtTerrainPosition(tx, ty);
			Ogre::Real val = (height - minHeight0) / fadeDist0;
			val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
			*pBlend0++ = val;

			val = (height - minHeight1) / fadeDist1;
			val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
			*pBlend1++ = val;
		}
	}

	blendMap0->dirty();
	blendMap1->dirty();
	blendMap0->update();
	blendMap1->update();
}


void TutorialApplication::configureTerrainDefaults(Ogre::Light* light)
{
	mTerrainGlobals->setMaxPixelError(8);
	mTerrainGlobals->setCompositeMapDistance(3000);
	mTerrainGlobals->setLightMapDirection(light->getDerivedDirection());
	mTerrainGlobals->setCompositeMapAmbient(mSceneMgr->getAmbientLight());
	mTerrainGlobals->setCompositeMapDiffuse(light->getDiffuseColour());


	Ogre::Terrain::ImportData& importData = mTerrainGroup->getDefaultImportSettings();
	importData.terrainSize = 513;
	importData.worldSize = 12000.0;
	importData.inputScale = 600;
	importData.minBatchSize = 33;
	importData.maxBatchSize = 65;


	importData.layerList.resize(3);

	importData.layerList[0].worldSize = 100;
	importData.layerList[0].textureNames.push_back(
		"dirt_grayrocky_diffusespecular.dds");
	importData.layerList[0].textureNames.push_back(
		"dirt_grayrocky_normalheight.dds");
	importData.layerList[1].worldSize = 30;
	importData.layerList[1].textureNames.push_back(
		"grass_green-01_diffusespecular.dds");
	importData.layerList[1].textureNames.push_back(
		"grass_green-01_normalheight.dds");
	importData.layerList[2].worldSize = 200;
	importData.layerList[2].textureNames.push_back(
		"growth_weirdfungus-03_diffusespecular.dds");
	importData.layerList[2].textureNames.push_back(
		"growth_weirdfungus-03_normalheight.dds");
}


bool TutorialApplication::frameRenderingQueued(const Ogre::FrameEvent& fe)
{
  if(!processUnbufferedInput(fe))
		return false;

	bool ret = BaseApplication::frameRenderingQueued(fe);

	
	

	return ret;
}
 

bool TutorialApplication::processUnbufferedInput(const Ogre::FrameEvent& fe)
{
	static bool mouseDownLastFrame = false;
	static Ogre::Real toggleTimer = 0.0;
	static Ogre::Real rotate = .13;
	static Ogre::Real move = 250;

	bool leftMouseDown = mMouse->getMouseState().buttonDown(OIS::MB_Left);
	if (leftMouseDown && !mouseDownLastFrame)
	{
		Ogre::Light* light = mSceneMgr->getLight("TestLight");
		light->setVisible(!light->isVisible());
	}
	mouseDownLastFrame = leftMouseDown;


	toggleTimer -= fe.timeSinceLastFrame;
	if ((toggleTimer < 0) && mMouse->getMouseState().buttonDown(OIS::MB_Right))
	{
		toggleTimer  = 0.5;

		Ogre::Light* light = mSceneMgr->getLight("TestLight");
		light->setVisible(!light->isVisible());
	}

	Ogre::Vector3 dirVec = Ogre::Vector3::ZERO;
	if (mKeyboard->isKeyDown(OIS::KC_I))
		dirVec.z -= move;
	if (mKeyboard->isKeyDown(OIS::KC_K))
		dirVec.z += move;
	if (mKeyboard->isKeyDown(OIS::KC_U))
		dirVec.y += move;
	if (mKeyboard->isKeyDown(OIS::KC_O))
		dirVec.y -= move;

	if (mKeyboard->isKeyDown(OIS::KC_J))
	{    
		if(mKeyboard->isKeyDown(OIS::KC_LSHIFT))
			mSceneMgr->getSceneNode("NinjaNode")->yaw(Ogre::Degree(5 * rotate));
		else
			dirVec.x -= move;
	}

	if (mKeyboard->isKeyDown(OIS::KC_L))
	{
		if(mKeyboard->isKeyDown(OIS::KC_LSHIFT))
			mSceneMgr->getSceneNode("NinjaNode")->yaw(Ogre::Degree(-5 * rotate));
		else
			dirVec.x += move;
	}


	mSceneMgr->getSceneNode("NinjaNode")->translate(
		dirVec * fe.timeSinceLastFrame,
		Ogre::Node::TS_LOCAL);


	return true;
}


#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
    int main(int argc, char *argv[])
#endif
    {
        // Create application object
        TutorialApplication app;

        try {
            app.go();
        } catch(Ogre::Exception& e)  {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            MessageBox(NULL, e.getFullDescription().c_str(), "An exception has occurred!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
            std::cerr << "An exception has occurred: " <<
                e.getFullDescription().c_str() << std::endl;
#endif
        }

        return 0;
    }

#ifdef __cplusplus
}
#endif

//---------------------------------------------------------------------------
