﻿/*
 * This source file is part of RmlUi, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://github.com/mikke89/RmlUi
 *
 * Copyright (c) 2018 Michael R. P. Ragazzon
 * Copyright (c) 2019 The RmlUi Team, and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <RmlUi/Core.h>
#include <RmlUi/Controls.h>
#include <RmlUi/Debugger.h>
#include <Input.h>
#include <Shell.h>
#include <ShellRenderInterfaceOpenGL.h>
#include <RmlUi/Core/TransformPrimitive.h>
#include <RmlUi/Core/StreamMemory.h>

static const Rml::Core::String sandbox_default_rcss = R"(
body { top: 0; left: 0; right: 0; bottom: 0; overflow: hidden auto; }
scrollbarvertical { width: 15px; }
scrollbarvertical slidertrack { background: #eee; }
scrollbarvertical slidertrack:active { background: #ddd; }
scrollbarvertical sliderbar { width: 15px; min-height: 30px; background: #aaa; }
scrollbarvertical sliderbar:hover { background: #888; }
scrollbarvertical sliderbar:active { background: #666; }
)";


class DemoWindow : public Rml::Core::EventListener
{
public:
	DemoWindow(const Rml::Core::String &title, const Rml::Core::Vector2f &position, Rml::Core::Context *context)
	{
		using namespace Rml::Core;
		document = context->LoadDocument("basic/demo/data/demo.rml");
		if (document != nullptr)
		{
			{
				document->GetElementById("title")->SetInnerRML(title);
				document->SetProperty(PropertyId::Left, Property(position.x, Property::PX));
				document->SetProperty(PropertyId::Top, Property(position.y, Property::PX));
			}

			// Add sandbox default text.
			if (auto source = static_cast<Rml::Controls::ElementFormControl*>(document->GetElementById("sandbox_rml_source")))
			{
				auto value = source->GetValue();
				value += "<p>Write your RML here</p>\n\n<!-- <img src=\"assets/high_scores_alien_1.tga\"/> -->";
				source->SetValue(value);
			}

			// Prepare sandbox document.
			if (auto target = document->GetElementById("sandbox_target"))
			{
				iframe = context->CreateDocument();
				auto iframe_ptr = iframe->GetParentNode()->RemoveChild(iframe);
				target->AppendChild(std::move(iframe_ptr));
				iframe->SetProperty(PropertyId::Position, Property(Style::Position::Absolute));
				iframe->SetProperty(PropertyId::Display, Property(Style::Display::Block));
				iframe->SetInnerRML("<p>Rendered output goes here.</p>");

				// Load basic RML style sheet
				Rml::Core::String style_sheet_content;
				{
					// Load file into string
					auto file_interface = Rml::Core::GetFileInterface();
					Rml::Core::FileHandle handle = file_interface->Open("assets/rml.rcss");
					
					size_t length = file_interface->Length(handle);
					style_sheet_content.resize(length);
					file_interface->Read((void*)style_sheet_content.data(), length, handle);
					file_interface->Close(handle);

					style_sheet_content += sandbox_default_rcss;
				}

				Rml::Core::StreamMemory stream((Rml::Core::byte*)style_sheet_content.data(), style_sheet_content.size());
				stream.SetSourceURL("sandbox://default_rcss");

				rml_basic_style_sheet = std::make_shared<Rml::Core::StyleSheet>();
				rml_basic_style_sheet->LoadStyleSheet(&stream);
			}

			// Add sandbox style sheet text.
			if (auto source = static_cast<Rml::Controls::ElementFormControl*>(document->GetElementById("sandbox_rcss_source")))
			{
				Rml::Core::String value = "/* Write your RCSS here */\n\n/* body { color: #fea; background: #224; }\nimg { image-color: red; } */";
				source->SetValue(value);
				SetSandboxStylesheet(value);
			}
			
			document->Show();
		}
	}

	void Update() {
		if (iframe)
		{
			iframe->UpdateDocument();
		}
	}

	void Shutdown() {
		if (document)
		{
			document->Close();
			document = nullptr;
		}
	}

	void ProcessEvent(Rml::Core::Event& event) override
	{
		using namespace Rml::Core;

		switch (event.GetId())
		{
		case EventId::Keydown:
		{
			Rml::Core::Input::KeyIdentifier key_identifier = (Rml::Core::Input::KeyIdentifier) event.GetParameter< int >("key_identifier", 0);
			bool ctrl_key = event.GetParameter< bool >("ctrl_key", false);

			if (key_identifier == Rml::Core::Input::KI_ESCAPE)
			{
				Shell::RequestExit();
			}
			else if (key_identifier == Rml::Core::Input::KI_F8)
			{
				Rml::Debugger::SetVisible(!Rml::Debugger::IsVisible());
			}
		}
		break;

		default:
			break;
		}
	}

	Rml::Core::ElementDocument * GetDocument() {
		return document;
	}

	void SetSandboxStylesheet(const Rml::Core::String& string)
	{
		if (iframe && rml_basic_style_sheet)
		{
			auto style = std::make_shared<Rml::Core::StyleSheet>();
			Rml::Core::StreamMemory stream((const Rml::Core::byte*)string.data(), string.size());
			stream.SetSourceURL("sandbox://rcss");

			style->LoadStyleSheet(&stream);
			style = rml_basic_style_sheet->CombineStyleSheet(*style);
			iframe->SetStyleSheet(style);
		}
	}

	void SetSandboxBody(const Rml::Core::String& string)
	{
		if (iframe)
		{
			iframe->SetInnerRML(string);
		}
	}

private:
	Rml::Core::ElementDocument *document = nullptr;
	Rml::Core::ElementDocument *iframe = nullptr;
	Rml::Core::SharedPtr<Rml::Core::StyleSheet> rml_basic_style_sheet;
};


Rml::Core::Context* context = nullptr;
ShellRenderInterfaceExtensions *shell_renderer;
std::unique_ptr<DemoWindow> demo_window;

void GameLoop()
{
	context->Update();
	demo_window->Update();

	shell_renderer->PrepareRenderBuffer();
	context->Render();
	shell_renderer->PresentRenderBuffer();
}




class DemoEventListener : public Rml::Core::EventListener
{
public:
	DemoEventListener(const Rml::Core::String& value, Rml::Core::Element* element) : value(value), element(element) {}

	void ProcessEvent(Rml::Core::Event& event) override
	{
		using namespace Rml::Core;

		if (value == "exit")
		{
			// Test replacing the current element.
			// Need to be careful with regard to lifetime issues. The event's current element will be destroyed, so we cannot
			// use it after SetInnerRml(). The library should handle this case safely internally when propagating the event further.
			auto parent = element->GetParentNode();
			parent->SetInnerRML("<button onclick='confirm_exit' onblur='cancel_exit' onmouseout='cancel_exit'>Are you sure?</button>");
			if (auto child = parent->GetChild(0))
				child->Focus();
		}
		else if (value == "confirm_exit")
		{
			Shell::RequestExit();
		}
		else if (value == "cancel_exit")
		{
			if(auto parent = element->GetParentNode())
				parent->SetInnerRML("<button id='exit' onclick='exit'>Exit</button>");
		}
		else if (value == "rating")
		{
			auto el_rating = element->GetElementById("rating");
			auto el_rating_emoji = element->GetElementById("rating_emoji");
			if (el_rating && el_rating_emoji)
			{
				enum { Sad, Mediocre, Exciting, Celebrate, Champion, CountBonus };
				static const Rml::Core::String emojis[CountBonus] = { u8"😢", u8"😐", u8"😮", u8"😎", u8"🏆" };
				int value = std::atoi(static_cast<Rml::Controls::ElementFormControl*>(element)->GetValue().c_str());
				
				Rml::Core::String emoji;
				if (value <= 0)
					emoji = emojis[Sad];
				else if(value < 50)
					emoji = emojis[Mediocre];
				else if (value < 75)
					emoji = emojis[Exciting];
				else if (value < 100)
					emoji = emojis[Celebrate];
				else
					emoji = emojis[Champion];

				el_rating->SetInnerRML(Rml::Core::CreateString(30, "%d%%", value));
				el_rating_emoji->SetInnerRML(emoji);
			}
		}
		else if (value == "submit_form")
		{
			if (auto el_output = element->GetElementById("form_output"))
			{
				const auto& p = event.GetParameters();
				Rml::Core::String output = "<p>";
				for (auto& entry : p)
				{
					auto value = Rml::Core::StringUtilities::EncodeRml(entry.second.Get<Rml::Core::String>());
					if (entry.first == "message")
						value = "<br/>" + value;
					output += "<strong>" + entry.first + "</strong>: " + value + "<br/>";
				}
				output += "</p>";

				el_output->SetInnerRML(output);
			}
		}
		else if (value == "set_sandbox_body")
		{
			if (auto source = static_cast<Rml::Controls::ElementFormControl*>(element->GetElementById("sandbox_rml_source")))
			{
				auto value = source->GetValue();
				demo_window->SetSandboxBody(value);
			}
		}
		else if (value == "set_sandbox_style")
		{
			if (auto source = static_cast<Rml::Controls::ElementFormControl*>(element->GetElementById("sandbox_rcss_source")))
			{
				auto value = source->GetValue();
				demo_window->SetSandboxStylesheet(value);
			}
		}
	}

	void OnDetach(Rml::Core::Element* element) override { delete this; }

private:
	Rml::Core::String value;
	Rml::Core::Element* element;
};



class DemoEventListenerInstancer : public Rml::Core::EventListenerInstancer
{
public:
	Rml::Core::EventListener* InstanceEventListener(const Rml::Core::String& value, Rml::Core::Element* element) override
	{
		return new DemoEventListener(value, element);
	}
};


#if defined RMLUI_PLATFORM_WIN32
#include <windows.h>
int APIENTRY WinMain(HINSTANCE RMLUI_UNUSED_PARAMETER(instance_handle), HINSTANCE RMLUI_UNUSED_PARAMETER(previous_instance_handle), char* RMLUI_UNUSED_PARAMETER(command_line), int RMLUI_UNUSED_PARAMETER(command_show))
#else
int main(int RMLUI_UNUSED_PARAMETER(argc), char** RMLUI_UNUSED_PARAMETER(argv))
#endif
{
#ifdef RMLUI_PLATFORM_WIN32
	RMLUI_UNUSED(instance_handle);
	RMLUI_UNUSED(previous_instance_handle);
	RMLUI_UNUSED(command_line);
	RMLUI_UNUSED(command_show);
#else
	RMLUI_UNUSED(argc);
	RMLUI_UNUSED(argv);
#endif

	const int width = 1600;
	const int height = 950;

	ShellRenderInterfaceOpenGL opengl_renderer;
	shell_renderer = &opengl_renderer;

	// Generic OS initialisation, creates a window and attaches OpenGL.
	if (!Shell::Initialise() ||
		!Shell::OpenWindow("Demo Sample", shell_renderer, width, height, true))
	{
		Shell::Shutdown();
		return -1;
	}

	// RmlUi initialisation.
	Rml::Core::SetRenderInterface(&opengl_renderer);
	opengl_renderer.SetViewport(width, height);

	ShellSystemInterface system_interface;
	Rml::Core::SetSystemInterface(&system_interface);

	Rml::Core::Initialise();

	// Create the main RmlUi context and set it on the shell's input layer.
	context = Rml::Core::CreateContext("main", Rml::Core::Vector2i(width, height));
	if (context == nullptr)
	{
		Rml::Core::Shutdown();
		Shell::Shutdown();
		return -1;
	}

	Rml::Controls::Initialise();
	Rml::Debugger::Initialise(context);
	Input::SetContext(context);
	shell_renderer->SetContext(context);
	
	context->SetDensityIndependentPixelRatio(1.0f);

	DemoEventListenerInstancer event_listener_instancer;
	Rml::Core::Factory::RegisterEventListenerInstancer(&event_listener_instancer);

	Shell::LoadFonts("assets/");

	demo_window = std::make_unique<DemoWindow>("Demo sample", Rml::Core::Vector2f(150, 80), context);
	demo_window->GetDocument()->AddEventListener(Rml::Core::EventId::Keydown, demo_window.get());
	demo_window->GetDocument()->AddEventListener(Rml::Core::EventId::Keyup, demo_window.get());
	demo_window->GetDocument()->AddEventListener(Rml::Core::EventId::Animationend, demo_window.get());

	Shell::EventLoop(GameLoop);

	demo_window->Shutdown();

	// Shutdown RmlUi.
	Rml::Core::Shutdown();

	Shell::CloseWindow();
	Shell::Shutdown();

	demo_window.reset();

	return 0;
}