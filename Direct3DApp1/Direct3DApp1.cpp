#include "pch.h"
#include "Direct3DApp1.h"
#include "BasicTimer.h"

#include "../lua-5.2.2/src/lua.h"
#include "../lua-5.2.2/src/lauxlib.h"
#include "../lua-5.2.2/src/lualib.h"

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace concurrency;

Direct3DApp1::Direct3DApp1() :
	m_windowClosed(false),
	m_windowVisible(true)
{
}

int calledFromLua(lua_State *L)
{
  int argc = lua_gettop(L);

  OutputDebugString(L"-- function called\n");

  for ( int n=1; n<=argc; ++n ) {
	  wchar_t luaBuff[128];
	  swprintf(luaBuff, 128, L"%s", lua_tostring(L, n));

	  OutputDebugString(L"-- argument: ");
      OutputDebugString(luaBuff);
  }

  lua_pushnumber(L, 123); // return value
  return 1; // number of return values
}

void Direct3DApp1::Initialize(CoreApplicationView^ applicationView)
{
	applicationView->Activated +=
        ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &Direct3DApp1::OnActivated);

	CoreApplication::Suspending +=
        ref new EventHandler<SuspendingEventArgs^>(this, &Direct3DApp1::OnSuspending);

	CoreApplication::Resuming +=
        ref new EventHandler<Platform::Object^>(this, &Direct3DApp1::OnResuming);

	m_renderer = ref new CubeRenderer();

	// LUA : Initialize
	lua_State *L = luaL_newstate();

	luaopen_io(L);
	luaopen_base(L);
	luaopen_table(L);
	luaopen_string(L);
	luaopen_math(L);
	
	int s = luaL_loadfile(L, "helloworld.lua");

	lua_register(L, "calledFromLua", calledFromLua);

	if( s == 0 ) {
		s = lua_pcall(L, 0, 1, 0);
	}

	if( s != 0 ) {
		// Report errors
		auto sss = lua_tostring(L, -1);
		wchar_t szBuff[128];
		swprintf(szBuff, 128, L"LUA ERROR %hs", sss);
		OutputDebugString(szBuff);
		OutputDebugString(L"\n");
		lua_pop(L, 1);
	}

	if(lua_istable(L, -1))
	{
		OutputDebugString(L"Received a table!");
	}

	lua_close(L);
}

void Direct3DApp1::SetWindow(CoreWindow^ window)
{
	window->SizeChanged += 
        ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &Direct3DApp1::OnWindowSizeChanged);
	  
	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &Direct3DApp1::OnVisibilityChanged);

	window->Closed += 
        ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &Direct3DApp1::OnWindowClosed);

	window->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);

	window->PointerPressed +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &Direct3DApp1::OnPointerPressed);

	window->PointerMoved +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &Direct3DApp1::OnPointerMoved);

	m_renderer->Initialize(CoreWindow::GetForCurrentThread());
}

void Direct3DApp1::Load(Platform::String^ entryPoint)
{
}

void Direct3DApp1::Run()
{
	BasicTimer^ timer = ref new BasicTimer();

	while (!m_windowClosed)
	{
		if (m_windowVisible)
		{
			timer->Update();
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
			m_renderer->Update(timer->Total, timer->Delta);
			m_renderer->Render();
			m_renderer->Present(); // This call is synchronized to the display frame rate.
		}
		else
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
		}
	}
}

void Direct3DApp1::Uninitialize()
{
}

void Direct3DApp1::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
	m_renderer->UpdateForWindowSizeChange();
}

void Direct3DApp1::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
}

void Direct3DApp1::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
	m_windowClosed = true;
}

void Direct3DApp1::OnPointerPressed(CoreWindow^ sender, PointerEventArgs^ args)
{
	// Insert your code here.
}

void Direct3DApp1::OnPointerMoved(CoreWindow^ sender, PointerEventArgs^ args)
{
	// Insert your code here.
}

void Direct3DApp1::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
	CoreWindow::GetForCurrentThread()->Activate();
}

void Direct3DApp1::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
	// Save app state asynchronously after requesting a deferral. Holding a deferral
	// indicates that the application is busy performing suspending operations. Be
	// aware that a deferral may not be held indefinitely. After about five seconds,
	// the app will be forced to exit.
	SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

	create_task([this, deferral]()
	{
		// Insert your code here.

		deferral->Complete();
	});
}
 
void Direct3DApp1::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
	// Restore any data or state that was unloaded on suspend. By default, data
	// and state are persisted when resuming from suspend. Note that this event
	// does not occur if the app was previously terminated.
}

IFrameworkView^ Direct3DApplicationSource::CreateView()
{
    return ref new Direct3DApp1();
}

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	auto direct3DApplicationSource = ref new Direct3DApplicationSource();
	CoreApplication::Run(direct3DApplicationSource);
	return 0;
}
