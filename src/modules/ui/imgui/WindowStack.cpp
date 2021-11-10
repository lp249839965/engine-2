/**
 * @file
 */

#include "WindowStack.h"
#include "command/Command.h"
#include "core/Log.h"

namespace ui {
namespace imgui {

void WindowStack::construct() {
	command::Command::registerCommand("ui_pop", [this](const command::CmdArgs &args) { pop(); });
	command::Command::registerCommand("ui_push", [this](const command::CmdArgs &args) {
		if (args.empty()) {
			Log::info("usage: ui_push <windowid>");
			return;
		}
		push(args[0]);
	});
	command::Command::registerCommand("ui_root", [this](const command::CmdArgs &args) {
		if (args.empty()) {
			Log::info("usage: ui_root <windowid>");
			return;
		}
		setNewRootWindow(args[0]);
	});
}

bool WindowStack::setNewRootWindow(const core::String &name) {
	auto i = _windows.find(name);
	if (i == _windows.end()) {
		Log::warn("Could not find window %s", name.c_str());
		return false;
	}
	_stack[0] = i->value;
	return true;
}

bool WindowStack::push(const core::String &name) {
	if (_stack.size() >= _stack.maxSize()) {
		Log::warn("Could not push window %s - max windows reached", name.c_str());
		return false;
	}
	auto i = _windows.find(name);
	if (i == _windows.end()) {
		Log::warn("Could not find window %s", name.c_str());
		return false;
	}
	_stack.push(i->value);
	return true;
}

bool WindowStack::pop() {
	// the root window will stay
	if (_stack.size() <= 1) {
		return false;
	}
	_stack.pop();
	return true;
}

bool WindowStack::init() {
	return true;
}

void WindowStack::shutdown() {
	_stack.clear();
	for (auto e : _windows) {
		delete e->value;
	}
}

void WindowStack::update(double deltaFrameSeconds) {
	const int n = (int)_stack.size();
	for (int i = n - 1; i >= 0; --i) {
		if (_stack[i]->isMinimized()) {
			continue;
		}
		_stack[i]->update(deltaFrameSeconds);
		if (_stack[i]->isFullscreen()) {
			break;
		}
	}
}

bool WindowStack::registerWindow(const core::String &name, Window *window) {
	if (_windows.find(name) != _windows.end()) {
		Log::warn("Window %s is already registered", name.c_str());
		delete window;
		return false;
	}
	_windows.put(name, window);
	return true;
}

} // namespace imgui
} // namespace ui