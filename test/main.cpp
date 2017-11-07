/**
 * @file main.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2001-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include <o3d/core/memorymanager.h>
#include <o3d/core/main.h>
#include <o3d/core/keyboard.h>
#include <o3d/core/appwindow.h>
#include <o3d/engine/utils/framemanager.h>
#include <o3d/engine/context.h>
#include <o3d/engine/hierarchy/node.h>
#include <o3d/engine/hierarchy/hierarchytree.h>
#include <o3d/engine/screenviewport.h>
#include <o3d/engine/viewportmanager.h>
#include <o3d/engine/scene/scene.h>
#include <o3d/engine/object/light.h>
#include <o3d/engine/object/skin.h>
#include <o3d/engine/scene/sceneobjectmanager.h>
#include <o3d/engine/renderer.h>
#include <o3d/engine/deferred/deferreddrawer.h>
#include <o3d/engine/animation/animationplayermanager.h>
#include <o3d/engine/animation/animationmanager.h>

#include <o3d/engine/object/primitive.h>
#include <o3d/engine/object/ftransform.h>
#include <o3d/engine/object/mtransform.h>
#include <o3d/engine/object/camera.h>
#include <o3d/engine/picking.h>

#include <o3d/gui/gui.h>
#include <o3d/gui/truetypefont.h>
#include <o3d/gui/fontmanager.h>
#include <o3d/gui/thememanager.h>
#include <o3d/gui/widgetmanager.h>

#include <o3d/core/commandline.h>
#include <o3d/core/diskfileinfo.h>
#include <o3d/core/diskdir.h>
#include <o3d/core/filemanager.h>
#include <o3d/core/application.h>

#include "o3d/collada/collada.h"

using namespace o3d;
using namespace o3d::collada;

class KeyMapping
{
public:

    enum Keys
    {
        LEFT = 0,
        RIGHT,
        UP,
        DOWN = 3,
        FORWARD,
        BACKWARD
    };

    inline VKey left() const { return m_keys[LEFT]; }
    inline VKey right() const { return m_keys[RIGHT]; }
    inline VKey up() const { return m_keys[UP]; }
    inline VKey down() const { return m_keys[DOWN]; }
    inline VKey forward() const { return m_keys[FORWARD]; }
    inline VKey backward() const { return m_keys[BACKWARD]; }

    inline VKey key(UInt32 c) { return m_keys[c]; }

    inline UInt32 type() { return m_type; }

protected:

    UInt32 m_type;

    VKey m_keys[255];
};

struct KeyMapAzerty : public KeyMapping
{
public:

    KeyMapAzerty()
    {
        m_type = 0;

        m_keys[LEFT] = KEY_S;
        m_keys[RIGHT] = KEY_F;

        m_keys[UP] = KEY_A;
        m_keys[DOWN] = KEY_Q;

        m_keys[FORWARD] = KEY_E;
        m_keys[BACKWARD] = KEY_D;
    }
};

struct KeyMapQwerty : public KeyMapping
{
public:

    KeyMapQwerty()
    {
        m_type = 1;

        m_keys[LEFT] = KEY_S;
        m_keys[RIGHT] = KEY_F;

        m_keys[UP] = KEY_Q;
        m_keys[DOWN] = KEY_W;

        m_keys[FORWARD] = KEY_E;
        m_keys[BACKWARD] = KEY_D;
    }
};

struct KeyMapBepo : public KeyMapping
{
public:

    KeyMapBepo()
    {
        m_type = 2;

        m_keys[LEFT] = KEY_U;
        m_keys[RIGHT] = KEY_E;

        m_keys[UP] = KEY_B;
        m_keys[DOWN] = KEY_A;

        m_keys[FORWARD] = KEY_P;
        m_keys[BACKWARD] = KEY_I;
    }
};

class ColladaTest : public EvtHandler
{
private:

    AppWindow *m_appWindow;
    Renderer* m_glRenderer;
    Scene *m_scene;
    Gui *m_gui;
    AutoPtr<KeyMapping> m_keys;

public:

	ColladaTest(const String &sceneRoot):
		m_speed(20.f),
        m_font(nullptr),
		m_rotateCam(False),
        m_nodeObject(nullptr),
        m_picked(nullptr)
    {
        m_keys = new KeyMapAzerty;

        // Create a new window
        m_appWindow = new AppWindow;

        // OpenGL renderer
        m_glRenderer = new Renderer;

        m_appWindow->setTitle("Objective-3D Collada viewer");
        m_appWindow->create(800, 600, AppWindow::COLOR_RGBA8, AppWindow::DEPTH_24_STENCIL_8, AppWindow::MSAA16X, False, True);

        m_glRenderer->create(m_appWindow);

        // create a scene and attach it to the window
        m_scene = new Scene(nullptr, sceneRoot, m_glRenderer);
        m_scene->setSceneName("collada");
        m_scene->defaultAttachment(m_appWindow);

        // new gui manager and attach it to the scene
        m_gui = new Gui(m_scene);
        m_scene->setGui(m_gui);
        m_gui->defaultAttachment(m_appWindow);

		//getWindow()->setPaintFreq(16);
		
		// We listen synchronously to each update event coming from the main window.
		// The first parameter is an helper macro that take :
		// - Object class name to listen
		// - Instance of the object to listen
		// - Name of the signal to listen
		// - Instance of the receiver (in our case this)
		// - Method called on an event
		m_appWindow->onUpdate.connect(this, &ColladaTest::onSceneUpdate);
		m_appWindow->onDraw.connect(this, &ColladaTest::onSceneDraw);
		m_appWindow->onMouseMotion.connect(this, &ColladaTest::onMouseMotion);
		m_appWindow->onClose.connect(this, &ColladaTest::onClose);
		m_appWindow->onMouseButton.connect(this, &ColladaTest::onMouseButton);
		m_appWindow->onKey.connect(this, &ColladaTest::onKey);
        m_appWindow->onDestroy.connect(this, &ColladaTest::onDestroy);

		// Notice that update and draw event of the main window (MainWindow) are
		// thrown by two timers. And that it is possible to change easily these timings.

		// Add true type font for show informations.
		DiskFileInfo fontFile("arial.ttf");
		if (fontFile.isExist())
            m_font = getGui()->getFontManager()->addTrueTypeFont("arial.ttf");
		else
            m_font = getGui()->getFontManager()->addTrueTypeFont(sceneRoot + "/gui/arial.ttf");

		m_font->setTextHeight(16);
		m_font->setColor(Color(0.f, 1.f, 0.f));

		getScene()->getPicking()->setMode(Picking::COLOR);

		// We need a mouse look to pick on the screen, so simply load a GUI theme
		Theme *theme;

		DiskFileInfo themeFile("revolutioning.xml");
		if (themeFile.isExist())
            theme = getGui()->getThemeManager()->addTheme("revolutioning.xml");
		else
            theme = getGui()->getThemeManager()->addTheme(sceneRoot + "/gui/revolutioning.xml");

		// and set it as the default theme to use
        getGui()->getWidgetManager()->setDefaultTheme(theme);
	}

	virtual ~ColladaTest()
	{
    }

    AppWindow* getWindow() { return m_appWindow; }
    Scene* getScene() { return m_scene; }
    Gui* getGui() { return m_gui; }

    void onDestroy()
    {
        deletePtr(m_scene);
        deletePtr(m_glRenderer);

        this->getWindow()->logFps();

        // it is deleted by the application
        m_appWindow = nullptr;
    }

	// Method called on main window update
	void onSceneUpdate()
	{
		// Get the keyboard object from the input manager of the main window
		Keyboard * lpKeyboard = getWindow()->getInput().getKeyboard();

		// Get the time (in ms) elapsed since the last update
		Float elapsed = getScene()->getFrameManager()->getFrameDuration();
		Float dist = 0.f;

		// Inc/dec a rotation angle depending of the left and right key states.
/*		if (lpKeyboard->isKeyDown(KEY_LEFT)) angleX = -1.f*elapsed;
		if (lpKeyboard->isKeyDown(KEY_RIGHT)) angleX = 1.f*elapsed;
		if (lpKeyboard->isKeyDown(KEY_UP)) dist = -1.f*elapsed;
		if (lpKeyboard->isKeyDown(KEY_DOWN)) dist = 1.f*elapsed;
*/
		// Transform
		//Node *node = dynamicCast(getScene()->getSceneObjectManager()->searchName("joint1"), O3DNode);
		if (m_nodeObject)
		{
			// We want to apply the rotation to this node, but by default there is no
			// transformation, so we create ones if necessary.
			if (!m_nodeObject->getTransform())
			{
				// Its a simple matrix transform using a {Position,Quaternion,Scale} uplet
				// that is transformed into a 4x4 matrix.
				MTransform *transform = new MTransform;
				m_nodeObject->addTransform(transform);
			}
		}
/*
		node = dynamicCast(getScene()->getSceneObjectManager()->searchName("joint1"), O3DNode);
		if (node)
		{
			// Rotate on the Y axis
			if (dist != 0.f)
				node->getTransform()->translate(O3DVector3(0, 0, dist*1000));;
		}
*/
		Float x = 0.f, y = 0.f, z = 0.f;

        // TODO like with MS3D sample
        if (lpKeyboard->isKeyDown(m_keys->forward())) z = -m_speed*elapsed;
        if (lpKeyboard->isKeyDown(m_keys->backward())) z = m_speed*elapsed;
        if (lpKeyboard->isKeyDown(m_keys->left())) x = -m_speed*elapsed;
        if (lpKeyboard->isKeyDown(m_keys->right())) x = m_speed*elapsed;
        if (lpKeyboard->isKeyDown(m_keys->down())) y = -m_speed*elapsed;
        if (lpKeyboard->isKeyDown(m_keys->up())) y = m_speed*elapsed;

		// move the camera using ESDFQA
		BaseNode *cameraNode = getScene()->getSceneObjectManager()->searchName("Camera")->getNode();
		if (cameraNode)
		{
			cameraNode->getTransform()->translate(Vector3(x, y, z));
		}
	}

	void onSceneDraw()
	{
		Matrix4 ortho2d;
		ortho2d.buildOrtho(0.f, (Float)getWindow()->getWidth(), (Float)getWindow()->getHeight(), 0.f, -1.f ,1.f);

		getScene()->getContext()->projection().set(ortho2d);
		getScene()->getContext()->modelView().identity();
		//getScene()->getContext()->modelView().translate(Vector3(0.5f, 0.5f, 0.f));

        m_font->write(Vector2i(5, 16), String("FPS: ") << (UInt32)getWindow()->getLastFps());
        m_font->write(Vector2i(5, 32), String("Tris/lines: ") << getScene()->getFrameManager()->getNumTriangles() << "/" << getScene()->getFrameManager()->getNumLines());
        m_font->write(Vector2i(5, 48), String("Speed: ") << m_speed);

		// Check for a hit
		if (getScene()->getPicking()->getSingleHit())
		{
			// Dynamic cast to scene object, or you can use two static_cast.
			// Casts are tested using pointer and reference.
			SceneObject *object = o3d::dynamicCast<SceneObject*>(getScene()->getPicking()->getSingleHit());
            m_nodeObject = object->getNode();

			m_picked = object;

            if (m_nodeObject)
            {
                if (m_nodeObject->getTransform())
                    m_pickedMat = m_nodeObject->getTransform()->getMatrix();
                else
                    m_pickedMat.identity();

                // Draw the name of the picked object.
                m_pickedPos = getScene()->getPicking()->getHitPos();

                // And clear all listed hits.
                getScene()->getPicking()->clearPickedList();

                Keyboard * lpKeyboard = getWindow()->getInput().getKeyboard();
                if (lpKeyboard->isKeyDown(KEY_H))
                    object->disableVisibility();
            }
		}
		
		if (m_picked)
			m_font->write(Vector2i(5, 64), m_picked->getName() + m_pickedPos);
	}

	void onMouseMotion(Mouse* mouse)
	{
		m_rotateCam = mouse->isRightDown();
		m_rotateObj = mouse->isLeftDown() && !m_rotateCam;

		if (m_rotateCam)
		{
			BaseNode *cameraNode = getScene()->getSceneObjectManager()->searchName("Camera")->getNode();
			if (cameraNode)
            {
				cameraNode->getTransform()->rotate(Y, -mouse->getDeltaX() * getScene()->getFrameManager()->getFrameDuration());
				cameraNode->getTransform()->rotate(X, -mouse->getDeltaY() * getScene()->getFrameManager()->getFrameDuration());
			}
		}

		if (m_rotateObj && m_nodeObject)
		{
            if (getWindow()->getInput().getKeyboard()->isKeyDown(KEY_LCONTROL))
			{
				if (m_nodeObject->getTransform())
				{
					if (fabs(mouse->getDeltaY()) > fabs(mouse->getDeltaX()))
						m_nodeObject->getTransform()->rotate(X, -mouse->getDeltaY() * getScene()->getFrameManager()->getFrameDuration());
					else if (fabs(mouse->getDeltaX()) > fabs(mouse->getDeltaY()))
						m_nodeObject->getTransform()->rotate(Y, -mouse->getDeltaX() * getScene()->getFrameManager()->getFrameDuration());
				}
			}
			else
			{
				if (m_nodeObject->getTransform())
				{
					m_nodeObject->getTransform()->rotate(Y, -mouse->getDeltaX() * getScene()->getFrameManager()->getFrameDuration());
					m_nodeObject->getTransform()->rotate(X, -mouse->getDeltaY() * getScene()->getFrameManager()->getFrameDuration());
				}
			}
		}
	}

	void onMouseButton(Mouse* mouse, ButtonEvent event)
	{
		m_rotateCam = mouse->isRightDown();
		m_rotateObj = mouse->isLeftDown() && !m_rotateCam;

		if (event.isPressed() && (event.button() == Mouse::LEFT))
		{
            m_picked = nullptr;

			// Process to a picking a next draw pass.
			// The mouse Y coordinate should be inverted because Y+ is on top of the screen for OpenGL.
			getScene()->getPicking()->postPickingEvent(
					mouse->getMappedPosition().x(),
                    getScene()->getViewPortManager()->getReshapeHeight() - mouse->getMappedPosition().y());
        }
	}

	void onKey(Keyboard* keyboard, KeyEvent event)
	{
        if (event.isPressed() && (event.key() == KEY_F1))
        {
            if (m_keys->type() == 0)
                m_keys = new KeyMapQwerty;
            else if (m_keys->type() == 1)
                m_keys = new KeyMapBepo;
            else if (m_keys->type() == 2)
                m_keys = new KeyMapAzerty;
        }

		if (event.isPressed() && (event.key() == KEY_W))
		{
			if (getScene()->getContext()->getDrawingMode() == Context::DRAWING_WIREFRAME)
				getScene()->getContext()->setOverrideDrawingMode(Context::DRAWING_FILLED);
			else
				getScene()->getContext()->setOverrideDrawingMode(Context::DRAWING_WIREFRAME);
		}

		if (event.isPressed() && (event.key() == KEY_N))
		{
			if (getScene()->getDrawObject(o3d::Scene::DRAW_LOCAL_SPACE))
				getScene()->setDrawObject(o3d::Scene::DRAW_LOCAL_SPACE, False);
			else
				getScene()->setDrawObject(o3d::Scene::DRAW_LOCAL_SPACE, True);
		}

		if (event.isPressed() && (event.key() == KEY_X))
		{
			if (getScene()->getDrawObject(o3d::Scene::DRAW_LOCAL_AXIS))
			{
				getScene()->setDrawObject(o3d::Scene::DRAW_LOCAL_AXIS, False);
				getScene()->setDrawObject(o3d::Scene::DRAW_MESH_LOCAL_AXIS, False);
			}
			else
			{
				getScene()->setDrawObject(o3d::Scene::DRAW_LOCAL_AXIS, True);
				getScene()->setDrawObject(o3d::Scene::DRAW_MESH_LOCAL_AXIS, True);
			}
		}

        if (event.isPressed() && (event.key() == KEY_F9))
		{
			if (getScene()->getDrawObject(o3d::Scene::DRAW_BONES))
				getScene()->setDrawObject(o3d::Scene::DRAW_BONES, False);
			else
				getScene()->setDrawObject(o3d::Scene::DRAW_BONES, True);
		}

		if (event.isPressed() && (event.key() == KEY_V))
		{
			if (getScene()->getDrawObject(o3d::Scene::DRAW_BOUNDING_VOLUME))
				getScene()->setDrawObject(o3d::Scene::DRAW_BOUNDING_VOLUME, False);
			else
				getScene()->setDrawObject(o3d::Scene::DRAW_BOUNDING_VOLUME, True);
		}
	
		if (event.isPressed() && (event.key() == KEY_L))
			m_lightObject->toggleActivity();

        if (event.isPressed() && (event.key() == KEY_H))
        {
            if (m_picked)
                m_picked->toggleVisibility();
        }

		if (event.isPressed() && (event.key() == KEY_R) && m_nodeObject)
			m_nodeObject->getTransform()->setMatrix(m_pickedMat);

		if (event.isPressed() && (event.key() == KEY_U) && m_picked)			
			m_picked->enableVisibility();

		if((event.isPressed() || event.isRepeat()) && (event.key() == KEY_NUMPAD_ADD))
		{
			m_speed += 1.f;
			m_speed = clamp(m_speed, 0.f, 100.f);
		}
		
		if ((event.isPressed() || event.isRepeat()) && (event.key() == KEY_NUMPAD_SUBTRACT))
		{
			m_speed -= 1.f;
			m_speed = clamp(m_speed, 0.f, 100.f);
		}

        if (event.isPressed() && (event.key() == KEY_SPACE))
            getScene()->getAnimationPlayerManager()->togglePlayPause(0);

        if (event.isPressed() && (event.key() == KEY_F6))
            getScene()->getAnimationPlayerManager()->stepBy(0, -0.05f);
        if (event.isPressed() && (event.key() == KEY_F7))
            getScene()->getAnimationPlayerManager()->stepBy(0, 0.05f);

        if (event.isPressed() && (event.key() == KEY_F12) && m_picked)
        {
            if (m_picked->getType() == ENGINE_SKINNING)
                static_cast<Skinning*>(m_picked)->toggleSkinning();
        }

        // toggle between forward lighting and deferred shading
        if (event.isPressed() && (event.key() == KEY_F3))
        {
            if (getScene()->getViewPortManager()->getViewPort(1)->getActivity())
            {
                getScene()->getViewPortManager()->getViewPort(1)->disable();
                getScene()->getViewPortManager()->getViewPort(2)->enable();
            }
            else
            {
                getScene()->getViewPortManager()->getViewPort(2)->disable();
                getScene()->getViewPortManager()->getViewPort(1)->enable();
            }
        }
	}

	void onClose()
	{
		getWindow()->terminate();
	}

	void setObject(SceneObject *object)
	{
		m_nodeObject = object->getNode();
	}

	void setLightObject(Light *light)
	{
		m_lightObject = light;
	}

private:

	Float m_speed;
	ABCFont *m_font;
	Vector3 m_pickedPos;

	Bool m_rotateCam;
	Bool m_rotateObj;
	BaseNode *m_nodeObject;
	Light *m_lightObject;
	SceneObject *m_picked;
	Matrix4 m_pickedMat;

public:

	static Int32 main()
	{
        // cleared log out file with new header
        Debug::instance()->setDefaultLog("collada.log");
        Debug::instance()->getDefaultLog().clearLog();
        Debug::instance()->getDefaultLog().writeHeaderLog();

		MemoryManager::instance()->enableLog(MemoryManager::MEM_RAM, 128);
		MemoryManager::instance()->enableLog(MemoryManager::MEM_GFX);

		// parse command line
		Application::getCommandLine()->registerArgument("input");
		Application::getCommandLine()->addOption('r',"root");
		Application::getCommandLine()->addOption('o',"output");

		if (!Application::getCommandLine()->parse())
		{
			System::print("--- O3DCollada lib test ---", "collada");
			System::print("Usage: run.sh <--dir=workingdir> <-root=datadir> <--output=filename> filename.dae", "collada");
			System::print("Use --root option to specifiy where the scene data are located", "collada");
			System::print("If the --output option is present an O3D scene is exported in the scene root directory", "collada");
			System::print("Check for some samples into the test/ directory", "collada");
			return 0;
		}

		String daeFile = Application::getCommandLine()->getArgumentValue("input");
		String outputFilename = Application::getCommandLine()->getOptionValue("output");
		String sceneRoot = Application::getCommandLine()->getOptionValue("root");

		if (sceneRoot.isEmpty())
			sceneRoot = FileManager::instance()->getWorkingDirectory();

		// create resources directories if necessary
		DiskDir sceneDir(sceneRoot);
		sceneDir.makeAbsolute();

		if (sceneDir.check("models") != Dir::SUCCESS)
			sceneDir.makeDir("models");

		if (sceneDir.check("textures") != Dir::SUCCESS)
			sceneDir.makeDir("textures");

		if (sceneDir.check("sounds") != Dir::SUCCESS)
			sceneDir.makeDir("sounds");

		if (sceneDir.check("animations") != Dir::SUCCESS)
			sceneDir.makeDir("animations");

		// Our application object
        ColladaTest *myApp = new ColladaTest(sceneRoot);

		// Import the COLLADA scene
		Int64 t = System::getTime();
		Collada collada;
        collada.setScene(myApp->getScene());
        collada.processImport(daeFile);
		t = System::getTime() - t;
		System::print(String::print("%f s", Float(t) / System::getTimeFrequency()), "collada");

		// Export before to add the camera and lights
		if (outputFilename.isValid())
		{
			System::print(String("Export O3D scene into ") + outputFilename + "...", "collada");
            myApp->getScene()->exportScene(outputFilename, SceneIO());
		}

        myApp->getScene()->setGlobalAmbient(Color(0.8f, 0.8f, 0.8f, 1.0f));

		// Create a camera into the scene.
        Camera *lpCamera = new Camera(myApp->getScene());

        // And two viewport, one simple multipass forward lighting, another deferred shading
        ScreenViewPort *pViewPort = myApp->getScene()->getViewPortManager()->addScreenViewPort(lpCamera,0,0);
        ScreenViewPort *pDeferredViewPort = myApp->getScene()->getViewPortManager()->addScreenViewPort(
                    lpCamera,
                    new DeferredDrawer(myApp->getScene()),
                    0);

        pViewPort->disable();

        //pDeferredViewPort->disable();
        GBuffer *gbuffer = new GBuffer(pDeferredViewPort);
        gbuffer->create(800, 600);
        ((DeferredDrawer*)(pDeferredViewPort)->getSceneDrawer())->setGBuffer(gbuffer);

        myApp->getScene()->getPicking()->setCamera(lpCamera);

		lpCamera->setName("Camera");

		// Define Z clipping planes
		lpCamera->setZnear(0.25f);
		lpCamera->setZfar(10000.0f);

		// Compute the projection matrix of the camera as a perspective projection.
		lpCamera->computePerspective();
		// We don't want to see it
		lpCamera->disableVisibility();

		// Create a new node to contain our new camera
        Node *cameraNode = myApp->getScene()->getHierarchyTree()->addNode(lpCamera);
		// We also need a first view person transformation on this node
		FTransform *lftransfrom = new FTransform;
		cameraNode->addTransform(lftransfrom);

		lftransfrom->rotate(Y, o3d::HALF_PI/2);
		lftransfrom->translate(Vector3(0.0f, 0.0f, 20.0f));

		// Create a directional light
        Light *lpLight = new Light(myApp->getScene(), Light::DIRECTIONAL_LIGHT);
		lpLight->setName("light1");
		lpLight->setAmbient(0.f, 0.f, 0.f, 1.f);
		lpLight->setDiffuse(1.f, 1.f, 1.f, 1.f);
		lpLight->setSpecular(0.7f,0.7f,0.7f, 1.f);
        lpLight->disable();

        // Create a new node to contain our new light
        Node *lightNode = myApp->getScene()->getHierarchyTree()->addNode(lpLight);
        // We also need a transformation on this node
        MTransform *ltransfrom = new MTransform;
        lightNode->addTransform(ltransfrom);

        // down-forward-left
		ltransfrom->setDirectionZ(Vector3(1.0f, -1.0f, -1.0f));
        ltransfrom->setDirectionZ(Vector3(0.f, -1.0f, .0f));

		// Create a point light at 0,0,0
        Light *pointLight = new Light(myApp->getScene(), Light::POINT_LIGHT);
		pointLight->setName("light2");
		pointLight->setAmbient(0.f, 0.f, 0.f, 1.f);
		pointLight->setDiffuse(1.f, 1.f, 1.f, 1.f);
        pointLight->setAttenuation(1.f, 0.f, 0.0002f);
        pointLight->setSpecular(0.7f,0.7f,0.7f, 1.f);

        myApp->setLightObject(pointLight);

		// Create a new node to contain our new light
        Node *pointLightNode = myApp->getScene()->getHierarchyTree()->addNode(pointLight);
		// We also need a transformation on this node
		MTransform *pointLightTransfrom = new MTransform;
		pointLightNode->addTransform(pointLightTransfrom);
        pointLightTransfrom->translate(Vector3(1.0f,1.0f,1.0f));

		// Change the clear color
        myApp->getScene()->getContext()->setBackgroundColor(0.633f, 0.792f, 0.914f, 0.0f);
        //myApp->getScene()->getContext()->setBackgroundColor(0.f, 0.f, 0.f, 0.f);

		// Set this parameter to True if you want to visualize the bones
        myApp->getScene()->setDrawObject(Scene::DRAW_BONES, True);
		// Set the parameter to True if you want to visualize the bounding volumes
        myApp->getScene()->setDrawObject(Scene::DRAW_BOUNDING_VOLUME, False);
		// Set the parameter to False if you want to disable the rendering step of any skinned meshes
        myApp->getScene()->setDrawObject(Scene::DRAW_SKIN, True);
        myApp->getScene()->hideAllSymbolicObject();

    //	myApp->getScene()->importMS3D("E:\\dev\\models\\elite_trooper_231\\models\\Lod1\\elite_trooper_lod1.ms3d");

		// find the first imported mesh/skin parent node.
        const T_SonList &sons = myApp->getScene()->getHierarchyTree()->getRootNode()->getSonList();
		Bool found = False;
		for (CIT_SonList it = sons.begin(); it != sons.end(); ++it)
		{
			if ((*it)->isNodeObject())
			{
				const T_SonList &subSons = ((Node*)(*it))->getSonList();
				for (CIT_SonList it2 = subSons.begin(); it2 != subSons.end(); ++it2)
				{
					if (((*it2)->getType() == ENGINE_MESH) || ((*it2)->getType() == ENGINE_SKIN) || ((*it)->getType() == ENGINE_BONES))
					{
                        myApp->setObject(*it2);
						found = True;
						break;
					}
				}
			}
			else if (((*it)->getType() == ENGINE_MESH) || ((*it)->getType() == ENGINE_SKIN) || ((*it)->getType() == ENGINE_BONES))
			{
                myApp->setObject(*it);
				found = True;
			}

			if (found)
				break;
		}

		// Run the event loop
        Application::run();

		// Destroy any content
        deletePtr(myApp);

        // write footer log
        Debug::instance()->getDefaultLog().writeFooterLog();

		return 0;
	}
};

O3D_CONSOLE_MAIN(ColladaTest, O3D_DEFAULT_CLASS_SETTINGS)
