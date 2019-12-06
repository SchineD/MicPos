# invoke SourceDir generated makefile for tcp_client.pem4f
tcp_client.pem4f: .libraries,tcp_client.pem4f
.libraries,tcp_client.pem4f: package/cfg/tcp_client_pem4f.xdl
	$(MAKE) -f C:\Users\Daniel\workspace_v8\MicPos_tcp_mic/src/makefile.libs

clean::
	$(MAKE) -f C:\Users\Daniel\workspace_v8\MicPos_tcp_mic/src/makefile.libs clean

