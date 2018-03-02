#include <string>
#include <set>
#include <algorithm>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")


extern "C" void *(*pcre_malloc)(size_t) = nullptr;
extern "C" void(*pcre_free)(void *) = nullptr;


// enable UPSTREAM_WIFI support
static std::string get_wifi_ip_addr();

static thread_local SOCKADDR_IN _adapter_addr{};
static std::set<std::string> _wifi_adapter_preferred;


extern "C"
void set_wifi_adapter_preferred(const char *upstream_server)
{
    if (upstream_server && *upstream_server) {
        _wifi_adapter_preferred.insert(upstream_server);
    }
}

extern "C"
int get_preferred_adapter_addr(const char *upstream_ip, int upstream_ip_len,
	struct sockaddr **adapter_addr)
{
    if (_wifi_adapter_preferred.empty()) {
        return 0;
    }
    else if (upstream_ip == nullptr || upstream_ip_len <= 0) {
        return 0;
    }
    else {
        // upstream_ip => ip string
        std::string up_ip_str(upstream_ip, upstream_ip_len);
        auto it = _wifi_adapter_preferred.find(up_ip_str);
        if (it == _wifi_adapter_preferred.end()) {
            return 0;
        }
    }

    // Now, prefer to use WIFI ...
    std::string wifi_ip = get_wifi_ip_addr();
    if (wifi_ip.empty()) {
        printf("get_wifi_ip_addr failed\n");
        return 0;
    }

    int r = InetPton(AF_INET, wifi_ip.c_str(), &_adapter_addr.sin_addr);
    if (1 == r) {
        _adapter_addr.sin_family = AF_INET;
        _adapter_addr.sin_port = 0;

        if (adapter_addr) {
            *adapter_addr = (struct sockaddr *)&_adapter_addr;
        }

        return sizeof(_adapter_addr);
    }

    return 0;
}


#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

static std::string get_wifi_ip_addr()
{
    std::string wifi_ip;
    PIP_ADAPTER_INFO pAdapterInfo = NULL;
    PIP_ADAPTER_INFO pAdapter = NULL;
    DWORD dwRetVal = 0;
    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

    {
        IP_ADAPTER_INFO dummy;
        if (GetAdaptersInfo(&dummy, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
            pAdapterInfo = (IP_ADAPTER_INFO *)MALLOC(ulOutBufLen);
            if (pAdapterInfo == NULL) {
                printf("Error allocating memory needed to call GetAdaptersinfo\n");
                return wifi_ip;
            }
        }
    }

    if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
        pAdapter = pAdapterInfo;
        while (pAdapter) {
            std::string desc(pAdapter->Description);
            std::transform(desc.begin(), desc.end(), desc.begin(), ::tolower);

            if (std::string::npos != desc.find("wireless") ||
                std::string::npos != desc.find("802.11") ||
                std::string::npos != desc.find("wlan") || 
                std::string::npos != desc.find("wi-fi"))
            {
                wifi_ip = pAdapter->IpAddressList.IpAddress.String;
                break;
            }
            pAdapter = pAdapter->Next;
        }
    }
    else {
        printf("GetAdaptersInfo failed with error: %d\n", dwRetVal);
    }

    if (pAdapterInfo) {
        FREE(pAdapterInfo);
    }
    return wifi_ip;
}
