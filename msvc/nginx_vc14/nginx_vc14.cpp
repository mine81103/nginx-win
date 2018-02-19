#include <winsock.h>

extern "C" void *(*pcre_malloc)(size_t) = nullptr;
extern "C" void(*pcre_free)(void *) = nullptr;

static SOCKADDR_IN _adapter_addr{};

extern "C"
int get_preferred_adapter_addr(struct sockaddr *upstream_addr, int upstream_len,
	struct sockaddr **adapter_addr)
{
	if (_adapter_addr.sin_family == AF_UNSPEC) {
		_adapter_addr.sin_addr.S_un.S_addr = inet_addr("10.124.94.147");
		_adapter_addr.sin_family = AF_INET;
		_adapter_addr.sin_port = 0;
	}
	if (adapter_addr) {
		*adapter_addr = (struct sockaddr *)&_adapter_addr;
	}
	return sizeof(_adapter_addr);
}
