#ifdef _MSVC_LANG
#define _CRT_SECURE_NO_WARNINGS
#endif

#define LIBSSH_STATIC 1
#include <libssh/libssh.h>

#include <Platform/PlatformAPI.h>
#include <Base/Log.h>

#include <Engine/cppAPI/Rendering.hpp>
#include <Engine/Engine.h>
#include <Engine/Events.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "Text.h"
#include "Plot.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>



namespace PH::RpGui {

	using namespace PH::Platform;

	CONSOLE_WRITE(consoleWrite) {
		Platform::consoleWrite(str);
	}

	PH::Base::LogStream<consoleWrite> INFO;
	PH::Base::LogStream<consoleWrite> WARN;
	PH::Base::LogStream<consoleWrite> ERR;

	struct Context {
		Engine::Renderer2D::Context* renderer2D;
		PH::Platform::GFX::GraphicsPipeline pipeline2D;
		PH::Platform::GFX::GraphicsPipeline fontpipeline2D;

		Engine::ArrayList<glm::vec2> buffer;

		Font font;
	};

	Context* context;

	Engine::Renderer2D::Wrapper renderer2D;
}

int verify_knownhost(ssh_session session)
{
	enum ssh_known_hosts_e state;
	unsigned char* hash = NULL;
	ssh_key srv_pubkey = NULL;
	size_t hlen;
	char buf[10];
	char* p = NULL;
	int cmp;
	int rc;

	rc = ssh_get_server_publickey(session, &srv_pubkey);
	if (rc < 0) {
		return -1;
	}

	rc = ssh_get_publickey_hash(srv_pubkey,
		SSH_PUBLICKEY_HASH_SHA256,
		&hash,
		&hlen);
	ssh_key_free(srv_pubkey);
	if (rc < 0) {
		return -1;
	}

	state = ssh_session_is_known_server(session);
	switch (state) {
	case SSH_KNOWN_HOSTS_OK:
		/* OK */

		break;
	case SSH_KNOWN_HOSTS_CHANGED:
		fprintf(stderr, "Host key for server changed: it is now:\n");
		ssh_print_hash(SSH_PUBLICKEY_HASH_SHA256, hash, hlen);
		fprintf(stderr, "For security reasons, connection will be stopped\n");
		ssh_clean_pubkey_hash(&hash);

		return -1;
	case SSH_KNOWN_HOSTS_OTHER:
		fprintf(stderr, "The host key for this server was not found but an other"
			"type of key exists.\n");
		fprintf(stderr, "An attacker might change the default server key to"
			"confuse your client into thinking the key does not exist\n");
		ssh_clean_pubkey_hash(&hash);

		return -1;
	case SSH_KNOWN_HOSTS_NOT_FOUND:
		fprintf(stderr, "Could not find known host file.\n");
		fprintf(stderr, "If you accept the host key here, the file will be"
			"automatically created.\n");

		/* FALL THROUGH to SSH_SERVER_NOT_KNOWN behavior */

	case SSH_KNOWN_HOSTS_UNKNOWN:
		fprintf(stderr, "The server is unknown. Do you trust the host key?\n");
		fprintf(stderr, "Public key hash: ");
		ssh_print_hash(SSH_PUBLICKEY_HASH_SHA256, hash, hlen);
		ssh_clean_pubkey_hash(&hash);
		p = fgets(buf, sizeof(buf), stdin);
		if (p == NULL) {
			return -1;
		}

		cmp = PH::Base::stringCompare(buf, "yes", 3);
		if (cmp != 0) {
			return -1;
		}

		rc = ssh_session_update_known_hosts(session);
		if (rc < 0) {
			fprintf(stderr, "Error %s\n", strerror(errno));
			return -1;
		}

		break;
	case SSH_KNOWN_HOSTS_ERROR:
		fprintf(stderr, "Error %s", ssh_get_error(session));
		ssh_clean_pubkey_hash(&hash);
		return -1;
	}

	ssh_clean_pubkey_hash(&hash);
	return 0;
}

using namespace PH;

PH_DLL_EXPORT PH_APPLICATION_INITIALIZE(applicationInitialize) {

	//sets up the engine allocators and other systems that rely on the engine allocator, such as the console log stream
	PH::Engine::EngineInitInfo engineinit{};
	engineinit.memory = (PH::uint8*)context.appmemory;
	engineinit.memorysize = context.appmemsize;
	engineinit.platformcontext = &context;
	PH::Engine::init(engineinit);


	///ssh connection example, this is just to test that the libssh library is working and can connect to a server, it is not used for anything else in the application
	ssh_session my_ssh_session = NULL;
	int rc;
	const char* password = "root";

	// Open session and set options
	my_ssh_session = ssh_new();
	if (my_ssh_session == NULL)
		exit(-1);
	ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, "root@rp-f083c2.local");

	// Connect to server
	rc = ssh_connect(my_ssh_session);
	if (rc != SSH_OK)
	{
		fprintf(stderr, "Error connecting to localhost: %s\n",
			ssh_get_error(my_ssh_session));
		ssh_free(my_ssh_session);
		exit(-1);
	}

	// Verify the server's identity
	// For the source code of verify_knownhost(), check previous example
	if (verify_knownhost(my_ssh_session) < 0)
	{
		ssh_disconnect(my_ssh_session);
		ssh_free(my_ssh_session);
		exit(-1);
	}

	
	rc = ssh_userauth_password(my_ssh_session, NULL, password);
	if (rc != SSH_AUTH_SUCCESS)
	{
		fprintf(stderr, "Error authenticating with password: %s\n",
			ssh_get_error(my_ssh_session));
		ssh_disconnect(my_ssh_session);
		ssh_free(my_ssh_session);
		exit(-1);
	}


	ssh_disconnect(my_ssh_session);
	ssh_free(my_ssh_session);

	RpGui::context = (RpGui::Context*)Engine::Allocator::alloc(sizeof(RpGui::Context));

	RpGui::context->font = RpGui::loadFont("c:/windows/fonts/times.ttf", 512, 32.0f);

	RpGui::context->pipeline2D = Engine::Renderer2D::createGraphicsPipelineFromGLSLSource(Engine::getParentDisplay(), "res/shaders/default_quadshader.vert", "res/shaders/default_quadshader.frag", {nullptr, 0});
	RpGui::context->fontpipeline2D = Engine::Renderer2D::createGraphicsPipelineFromGLSLSource(Engine::getParentDisplay(), "res/shaders/default_fontshader.vert", "res/shaders/default_fontshader.frag", { &RpGui::fontuserlayout, 1 });
	

	Engine::Renderer2D::InitInfo init{};
	init.currentpipeline = RpGui::context->pipeline2D;
	init.descriptorsetlayouts = { nullptr, 0 };
	init.instancebuffersize = 1 * MEGA_BYTE;
	init.shadowmapdimensions = 0;

	RpGui::context->buffer = Engine::ArrayList<glm::vec2>::create(10);

	RpGui::renderer2D = Engine::Renderer2D::Wrapper::create(init);
	return true;
}


struct FunctionParams {
	real32 cutoff;
	real32 Qfactor;
	real32 gain;
};

real32 bandpass(real32 x, void* params_) {

	FunctionParams* params = (FunctionParams*)params_;
	return (x * x + params->cutoff * params->cutoff) / (x * x + ((x * params->cutoff) / params->Qfactor) + params->cutoff * params->cutoff);
}


PH_DLL_EXPORT PH_APPLICATION_UPDATE(applicationUpdate) {

	PH::Engine::beginNewFrame(&context);
	PH::Engine::Events::startNewFrame();

	real32 scrollspeed = 0.001f;
	static RpGui::Box2D range = { 0.0f, 0.0f, 10.0f, 5.0f };

	//update all events events;
	for (const auto& event : context.events) {
		PH::Engine::Events::onEvent(event);

		if (event.type == PH_EVENT_TYPE_MOUSE_SCROLLED) {
			int16 delta = (int16)(void*)event.lparam;

			real32 zoom = 1.0f + (real32)delta * scrollspeed;

			glm::vec2 mousepos = Engine::Events::getMousePos();
			mousepos.y = context.windowheight - mousepos.y; //flip y coordinate because the window coordinate system has y going down and the plot coordinate system has y going up

			//Engine::INFO << "mouse pos: " << mousepos.x << ", " << mousepos.y << "\n";	

			if (PH::Engine::Events::isKeyPressed(PH_CONTROL)) {
				range.right *= zoom;
				range.left *= zoom;
			}
			else {
				range.top *= zoom;
				range.bottom *= zoom;
			}
		}
	}

	RpGui::Box2D region;
	region.left = 0.0f;
	region.right = (real32)context.windowwidth;
	region.bottom = 0.0f;
	region.top = (real32)context.windowheight;
	
	auto& io = ImGui::GetIO();

	glm::vec2 mousefactor = { (range.right - range.left) / (region.right - region.left), (range.top - range.bottom) / (region.top - region.bottom)};
	if (!io.WantCaptureMouse && PH::Engine::Events::isMouseButtonPressed(PH_LMBUTTON)) {

		glm::vec2 deltam = PH::Engine::Events::getDeltaMousePos();

		range.left -= mousefactor.x * deltam.x;
		range.right -= mousefactor.x * deltam.x;
		range.top += mousefactor.y * deltam.y;
		range.bottom += mousefactor.y * deltam.y;
	}
	

	
	RpGui::context->buffer.clear();

	static int32 nsamples = 1000;
	
	real32 xrange = range.right - range.left;
	real32 dx = xrange / (real32)nsamples;

	static FunctionParams params = { 100.0f, 4.0f, 10.0f };
	
	for (real32 x = range.left; x <= range.right; x += dx) {

		glm::vec2 coordinate = { x, 20 * log10(bandpass(powf(10.0f, x), (void*)&params))};
		RpGui::context->buffer.pushBack(coordinate);
	}
	

	PH::Engine::beginRenderPass(*Engine::getParentDisplay());

	glm::vec2 linethickness = {1.0f, 1.0f};

	RpGui::renderer2D.begin();
	
	//start drawing the plot, first set the pipeline and the view and projection matrices, then draw the background and the plot itself, then end the renderer and flush it to the GPU
	RpGui::renderer2D.pushGraphicsPipeline(RpGui::context->pipeline2D);
	RpGui::renderer2D.setView(glm::mat4(1.0f));
	RpGui::renderer2D.setProjection(glm::ortho(0.0f, (real32)context.windowwidth, (real32)context.windowheight, 0.0f));
	
	drawPlotScaleLines(range, region);
	drawPlot(RpGui::context->buffer.getArray(), range, region);
	//RpGui::renderer2D.drawTexturedQuad({ 500.0f, 500.0f, 0.0f }, { 1000.0f, 1000.0f }, RpGui::context->font.atlas);
	
	static real32 textscale = 0.5f;

	//start drawing the text
	RpGui::renderer2D.pushGraphicsPipeline(RpGui::context->fontpipeline2D, { &RpGui::context->font.cdata, 1 });
	RpGui::renderer2D.pushTexture(RpGui::context->font.atlas);
	RpGui::drawPlotScaleValues(range, region, &RpGui::context->font, textscale);


	RpGui::renderer2D.end();
	
	RpGui::renderer2D.flush({ nullptr, 0 });

	real32 speed = 0.01;

	static bool open = true;
	if(ImGui::Begin("function", &open)) {
		ImGui::DragFloat("cutoff", &params.cutoff, params.cutoff * speed);
		ImGui::DragFloat("Qfactor", &params.Qfactor, params.Qfactor * speed);
		ImGui::DragFloat("gain", &params.gain, params.gain * speed);
		ImGui::DragInt("nsamples", &nsamples, 1.0f, 0, 2000);
	} ImGui::End();

	static bool statsopen;
	if (ImGui::Begin("stats")) {
		ImGui::Text("framerate %f", 1.0f / Engine::getTimeStep());
		ImGui::DragFloat("text scale", &textscale, textscale * speed);
	} ImGui::End();

	ImGui::Render();
	PH::Platform::GFX::drawImguiWidgets(ImGui::GetDrawData());
	PH::Engine::endRenderPass(*Engine::getParentDisplay());
	return true;
}

PH_DLL_EXPORT PH_APPLICATION_DESTROY(applicationDestroy) {

	PH::RpGui::INFO << "destroying Rp-Gui application...\n";
	return true;
}