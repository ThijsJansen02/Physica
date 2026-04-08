#pragma once
//this define is needed to use libssh as a static library on windows, because otherwise the functions will be exported with __declspec(dllimport) which will cause linker errors when trying to link against the static library
//libssh should be included as the first header to avoid any issues with the windows.h header that is included by the platform layer, because windows.h defines some macros that can cause issues with the libssh headers if they are included after windows.h
#define LIBSSH_STATIC 1
#include <libssh/libssh.h>

#include "RpGui.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <Engine/Engine.h>

#include <Base/Log.h>

#include <Base/Datastructures/String.h>
#include <Base/Datastructures/circularworkqueue.h>

#include <Platform/platformAPI.h>



namespace PH::RpGui {
	
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
			WARN << "Host key for server changed: it is now:\n";
			ssh_print_hash(SSH_PUBLICKEY_HASH_SHA256, hash, hlen);
			WARN << "For security reasons, connection will be stopped\n";
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

	int show_remote_processes(ssh_session session)
	{
		ssh_channel channel = NULL;
		int rc;
		char buffer[256];
		int nbytes;

		channel = ssh_channel_new(session);
		if (channel == NULL)
			return SSH_ERROR;

		rc = ssh_channel_open_session(channel);
		if (rc != SSH_OK)
		{
			ssh_channel_free(channel);
			return rc;
		}

		rc = ssh_channel_request_exec(channel, "ls");
		if (rc != SSH_OK)
		{
			ssh_channel_close(channel);
			ssh_channel_free(channel);
			return rc;
		}

		nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);

		/*
		while (nbytes > 0)
		{
			if (write(1, buffer, nbytes) != (unsigned int)nbytes)
			{
				ssh_channel_close(channel);
				ssh_channel_free(channel);
				return SSH_ERROR;
			}
			nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
		}
		*/

		INFO << "remote processes:\n" << buffer << "\n";

		if (nbytes < 0)
		{
			ssh_channel_close(channel);
			ssh_channel_free(channel);
			return SSH_ERROR;
		}

		ssh_channel_send_eof(channel);
		ssh_channel_close(channel);
		ssh_channel_free(channel);

		return SSH_OK;
	}

	struct ThreadCommand {
		const char* command;
	};

	struct ThreadData {
		bool32 connected = false;
		bool32 open = true;

		DWORD threadid;
		HANDLE semaphore;

		PH::Base::CircularWorkQueue<ThreadCommand, Engine::Allocator> commandqueue;
	};

	//open ssh connection in a seperate thread to test the thread safety of libssh, and to test the performance of libssh when used in a separate thread. also to test the performance of libssh when used in a separate thread with a separate memory allocator
	PH::int32 examplethread(
		void* userdata
	) {
		INFO << "hello from thread! userdata: " << (uint64)userdata << "\n";
#if 0

		ssh_session my_ssh_session = NULL;
		int rc;
		int verbosity = SSH_LOG_PROTOCOL;
		int port = 22;

		my_ssh_session = ssh_new();
		if (my_ssh_session == NULL) {
			return -1;
		}


		ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, "root@rp-f083c2.local");
		ssh_options_set(my_ssh_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
		ssh_options_set(my_ssh_session, SSH_OPTIONS_PORT, &port);

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

		// Authenticate ourselves
		const char* password = "root";
		rc = ssh_userauth_password(my_ssh_session, NULL, password);
		if (rc != SSH_AUTH_SUCCESS)
		{
			WARN << "Error authenticating with password: %s\n" << ssh_get_error(my_ssh_session) << "\n";
			ssh_disconnect(my_ssh_session);
			ssh_free(my_ssh_session);
			return -1;
		}

		show_remote_processes(my_ssh_session);

		ssh_disconnect(my_ssh_session);
		ssh_free(my_ssh_session);
#endif

		ThreadData* info = (ThreadData*)userdata;

		WaitForSingleObjectEx(info->semaphore, INFINITE, FALSE);

		while (true && info->open) {

			ThreadCommand* work = info->commandqueue.pop();
			if (work) {
				INFO << "Thread: " << (uint32)info->threadid << " is executing command: " << work->command << "\n";
			}
			else {
				INFO << "thread: " << (uint32)info->threadid << " is going to sleep\n";
				WaitForSingleObjectEx(info->semaphore, INFINITE, FALSE);
				INFO << "thread: " << (uint32)info->threadid << " woke up\n";
			}
		}
		return 0;
	}

}