#pragma once
#include <Engine/AssetDescription.h>
#include <Engine/AssetLibrary.h>
#include <Base/Datastructures/circularworkqueue.h>

#define PH_ASSET_SSH_CONNECTION 6498485161872

#ifdef LC_SSH_EXPORT
#define LC_SSH_API  __declspec(dllexport)
#else
#define LC_SSH_API __declspec(dllimport)
#endif

namespace PH::LabCoreSSH {

	enum CommandType {
		TYPE_SSH_EXEC,
		TYPE_SFTP_WRITE,
		TYPE_SFTP_READ
	};

	struct SFTPwrite;
	struct SSHshellExec;

	typedef void (*SFTPwriteCallback)(SFTPwrite* write);
	typedef void (*SSHshellExecCallback)(SSHshellExec* exec, Base::SubString output, Base::SubString err);

	//writes the data on the remote at the specified path
	struct SFTPwrite {
		Engine::String path;
		Engine::DynamicArray<uint8> data;
		SFTPwriteCallback callback_;
	};

	//runs a shell command on the remote, returns the output stream and error to the callback.
	struct SSHshellExec {
		Engine::String command;
		SSHshellExecCallback callback_;
	};

	struct RemoteCommand {
		CommandType type;
		union {
			SFTPwrite sftpwrite;
			SSHshellExec shellexec;
		};
	};

	struct SFTPwriteResult {
		uint32 resultcode;
	};

	struct SSHshellExecResult {
		Engine::String output;
		Engine::String err;
	};

	struct RemoteResult {
		CommandType type;
		union {
			SSHshellExecResult shellexecresult;
			SFTPwriteResult sftpwriteresult;
		};

		union {
			SSHshellExec shellexec;
			SFTPwrite sftpwrite;
		};
	};

	struct Connection : Engine::AssetBase {

		static Engine::AssetDescriptionID getAssetDescriptionID() {
			return PH_ASSET_SSH_CONNECTION;
		}

		static const char* getExtension() {
			return ".lcssh";
		}

		Engine::String remoteip;

		bool32 connected = false;

		PH::Platform::Thread thread;
		HANDLE semaphore;

		PH::Base::CircularWorkQueue<RemoteResult, Engine::Allocator> inqueue;
		PH::Base::CircularWorkQueue<RemoteCommand, Engine::Allocator> outqueue;
	};

	Engine::AssetDescription createConnectionAssetdescription();
	PH::bool32 pollOpenConnectionResultsCallback(Engine::AssetLibrary* assets);

	//creates a connection asset but does not open the connection, call openConnection to open the ssh connection
	LC_SSH_API Connection createConnection(const char* remoteip);
	LC_SSH_API void openConnection(Connection* connection);

	//blocks the current thread until a connection is formed with the remote. returns true if connection is established, false if time out
	LC_SSH_API bool32 waitForConnection(Connection* connection, sizeptr timeout_ms);
	LC_SSH_API void sendCommand(Connection* connection, const RemoteCommand& command);
}