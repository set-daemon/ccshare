/*
 * file name: ip_addr.h
 * purpose  :
 * author   : set_daemon@126.com
 * date     : 2014-08-29
 * history  :
 */
#ifndef __IP_ADDR_H__
#define __IP_ADDR_H__

#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <string>
using namespace std;

// it supports IPv4 only.
class IpAddr {
public:
	IpAddr(const string &ipaddr) : port(0), u_ip(0) {
		parse(ipaddr);
	}
	~IpAddr() { }

	// ip_str like: 192.168.1.1:13456
	bool parse(const string& ipaddr) {
		size_t pos = ipaddr.find_first_of(':');
		if (pos == string::npos) {
			ip = ipaddr.substr(0);
		} else {
			ip = ipaddr.substr(0, pos);
			port = strtoul(ipaddr.substr(pos+1).c_str(), NULL, 10);
		}

		inet_pton(AF_INET, ip.c_str(), &u_ip);
		
		sock_addr.sin_family = AF_INET;
		sock_addr.sin_addr.s_addr = htonl(u_ip);
		sock_addr.sin_port = htons(port);
	}

	int get_port() {
		return port;
	}

	const char* get_ip_c() {
		return ip.c_str();
	}

	string& get_ip() {
		return ip;
	}

	unsigned int get_uip() {
		return u_ip;
	}

	string 				ip;
	unsigned int 		u_ip;
	int 				port;
	struct sockaddr_in 	sock_addr;	
};

#endif // __IP_ADDR_H__
