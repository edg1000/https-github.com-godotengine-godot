#include "stream_peer_ssl.h"


StreamPeerSSL* (*StreamPeerSSL::_create)()=NULL;


StreamPeerSSL *StreamPeerSSL::create() {

	return _create();
}

void StreamPeerSSL::set_verify_mode(int mode) {
	verify_mode = mode;
}
int StreamPeerSSL::get_verify_mode() const{
	return verify_mode;
}


void StreamPeerSSL::_bind_methods() {


	ObjectTypeDB::bind_method(_MD("set_verify_mode", "mode"),&StreamPeerSSL::set_verify_mode);
	ObjectTypeDB::bind_method(_MD("get_verify_mode"),&StreamPeerSSL::get_verify_mode);

	ObjectTypeDB::bind_method(_MD("accept:Error","stream:StreamPeer"),&StreamPeerSSL::accept);
	ObjectTypeDB::bind_method(_MD("connect:Error","stream:StreamPeer","validate_certs","for_hostname"),&StreamPeerSSL::connect,DEFVAL(false),DEFVAL(String()));
	ObjectTypeDB::bind_method(_MD("get_status"),&StreamPeerSSL::get_status);
	ObjectTypeDB::bind_method(_MD("disconnect"),&StreamPeerSSL::disconnect);
	BIND_CONSTANT( STATUS_DISCONNECTED );
	BIND_CONSTANT( STATUS_CONNECTED );
	BIND_CONSTANT( STATUS_ERROR_NO_CERTIFICATE );
	BIND_CONSTANT( STATUS_ERROR_HOSTNAME_MISMATCH );

}

StreamPeerSSL::StreamPeerSSL()
{
}
