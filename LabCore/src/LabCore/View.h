#pragma once
#include <Base/Base.h>
#include <Engine/Events.h>
#include <Engine/RenderPrimitives.h>

namespace PH::LabCore {

	struct AppContext;
	extern AppContext* appcontext;

	typedef bool32 (*ViewDrawFunction)(void* instancedata);
	typedef bool32 (*ViewDestroyFunction)(void* instancedata);
	typedef bool32 (*ViewOnEventFunction)(void* instancedata, const PH::Platform::Event& event);
	typedef bool32 (*ViewOnUpdateFunction)(void* instancedata);
	typedef bool32 (*ViewInstantiateFunction)(void* instancedata);


	

	struct View {

		//the size of the instance specific data that is allocated for each instance of the view
		sizeptr instancedatasize;

		//functionality of the view, these are called by the application to draw, destroy, handle events and update the view
		ViewDrawFunction draw_;
		ViewDestroyFunction destroy_;
		ViewOnEventFunction onEvent_;
		ViewOnUpdateFunction onUpdate_;
		ViewInstantiateFunction instantiate_;

		//name of the view, used for identification and display in the GUI
		Engine::String name;		
	};

	//instance of a view, this is created by the application when a view is added to the application, and it is destroyed when the view is removed from the application
	struct ViewInstance {
		View* view;
		void* instancedata;
		Engine::UUID instanceid;
		
		bool32 isfocussed;

		Engine::Box2D region;
	};

	//the view that is currently being drawn or updated
	extern ViewInstance* openview;

	ViewInstance createViewInstance(View* view);
	ViewInstance createViewInstance(View* view, Engine::UUID id);
	
	//updates all views in the application, this is called by the application in the update loop
	void updateViews(Engine::ArrayList<ViewInstance>& views);

	//draws all views in the application, this is called by the application in the update loop
	void drawViews(Engine::ArrayList<ViewInstance>& views);

	//passes the event trough to the focussed view
	bool32 interpretEventForViews(Engine::ArrayList<ViewInstance>& views, const PH::Platform::Event& e);
}