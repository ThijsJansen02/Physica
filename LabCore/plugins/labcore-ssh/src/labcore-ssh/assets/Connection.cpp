#define LIBSSH_STATIC 1
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <fcntl.h>

#include "Connection.h"
#include <yaml-cpp/yaml.h>

#include <Engine/AssetLibrary.h>

namespace PH::LabCoreSSH {

	int verify_knownhost(ssh_session session);
	PH::int32 SSHconnectionThreadProc(void* userdata);

	Connection createConnection(const char* remoteip) {
		Connection c;
		c.connected = false;
		c.remoteip = Engine::String::create(remoteip);
		c.inqueue = PH::Base::CircularWorkQueue<RemoteResult, Engine::Allocator>::create(10);
		c.outqueue = PH::Base::CircularWorkQueue<RemoteCommand, Engine::Allocator>::create(10);
		c.thread = {};
		c.semaphore = CreateSemaphoreEx(0, 0, 10, nullptr, 0, SEMAPHORE_ALL_ACCESS);

		return c;
	}

	void openConnection(Connection* connection) {

		//should add code to destroy old thread if exists;

		PH::Platform::ThreadCreateInfo threadinfo{};
		threadinfo.threadworkmemorysize = 0;
		threadinfo.usegfx = false;
		threadinfo.userdata = (void*)connection;
		threadinfo.threadproc = SSHconnectionThreadProc;

		PH::Platform::createThread(threadinfo, &connection->thread);
	}

	bool32 waitForConnection(Connection* connection, sizeptr timeout_ms) {

		sizeptr wait = 0;

		//simple code for now:
		while (!connection->connected) {
			Sleep((DWORD)100);
			wait += 100;

			if (wait > timeout_ms) {
				return false;
			}
		}
		return true;
	}

	void sendCommand(Connection* connection, const RemoteCommand& command) {
		connection->outqueue.push(command);
		ReleaseSemaphore(connection->semaphore, 1, nullptr);
	}

	void connectionDeserialize(YAML::Node in, Connection* connect) {
		if (in["remoteIP"]) {
			Engine::String ip = in["remoteIP"].as<Engine::String>();

			*connect = createConnection(ip.getC_Str());
			Engine::String::destroy(&ip);
			openConnection(connect);
		}
		else {
			*connect = createConnection("");
		}
	}

	void connectionSerialize(YAML::Emitter& out, Connection* connect) {
		out << YAML::BeginMap;
		out << YAML::Key << "remoteIP" << connect->remoteip.getC_Str();
		out << YAML::EndMap;
	}

	void connectionDestroy(Connection* connect) {
		Engine::String::destroy(&connect->remoteip);
		//add destructors for the queues;
	}

	void pollInQueue(Connection* connection) {

		while (connection->inqueue.hasWork()) {
			RemoteResult r = *connection->inqueue.pop();

			if (r.type == TYPE_SSH_EXEC) {
				if (r.shellexec.callback_) {
					r.shellexec.callback_(&r.shellexec, r.shellexecresult.output.getSubString(), r.shellexecresult.err.getSubString());
				}

				//destroy results
				Engine::String::destroy(&r.shellexec.command);
				Engine::String::destroy(&r.shellexecresult.output);
				Engine::String::destroy(&r.shellexecresult.err);
			}

			if (r.type == TYPE_SSH_EXEC) {
				if (r.sftpwrite.callback_) {
					r.sftpwrite.callback_(&r.sftpwrite);
				}

				Engine::String::destroy(&r.sftpwrite.path);
			}
		}
	}

	PH::bool32 pollOpenConnectionResultsCallback(Engine::AssetLibrary* assets) {
		auto connections = assets->getAllIdentifiersOfType<Connection>();

		for (auto& c : connections) {
			if (c->status == Engine::LOADED) {
				Connection* conn = (Connection*)c->assetdata;
				pollInQueue(conn);
			}
		}

		return true;
	}

	Engine::AssetDescription createConnectionAssetdescription() {
		Engine::AssetDescription desc{};
		desc.deserialize_ = (Engine::deserializeAsset)connectionDeserialize;
		desc.serialize_ = (Engine::SerializeAsset)connectionSerialize;
		desc.destroy_ = (Engine::destroyAsset)connectionDestroy;

		desc.id = Connection::getAssetDescriptionID();
		desc.localdatasize = sizeof(Connection);
		desc.extension = Connection::getExtension();

		return desc;
	}


	//open ssh connection in a seperate thread to test the thread safety of libssh, and to test the performance of libssh when used in a separate thread. also to test the performance of libssh when used in a separate thread with a separate memory allocator
	PH::int32 SSHconnectionThreadProc(
		void* userdata
	) {


		Connection* info = (Connection*)userdata;
		Engine::INFO << "starting connection with " << info->remoteip.getC_Str() << "\n";
#if 1

		ssh_session my_ssh_session = NULL;
		int rc;
		int verbosity = SSH_LOG_PACKET;
		int port = 22;

		my_ssh_session = ssh_new();
		if (my_ssh_session == NULL) {
			return -1;
		}


		ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, info->remoteip.getC_Str());
		ssh_options_set(my_ssh_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
		ssh_options_set(my_ssh_session, SSH_OPTIONS_PORT, &port);
		ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, "root");

		rc = ssh_connect(my_ssh_session);
		if (rc != SSH_OK)
		{
			Engine::ERR << "Error connecting to localhost: " << ssh_get_error(my_ssh_session) << "\n";
			return -1;
		}


		// Verify the server's identity
		if (verify_knownhost(my_ssh_session) < 0)
		{
			ssh_disconnect(my_ssh_session);
			ssh_free(my_ssh_session);
			return -1;
		}

		//for now, all connections are with password root
		// Authenticate ourselves
		const char* password = "root";
		rc = ssh_userauth_password(my_ssh_session, NULL, password);
		if (rc != SSH_AUTH_SUCCESS)
		{
			Engine::WARN << "Error authenticating with password: %s\n" << ssh_get_error(my_ssh_session) << "\n";
			ssh_disconnect(my_ssh_session);
			ssh_free(my_ssh_session);
			return -1;
		}

		info->connected = true;
		Engine::INFO << "connected with Rp " << info->remoteip.getC_Str() << "\n";
#endif

		while (true) {

			//wait for the semaphore to be released
			WaitForSingleObjectEx(info->semaphore, INFINITE, FALSE);

			RemoteCommand* workptr = info->outqueue.pop();

			//if there is a command -> send it to the rp
			if (workptr != nullptr) {

				RemoteCommand work = *workptr;

				//if the command type is shell, upload shell command
				if (work.type == TYPE_SSH_EXEC) {

					Engine::INFO << "SSH exec: " << work.shellexec.command.getC_Str() << "\n";

					ssh_channel channel = NULL;
					int rc;
					
					RemoteResult r{};
					r.type = TYPE_SSH_EXEC;
					r.shellexec = work.shellexec;
					r.shellexecresult.output = Engine::String::create("");
					r.shellexecresult.err = Engine::String::create("");

					char buffer[256];

					int nbytes;

					channel = ssh_channel_new(my_ssh_session);
					if (channel == NULL) {
						Engine::WARN << "SSH connection failed to create new channel!\n";
						continue;
					}

					rc = ssh_channel_open_session(channel);
					if (rc != SSH_OK)
					{
						Engine::WARN << "SSH failed to open session on channel\n";
						ssh_channel_free(channel);
						continue;
					}

					rc = ssh_channel_request_exec(channel, work.shellexec.command.getC_Str());
					if (rc != SSH_OK)
					{
						ssh_channel_close(channel);
						ssh_channel_free(channel);

						Engine::WARN << "SSH failed to execute reguest\n";
						continue;
					}

					//read the output stream
					nbytes = ssh_channel_read(channel, buffer, sizeof(buffer) - 1, 0);

					while (nbytes > 0) {
						buffer[nbytes - 1] = 0;
						Engine::INFO << buffer;

						r.shellexecresult.output.append(buffer);

						nbytes = ssh_channel_read(channel, buffer, sizeof(buffer) - 1, 0);
					}

					if (nbytes < 0)
					{
						ssh_channel_close(channel);
						ssh_channel_free(channel);
						continue;
					}

					//read the error stream
					nbytes = ssh_channel_read(channel, buffer, sizeof(buffer) - 1,  1);

					while (nbytes > 0) {
						buffer[nbytes - 1] = 0;
						r.shellexecresult.err.append(buffer);

						nbytes = ssh_channel_read(channel, buffer, sizeof(buffer) - 1, 1);
					}

					if (nbytes < 0)
					{
						ssh_channel_close(channel);
						ssh_channel_free(channel);
						continue;
					}

					info->inqueue.push(r);

					ssh_channel_send_eof(channel);
					ssh_channel_close(channel);
					ssh_channel_free(channel);
				}

				if (work.type == TYPE_SFTP_WRITE) {
					// Create SFTP session
					sftp_session sftp = sftp_new(my_ssh_session);
					if (!sftp) {
						sftp_free(sftp);
						Engine::ERR << "Could not create SFTP session\n";
						continue;
					}

					if (sftp_init(sftp) != SSH_OK) {
						Engine::ERR << "SFTP initialization failed: " << ssh_get_error(my_ssh_session) << '\n';
						sftp_free(sftp);
						continue;
					}
					constexpr int remotefilepermissions = 0644;
					sftp_file remotefile = sftp_open(sftp, work.sftpwrite.path.getC_Str(), O_WRONLY | O_CREAT, 0644);

					if (!remotefile) {
						Engine::ERR << "Could not open remote file: "
							<< ssh_get_error(my_ssh_session) << '\n';
						sftp_free(sftp);
						continue;
					}

					sftp_limits_t limits = sftp_limits(sftp);

					sizeptr writesize = limits->max_write_length;
					sizeptr offset = 0;

					while (offset < work.sftpwrite.data.getCapacity()) {
						sizeptr nbytesleft = work.sftpwrite.data.getCapacity() - offset;

						ssize_t written = sftp_write(remotefile, (uint8*)work.sftpwrite.data.raw() + offset, Base::minSizeptr(nbytesleft, writesize));
						offset += written;
					}

					sftp_close(remotefile);
					sftp_free(sftp);

					RemoteResult r{};
					r.type = LabCoreSSH::CommandType::TYPE_SFTP_WRITE;
					r.sftpwriteresult.resultcode = 1;
					r.sftpwrite = work.sftpwrite;

					info->inqueue.push(r);
				}
			}
		}

		ssh_disconnect(my_ssh_session);
		ssh_free(my_ssh_session);

		info->connected = false;
		return 0;
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
			Engine::WARN << "Host key for server changed: it is now:\n";
			ssh_print_hash(SSH_PUBLICKEY_HASH_SHA256, hash, hlen);
			Engine::WARN << "For security reasons, connection will be stopped\n";
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

			cmp = Base::stringCompare(buf, "yes", 5);
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

}