/*************************************************************************/
/*  ip.h                                                                 */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2016 Juan Linietsky, Ariel Manzur.                 */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#ifndef IP_H
#define IP_H


#include "os/os.h"
#include "io/ip_address.h"

struct _IP_ResolverPrivate;

class IP : public Object {
	OBJ_TYPE( IP, Object );
	OBJ_CATEGORY("Networking");
public:

	enum ResolverStatus {

		RESOLVER_STATUS_NONE,
		RESOLVER_STATUS_WAITING,
		RESOLVER_STATUS_DONE,
		RESOLVER_STATUS_ERROR,
	};

	enum {
		RESOLVER_MAX_QUERIES = 32,
		RESOLVER_INVALID_ID=-1
	};


	typedef int ResolverID;


private:

	_IP_ResolverPrivate *resolver;
protected:

	static IP*singleton;
	static void _bind_methods();

	virtual IP_Address _resolve_hostname(const String& p_hostname)=0;
	Array _get_local_addresses() const;

	static IP* (*_create)();
public:



	IP_Address resolve_hostname(const String& p_hostname);
	// async resolver hostname
	ResolverID resolve_hostname_queue_item(const String& p_hostname);
	ResolverStatus get_resolve_item_status(ResolverID p_id) const;
	IP_Address get_resolve_item_address(ResolverID p_id) const;
	virtual void get_local_addresses(List<IP_Address> *r_addresses) const=0;
	void erase_resolve_item(ResolverID p_id);

	static IP* get_singleton();

	static IP* create();

	IP();
	~IP();


};

#endif // IP_H
