

#include "PythonShell.h"
#include <Engine/Display.h>
#include <LabCore/LabCore.h>
#include <Engine/coreassets/Font.h>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/numpy.h>
#include <Engine/YamlExtensions.h>

namespace py = pybind11;

namespace PH::LabCore {

	enum ConsoleMessageSource {
		CONSOLE,
		PYTHON
	};

	struct ShellMessage {
		glm::vec4 color;
		ConsoleMessageSource source;
		uint32 lineheight;
		Engine::String message;
	};

	struct pythonShellInstance {
		Engine::ImGuiDisplay display;

		Engine::ArrayList<ShellMessage> textlines;
		sizeptr messageretrieveptr;

		ShellMessage current;

		uint32 cursorposition;
		Engine::Box2D cursorbox;

		Engine::Font* font;

		real32 screenbottom;
	};

	void serializeShellMessage(const ShellMessage& message, YAML::Emitter& out) {
		out << YAML::BeginMap;
		out << YAML::Key << "message" << YAML::Value << message.message.getC_Str();
		out << YAML::Key << "source" << YAML::Value << message.source;
		out << YAML::EndMap;
	}

	void pythonShellSerialize(pythonShellInstance* shell, YAML::Emitter& out) {
		out << YAML::BeginMap;
		out << YAML::Key << "messages" << YAML::Value << YAML::BeginSeq;

		for (auto& msg : shell->textlines) {
			out << YAML::Value;
			serializeShellMessage(msg, out);
		}
		YAML::EndSeq;
		out << YAML::EndMap;
	}

	void pythonShellDeserialize(pythonShellInstance* shell, YAML::Node in) {

		if (in["messages"]) {
			auto msgs = in["messages"];

			for (auto msg : msgs) {
				ShellMessage shellmsg;
				shellmsg.message = msg["message"].as<Engine::String>();
				shellmsg.source = (ConsoleMessageSource)msg["source"].as<int>();
				if (shellmsg.source == CONSOLE) {
					shellmsg.color = glm::vec4(1.0f);
				}
				else {
					shellmsg.color = glm::vec4(0.2f, 0.8f, 0.2f, 1.0f);
				}
				shell->textlines.pushBack(shellmsg);
			}
			shell->messageretrieveptr = shell->textlines.getCount() - 1;
		}

	}

	pythonShellInstance* focussedshell = nullptr;

	bool32 pythonShellUpdate(void* instancedata) {
		return true;
	}

	bool32 pythonShellInstantiate(pythonShellInstance* shell) {
		shell->display = Engine::createImGuiDisplay(1920, 1080);
		shell->font = appcontext->assets.getAssetByReference<Engine::Font>("defaultfont");

		shell->textlines = Engine::ArrayList<ShellMessage>::create();

		shell->current.message = Engine::String::create("");
		shell->current.color = glm::vec4(1.0f);
		shell->current.source = CONSOLE;

		shell->messageretrieveptr = 0;

		return true;
	}

	Engine::Box2D calcCursorBox(pythonShellInstance* shell) {

		auto* lastline = &shell->textlines.getLast();

		real32 lengthtillcursor = Engine::getTextLength(shell->font, Base::SubString::create(shell->current.message.getC_Str(), shell->cursorposition), 1.0f);

		Engine::Box2D box;
		box.bottomleft = {lengthtillcursor, -1.0f * shell->textlines.getCount() * shell->font->pixelheight - 4.0f };
		box.topright = { lengthtillcursor + 1.0f, -1.0f * (shell->textlines.getCount() - 1) * shell->font->pixelheight - 4.0f };
		return box;
	}


	bool32 pythonShellDraw(pythonShellInstance* shell) {
		
		//physica draw part
		Engine::beginRenderPass(shell->display);

		renderer2D->begin();

		//set the projection and view matrices for the renderer2D wrapper, this is going to be used to draw the plot and the background of the plot, this is going to be set to the size of the display, so that we can draw the plot and the background of the plot in the correct position and size
		glm::mat4 projection = glm::ortho(0.0f, (real32)shell->display.viewport.x, 0.0f, -(real32)shell->display.viewport.y);
		renderer2D->pushProjection(projection);
		renderer2D->pushView(glm::mat4(1.0f));

		
		renderer2D->pushGraphicsPipeline(LabCore::appcontext->defaultgraphicspipeline2D, { nullptr, 0 });

		renderer2D->drawColoredBox2D(shell->cursorbox, glm::vec4(1.0f));

		real32 textpadding = 30.0f;
		real32 leftpadding = 5.0f;

		renderer2D->pushGraphicsPipeline(LabCore::appcontext->defaultfontpipeline2D, { &shell->font->cdata, 1 });
		renderer2D->pushTexture(shell->font->atlas);

		glm::vec2 drawptr = {leftpadding, -shell->font->pixelheight};

		sizeptr consolemsgcount = 0;
		for (auto& msg : shell->textlines) {

			drawptr.x = leftpadding;

			//draw message id
			if (msg.source == CONSOLE) {
				char buffer[16];
				sprintf_s(buffer, 16, "[%u]: ", consolemsgcount++);
				Engine::drawText(shell->font, buffer, drawptr, 1.0f, msg.color, renderer2D->getContext());
			}

			drawptr.x += textpadding;

			glm::vec2 nextpos = Engine::drawText(shell->font, msg.message.getC_Str(), drawptr, 1.0f, msg.color, renderer2D->getContext());
			drawptr.y = nextpos.y - shell->font->pixelheight;
		}
		
		//drawing the current edited item
		drawptr.x = leftpadding;
		char buffer[16];
		sprintf_s(buffer, 16, "[%u]: ", consolemsgcount++);
		Engine::drawText(shell->font, buffer, drawptr, 1.0f, glm::vec4(1.0f), renderer2D->getContext());
		drawptr.x += textpadding;

		glm::vec2 cursorpos = Engine::getCharPosition(shell->font, shell->current.message.getSubString(), 1.0f, shell->cursorposition) + drawptr;
		shell->cursorbox.bottomleft = cursorpos;

		real32 pixelheight = shell->font->pixelheight;
		shell->cursorbox.topright = cursorpos + glm::vec2(1.0f, pixelheight - 4);
		shell->cursorbox.bottom -= 4;

		glm::vec2 nextpos = Engine::drawText(shell->font, shell->current.message.getC_Str(), drawptr, 1.0f, shell->current.color, renderer2D->getContext());
		drawptr.y = nextpos.y - shell->font->pixelheight;

		renderer2D->end();
		renderer2D->flush({nullptr, 0});

		Engine::endRenderPass(shell->display);
		
		if(ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("shell")) {

				if (ImGui::MenuItem("clear shell")) {
					for (auto& line : shell->textlines) {
						Engine::String::destroy(&line.message);
					}

					shell->textlines.clear();
					shell->current.message.set("");
					shell->cursorposition = 0;

					shell->messageretrieveptr = 0;
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		//imgui draw part
		real32 titlebarheight = ImGui::GetFrameHeight();
		ImVec2 displaysize = ImGui::GetContentRegionAvail();
		shell->display.viewport = { displaysize.x, displaysize.y };
		ImGui::Image(
			shell->display.imguitexture,
			displaysize,
			{ 0.0f, 0.0f }, //bottom left uv
			{ displaysize.x / shell->display.framebuffersize.x, displaysize.y / shell->display.framebuffersize.y } //top right UV
		);
	

		return true;
	}

	bool32 pythonShellDestroy(void* instancedata) {
		return true;
	}

	void printToConsole(const char* message) {
		if (focussedshell) {
			ShellMessage msg;
			msg.color = glm::vec4(0.2f, 0.8f, 0.2f, 1.0f);
			msg.source = PYTHON;
			msg.message = Engine::String::create(message);

			focussedshell->textlines.pushBack(msg);
		}
	}

	void removeCharacterAtCursor(pythonShellInstance* shell) {
		auto& currentline = shell->current;
		currentline.message.remove(shell->cursorposition - 1);
		shell->cursorposition--;
	}

	//returns wheter the character c should terminate the deletion
	bool32 isDeleteTerminationCharacter(char c) {
		return c == ' ' || c == '.' || c == '(' || c == ')';
	}

	bool32 pythonShellOnEvent(pythonShellInstance* shell, const PH::Platform::Event& event) {

		focussedshell = shell;

		if (event.type == PH_EVENT_TYPE_MOUSEBUTTON_PRESSED) {
			Engine::String* laststring = &shell->current.message;
			shell->cursorposition = laststring->getLength();
		}

		if (event.type == PH_EVENT_TYPE_CHAR) {
			char c = (char)event.lparam;

			if (c == '\r' || c == '\b' || c == 127) {
				return true;
			}

			Engine::String* laststring = &shell->current.message;
			laststring->insert((char)event.lparam, shell->cursorposition++);
		}

		if (event.type == PH_EVENT_TYPE_KEY_PRESSED) {
			if (event.lparam == PH_LEFT) {
				if (shell->cursorposition > 0) {
					shell->cursorposition--;
				}
				return true;
			}

			if (event.lparam == PH_RIGHT) {
				if (shell->cursorposition < shell->current.message.getLength()) {
					shell->cursorposition++;
				}
				return true;
			}

			if (event.lparam == PH_RETURN) {

				if (Engine::Events::isKeyPressed(PH_CONTROL)) {
					shell->current.message.insert('\n', shell->cursorposition);
					return true;
				}

				auto& last = shell->textlines.getLast();
				shell->textlines.pushBack(shell->current);

				try {
					py::exec(shell->current.message.getC_Str());
				}
				catch(py::error_already_set& e) {
					printToConsole(e.what());
				}

				shell->messageretrieveptr = shell->textlines.getCount() - 1;
				shell->current.message = Engine::String::create("");

				shell->cursorposition = 0;
				return true;
			}

			if (event.lparam == PH_UP) {
				for (int32 i = shell->messageretrieveptr; i >= 0; i--) {
					if (shell->textlines[i].source == CONSOLE) {
						shell->messageretrieveptr = i;
						break;
					}
				}
				shell->current.message.set(shell->textlines[shell->messageretrieveptr].message.getC_Str());
				shell->cursorposition = shell->current.message.getLength();

				if (shell->messageretrieveptr > 0) {
					shell->messageretrieveptr--;
				}
				return true;
			}

			if (event.lparam == PH_DOWN) {
				for (int32 i = shell->messageretrieveptr; i < shell->textlines.getCount(); i++) {
					if (shell->textlines[i].source == CONSOLE) {
						shell->messageretrieveptr = i;
					}
				}

				shell->current.message.set(shell->textlines[shell->messageretrieveptr].message.getC_Str());
				shell->cursorposition = shell->current.message.getLength();

				if (shell->messageretrieveptr < shell->textlines.getCount() - 1) {
					shell->messageretrieveptr++;
				}
				return true;
			}

			if (event.lparam == PH_BACK) {
				if (shell->cursorposition > 0) {
					if (Engine::Events::isKeyPressed(PH_CONTROL)) {
						//should really make this more memory efficient
						auto& currentline = shell->current.message;

						//remove all leading spaces
						while (shell->cursorposition > 0 && currentline[shell->cursorposition - 1] == ' ') {
							removeCharacterAtCursor(shell);
						}

						//remove at least the first character
						if (shell->cursorposition > 0) {
							removeCharacterAtCursor(shell);
						}

						//remove all characters until a delete termination character is found
						while (shell->cursorposition > 0 && !isDeleteTerminationCharacter(currentline[shell->cursorposition - 1]) ) {
							removeCharacterAtCursor(shell);
						}
						return true;
					}

					removeCharacterAtCursor(shell);
					return true;
				}

			}
		}

		return true;
	}

	

	PYBIND11_EMBEDDED_MODULE(labcore, m) {
		m.def("print", printToConsole, py::arg("str"));
	}

	View getPythonShellView() {

		View v{};
		v.instancedatasize = sizeof(pythonShellInstance);
		v.destroy_ = pythonShellDestroy;
		v.draw_ = (ViewDrawFunction)pythonShellDraw;
		v.onEvent_ = (ViewOnEventFunction)pythonShellOnEvent;
		v.onUpdate_ = pythonShellUpdate;
		v.instantiate_ = (ViewInstantiateFunction)pythonShellInstantiate;
		v.serialize_ = (ViewSerializeFunction)pythonShellSerialize;
		v.deserialize_ = (ViewDeserializeFunction)pythonShellDeserialize;
		v.name = Engine::String::create("Python Shell");

		return v;
	}
}