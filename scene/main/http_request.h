#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include "node.h"
#include "io/http_client.h"
#include "os/file_access.h"

class HTTPRequest : public Node {

	OBJ_TYPE(HTTPRequest,Node);
public:

	enum Result {
		RESULT_SUCCESS,
		//RESULT_NO_BODY,
		RESULT_CHUNKED_BODY_SIZE_MISMATCH,
		RESULT_CANT_CONNECT,
		RESULT_CANT_RESOLVE,
		RESULT_CONNECTION_ERROR,
		RESULT_SSL_HANDSHAKE_ERROR,
		RESULT_NO_RESPONSE,
		RESULT_BODY_SIZE_LIMIT_EXCEEDED,
		RESULT_REQUEST_FAILED,
		RESULT_DOWNLOAD_FILE_CANT_OPEN,
		RESULT_DOWNLOAD_FILE_WRITE_ERROR,
		RESULT_REDIRECT_LIMIT_REACHED

	};

private:

	bool requesting;

	String request_string;
	String url;
	int port;
	Vector<String> headers;
	bool validate_ssl;
	bool use_ssl;

	bool request_sent;
	Ref<HTTPClient> client;
	ByteArray body;
	bool use_threads;

	bool got_response;
	int response_code;
	DVector<String> response_headers;

	String download_to_file;

	FileAccess *file;

	int body_len;
	int downloaded;
	int body_size_limit;

	int redirections;

	HTTPClient::Status status;

	bool _update_connection();

	int max_redirects;

	void _redirect_request(const String& p_new_url);

	bool _handle_response(bool *ret_value);

	Error _parse_url(const String& p_url);
	Error _request();


protected:

	void _notification(int p_what);
	static void _bind_methods();
public:

	Error request(const String& p_url,const Vector<String>& p_custom_headers=Vector<String>(),bool p_ssl_validate_domain=true); //connects to a full url and perform request
	void cancel_request();
	HTTPClient::Status get_http_client_status() const;

	void set_use_threads(bool p_use);
	bool is_using_threads() const;

	void set_download_file(const String& p_file);
	String get_download_file() const;

	void set_body_size_limit(int p_bytes);
	int get_body_size_limit() const;

	void set_max_redirects(int p_max);
	int get_max_redirects() const;

	int get_downloaded_bytes() const;
	int get_body_size() const;

	HTTPRequest();
	~HTTPRequest();
};

#endif // HTTPREQUEST_H
