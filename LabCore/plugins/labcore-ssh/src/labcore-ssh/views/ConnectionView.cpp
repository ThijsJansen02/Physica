
#include "ConnectionView.h"
#include <LabCore/LabCoreAPI.h>
#include <Engine/imgui/extensions.h>


namespace PH::LabCoreSSH {

	bool32 openCreatePopupModal = false;
	bool32 openFileTransferpopup = false;

	Connection* targetconnection = nullptr;

	void shellReturnPrintConsol(SSHshellExec* exec, Base::SubString output, Base::SubString err) {
		Engine::INFO << output << "\n";
		Engine::ERR << err << "\n";
	}

	bool32 openCreateConnectionPopup() {
		openCreatePopupModal = true;
		return true;
	}

	PH::bool32 drawConnectionView(ConnectionView* view) {

		ImGui::PushItemWidth(200.0f);
		view->current = ImGuiext::assetSelecter<Connection>("connection", view->current, "select connection", LabCore::getAssetLibrary());
		
		ImGui::SameLine();

		if(ImGui::Button("Create Connection")) {
			openCreatePopupModal = true;
		}	

		if (view->current) {

			Connection* c = view->current;
			ImGuiext::Input("remote ip", & c->remoteip);

			if (c->connected) {
				if (ImGui::Button("Disconnect")) {
					//add code to close thread
				}
			}
			else {
				if (ImGui::Button("Connect")) {
					openConnection(c);
				}
			}

			ImGuiext::Input("shell command", &view->currentcommand);
			ImGui::SameLine();
			if(ImGui::Button("send")) {
				RemoteCommand command{};
				command.type = TYPE_SSH_EXEC;
				command.shellexec.command = Engine::String::create(view->currentcommand.getC_Str());
				command.shellexec.callback_ = shellReturnPrintConsol;

				sendCommand(c, command);
			}

			if (ImGui::Button("Transfer File")) {
				openFileTransferpopup = true;
				targetconnection = c;
			}
		}
		ImGui::PopItemWidth();
		return true;
	}

	bool32 FileTransferPopupCallBack(Engine::AssetLibrary* lib) {
		if (openFileTransferpopup) {
			ImGui::OpenPopup("SSH File Transfer");
			openFileTransferpopup = false;
		}

		Platform::setCurrentDirectory(PH::LabCore::getCurrentProjectDir());
		ImGuiIO& io = ImGui::GetIO();

		ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Appearing);
		ImGui::SetNextWindowPos(
			ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
			ImGuiCond_Appearing,
			ImVec2(0.5f, 0.5f)
		);

		static Engine::String dst = Engine::String::create("");
		static Engine::String src = Engine::String::create("");
		
		if (ImGui::BeginPopup("SSH File Transfer")) {
			targetconnection = ImGuiext::assetSelecter<Connection>("connection", targetconnection, "select connection", LabCore::getAssetLibrary());
		
			if (ImGuiext::FilePath("Source", &src, "", LabCore::getCurrentProjectDir().getC_Str(), PH_DIALOG_FILEMUSTEXIST)) {
				if (PH::Engine::Events::isKeyPressed(PH_RETURN)) {
					Base::SubString filename = Base::getFileName(src.getSubString());
					dst.set(filename);
				}
			}
			
			
			ImGuiext::Input("Destination", &dst);

			if (ImGui::Button("transfer")) {
				PH::Platform::FileBuffer f{};
				if (PH::Platform::loadFile(&f, src.getC_Str())) {
					LabCoreSSH::RemoteCommand rc{};
					rc.type = TYPE_SFTP_WRITE;
					rc.sftpwrite.callback_ = nullptr;
					rc.sftpwrite.data = Engine::DynamicArray<uint8>::create(f.size, (uint8)f.data);
					rc.sftpwrite.path = Engine::String::create(dst.getC_Str());
					
					LabCoreSSH::sendCommand(targetconnection, rc);
					PH::Platform::unloadFile(&f);
				}
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();
			if (ImGui::Button("cancel")) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		return true;
	}

	bool32 connectionCreatePopupModalCallback(Engine::AssetLibrary* lib) {

		if (openCreatePopupModal) {
			ImGui::OpenPopup("Create SSH connection");
			openCreatePopupModal = false;
		}

		Platform::setCurrentDirectory(PH::LabCore::getCurrentProjectDir());
		ImGuiIO& io = ImGui::GetIO();

		ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Appearing);
		ImGui::SetNextWindowPos(
			ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
			ImGuiCond_Appearing,
			ImVec2(0.5f, 0.5f)
		);

		static Engine::String dir = Engine::String::create("assets/SSH");
		static Engine::String remoteIP = Engine::String::create("");
		static bool openOnCreate = false;

		if (ImGui::BeginPopupModal("Create SSH connection")) {

			ImGuiext::FilePath("directory", &dir, "", LabCore::getCurrentProjectDir().getC_Str(), PH_DIALOG_PATHMUSTEXIST);
			ImGuiext::Input("remote ip", &remoteIP);
			openOnCreate = ImGui::RadioButton("open on create", openOnCreate);

			if (ImGui::Button("Create")) {
				PH::Platform::setCurrentDirectory(LabCore::getCurrentProjectDir());
				PH::Platform::createDirectory(dir.getSubString());

				Engine::String filepath = Engine::String::create(dir.getC_Str()).append("/").append(remoteIP.getC_Str()).append(Connection::getExtension());
				auto* asset = Engine::Allocator::push<Connection>(createConnection(remoteIP.getC_Str()));
				if (openOnCreate) {
					openConnection(asset);
				}

				lib->addAsset<Connection>(asset, filepath.getC_Str(), remoteIP.getC_Str(), Engine::AssetFlagsBits::PH_ASSET_SERIALIZABLE);
				Engine::String::destroy(&filepath);

				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel")) {
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
		return true;
	}

	PH::bool32 updateConnectionView(ConnectionView* view) {


		return true;
	}

	PH::bool32 instantiateConnectionView(ConnectionView* view) {
		view->current = nullptr;
		view->currentcommand = Engine::String::create("");
		return true;
	}

	PH::bool32 destroyConnectionView(ConnectionView* data) {
		return true;
	}

	PH::bool32 connectionViewOnEvent(ConnectionView* data, const PH::Platform::Event& e) {
		return true;
	}

	PH::bool32 deserializeConnectionView(ConnectionView* data, const YAML::Node& node) {
		if (node["current"]) {
			data->current = LabCore::getAssetLibrary()->getLoadedAsset<Connection>(node["current"].as<Engine::AssetID>());
		}
	
		return true;
	}

	PH::bool32 serializeConnectionView(ConnectionView* data, YAML::Emitter& out) {
		
		out << YAML::BeginMap;

		if (data->current) {
			out << YAML::Key << "current" << YAML::Value << data->current->identifier->assetid;
		}

		out << YAML::EndMap;

		return true;
	}

	LabCore::ViewDescription createConnectionViewDescription() {
		LabCore::ViewDescription d{};
		d.draw_ = (LabCore::ViewDrawFunction)drawConnectionView;
		d.onUpdate_ = (LabCore::ViewOnUpdateFunction)updateConnectionView;
		d.onEvent_ = (LabCore::ViewOnEventFunction)connectionViewOnEvent;

		d.destroy_ = (LabCore::ViewDestroyFunction)destroyConnectionView;
		d.instantiate_ = (LabCore::ViewInstantiateFunction)instantiateConnectionView;

		d.windowpadding = { 5,5 };
		d.windowflags = 0;

		d.deserialize_ = (LabCore::ViewDeserializeFunction)deserializeConnectionView;
		d.serialize_ = (LabCore::ViewSerializeFunction)serializeConnectionView;

		d.name = Engine::String::create("SSH connections");
		d.instancedatasize = sizeof(ConnectionView);

		return d;
	}
}