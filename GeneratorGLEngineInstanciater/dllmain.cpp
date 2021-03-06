// dllmain.cpp : Définit le point d'entrée pour l'application DLL.
#include "stdafx.h"

// std
#include <thread>
#include <chrono>
#include <iostream>
#include <string>
#define _USE_MATH_DEFINES
#include <math.h>
#include <cstdlib>

// GLEngine
#include <GLEngine.h>
#include <Texture\TextureManager.h>
#include <Camera\PerspectiveCamera.h>
#include <Render\Model.h>
#include <SceneGraph\SceneManager.h>
#include <SceneGraph\SceneNode.h>
#include <Actor\SpinnerActor.h>
#include <Render\RenderManager.h>
#include <Render\EnvironmentMapSky.h>
#include <Actor\ThirdViewOrientationActor.h>
#include <Render\PostProcesses\PostProcess.h>
#include <Render\PostProcesses\BloomPostProcess.h>
#include <Render\Lighting\PointLight.h>
#include <Render\PostProcesses\GammaCorrectionPostProcess.h>
#include <Render\PostProcesses\LensPostProcess.h>
#include <Render\PostProcesses\FogPostProcess.h>

// Generator
#include "GLEngineObjectInstanciater.h"
#include <SceneGraphManager.h>
#include <LevelFactory.h>
#include <DependenceTreeDataModel.h>
#include <Item.h>
#include <ParametricPlane.h>


GLEngineMath::Vector3 _globalCameraPosition = GLEngineMath::Vector3(0, 0, 0);
GLEngineMath::Vector3 _globalCameraForwardVector = GLEngineMath::Vector3(0, 0, 1);
GLEngineMath::Vector3 _globalCameraSpeed = GLEngineMath::Vector3(0, 0, 0);
GLEngineMath::Vector3 _globalTargetPosition = GLEngineMath::Vector3(0, 0, 0);

float _globalAcceleration = 0.01f;
float _globalFriction = 0.1f;
float _sphereRadius = 8.0f;
float _thetaSpeed = 0, _phiSpeed = 0;
float _theta = 1.7f, _phi = 0;

bool *_zPressed, *_sPressed, *_qPressed, *_dPressed, *_majPressed;

GLEngineMath::Vector2 _previousCursorPosition = GLEngineMath::Vector2();
GLEngineMath::Vector2 _cursorDifference = GLEngineMath::Vector2();

void initializeFileReadingTestScene(Generator::SceneGraphManager& sceneManager)
{
	DataModel::DependenceTreeDataModel dependenceTree = DataModel::DependenceTreeDataModel();
	Generator::LevelFactory* rootFactory = dependenceTree.Read("C:/Utils/GeneratorScenes/demo0_GLEngine/main.txt");

	shared_ptr<Generator::SimpleObjectDisplayable> object0 = make_shared<Generator::SimpleObjectDisplayable>("C:/Utils/GLEngineMedia/GeneratorMedia/ABrick.obj", "C:/Utils/OgreMedia/textures/debug_texture.png");
	shared_ptr<Generator::Item> item0 = std::make_shared<Generator::Item>(Math::Matrix4(Math::Vector3(0, 0, 0)), shared_ptr<Generator::Item>(), 100000.0f, vector<Math::ParametricPlane*>(), false, object0, rootFactory);

	//shared_ptr<Generator::Item> item0 = std::make_shared<Generator::Item>();
	item0->SetId(10);
	sceneManager.QueueAddItem(item0);
}

void refreshGenerator(Generator::SceneGraphManager* sceneManager, Math::Vector3 cameraPos)
{
	sceneManager->Update(cameraPos, Math::Vector3(0, 0, 0));
}

std::chrono::high_resolution_clock::time_point originTime = std::chrono::high_resolution_clock::now();

void generatorUpdate(GLEngineObjectInstanciater& instanciater,
	Generator::SceneGraphManager& sceneManager,
	std::thread& proceduralGeneratorThread,
	bool& threadInitialized,
	std::chrono::high_resolution_clock::time_point& lastUpdateAt,
	const GLEngineMath::Vector3& cameraPosition)
{
	// Flush refresh
	//if (timeSinceLastUpdate > 0.5 && _ogreInstanciater->IsFlushCompleted())
	std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
	// Updates every half seconds.
	/*if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastUpdateAt).count() > 500
		&& std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - originTime).count() < 20000)*/

	// Updates every time the generator update is completed.
	/*if (sceneManager.IsUpdateFinished())*/

	// Updates every time the generator update is completed and the instanciater finished it's work.
	if (sceneManager.IsUpdateFinished() && instanciater.IsFlushCompleted())
	{
		Math::Vector3 cameraPos = Math::Vector3(cameraPosition.X(), cameraPosition.Y(), cameraPosition.Z());

		if (threadInitialized)
		{
			std::chrono::high_resolution_clock::time_point beforeJoin = std::chrono::high_resolution_clock::now();
			proceduralGeneratorThread.join();
			std::chrono::high_resolution_clock::time_point afterJoin = std::chrono::high_resolution_clock::now();

			double joinWaitTime = std::chrono::duration_cast<std::chrono::milliseconds>(afterJoin - beforeJoin).count();

			//std::cout << "join wait time : " << joinWaitTime << std::endl;
		}

		std::chrono::high_resolution_clock::time_point tpSceneManager0 = std::chrono::high_resolution_clock::now();

		sceneManager.Flush();

		/*high_resolution_clock::time_point tpSceneManager1 = high_resolution_clock::now();
		auto sceneManagerDuration = duration_cast<microseconds>(tpSceneManager1 - tpSceneManager0).count();*/
		//std::cout << "Scene manager " << sceneManagerDuration << std::endl;

		proceduralGeneratorThread = std::thread(refreshGenerator, &sceneManager, cameraPos);

		threadInitialized = true;
		lastUpdateAt = std::chrono::high_resolution_clock::now();
	}

	//std::chrono::high_resolution_clock::time_point tpOgreInstanciater0 = std::chrono::high_resolution_clock::now();

	//instanciater.Flush(30000000, 30000000);
	instanciater.Flush(500, 500);

	/*high_resolution_clock::time_point tpOgreInstanciater1 = high_resolution_clock::now();
	auto ogreInstanciaterDuration = duration_cast<microseconds>(tpOgreInstanciater1 - tpOgreInstanciater0).count();
	std::cout << "Instanciater : " << ogreInstanciaterDuration << std::endl;*/
}


GLEngineMath::Vector2 UpdateCursorPosition(GLEngine::GLEngine& engine)
{
	double xpos, ypos;
	engine.GLFWGetCursorPos(&xpos, &ypos);

	_cursorDifference = GLEngineMath::Vector2(xpos - _previousCursorPosition.X(), ypos - _previousCursorPosition.Y());

	//std::cout << "new location : x= " << _cursorDifference.X() << "; y= " << _cursorDifference.Y() << std::endl;

	_previousCursorPosition.X(xpos);
	_previousCursorPosition.Y(ypos);

	return _cursorDifference;
}

void MoveCamera()
{
	// Mouse rotation for first person.
	_phi += 0.005f * _cursorDifference.X();
	_theta += 0.005f * _cursorDifference.Y();

	_globalCameraForwardVector.X(_sphereRadius * cosf(_theta) * cosf(_phi));
	_globalCameraForwardVector.Y(_sphereRadius * sinf(_theta));
	_globalCameraForwardVector.Z(_sphereRadius * cosf(_theta) * sinf(_phi));

	// Acceleration multiplier.
	float accelerationMultiplier = 1.0f;
	if (*_majPressed)
	{
		accelerationMultiplier = 100.0f;
	}

	// Keyboard handling for first person movements.
	if (*_zPressed)
	{
		_globalCameraSpeed = _globalCameraSpeed + _globalCameraForwardVector * _globalAcceleration * accelerationMultiplier;
	}

	if (*_sPressed)
	{
		_globalCameraSpeed = _globalCameraSpeed - _globalCameraForwardVector * _globalAcceleration * accelerationMultiplier;
	}

	if (*_dPressed)
	{
		_globalCameraSpeed = _globalCameraSpeed - GLEngineMath::Vector3::Cross(GLEngineMath::Vector3(0, 1, 0), _globalCameraForwardVector) * _globalAcceleration * accelerationMultiplier;
	}

	if (*_qPressed)
	{
		_globalCameraSpeed = _globalCameraSpeed + GLEngineMath::Vector3::Cross(GLEngineMath::Vector3(0, 1, 0), _globalCameraForwardVector) * _globalAcceleration * accelerationMultiplier;
	}

	// Add friction.
	_globalCameraSpeed = _globalCameraSpeed * (1 - _globalFriction);

	// Update position accordig to the speed.
	_globalCameraPosition = _globalCameraPosition + _globalCameraSpeed;
}

int main()
{
	// Booleans for keys callbacks.
	bool * keys = new bool[5];

	keys[0] = false;
	keys[1] = false;
	keys[2] = false;
	keys[3] = false;
	keys[4] = false;

	_zPressed =		&keys[0];
	_sPressed =		&keys[1];
	_dPressed =		&keys[2];
	_qPressed =		&keys[3];
	_majPressed =	&keys[4];

	// Window size.
	int width = 1280, height = 768;

	// GLEngine creation.
	GLEngine::GLEngine engine = GLEngine::GLEngine();
	// GLFW initialization and OpenGL context creation.
	engine.InitializeContext(width, height, keys);

	// Graphics Resource Manager.
	GLEngine::GraphicsResourceManager* graphicsResourceManager = new GLEngine::GraphicsResourceManager(width, height);

	// Render setting.
	GLEngine::RenderManager* renderManager = new GLEngine::RenderManager(width, height, graphicsResourceManager);

	// Texture manager.
	GLEngine::TextureManager* textureManager = graphicsResourceManager->GetTextureManager();

	// Camera
	GLEngine::PerspectiveCamera* camera = new GLEngine::PerspectiveCamera(0.1f, 10000.0f, 20.0f, (float)height / (float)width);

	// Post processes.
	//camera->AddPostProcess(new LensPostProcess(width, height, graphicsResourceManager->GetTextureManager()));
	//camera->AddPostProcess(new BloomPostProcess(width, height, graphicsResourceManager->GetTextureManager()));
	camera->AddPostProcess(new GLEngine::FogPostProcess(width, height, graphicsResourceManager->GetTextureManager(), *renderManager));
	camera->AddPostProcess(new GLEngine::GammaCorrectionPostProcess(width, height, graphicsResourceManager->GetTextureManager()));

	// Scene setting

	GLEngine::SceneManager* sceneManager = new GLEngine::SceneManager();
	sceneManager->SetCurrentCamera(camera);

	// Camera addition under a scene node.
	// Logic goes like this.
	//									cameraMainNode
	//									/			  \
			//					cameraRotatingNode			cameraTargetNode
//					/				\
		//		PerspectiveCamera			ThirdViewOrientationActor(cameraTargetNode)

// The cameraMainNode hold both the target node and the actual camera node.
// It is the node that will get translated around in the scene.
	GLEngine::SceneNode* cameraMainNode = sceneManager->GetRootNode()->CreateChild();

	GLEngine::SceneNode* cameraTargetNode = cameraMainNode->CreateChild();
	cameraTargetNode->SetRelativeTransformation(GLEngineMath::Matrix4::CreateTranslation(GLEngineMath::Vector3(0, 0, 4)));

	GLEngine::SceneNode* cameraRotatingNode = cameraMainNode->CreateChild();
	cameraRotatingNode->AddSubElement(camera);
	cameraRotatingNode->AddSubElement(new GLEngine::ThirdViewOrientationActor(*sceneManager, cameraTargetNode));

	// Sky
	//GLEngine::Texture2D* texEnvmapTest = textureManager->GetTexture("C:/Utils/GLEngineMedia/redCliffs.jpg");
	GLEngine::Texture2D* texEnvmapTest = textureManager->GetTexture("C:/Utils/GLEngineMedia/Factory_Catwalk_2k.hdr"); 
	GLEngine::EnvironmentMapSky* envmapTest = new GLEngine::EnvironmentMapSky(texEnvmapTest);

	sceneManager->SetSky(envmapTest);


	// Environment generator initialization.
	GLEngineObjectInstanciater glEngineInstanciater = GLEngineObjectInstanciater(sceneManager, textureManager, 50.0f);
	Generator::SceneGraphManager generatorSceneManager = Generator::SceneGraphManager(&glEngineInstanciater);
	initializeFileReadingTestScene(generatorSceneManager);

	// Variable necessary to the generator update.
	std::thread proceduralGeneratorThread;
	bool threadIntialized = false;
	std::chrono::high_resolution_clock::time_point lastUpdateAt = std::chrono::high_resolution_clock::now();

	// Game loop
	int frameCount = 0;
	while (!engine.GLFWWindowShouldClose())
	{
		double timeAtMainLoopStart = engine.GLFWGetTime();

		engine.GLFWPollEvents();

		UpdateCursorPosition(engine);

		// Move the camera "physic" model.
		MoveCamera();

		// Update camera matrix.
		cameraMainNode->SetRelativeTransformation(GLEngineMath::Matrix4::CreateTranslation(_globalCameraPosition));
		cameraTargetNode->SetRelativeTransformation(GLEngineMath::Matrix4::CreateTranslation(_globalCameraForwardVector));

		double timeBeforeGenerator = engine.GLFWGetTime();

		// Use the environment generator to instanciate new objects.
		generatorUpdate(glEngineInstanciater,
			generatorSceneManager,
			proceduralGeneratorThread,
			threadIntialized,
			lastUpdateAt,
			_globalCameraPosition);

		double timeBeforeUpdate = engine.GLFWGetTime();

		//Scene graph update.
		sceneManager->Update();

		double timeAfterUpdate = engine.GLFWGetTime();

		// Rendering.
		renderManager->Render(sceneManager, graphicsResourceManager);

		// Main buffers swap.
		engine.GLFWSwapBuffers();

		double timeAtMainLoopEnd = engine.GLFWGetTime();

		double generatorTime = timeBeforeUpdate - timeBeforeGenerator;
		double updateTime = timeAfterUpdate - timeBeforeUpdate;
		double renderTime = timeAtMainLoopEnd - timeAfterUpdate;
		double mainLoopTime = timeAtMainLoopEnd - timeAtMainLoopStart;

		if (frameCount % 10 == 0)
		{
			
			std::cout << "Generator time: " << generatorTime * 1000 << "ms; FPS: " << 1 / generatorTime << std::endl;
			std::cout << "Update time: " << updateTime * 1000 << "ms; FPS: " << 1 / updateTime << std::endl;
			std::cout << "Render time: " << renderTime * 1000 << "ms; FPS: " << 1 / renderTime << std::endl;
			std::cout << "Main loop time: " << mainLoopTime * 1000 << "ms; FPS: " << 1 / mainLoopTime << std::endl << std::endl;
		}
		++frameCount;
	}

	// Clean resources
	engine.GLFWTerminate();
}